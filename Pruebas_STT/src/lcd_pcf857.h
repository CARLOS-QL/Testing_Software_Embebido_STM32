/*
 * lcd_pcf857.h
 *
 *  Created on: Apr 23, 2026
 *      Author: Carlos Qusipe
 */

#ifndef API_INC_LCD_PCF857_H_
#define API_INC_LCD_PCF857_H_


#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "i2c_drv.h"

#define LCD_DIR 0x27

//Etiquetas para uso de retardos

#define LCD_DELAY_20_MS    20U
#define LCD_DELAY_5_MS     5U
#define LCD_DELAY_1_MS     1U
#define LCD_DELAY_2_MS     2U

// Definición de
#define     LCD_CLEAR	    0b00000001  // Limpia pantalla
#define     LCD_HOME	    0b00000010  // Retorno al inicio
#define     LCD_CURSOR_ON	0b00001111  // Cursor on
#define     LCD_CURSOR_OFF	0b00001100  // Cursor off
#define     LCD_ROW1	    0b00000000  // Primera Fila
#define     LCD_ROW2		0b11000000	// Segunda Fila
#define     LCD_ROW3		0b10010100	// Tercera Fila
#define     LCD_ROW4		0b11010100	// Cuarta Fila
#define     LCD_LEFT		0b00010000	// Cursor a la izquierda
#define     LCD_RIGHT		0b00010100	// Cursor a la derecha
#define     LCD_ROT_LEFT	0b00011000	// Rotar a la izquierda
#define     LCD_ROT_RIGHT	0b00011100	// Rotar a la derecha
#define     LCD_OFF 		0b00001000	// apaga el display

#define 	LCD_ENABLE         0x04U
#define 	LCD_8BIT      0x30U
#define 	LCD_4BIT      0x20U

/**
 * @brief Inicializa la pantalla LCD mediante interfaz I2C (PCF8574).
 * Configura el LCD en modo de 4 bits, establece el número de líneas,
 * limpia la pantalla y habilita la visualización sin cursor.
 * Si el LCD no responde, no bloquea y deja el display marcado como no disponible.
 * @return void
 */
void LCD_I2C_init(void);

/**
 * @brief Indica si el LCD I2C está presente y operativo.
 * @return true si el LCD responde, false si no hay comunicación I2C.
 */
bool LCD_I2C_isAvailable(void);

/**
 * @brief Envía un comando al LCD.
 * Permite configurar el LCD (clear, cursor, modo, etc.)
 * mediante el envío de un byte de comando en formato de 4 bits.
 * @param lcdCmd Comando a enviar al LCD.
 * @return void
 */
void LCD_I2C_cmd(uint8_t lcdCmd);

/**
 * @brief Envía un carácter al LCD.
 * Escribe un dato ASCII en la posición actual del cursor.
 * @param lcdData Carácter a enviar al LCD.
 * @return void
 */
void LCD_I2C_char(uint8_t lcdData);

/**
 * @brief Envía una cadena de texto al LCD.
 * Recorre un string terminado en '\0' y envía cada carácter
 * de forma secuencial al LCD.
 * @param lcdText Puntero a la cadena de texto a mostrar.
 * @return void
 */
void LCD_I2C_write_text(uint8_t* lcdText);



#endif /* API_INC_LCD_PCF857_H_ */
