/*
 * i2c_drv.h
 *
 *  Created on: Apr 23, 2026
 *      Author: CARLOSQL
 */

#ifndef API_INC_I2C_DRV_H_
#define API_INC_I2C_DRV_H_

#include <stdbool.h>
#include <stdint.h>


#define I2C_TIMEOUT_MS    20U

void LCD_Write_Byte(uint8_t valor);

bool PCF8574_write(uint8_t lcdDato);
bool PCF8574_isReady(void);

bool i2cDeviceReady(uint8_t addr7);
bool i2cMemWrite(uint8_t addr7, uint8_t memAddr, uint8_t *data, uint16_t len);
bool i2cMemRead(uint8_t addr7, uint8_t memAddr, uint8_t *data, uint16_t len);

#endif /* API_INC_I2C_DRV_H_ */
