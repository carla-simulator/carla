@echo off
setlocal enabledelayedexpansion

rem BAT script that creates the library for conversion from OSM to OpenDRIVE (carla.org).
rem Run it through a cmd with the x64 Visual C++ Toolset enabled.

set LOCAL_PATH=%~dp0
set FILE_N=-[%~n0]:

rem Print batch params (debug purpose)
echo %FILE_N% [Batch params]: %*

rem ============================================================================
rem -- Parse arguments ---------------------------------------------------------
rem ============================================================================

set DOC_STRING=Build LibCarla.
set USAGE_STRING=Usage: %FILE_N% [-h^|--help] [--rebuild] [--build] [--clean] [--no-pull]

set REMOVE_INTERMEDIATE=false
set BUILD_OSM2ODR=false
set GIT_PULL=true
set CURRENT_OSM2ODR_COMMIT=ee0c2b9241fef5365a6bc044ac82e6580b8ce936
set OSM2ODR_BRANCH=carla_osm2odr
set OSM2ODR_REPO=https://github.com/carla-simulator/sumo.git

:arg-parse
if not "%1"=="" (
    if "%1"=="--rebuild" (
        set REMOVE_INTERMEDIATE=true
        set BUILD_OSM2ODR=true
    )
    if "%1"=="--build" (
        set BUILD_OSM2ODR=true
    )
    if "%1"=="--no-pull" (
        set GIT_PULL=false
    )
    if "%1"=="--clean" (
        set REMOVE_INTERMEDIATE=true
    )
    if "%1"=="-h" (
        echo %DOC_STRING%
        echo %USAGE_STRING%
        GOTO :eof
    )
    if "%1"=="--help" (
        echo %DOC_STRING%
        echo %USAGE_STRING%
        GOTO :eof
    )
    shift
    goto :arg-parse
)

if %REMOVE_INTERMEDIATE% == false (
    if %BUILD_OSM2ODR% == false (
        echo Nothing selected to be done.
        goto :eof
    )
)

rem ============================================================================
rem -- Local Variables ---------------------------------------------------------
rem ============================================================================

rem Set the visual studio solution directory
rem
set OSM2ODR_VSPROJECT_PATH=%INSTALLATION_DIR:/=\%osm2odr-visualstudio\
set OSM2ODR_SOURCE_PATH=%INSTALLATION_DIR:/=\%om2odr-source\
set OSM2ODR_INSTALL_PATH=%ROOT_PATH:/=\%PythonAPI\carla\dependencies\

if %REMOVE_INTERMEDIATE% == true (
    rem Remove directories
    for %%G in (
        "%OSM2ODR_INSTALL_PATH%",
    ) do (
        if exist %%G (
            echo %FILE_N% Cleaning %%G
            rmdir /s/q %%G
        )
    )
)

rem Build OSM2ODR
if %BUILD_OSM2ODR% == true (

    if  %GIT_PULL% == true (
        if not exist "%OSM2ODR_SOURCE_PATH%" git clone -b %OSM2ODR_BRANCH% %OSM2ODR_REPO% %OSM2ODR_SOURCE_PATH%
        cd "%OSM2ODR_SOURCE_PATH%"
        git fetch
        git checkout %CURRENT_OSM2ODR_COMMIT%
    )

    if not exist "%OSM2ODR_VSPROJECT_PATH%" mkdir "%OSM2ODR_VSPROJECT_PATH%"
    cd "%OSM2ODR_VSPROJECT_PATH%"

    cmake -G "Visual Studio 16 2019" -A x64^
        -DCMAKE_CXX_FLAGS_RELEASE="/MD /MP"^
        -DCMAKE_INSTALL_PREFIX="%OSM2ODR_INSTALL_PATH:\=/%"^
        -DPROJ_INCLUDE_DIR=%INSTALLATION_DIR:/=\%\proj-install\include^
        -DPROJ_LIBRARY=%INSTALLATION_DIR:/=\%\proj-install\lib\proj.lib^
        -DXercesC_INCLUDE_DIR=%INSTALLATION_DIR:/=\%\xerces-c-3.2.3-install\include^
        -DXercesC_LIBRARY=%INSTALLATION_DIR:/=\%\xerces-c-3.2.3-install\lib\xerces-c.lib^
        "%OSM2ODR_SOURCE_PATH%"
    if %errorlevel% neq 0 goto error_cmake

    cmake --build . --config Release --target install | findstr /V "Up-to-date:"
    if %errorlevel% neq 0 goto error_install
)

goto success

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
rem ============================================================================

:success
    if %BUILD_OSM2ODR% == true echo %FILE_N% OSM2ODR has been successfully installed in "%OSM2ODR_INSTALL_PATH%"!
    goto good_exit

:error_cmake
    echo.
    echo %FILE_N% [ERROR] An error ocurred while executing the cmake.
    echo           [ERROR] Possible causes:
    echo           [ERROR]  - Make sure "CMake" is installed.
    echo           [ERROR]  - Make sure it is available on your Windows "path".
    echo           [ERROR]  - CMake 3.9.0 or higher is required.
    goto bad_exit

:error_install
    echo.
    echo %FILE_N% [ERROR] An error ocurred while installing using Visual Studio 16 2019 Win64.
    echo           [ERROR] Possible causes:
    echo           [ERROR]  - Make sure you have Visual Studio installed.
    echo           [ERROR]  - Make sure you have the "x64 Visual C++ Toolset" in your path.
    echo           [ERROR]    For example using the "Visual Studio x64 Native Tools Command Prompt",
    echo           [ERROR]    or the "vcvarsall.bat".
    goto bad_exit

:good_exit
    endlocal
    exit /b 0

:bad_exit
    endlocal
    exit /b %errorlevel%
