@echo off
REM Basic Visual Studio Environment Check Script

echo ======================================================
echo Basic Environment Check for CARLA UE5.5 Build

echo 1. Checking OS Version...
ver

echo.
echo 2. Checking Visual Studio 2022 Installation...
if exist "D:\VS2022\Community\Common7\IDE\devenv.exe" (
    echo Found VS2022 at D:\VS2022\Community
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\devenv.exe" (
    echo Found VS2022 at C:\Program Files\Microsoft Visual Studio\2022\Community
) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2022\Community\Common7\IDE\devenv.exe" (
    echo Found VS2022 at C:\Program Files (x86)\Microsoft Visual Studio\2022\Community
) else (
    echo Visual Studio 2022 NOT FOUND!
)

echo.
echo 3. Checking for C++ Compiler (cl.exe)...
where cl.exe
if %errorLevel% neq 0 (
    echo cl.exe NOT FOUND! Critical component missing for CARLA build.
    echo Solution: Use Visual Studio Installer to add "Desktop development with C++" workload.
) else (
    echo cl.exe found successfully!
)

echo.
echo 4. Checking CMake Installation...
where cmake.exe
if %errorLevel% neq 0 (
    echo CMake NOT FOUND! Required version: 3.24+
) else (
    echo CMake found! Checking version...
    cmake --version
)

echo.
echo ======================================================
echo Environment Check Complete!