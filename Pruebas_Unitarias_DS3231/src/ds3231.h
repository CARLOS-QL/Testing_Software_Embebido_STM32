/*
 * ds3231.h
 *
 *  Created on: Oct 15, 2026
 *      Author: Carlos Quispe
 *
 *  Driver del RTC DS3231. La API pública es equivalente a la del DS1307
 *  (registros 0x00..0x06, formato 12/24 h y conversión BCD).
 */

#ifndef API_INC_DS3231_H_
#define API_INC_DS3231_H_

#include <stdint.h>
#include <stdbool.h>

/** Dirección I2C de 7 bits utilizada por HAL. */
#define DS3231_ADDR            0x68U
/** Dirección I2C de escritura (8 bits), equivalente a 0x68 << 1. */
#define DS3231_ADDR_WRITE      0xD0U
/** Dirección I2C de lectura (8 bits). */
#define DS3231_ADDR_READ       0xD1U

/* ================================
 * Tipos de datos
 * ================================ */

/**
 * @brief Formato de representación de la hora en el DS3231.
 */
typedef enum
{
	DS3231_FORMAT_24H = 0,	/**< Hora en formato 24 horas (00..23). */
	DS3231_FORMAT_12H		/**< Hora en formato 12 horas (01..12) + AM/PM. */

} DS3231_HourFormat_t;


/**
 * @brief Indicador AM/PM. Solo tiene sentido si hourFormat es 12 h.
 */
typedef enum
{
	DS3231_AM = 0,			/**< Ante meridiem (00:00 a 11:59). */
	DS3231_PM				/**< Post meridiem (12:00 a 23:59). */

} DS3231_AMPM_t;


/**
 * @brief Fecha y hora del RTC.
 * @note year se almacena en dos dígitos (00..99), correspondientes a 2000..2099.
 */
typedef struct
{
	uint8_t hour;						/**< Hora: 0..23 (24 h) o 1..12 (12 h). */
	uint8_t minute;						/**< Minutos: 0..59. */
	uint8_t second;						/**< Segundos: 0..59. */

	uint8_t weekDay;					/**< Día de la semana: 1..7. */
	uint8_t day;						/**< Día del mes: 1..31. */
	uint8_t month;						/**< Mes: 1..12. */
	uint8_t year;						/**< Año: 00..99 (20xx). */

	DS3231_HourFormat_t hourFormat;		/**< Formato de hora activo (12 h / 24 h). */
	DS3231_AMPM_t amPm;					/**< AM/PM. En formato 24 h se ignora. */

} DS3231_DateTime_t;


/* ================================
 * Funciones públicas
 * ================================ */

/**
 * @brief  Inicializa el DS3231 y configura el formato de hora.
 * @note   Verifica la presencia del dispositivo en el bus I2C, habilita el
 *         oscilador interno y convierte el registro de hora al formato
 *         solicitado (12 h o 24 h). Si el RTC no responde, no bloquea
 *         el firmware y deja el módulo marcado como no disponible.
 * @param  format Formato de hora a utilizar (DS3231_FORMAT_24H o DS3231_FORMAT_12H).
 * @retval None
 */
void DS3231_Init(DS3231_HourFormat_t format);

/**
 * @brief  Deja el driver en estado inicial (no disponible).
 * @note   Útil en setUp() de pruebas unitarias para aislar cada test.
 * @retval None
 */
void DS3231_DeInit(void);

/**
 * @brief  Convierte el registro de horas entre formato 12 h y 24 h.
 * @note   Función pura (sin I2C): el bit 6 indica 12 h y el bit 5 indica PM.
 *         Si el registro ya está en el formato pedido, se devuelve igual.
 * @param  hourReg Valor crudo del registro 0x02.
 * @param  format  Formato destino (DS3231_FORMAT_24H o DS3231_FORMAT_12H).
 * @return Registro de horas listo para escribir en el DS3231.
 */
uint8_t DS3231_ApplyHourFormat(uint8_t hourReg, DS3231_HourFormat_t format);

/**
 * @brief  Lee la hora actual (hora, minutos y segundos).
 * @note   Interpreta el bit 12/24 h del registro de horas y completa
 *         los campos hourFormat y amPm de la estructura.
 * @param  dt Puntero a la estructura donde se almacenará la hora.
 *            Si es NULL, la función retorna sin acción.
 * @retval None
 */
void DS3231_ReadTime(DS3231_DateTime_t *dt);

/**
 * @brief  Lee la fecha actual (día de semana, día, mes y año).
 * @param  dt Puntero a la estructura donde se almacenará la fecha.
 *            Si es NULL, la función retorna sin acción.
 * @retval None
 */
void DS3231_ReadDate(DS3231_DateTime_t *dt);

/**
 * @brief  Escribe la hora en el DS3231.
 * @note   Utiliza hourFormat y amPm para armar el registro de horas.
 *         Tras una escritura correcta se limpia el flag OSF (oscilador detenido).
 * @param  dt Puntero a la estructura con hora, minutos, segundos y formato.
 *            Si es NULL, la función retorna sin acción.
 * @retval None
 */
void DS3231_WriteTime(const DS3231_DateTime_t *dt);

/**
 * @brief  Escribe la fecha en el DS3231.
 * @param  dt Puntero a la estructura con weekDay, day, month y year.
 *            Si es NULL, la función retorna sin acción.
 * @retval None
 */
void DS3231_WriteDate(const DS3231_DateTime_t *dt);

/**
 * @brief  Indica si el DS3231 está presente y operativo en el bus I2C.
 * @retval true  El RTC respondió y las transferencias pueden realizarse.
 * @retval false El RTC no está conectado o falló la última transacción I2C.
 */
bool DS3231_IsAvailable(void);

/**
 * @brief  Lee la temperatura interna del DS3231.
 * @note   Esta función no existe en el DS1307. La resolución es 0.25 °C.
 * @param  temperature Puntero donde se almacenará la temperatura en °C.
 * @retval true  Lectura correcta.
 * @retval false Puntero nulo, RTC no disponible o error de I2C.
 */
bool DS3231_GetTemperature(float *temperature);

#endif /* API_INC_DS3231_H_ */
