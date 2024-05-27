@echo off

:: Set default values
set BUILD_TYPE=Debug
set PLATFORM=x64

:: Check for command line arguments
if "%1"=="" (
    echo No build type specified, defaulting to Debug.
) else (
    set BUILD_TYPE=%1
)

if "%2"=="" (
    echo No platform specified, defaulting to x64.
) else (
    set PLATFORM=%2
)

echo Configuring CMake for %BUILD_TYPE% and %PLATFORM%...
mkdir build\%PLATFORM%
cd build\%PLATFORM%
cmake -G "Visual Studio 17 2022" -A %PLATFORM% -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ../..
if %errorlevel% neq 0 (
    echo CMake configuration failed!
    cd ../..
    exit /b 1
)

:build_targets
echo Building Lua...
cmake --build . --target build_lua --config %BUILD_TYPE%
if %errorlevel% neq 0 (
    echo Building Lua target failed!
    cd ../..
    exit /b 1
)

echo Building Yaml...
cmake --build . --target build_yaml --config %BUILD_TYPE%
if %errorlevel% neq 0 (
    echo Building YAML target failed!
    cd ../..
    exit /b 1
)

cd ../..
