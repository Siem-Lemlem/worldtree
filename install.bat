@echo off
echo Installing wrldtree for Windows...

gcc -Wall -O2 wrldtree.c -o wrldtree.exe -std=gnu11

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Compilation failed. Make sure GCC is installed.
    echo Download from: https://www.mingw-w64.org/
    pause
    exit /b 1
)

echo [SUCCESS] Compiled wrldtree.exe

echo.
echo To use wrldtree from anywhere:
echo 1. Move wrldtree.exe to a folder in your PATH
echo 2. Or run: setx PATH "%%PATH%%;%CD%"
echo.
echo Try it: wrldtree --help
pause