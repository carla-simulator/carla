@echo off
setlocal enabledelayedexpansion

rem BAT script that creates the client python api of LibCarla (carla.org).
rem Run it through a cmd with the x64 Visual C++ Toolset enabled.

set LOCAL_PATH=%~dp0
set FILE_N=-[%~n0]:

rem Print batch params (debug purpose)
echo %FILE_N% [Batch params]: %*

rem ============================================================================
rem -- Parse arguments ---------------------------------------------------------
rem ============================================================================

set DOC_STRING=Build and package CARLA Python API.
set "USAGE_STRING=Usage: %FILE_N% [-h^|--help] [--build-wheel] [--rebuild]  [--clean]"

set REMOVE_INTERMEDIATE=false
set BUILD_PYTHONAPI=true
set INSTALL_PYTHONAPI=true

:arg-parse
if not "%1"=="" (
    if "%1"=="--rebuild" (
        set REMOVE_INTERMEDIATE=true
        set BUILD_PYTHONAPI=true
        set INSTALL_PYTHONAPI=true
    )

    if "%1"=="--build-wheel" (
        set BUILD_PYTHONAPI=true
        set INSTALL_PYTHONAPI=false
    )

    if "%1"=="--clean" (
        set REMOVE_INTERMEDIATE=true
        set BUILD_PYTHONAPI=false
        set INSTALL_PYTHONAPI=false
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

set PYTHON_LIB_PATH=%ROOT_PATH:/=\%PythonAPI\carla\

if %REMOVE_INTERMEDIATE% == false (
    if %BUILD_PYTHONAPI% == false (
        echo Nothing selected to be done.
        goto :eof
    )
)

if %REMOVE_INTERMEDIATE% == true (
    rem Remove directories
    for %%G in (
        "%PYTHON_LIB_PATH%build",
        "%PYTHON_LIB_PATH%dist",
        "%PYTHON_LIB_PATH%source\carla.egg-info"
    ) do (
        if exist %%G (
            echo %FILE_N% Cleaning %%G
            rmdir /s/q %%G
        )
    )
    if %BUILD_PYTHONAPI% == false (
        goto good_exit
    )
)

cd "%PYTHON_LIB_PATH%"
rem if exist "%PYTHON_LIB_PATH%dist" goto already_installed

rem ============================================================================
rem -- Check for py ------------------------------------------------------------
rem ============================================================================

where python 1>nul
if %errorlevel% neq 0 goto error_py

rem Build for Python 3
rem
if %BUILD_PYTHONAPI%==true (
    echo Building Python API wheel for Python 3.
    python -m build --wheel --outdir dist\.tmp .

    set WHEEL_FILE=
    for %%f in (dist\.tmp\*.whl) do set WHEEL_FILE=%%f

    if %INSTALL_PYTHONAPI%==true (
        python -m pip install --force-reinstall "!WHEEL_FILE!"
    )

    copy "!WHEEL_FILE!" dist
    rmdir /s /q dist\.tmp

    if %errorlevel% neq 0 goto error_build_wheel
)

goto success

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
rem ============================================================================

:success
    echo.
    if %BUILD_PYTHONAPI%==true echo %FILE_N% Carla lib for python has been successfully installed in "%PYTHON_LIB_PATH%dist"!
    goto good_exit

:already_installed
    echo.
    echo %FILE_N% [ERROR] Already installed in "%PYTHON_LIB_PATH%dist"
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

