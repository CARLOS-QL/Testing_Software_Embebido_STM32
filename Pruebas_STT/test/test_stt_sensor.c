/* STT-01: Prueba de transiciones sobre sensor_fsm.c REAL.
 * Dependencias hardware simuladas. No es prueba end-to-end con placa STM32.
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "main.h"
#include "sensor_fsm.h"
#include "mcp9700a.h"
#include "lcd_pcf857.h"
#include "ds3231.h"
#include "eeprom_drv.h"
#include "uart_drv.h"

GPIO_TypeDef gpio_a_fake;
static uint32_t tick_fake;
static uint32_t pin_value;
static unsigned pin_set_calls, pin_reset_calls, read_calls, log_calls, uart_calls;
static mcp9700a_status_t reading_status;
static float reading_temperature;
static char last_uart[160];

uint32_t HAL_GetTick(void) { return tick_fake; }
void HAL_GPIO_WritePin(GPIO_TypeDef *port, uint16_t pin, uint32_t state) {
    assert(port == GPIOA && pin == GPIO_PIN_5);
    pin_value = state;
    if (state == GPIO_PIN_SET) pin_set_calls++;
    else pin_reset_calls++;
}
mcp9700a_status_t mcp9700aReadTemperature(float *out) {
    read_calls++;
    if (reading_status == MCP9700A_OK) *out = reading_temperature;
    return reading_status;
}
bool eepromInit(void) { return true; }
bool eepromLogThreshold(const eeprom_record_t *rec) { assert(rec); log_calls++; return true; }
bool DS3231_IsAvailable(void) { return false; }
void DS3231_ReadTime(DS3231_DateTime_t *dt) { (void)dt; assert(!"RTC simulado como ausente"); }
void DS3231_ReadDate(DS3231_DateTime_t *dt) { (void)dt; assert(!"RTC simulado como ausente"); }
bool LCD_I2C_isAvailable(void) { return false; }
void LCD_I2C_cmd(uint8_t command) { (void)command; }
void LCD_I2C_write_text(uint8_t *s) { (void)s; }
bool_t uartSendString(uint8_t *s) {
    uart_calls++;
    snprintf(last_uart, sizeof(last_uart), "%s", (char *)s);
    return true;
}

static void report(const char *name) { printf("[PASS] %s\n", name); }
int main(void) {
    tick_fake=0;
    reading_status=MCP9700A_OK;
    reading_temperature=32.0f;
    sensorFsmInit();
    sensorSetSetpoint(30.0f);
    sensorSetHysteresis(1.0f);

    /* T1: IDLE -> IDLE antes de cumplir 500ms */
    tick_fake=499;
    sensorFsmUpdate();
    assert(read_calls==0 && pin_set_calls==0 && pin_reset_calls==0);
    report("T1: antes de 500ms no lee el sensor");

    /* T2: IDLE -> READ (sin leer aun) */
    tick_fake=500;
    sensorFsmUpdate();
    assert(read_calls==0);
    report("T2: 500ms habilita el estado READ");

    /* T3: READ -> CONTROL */
    sensorFsmUpdate();
    assert(read_calls==1 && pin_set_calls==0);
    report("T3: 32C valida entra en CONTROL");

    /* T7: CONTROL -> OUTPUT, setpoint=30, hyst=1, 32 >= 31 */
    sensorFsmUpdate();
    assert(pin_value==GPIO_PIN_SET && pin_set_calls>=1);
    assert(log_calls==1);
    report("T7: control activa PA5 y registra umbral");

    /* T8: OUTPUT -> IDLE */
    unsigned before_uart=uart_calls;
    sensorFsmUpdate();
    assert(pin_value==GPIO_PIN_SET && uart_calls>before_uart);
    assert(strstr(last_uart,"TEMP=32.00")!=NULL);
    report("T8: salida informa temperatura y vuelve a IDLE");

    /* Segundo ciclo: IDLE -> READ -> ERROR -> IDLE */
    tick_fake=1000;
    sensorFsmUpdate();                 /* T2 */
    reading_status=MCP9700A_ERROR_ADC;
    sensorFsmUpdate();                 /* T4: READ -> ERROR */
    assert(read_calls==2 && pin_value==GPIO_PIN_SET);
    report("T4: fallo ADC cambia a ERROR, aun no ejecuta accion segura");

    sensorFsmUpdate();                 /* T9: ERROR -> IDLE */
    assert(pin_value==GPIO_PIN_RESET && pin_reset_calls>=1);
    assert(strstr(last_uart,"ERROR: fallo de lectura ADC")!=NULL);
    report("T9: ERROR desactiva PA5 y reporta fallo ADC");

    /* T5: tercer ciclo, rango fuera de especificacion reportado por sensor */
    tick_fake=1500;
    sensorFsmUpdate();                /* T2: IDLE -> READ */
    reading_status=MCP9700A_ERROR_OUT_OF_RANGE;
    reading_temperature=130.0f;      /* valor ilustrativo fuera de -40 a 125 C */
    unsigned reads_before=read_calls;
    unsigned writes_before=pin_set_calls + pin_reset_calls;
    sensorFsmUpdate();                /* T5: READ -> ERROR */
    assert(read_calls==reads_before+1);
    assert(pin_set_calls + pin_reset_calls==writes_before);
    sensorFsmUpdate();                /* T9: ERROR -> IDLE */
    assert(pin_value==GPIO_PIN_RESET);
    assert(strstr(last_uart,"fuera de rango MCP9700A")!=NULL);
    report("T5: fuera de rango entra en ERROR y reporta el fallo");

    /* T6: cuarto ciclo, lectura aceptada por MCP9700A pero menor a 0 C */
    tick_fake=2000;
    sensorFsmUpdate();                /* T2: IDLE -> READ */
    reading_status=MCP9700A_OK;
    reading_temperature=-5.0f;       /* sensor admite negativos, la app no */
    reads_before=read_calls;
    writes_before=pin_set_calls + pin_reset_calls;
    sensorFsmUpdate();                /* T6: READ -> ERROR */
    assert(read_calls==reads_before+1);
    assert(pin_set_calls + pin_reset_calls==writes_before);
    sensorFsmUpdate();                /* T9: ERROR -> IDLE */
    assert(pin_value==GPIO_PIN_RESET);
    assert(strstr(last_uart,"TEMP negativa invalida")!=NULL);
    report("T6: temperatura negativa entra en ERROR y reporta el fallo");

    printf("\nSTT-01: 9 verificaciones aprobadas (4 recorridos).\n");
    printf("Cobertura de transiciones del modelo: T1,T2,T3,T4,T5,T6,T7,T8,T9 (9/9).\n");
    printf("Nota: prueba de MEF con hardware simulado, no end-to-end en STM32.\n");
    return 0;
}
