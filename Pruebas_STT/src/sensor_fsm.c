/*
 * sensor_fsm.c
 *
 *  Created on: Apr 12, 2026
 *      Author: Carlos Quispe
 */


#include "sensor_fsm.h"
#include "mcp9700a.h"
#include "uart_drv.h"
#include "lcd_pcf857.h"
#include "ds3231.h"
#include "eeprom_drv.h"
#include "main.h"

#define SENSOR_PERIOD_MS   500U

/* En Lima es poco probable una temperatura bajo cero: se considera inválida */
#define TEMP_APP_MIN_C     0.0f

typedef enum
{
	STATE_IDLE = 0,
	STATE_READ,
	STATE_CONTROL,
	STATE_OUTPUT,
	STATE_ERROR
} sensor_state_t;

typedef enum
{
	SENSOR_ERR_NONE = 0,
	SENSOR_ERR_ADC,
	SENSOR_ERR_OUT_OF_RANGE,
	SENSOR_ERR_INVALID_NEG
} sensor_error_t;

static sensor_state_t currentState = STATE_IDLE;
static sensor_error_t lastSensorError = SENSOR_ERR_NONE;

static uint32_t lastTick = 0;

static float temperature = 0.0f;
static float setpoint = 30.0f;
static float hysteresis = 1.0f;

static uint8_t outputState = 0;

/* 0 = desconocida, 1 = banda media, 2 = umbral superior, 3 = umbral inferior */
static uint8_t lastTempZone = 0U;

static void sensorReportError(void);
static void sensorLogThresholdIfNeeded(void);


/**
 * @brief Obtiene el valor actual de la temperatura.
 * @return float en grados Celsius.
 */
float sensorGetTemperature(void){
	  return temperature;
}

/**
 * @brief Inicializa la máquina de estados del sensor.
 * Configura el estado inicial del sistema en IDLE y almacena
 * el tiempo actual como referencia para la ejecución periódica
 * de la lectura del sensor.
 */
void sensorFsmInit(void)
{
	currentState = STATE_IDLE;
	lastSensorError = SENSOR_ERR_NONE;
	lastTempZone = 0U;
	lastTick = HAL_GetTick();
	(void)eepromInit();
}

/**
 * @brief Configura el setpoint del sistema.
 * Establece la temperatura objetivo utilizada por el control ON/OFF.
 * @param sp Valor del setpoint en grados Celsius.
 */
void sensorSetSetpoint(float sp)
{
	setpoint = sp;
}

/**
 * @brief Configura la histéresis del sistema.
 * Define el margen alrededor del setpoint para evitar oscilaciones
 * @param hyst Valor de histéresis en grados Celsius.
 */
void sensorSetHysteresis(float hyst)
{
	hysteresis = hyst;
}

/**
 * @brief Obtiene el valor actual del setpoint.
 * @return Valor del setpoint en grados Celsius.
 */
float sensorGetSetpoint(void)
{
    return setpoint;
}

/**
 * @brief Obtiene el valor actual de la histéresis.
 * @return Valor de la histéresis en grados Celsius.
 */
float sensorGetHysteresis(void)
{
    return hysteresis;
}


/**
 * @brief Ejecuta la máquina de estados del sensor.
 * Implementa un sistema no bloqueante que realiza:
 * - Lectura periódica de la temperatura
 * - Evaluación de control ON/OFF con histéresis
 * - Actualización de la salida (LED/relé)
 * - Envío de datos por UART
 */

void sensorFsmUpdate(void)
{
	switch (currentState)
	{
	case STATE_IDLE:

		if ((HAL_GetTick() - lastTick) >= SENSOR_PERIOD_MS)
		{
			lastTick = HAL_GetTick();
			currentState = STATE_READ;
		}
		break;

	case STATE_READ:
	{
		mcp9700a_status_t mcpStatus = mcp9700aReadTemperature(&temperature);

		if ((mcpStatus == MCP9700A_ERROR_ADC) || (mcpStatus == MCP9700A_ERROR_NULL_POINTER))
		{
			lastSensorError = SENSOR_ERR_ADC;
			currentState = STATE_ERROR;
		}
		else if (mcpStatus == MCP9700A_ERROR_OUT_OF_RANGE)
		{
			lastSensorError = SENSOR_ERR_OUT_OF_RANGE;
			currentState = STATE_ERROR;
		}
		else if (temperature < TEMP_APP_MIN_C)
		{
			/* Lectura dentro del MCP9700A, pero negativa e ilógica para Lima */
			lastSensorError = SENSOR_ERR_INVALID_NEG;
			currentState = STATE_ERROR;
		}
		else
		{
			lastSensorError = SENSOR_ERR_NONE;
			currentState = STATE_CONTROL;
		}
		break;
	}

	case STATE_CONTROL:

		if (temperature >= (setpoint + hysteresis))
		{
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
			outputState = 1;
		}
		else if (temperature <= (setpoint - hysteresis))
		{
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
			outputState = 0;
		}

		sensorLogThresholdIfNeeded();
		currentState = STATE_OUTPUT;
		break;

	case STATE_OUTPUT:
	{
		char msg[64];
		DS3231_DateTime_t dt = {0};
		bool rtcOk = DS3231_IsAvailable();

		if (rtcOk)
		{
			DS3231_ReadTime(&dt);
			rtcOk = DS3231_IsAvailable();
		}

		if (LCD_I2C_isAvailable())
		{
			sprintf(msg, "Temp: %.2f C", temperature);
			LCD_I2C_cmd(LCD_HOME);
			LCD_I2C_write_text((uint8_t *)msg);

			if (rtcOk)
			{
				snprintf(msg, sizeof(msg), "    %02u:%02u:%02u",
				         dt.hour, dt.minute, dt.second);
				LCD_I2C_cmd(LCD_ROW2);
				LCD_I2C_write_text((uint8_t *)msg);
			}
		}

		/* Si el LCD I2C no responde, la temperatura (y la hora) salen por UART */
		if (!LCD_I2C_isAvailable())
		{
			if (rtcOk)
			{
				snprintf(msg, sizeof(msg),
				         "TEMP=%.2f TIME=%02u:%02u:%02u\r\n",
				         temperature, dt.hour, dt.minute, dt.second);
			}
			else
			{
				snprintf(msg, sizeof(msg), "TEMP=%.2f\r\n", temperature);
			}
			uartSendString((uint8_t *)msg);
		}

		if (outputState)
		{
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);   // LED ON
		}
		else
		{
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET); // LED OFF
		}

		currentState = STATE_IDLE;
		break;
	}

	case STATE_ERROR:

		/* Estado seguro: no controlar con una lectura inválida */
		outputState = 0;
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

		sensorReportError();

		currentState = STATE_IDLE;
		break;

	default:
		currentState = STATE_IDLE;
		break;
	}
}

/**
 * @brief Guarda en EEPROM (Flash) un registro al cruzar umbral superior o inferior.
 * @note  Solo registra el flanco (cuando se alcanza), no mientras se permanece
 *        por encima o por debajo del límite, para no desgastar la Flash.
 */
static void sensorLogThresholdIfNeeded(void)
{
	const float upperLimit = setpoint + hysteresis;
	const float lowerLimit = setpoint - hysteresis;
	uint8_t zone;
	eeprom_record_t rec = {0};
	DS3231_DateTime_t dt = {0};
	char msg[80];

	if (temperature >= upperLimit)
	{
		zone = 2U;
	}
	else if (temperature <= lowerLimit)
	{
		zone = 3U;
	}
	else
	{
		zone = 1U;
	}

	if ((zone == lastTempZone) || (zone == 1U))
	{
		lastTempZone = zone;
		return;
	}

	rec.temperature = temperature;
	rec.setpoint = setpoint;
	rec.hysteresis = hysteresis;
	rec.event = (zone == 2U) ? EEPROM_EVT_UPPER : EEPROM_EVT_LOWER;

	if (DS3231_IsAvailable())
	{
		DS3231_ReadTime(&dt);
		DS3231_ReadDate(&dt);
		if (DS3231_IsAvailable())
		{
			rec.hour = dt.hour;
			rec.minute = dt.minute;
			rec.second = dt.second;
			rec.day = dt.day;
			rec.month = dt.month;
			rec.year = dt.year;
		}
	}

	if (eepromLogThreshold(&rec))
	{
		snprintf(msg, sizeof(msg),
		         "LOG: %s T=%.2f SP=%.2f %02u:%02u:%02u\r\n",
		         (rec.event == EEPROM_EVT_UPPER) ? "UPPER" : "LOWER",
		         rec.temperature, rec.setpoint,
		         rec.hour, rec.minute, rec.second);
		uartSendString((uint8_t *)msg);
	}

	lastTempZone = zone;
}

/**
 * @brief Informa el error de temperatura por LCD y/o UART.
 */
static void sensorReportError(void)
{
	char msg[80];
	char lcdMsg[17];

	switch (lastSensorError)
	{
	case SENSOR_ERR_OUT_OF_RANGE:
		snprintf(msg, sizeof(msg),
		         "ERROR: TEMP fuera de rango MCP9700A (%.2f C)\r\n", temperature);
		snprintf(lcdMsg, sizeof(lcdMsg), "ERR rango %.1f", temperature);
		break;

	case SENSOR_ERR_INVALID_NEG:
		snprintf(msg, sizeof(msg),
		         "ERROR: TEMP negativa invalida para Lima (%.2f C)\r\n", temperature);
		snprintf(lcdMsg, sizeof(lcdMsg), "ERR T<0  %.1f", temperature);
		break;

	case SENSOR_ERR_ADC:
		snprintf(msg, sizeof(msg), "ERROR: fallo de lectura ADC\r\n");
		snprintf(lcdMsg, sizeof(lcdMsg), "ERR ADC");
		break;

	default:
		snprintf(msg, sizeof(msg), "ERROR: sensor\r\n");
		snprintf(lcdMsg, sizeof(lcdMsg), "ERR sensor");
		break;
	}

	if (LCD_I2C_isAvailable())
	{
		LCD_I2C_cmd(LCD_CLEAR);
		LCD_I2C_write_text((uint8_t *)lcdMsg);
	}

	uartSendString((uint8_t *)msg);
}




