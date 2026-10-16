/*
 * eeprom_drv.h
 *
 *  Created on: Oct 15, 2026
 *      Author: Carlos Quispe
 *
 *  Emulación de EEPROM en Flash interna del STM32F446RE (sector 7).
 *  El micro no tiene EEPROM real; se reserva el último sector de 128 KB.
 */

#ifndef API_INC_EEPROM_DRV_H_
#define API_INC_EEPROM_DRV_H_

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Tipo de evento de umbral almacenado.
 */
typedef enum
{
	EEPROM_EVT_NONE = 0,	/**< Sin evento. */
	EEPROM_EVT_UPPER,		/**< Temperatura alcanzó el umbral superior. */
	EEPROM_EVT_LOWER		/**< Temperatura alcanzó el umbral inferior. */
} eeprom_event_t;

/**
 * @brief Registro de un cruce de umbral.
 */
typedef struct
{
	float temperature;		/**< Temperatura medida en °C. */
	float setpoint;			/**< Setpoint vigente al momento del evento. */
	float hysteresis;		/**< Histéresis vigente al momento del evento. */
	eeprom_event_t event;	/**< Umbral superior o inferior. */
	uint8_t hour;			/**< Hora del RTC (0 si no hay DS3231). */
	uint8_t minute;
	uint8_t second;
	uint8_t day;
	uint8_t month;
	uint8_t year;			/**< Año 00..99 (20xx). */
} eeprom_record_t;

/**
 * @brief  Inicializa la emulación EEPROM y localiza el próximo hueco libre.
 * @retval true  Sector localizado y listo para escribir.
 * @retval false Error al acceder a Flash.
 */
bool eepromInit(void);

/**
 * @brief  Almacena un evento de umbral en Flash (append).
 * @note   Si el sector está lleno, se borra y se comienza de nuevo.
 * @param  rec Puntero al registro a guardar.
 * @retval true  Escritura correcta.
 * @retval false Puntero nulo o fallo de programación.
 */
bool eepromLogThreshold(const eeprom_record_t *rec);

/**
 * @brief  Obtiene la cantidad de registros válidos almacenados.
 * @return Número de eventos guardados.
 */
uint16_t eepromGetCount(void);

/**
 * @brief  Lee un registro por índice (0 = más antiguo).
 * @param  index Índice del registro.
 * @param  rec   Destino de la lectura.
 * @retval true  Registro válido leído.
 * @retval false Índice fuera de rango o puntero nulo.
 */
bool eepromGetRecord(uint16_t index, eeprom_record_t *rec);

/**
 * @brief  Borra todos los registros (erase del sector de emulación).
 * @retval true  Borrado correcto.
 */
bool eepromClear(void);

#endif /* API_INC_EEPROM_DRV_H_ */
