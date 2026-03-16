@echo off
REM Pyro Programming Language - Windows Installer
REM Created by Aravind Pilla

echo.
echo   _   _
echo  ^| \ ^| ^| _____   ____ _
echo  ^|  \^| ^|/ _ \ \ / / _` ^|
echo  ^| ^|\  ^| (_) \ V / (_^| ^|
echo  ^|_^| \_^|\___/ \_/ \__,_^|
echo.
echo  Pyro Programming Language Installer
echo  Created by Aravind Pilla
echo.

REM Check for Visual Studio
where cl >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo Error: MSVC compiler not found.
    echo Please run this from a Visual Studio Developer Command Prompt.
    echo Or install Visual Studio Build Tools with C++ support.
    exit /b 1
)

REM Check for CMake
where cmake >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo Error: CMake not found.
    echo Download from: https://cmake.org/download/
    exit /b 1
)

echo Building Pyro...
if not exist build mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
cd ..

echo.
echo Copying nova.exe to build directory...
if exist "build\Release\nova.exe" (
    copy "build\Release\nova.exe" "build\nova.exe"
)

echo.
echo ========================================
echo  Pyro built successfully!
echo ========================================
echo.
echo  Binary: build\nova.exe
echo.
echo  To install system-wide, add build\ to your PATH:
echo    setx PATH "%%PATH%%;%CD%\build"
echo.
echo  Test with: pyro version
echo  Run:       pyro run examples\hello.ro
echo.
echo  Happy coding with Pyro! - Aravind Pilla
echo.
