@echo off
REM PocketMage PDA Desktop Emulator Build Script for Windows
setlocal enabledelayedexpansion

echo ===================================
echo PocketMage PDA Desktop Emulator Build
echo ===================================

REM Parse arguments
set CLEAN_BUILD=0
set BUILD_TYPE=Release

:parse_args
if "%~1"=="" goto :done_args
if "%~1"=="--clean" (
    set CLEAN_BUILD=1
    shift
    goto :parse_args
)
if "%~1"=="--debug" (
    set BUILD_TYPE=Debug
    shift
    goto :parse_args
)
shift
goto :parse_args
:done_args

REM Check for CMake
where cmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo ERROR: CMake is required but not found in PATH.
    echo Please install CMake from https://cmake.org/download/
    exit /b 1
)

REM Check for vcpkg SDL2
if not exist "C:\vcpkg\installed\x64-windows\lib\SDL2.lib" (
    echo WARNING: SDL2 not found in vcpkg.
    echo Please install SDL2 with: vcpkg install sdl2 sdl2-ttf:x64-windows
    echo.
    echo If vcpkg is not installed:
    echo   git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
    echo   C:\vcpkg\bootstrap-vcpkg.bat
    echo   C:\vcpkg\vcpkg integrate install
    echo   C:\vcpkg\vcpkg install sdl2 sdl2-ttf:x64-windows
    echo.
)

REM Download fonts if needed
if not exist "fonts\DejaVuSans.ttf" (
    echo Downloading fonts...
    if exist "fonts\download_fonts.ps1" (
        powershell -ExecutionPolicy Bypass -File fonts\download_fonts.ps1
    ) else (
        echo WARNING: Font download script not found. Fonts may be missing.
    )
)

REM Clean build if requested
if %CLEAN_BUILD%==1 (
    echo Cleaning build directory...
    if exist build rmdir /s /q build
)

REM Create build directory
if not exist build mkdir build
cd build

REM Configure with CMake
echo Configuring with CMake...
cmake .. -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
if %ERRORLEVEL% neq 0 (
    echo ERROR: CMake configuration failed.
    exit /b 1
)

REM Build
echo Building...
cmake --build . --config %BUILD_TYPE%
if %ERRORLEVEL% neq 0 (
    echo ERROR: Build failed.
    exit /b 1
)

echo.
echo ===================================
echo Build Complete!
echo ===================================
echo Run the emulator with: build\%BUILD_TYPE%\PocketMage_PDA_Emulator.exe
echo.

cd ..
