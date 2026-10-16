/*
 * uart_drv.h
 *
 *  Created on: Apr 12, 2026
 *      Author: Carlos Quispe
 */

#ifndef API_INC_UART_DRV_H_
#define API_INC_UART_DRV_H_

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "stm32f4xx_hal.h"

#define UART_MAX_SIZE	256U
#define	RX_TIMEOUT_MS	2U
#define	TX_TIMEOUT_MS	100U

extern UART_HandleTypeDef huart2;

typedef bool bool_t;

bool_t uartInit(uint32_t baudRate);
bool_t uartSendString(uint8_t *pstring);
bool_t uartSendStringSize(uint8_t *pstring, uint16_t size);
bool_t uartReceiveStringSize(uint8_t *pstring, uint16_t size);


#endif /* API_INC_UART_DRV_H_ */
