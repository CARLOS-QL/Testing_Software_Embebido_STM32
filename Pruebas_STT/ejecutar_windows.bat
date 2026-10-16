@echo off
cd /d "%~dp0"
gcc -std=c11 -Wall -Wextra -Iinclude -Isrc src\sensor_fsm.c test\test_stt_sensor.c -o stt_control.exe
if errorlevel 1 (
 echo ERROR DE COMPILACION
 exit /b 1
)
stt_control.exe
if errorlevel 1 (
 echo ERROR DE PRUEBA
 exit /b 1
)
