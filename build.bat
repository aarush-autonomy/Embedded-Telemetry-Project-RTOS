@echo off
echo   Compiling Embedded Telemetry Node System By Aarush Kavishwar


if not exist bin mkdir bin

gcc -Wall -Wextra -O2 -Iinclude src\vcu_logger.c -o bin\vcu_logger.exe
if %ERRORLEVEL% NEQ 0 (echo [ERROR] Failed to compile vcu_logger! & exit /b 1)

gcc -Wall -Wextra -O2 -Iinclude src\sensor_node.c -o bin\sensor_node.exe
if %ERRORLEVEL% NEQ 0 (echo [ERROR] Failed to compile sensor_node! & exit /b 1)

echo.
echo [SUCCESS] Binaries compiled into .\bin\
echo 1. Run .\bin\vcu_logger.exe
echo 2. Run .\bin\sensor_node.exe