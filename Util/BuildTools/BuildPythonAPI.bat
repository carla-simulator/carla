@echo off

setlocal

set LOCAL_PATH=%~dp0
set "FILE_N=    -[%~n0]:"

set DOC_STRING=Build and package CARLA Python API.
set "USAGE_STRING=Usage: %FILE_N% [-h^|--help] [--rebuild]  [--clean]"

set REMOVE_INTERMEDIATE=false
set BUILD_FOR_PYTHON2=false
set BUILD_FOR_PYTHON3=false

:arg-parse
if not "%1"=="" (
    if "%1"=="--rebuild" (
        set REMOVE_INTERMEDIATE=true
        set BUILD_FOR_PYTHON2=true
        set BUILD_FOR_PYTHON3=true
    )

    if "%1"=="--py2" (
        set BUILD_FOR_PYTHON2=true
    )

    if "%1"=="--py3" (
        set BUILD_FOR_PYTHON3=true
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

set PYTHON_LIB_PATH=%ROOT_PATH%PythonAPI\
set PYTHON_LIB_BUILD=%PYTHON_LIB_PATH%build
set PYTHON_LIB_DEPENDENCIES=%PYTHON_LIB_PATH%dependencies

if %REMOVE_INTERMEDIATE% == false (
    if %BUILD_FOR_PYTHON3% == false (
        if %BUILD_FOR_PYTHON2% == false (
          echo Nothing selected to be done.
          goto :eof
        )
    )
)

cd "%PYTHON_LIB_PATH%"

if %REMOVE_INTERMEDIATE% == true (
    echo %FILE_N% cleaning build folder
    if exist "%PYTHON_LIB_BUILD%" rmdir /S /Q "%PYTHON_LIB_BUILD%"
    if exist "%PYTHON_LIB_DEPENDENCIES%" rmdir /S /Q "%PYTHON_LIB_DEPENDENCIES%"
)

if %BUILD_FOR_PYTHON2%==true (
    echo Building Python API for Python 2.
    call py -2 setup.py bdist_egg
)

if %BUILD_FOR_PYTHON3%==true (
    echo Building Python API for Python 3.
    call py -3 setup.py bdist_egg
)

