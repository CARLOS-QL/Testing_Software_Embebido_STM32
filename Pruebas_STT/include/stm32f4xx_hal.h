#ifndef STM32F4XX_HAL_H
#define STM32F4XX_HAL_H
#include <stdint.h>
typedef struct { int dummy; } UART_HandleTypeDef;
typedef struct { int dummy; } GPIO_TypeDef;
extern GPIO_TypeDef gpio_a_fake;
#define GPIOA (&gpio_a_fake)
#define GPIO_PIN_5 5U
#define GPIO_PIN_SET 1U
#define GPIO_PIN_RESET 0U
uint32_t HAL_GetTick(void);
void HAL_GPIO_WritePin(GPIO_TypeDef *port, uint16_t pin, uint32_t state);
#endif
