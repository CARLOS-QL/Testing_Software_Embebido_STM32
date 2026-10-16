# Testing de software embebido — Control de temperatura STM32

**Autor:** CARLOS QUISPE LOPEZ  
**Curso:** Testing de Software de Sistemas Embebidos — FIUBA/UBA

## Contexto

Firmware de control de temperatura con **STM32F446RE** y sensor **MCP9700A**. Incluye lógica ON/OFF con histéresis, comunicación UART, LCD y reloj RTC DS3231 mediante I2C.

## Organización

| Carpeta | Contenido |
| --- | --- |
| `Firmware_STM32/` | Proyecto fuente de STM32CubeIDE, incluidos archivos `.ioc`, `Core/` y `Drivers/` (sin binarios de compilación). |
| `Pruebas_STT/` | Programa de pruebas para la máquina de estados `sensor_fsm.c`, con HAL y sensor simulados. |
| `Pruebas_Unitarias_DS3231/` | Driver real, interfaz I2C, 13 pruebas con Unity/CMock y configuración Ceedling. |
| `Documentacion/` | Presentación y guion de exposición. |

## Prueba STT

Desde `Pruebas_STT/`, en Windows con GCC en PATH:

```powershell
.\ejecutar_windows.bat
```

El programa recorre rutas normales y de fallo sobre la máquina de estados de temperatura. El trabajo expuesto reportó **9 verificaciones aprobadas en 4 recorridos**, con cobertura de 9/9 transiciones identificadas en el modelo. Estas verificaciones emplean **dependencias de hardware simuladas**; no reemplazan una validación sobre placa.

## Pruebas unitarias DS3231

En `Pruebas_Unitarias_DS3231/`, con Ruby, Ceedling 1.1.10 y GCC disponibles:

```powershell
ceedling test:test_ds3231
```

Para instrumentación de cobertura:

```powershell
ceedling gcov:all
```

Para generar HTML con gcovr, una vez disponibles `.gcno` y `.gcda`:

```powershell
py -m gcovr --root . --filter "src/ds3231\.c$" --html-details --output cobertura_ds3231.html build/gcov
```

Las pruebas de DS3231 ejecutan el driver real y sustituyen `i2c_drv` por mocks; **no prueban el periférico I2C físico**.

## Resultados documentados

En la ejecución del autor se registraron **13/13 pruebas DS3231 aprobadas** y cobertura reportada de **72,3 % líneas, 100 % funciones y 52,7 % ramas**. Estos resultados se incluyen como referencia histórica, no como mediciones ejecutadas automáticamente al descargar este repositorio. Repetir las pruebas en cada entorno.

## Nota sobre versiones

`test_ds3231.c` contiene las pruebas 1–13 del trabajo. En `test/pruebas_14_15_opcionales.txt` se conservan dos casos propuestos, **no incorporados** al conjunto de 13 aprobado. El firmware procede del archivo de proyecto aportado; los builds y configuraciones locales se excluyeron del repositorio.
