STT-01 - Sistema de control de temperatura STM32

Objeto: sensor_fsm.c ORIGINAL del proyecto ControlTemperatura_UART_RTC.
Se reemplazan HAL, MCP9700A, RTC, EEPROM, UART y LCD por funciones de prueba.
NO se prueba hardware real ni se certifica el sistema completo.

EJECUCION EN WINDOWS:
1. Descomprime la carpeta.
2. Abrela en VS Code.
3. Desde el terminal del directorio ejecuta: .\ejecutar_windows.bat
   (Requiere GCC en PATH, el mismo compilador utilizado por Ceedling.)

O manualmente:
gcc -std=c11 -Wall -Wextra -Iinclude -Isrc src/sensor_fsm.c test/test_stt_sensor.c -o stt_control.exe
.\stt_control.exe

CRITERIO:
Los estados son static, por eso verificamos transiciones mediante efectos observables.
Se evalua tiempo 499/500 ms, lectura valida 32C, activacion PA5, fallo ADC,
fuera de rango del sensor (130C) y temperatura negativa (-5C).
Cobertura de transiciones del modelo: T1 a T9 (9/9).
Son nueve verificaciones repartidas en cuatro recorridos; T2 y T9 se repiten.
No equivale a cobertura de codigo ni a prueba de sistema de extremo a extremo.
Importante: el archivo ejecutable se genera localmente, no se incluye.
