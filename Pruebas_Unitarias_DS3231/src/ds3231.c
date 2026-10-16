/*
 * ds3231.c
 *
 *  Created on: Oct 15, 2026
 *      Author: Carlos Quispe
 */

#include "ds3231.h"
#include "i2c_drv.h"

/* ================================
 * Registros DS3231 (hora/fecha = DS1307)
 * ================================ */

#define DS3231_REG_SECONDS     0x00U
#define DS3231_REG_MINUTES     0x01U
#define DS3231_REG_HOURS       0x02U
#define DS3231_REG_WEEKDAY     0x03U
#define DS3231_REG_DATE        0x04U
#define DS3231_REG_MONTH       0x05U
#define DS3231_REG_YEAR        0x06U
#define DS3231_REG_CONTROL     0x0EU
#define DS3231_REG_STATUS      0x0FU
#define DS3231_REG_TEMP_MSB    0x11U

#define DS3231_CTRL_EOSC       0x80U
#define DS3231_STAT_OSF        0x80U

static bool ds3231Available = false;

static uint8_t DS3231_BCDToDecimal(uint8_t bcd);
static uint8_t DS3231_DecimalToBCD(uint8_t decimal);
static bool DS3231_EnsureReady(void);
static void DS3231_EnableOscillator(void);

/* ================================
 * Funciones privadas
 * ================================ */

/**
 * @brief  Convierte un valor BCD a decimal.
 * @param  bcd Valor en formato BCD.
 * @retval Valor decimal equivalente.
 */
static uint8_t DS3231_BCDToDecimal(uint8_t bcd)
{
	return (uint8_t)(((bcd >> 4) * 10u) + (bcd & 0x0Fu));
}

/**
 * @brief  Convierte un valor decimal a BCD.
 * @param  decimal Valor decimal (0..99).
 * @retval Valor en formato BCD.
 */
static uint8_t DS3231_DecimalToBCD(uint8_t decimal)
{
	return (uint8_t)(((decimal / 10u) << 4) | (decimal % 10u));
}

/**
 * @brief  Verifica que el DS3231 esté disponible; si no, reintenta el probe I2C.
 * @retval true si el dispositivo responde.
 */
static bool DS3231_EnsureReady(void)
{
	if (ds3231Available)
	{
		return true;
	}

	ds3231Available = i2cDeviceReady(DS3231_ADDR);
	return ds3231Available;
}

/**
 * @brief  Asegura que el oscilador del DS3231 esté habilitado (EOSC = 0).
 * @retval None
 */
static void DS3231_EnableOscillator(void)
{
	uint8_t ctrl = 0U;

	if (!i2cMemRead(DS3231_ADDR, DS3231_REG_CONTROL, &ctrl, 1U))
	{
		ds3231Available = false;
		return;
	}

	if ((ctrl & DS3231_CTRL_EOSC) != 0U)
	{
		ctrl &= (uint8_t)~DS3231_CTRL_EOSC;
		if (!i2cMemWrite(DS3231_ADDR, DS3231_REG_CONTROL, &ctrl, 1U))
		{
			ds3231Available = false;
		}
	}
}

/**
 * @brief  Convierte el registro de horas entre formato 12 h y 24 h.
 * @param  hourReg Valor crudo del registro 0x02.
 * @param  format  Formato destino.
 * @return Registro convertido (o el mismo si ya tenía ese formato).
 */
uint8_t DS3231_ApplyHourFormat(uint8_t hourReg, DS3231_HourFormat_t format)
{
	uint8_t hour;
	uint8_t isPM;

	if (format == DS3231_FORMAT_24H)
	{
		if (hourReg & 0x40u)					/* ¿Actualmente está en 12 horas? */
		{
			isPM = hourReg & 0x20u;
			hour = DS3231_BCDToDecimal(hourReg & 0x1Fu);

			if (isPM)
			{
				if (hour != 12u)
				{
					hour += 12u;
				}
			}
			else
			{
				if (hour == 12u)
				{
					hour = 0u;
				}
			}
			hourReg = DS3231_DecimalToBCD(hour);
		}
	}
	else
	{
		if ((hourReg & 0x40u) == 0u)			/* ¿Actualmente está en 24 horas? */
		{
			hour = DS3231_BCDToDecimal(hourReg & 0x3Fu);

			if (hour == 0u)
			{
				hour = 12u;
				hourReg = DS3231_DecimalToBCD(hour) | 0x40u;
			}
			else if (hour < 12u)
			{
				hourReg = DS3231_DecimalToBCD(hour) | 0x40u;
			}
			else if (hour == 12u)
			{
				hourReg = DS3231_DecimalToBCD(hour) | 0x40u | 0x20u;
			}
			else
			{
				hour -= 12u;
				hourReg = DS3231_DecimalToBCD(hour) | 0x40u | 0x20u;
			}
		}
	}

	return hourReg;
}

/**
 * @brief  Deja el driver en estado no disponible.
 * @retval None
 */
void DS3231_DeInit(void)
{
	ds3231Available = false;
}

/**
 * @brief  Inicializa el DS3231 y configura el formato de hora.
 * @param  format Formato de hora a utilizar (12 h o 24 h).
 * @retval None
 */
void DS3231_Init(DS3231_HourFormat_t format)
{
	uint8_t hourReg = 0U;

	ds3231Available = i2cDeviceReady(DS3231_ADDR);
	if (!ds3231Available)
	{
		return;
	}

	DS3231_EnableOscillator();
	if (!ds3231Available)
	{
		return;
	}

	if (!i2cMemRead(DS3231_ADDR, DS3231_REG_HOURS, &hourReg, 1U))
	{
		ds3231Available = false;
		return;
	}

	hourReg = DS3231_ApplyHourFormat(hourReg, format);

	if (!i2cMemWrite(DS3231_ADDR, DS3231_REG_HOURS, &hourReg, 1U))
	{
		ds3231Available = false;
	}
}

/**
 * @brief  Indica si el DS3231 está presente y operativo.
 * @retval true si el RTC responde; false en caso contrario.
 */
bool DS3231_IsAvailable(void)
{
	return ds3231Available;
}

/**
 * @brief  Lee hora, minutos y segundos desde los registros 0x00..0x02.
 * @param  dt Puntero a la estructura de destino.
 * @retval None
 */
void DS3231_ReadTime(DS3231_DateTime_t *dt)
{
	uint8_t data[3] = {0};

	if (dt == 0)
	{
		return;
	}

	if (!DS3231_EnsureReady())
	{
		return;
	}

	if (!i2cMemRead(DS3231_ADDR, DS3231_REG_SECONDS, data, sizeof(data)))
	{
		ds3231Available = false;
		return;
	}

	dt->second = DS3231_BCDToDecimal(data[0] & 0x7Fu);
	dt->minute = DS3231_BCDToDecimal(data[1] & 0x7Fu);

	if (data[2] & 0x40u)					/* Modo 12 horas */
	{
		dt->hourFormat = DS3231_FORMAT_12H;
		dt->hour = DS3231_BCDToDecimal(data[2] & 0x1Fu);

		if (data[2] & 0x20u)
		{
			dt->amPm = DS3231_PM;
		}
		else
		{
			dt->amPm = DS3231_AM;
		}
	}
	else									/* Modo 24 horas */
	{
		dt->hourFormat = DS3231_FORMAT_24H;
		dt->hour = DS3231_BCDToDecimal(data[2] & 0x3Fu);
		dt->amPm = DS3231_AM;
	}
}

/**
 * @brief  Lee día de semana, día, mes y año desde los registros 0x03..0x06.
 * @param  dt Puntero a la estructura de destino.
 * @retval None
 */
void DS3231_ReadDate(DS3231_DateTime_t *dt)
{
	uint8_t data[4] = {0};

	if (dt == 0)
	{
		return;
	}

	if (!DS3231_EnsureReady())
	{
		return;
	}

	if (!i2cMemRead(DS3231_ADDR, DS3231_REG_WEEKDAY, data, sizeof(data)))
	{
		ds3231Available = false;
		return;
	}

	dt->weekDay = data[0] & 0x07u;
	dt->day     = DS3231_BCDToDecimal(data[1] & 0x3Fu);
	dt->month   = DS3231_BCDToDecimal(data[2] & 0x1Fu);
	dt->year    = DS3231_BCDToDecimal(data[3]);
}

/**
 * @brief  Escribe hora, minutos y segundos en los registros 0x00..0x02.
 * @param  dt Puntero a la estructura con los valores a programar.
 * @retval None
 */
void DS3231_WriteTime(const DS3231_DateTime_t *dt)
{
	uint8_t raw[3];
	uint8_t hourReg;
	uint8_t status = 0U;

	if (dt == 0)
	{
		return;
	}

	if (!DS3231_EnsureReady())
	{
		return;
	}

	if (dt->hourFormat == DS3231_FORMAT_12H)
	{
		hourReg = DS3231_DecimalToBCD(dt->hour);
		hourReg |= 0x40u;					/* Activar formato 12 h */

		if (dt->amPm == DS3231_PM)
		{
			hourReg |= 0x20u;
		}
	}
	else
	{
		hourReg = DS3231_DecimalToBCD(dt->hour);
	}

	raw[0] = DS3231_DecimalToBCD(dt->second);
	raw[1] = DS3231_DecimalToBCD(dt->minute);
	raw[2] = hourReg;

	if (!i2cMemWrite(DS3231_ADDR, DS3231_REG_SECONDS, raw, sizeof(raw)))
	{
		ds3231Available = false;
		return;
	}

	if (i2cMemRead(DS3231_ADDR, DS3231_REG_STATUS, &status, 1U))
	{
		status &= (uint8_t)~DS3231_STAT_OSF;
		(void)i2cMemWrite(DS3231_ADDR, DS3231_REG_STATUS, &status, 1U);
	}
}

/**
 * @brief  Escribe día de semana, día, mes y año en los registros 0x03..0x06.
 * @param  dt Puntero a la estructura con los valores a programar.
 * @retval None
 */
void DS3231_WriteDate(const DS3231_DateTime_t *dt)
{
	uint8_t raw[4];

	if (dt == 0)
	{
		return;
	}

	if (!DS3231_EnsureReady())
	{
		return;
	}

	raw[0] = dt->weekDay;
	raw[1] = DS3231_DecimalToBCD(dt->day);
	raw[2] = DS3231_DecimalToBCD(dt->month);
	raw[3] = DS3231_DecimalToBCD(dt->year);

	if (!i2cMemWrite(DS3231_ADDR, DS3231_REG_WEEKDAY, raw, sizeof(raw)))
	{
		ds3231Available = false;
	}
}

/**
 * @brief  Lee la temperatura interna del DS3231 (registros 0x11 y 0x12).
 * @param  temperature Puntero donde se devolverá el valor en °C.
 * @retval true si la lectura fue correcta; false si hubo error.
 */
bool DS3231_GetTemperature(float *temperature)
{
	uint8_t raw[2] = {0};
	int8_t msb;

	if (temperature == 0)
	{
		return false;
	}

	if (!DS3231_EnsureReady())
	{
		return false;
	}

	if (!i2cMemRead(DS3231_ADDR, DS3231_REG_TEMP_MSB, raw, sizeof(raw)))
	{
		ds3231Available = false;
		return false;
	}

	msb = (int8_t)raw[0];
	*temperature = (float)msb + (((raw[1] >> 6) & 0x03U) * 0.25f);

	return true;
}
