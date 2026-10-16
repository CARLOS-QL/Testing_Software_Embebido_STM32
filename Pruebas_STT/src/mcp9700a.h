/*
 * mcp9700a_drv.h
 *
 *  Created on: Apr 12, 2026
 *      Author: CARLOSQL
 */

#ifndef API_INC_MCP9700A_H_
#define API_INC_MCP9700A_H_

#include "stdint.h"
#include "stdbool.h"
#include "adc_drv.h"

/* Rango de operación del MCP9700A según datasheet */
#define MCP9700A_TEMP_MIN_C   (-40.0f)
#define MCP9700A_TEMP_MAX_C   (125.0f)

typedef enum
{
	MCP9700A_OK = 0,
	MCP9700A_ERROR_NULL_POINTER,
	MCP9700A_ERROR_ADC,
	MCP9700A_ERROR_OUT_OF_RANGE
} mcp9700a_status_t;


/**
 * @brief Lee la temperatura del sensor MCP9700A en grados Celsius.
 * @param temperatureC Puntero donde se almacenará la temperatura.
 * @return Estado de la operación.
 */
mcp9700a_status_t mcp9700aReadTemperature(float *temperature);

#endif /* API_INC_MCP9700A_H_ */
