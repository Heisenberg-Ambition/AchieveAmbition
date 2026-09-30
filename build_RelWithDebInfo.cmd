@echo off

chcp 65001 >nul

setlocal

set "ROOT=%~dp0."

@REM clear
cls

call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 (
    echo VS environment init failed
    pause
    exit /b %errorlevel%
)

set VSLANG=1033

set LANG=en_US.UTF-8

cmake -S "%ROOT%" -B "%ROOT%\build" -G "Ninja" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_TOOLCHAIN_FILE=C:/Users/lsh55758/vcpkg/scripts/buildsystems/vcpkg.cmake
if errorlevel 1 (
    echo CMake configure failed
    pause
    exit /b %errorlevel%
)

cmake --build "%ROOT%\build"  --parallel 20
if errorlevel 1 (
    echo Build failed
    pause
    exit /b %errorlevel%
)

echo Build completed successfully!!!

@REM pause

