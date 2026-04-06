@echo off
setlocal

set COMPILER=cl
set CXX=g++
set CXXFLAGS=/EHsc /std:c++17 /W4 /O2
set TARGET=katago_game.exe

where g++ >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo Error: g++ not found. Please install MinGW-w64 or Visual Studio.
    exit /b 1
)

echo Building %TARGET%...
g++ %CXXFLAGS% -o %TARGET% main.cpp -lws2_32

if %ERRORLEVEL% EQU 0 (
    echo Build successful: %TARGET%
    echo.
    echo To run the server:
    echo   .\%TARGET%
    echo.
    echo Then open http://localhost:8000 in your browser.
) else (
    echo Build failed!
    exit /b 1
)
