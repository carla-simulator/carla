@echo off
setlocal

rem BAT script that creates the python package for the CARLA agents library.
rem Run it through a cmd with the x64 Visual C++ Toolset enabled.

set LOCAL_PATH=%~dp0
set FILE_N=-[%~n0]:

rem Print batch params (debug purpose)
echo %FILE_N% [Batch params]: %*

rem ============================================================================
rem -- Parse arguments ---------------------------------------------------------
rem ============================================================================

set DOC_STRING=Build and package CARLA Python API.
set "USAGE_STRING=Usage: %FILE_N% [-h^|--help] [--rebuild] [--clean]"

set REMOVE_INTERMEDIATE=false
set BUILD_AGENTS=true

:arg-parse
if not "%1"=="" (
    if "%1"=="--rebuild" (
        set REMOVE_INTERMEDIATE=true
        set BUILD_AGENTS=true
    )

    if "%1"=="--clean" (
        set REMOVE_INTERMEDIATE=true
        set BUILD_AGENTS=false
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

set PYTHON_LIB_PATH=%ROOT_PATH:/=\%PythonAPI\agents\

if %REMOVE_INTERMEDIATE% == false (
    if %BUILD_AGENTS% == false (
        goto :eof
    )
)

if %REMOVE_INTERMEDIATE% == true (
    rem Remove directories
    for %%G in (
        "%PYTHON_LIB_PATH%build",
        "%PYTHON_LIB_PATH%dist",
        "%PYTHON_LIB_PATH%source\carla_agents.egg-info"
    ) do (
        if exist %%G (
            echo %FILE_N% Cleaning %%G
            rmdir /s/q %%G
        )
    )
    if %BUILD_AGENTS% == false (
        goto good_exit
    )
)

cd "%PYTHON_LIB_PATH%"

rem ============================================================================
rem -- Check for py ------------------------------------------------------------
rem ============================================================================

where py 1>nul
if %errorlevel% neq 0 goto error_py

rem Build package
rem
if %BUILD_AGENTS%==true (
    echo Building Agents package.
    py -3 setup.py bdist_egg bdist_wheel
    if %errorlevel% neq 0 goto error_build_wheel
)

goto success

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
rem ============================================================================

:success
    echo.
    if %BUILD_AGENTS%==true echo %FILE_N% Carla agents package successfully build in "%PYTHON_LIB_PATH%dist"!
    goto good_exit

:error_py
    echo.
    echo %FILE_N% [ERROR] An error ocurred while executing the py.
    echo %FILE_N% [ERROR] Possible causes:
    echo %FILE_N% [ERROR]  - Make sure "py" is installed.
    echo %FILE_N% [ERROR]  - py = python launcher. This utility is bundled with Python installation but not installed by default.
    echo %FILE_N% [ERROR]  - Make sure it is available on your Windows "py".
    goto bad_exit

:error_build_wheel
    echo.
    echo %FILE_N% [ERROR] An error occurred while building the wheel file.
    goto bad_exit

:good_exit
    endlocal
    exit /b 0

:bad_exit
    endlocal
    exit /b %errorlevel%
