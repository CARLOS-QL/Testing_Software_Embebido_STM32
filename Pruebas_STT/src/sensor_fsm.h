/*
 * sensor_fsm.h
 *
 *  Created on: Apr 12, 2026
 *      Author: Carlos Quispe
 */

#ifndef API_INC_SENSOR_FSM_H_
#define API_INC_SENSOR_FSM_H_

#include <stdint.h>
#include <stdio.h>

/**
 * @brief Inicializa la MEF del sensor.
 */
void sensorFsmInit(void);

/**
 * @brief Ejecuta la máquina de estados del sensor.
 */
void sensorFsmUpdate(void);

/**
 * @brief Permite configurar el setpoint.
 */
void sensorSetSetpoint(float sp);

/**
 * @brief Permite configurar la histéresis (% o valor absoluto).
 * @param hyst Valor de histéresis en grados Celsius.
 */
void sensorSetHysteresis(float hyst);

/**
 * @brief Obtiene el valor actual del setpoint.
 * @return Valor del setpoint configurado en grados Celsius.
 */
float sensorGetSetpoint(void);

/**
 * @brief Obtiene el valor actual de la histéresis.
 * @return Valor de la histéresis configurada en grados Celsius.
 */
float sensorGetHysteresis(void);

/**
 * @brief Obtiene el valor actual de la temperatura.
 * @return Valor de la temperatura configurada en grados Celsius.
 */
float sensorGetTemperature(void);


#endif /* API_INC_SENSOR_FSM_H_ */
