@echo off
echo 🌳 Installing wrldtree for Windows...

:: Check for GCC
where gcc >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] GCC not found. Please install MinGW-w64.
    pause
    exit /b 1
)

gcc -Wall -O2 wrldtree.c -o wrldtree.exe -std=gnu11

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Compilation failed.
    pause
    exit /b 1
)

echo [SUCCESS] Compiled wrldtree.exe
echo.
echo ---------------------------------------------------
echo ⚠ IMPORTANT: TO USE WRLDTREE FROM ANYWHERE ⚠
echo ---------------------------------------------------
echo 1. Create a folder like C:\bin (if you haven't already).
echo 2. Add C:\bin to your PATH using the Environment Variables GUI.
echo 3. Copy wrldtree.exe to C:\bin
echo.
echo DO NOT use 'setx PATH' commands - they truncate your settings!
pause