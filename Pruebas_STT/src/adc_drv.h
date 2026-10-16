/*
 * adc_drv.h
 *
 *  Created on: Apr 12, 2026
 *      Author: Carlos Quispe
 */

#ifndef API_INC_ADC_DRV_H_
#define API_INC_ADC_DRV_H_

#include "stdint.h"

#define ADC_TIMEOUT_MS   10U   /* Tiempo máximo de espera para completar la conversión ADC (ms) */

typedef enum
{
	ADC_OK = 0,
	ADC_ERROR_INIT,
	ADC_ERROR_CONFIG,
	ADC_ERROR_NULL_POINTER,
	ADC_ERROR_START,
	ADC_ERROR_TIMEOUT,
	ADC_ERROR_STOP
} adc_status_t;

/**
 * @brief Inicializa el periférico ADC y configura el canal.
 * @return Estado de la operación.
 */

adc_status_t adcInit(void);

/**
 * @brief Realiza una lectura cruda del ADC.
 * @param rawValue Puntero donde se almacenará el valor leído.
 * @return Estado de la operación.
 */

adc_status_t adcReadRaw(uint32_t *rawValue);



#endif /* API_INC_ADC_DRV_H_ */
