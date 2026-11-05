@echo off
setlocal EnableDelayedExpansion

rem ==============================================================================
rem -- Set up environment --------------------------------------------------------
rem ==============================================================================

for %%I in ("%~dp0..\..") do set "CARLA_ROOT=%%~fI"

set "WHEEL_DIR=%CARLA_ROOT%\Build\Release\PythonAPI\dist"
set "RESULTS_DIR=%CARLA_ROOT%\Build\test-results"
set PORT=3654
set CONNECT_TIMEOUT=60
set PYTHONS=
set FAILED=

rem ==============================================================================
rem -- Parse arguments -----------------------------------------------------------
rem ==============================================================================

set DOC_STRING=Run the smoke tests (PythonAPI\test\smoke_test_list.txt) once per Python interpreter given, each with the wheel built for its version, against a CARLA server already listening on port %PORT%. Writes the JUnit results to RESULTS_DIR\smoke-cpXY.xml and fails if any version fails.

set USAGE_STRING=Usage: %~nx0 [-h^|--help] [--wheel-dir DIR] [--results-dir DIR] [--connect-timeout SECONDS] PYTHON [PYTHON...]

:arg-parse
if not "%~1"=="" (
    if "%~1"=="--wheel-dir" (
        set "WHEEL_DIR=%~2"
        shift
    ) else if "%~1"=="--results-dir" (
        set "RESULTS_DIR=%~2"
        shift
    ) else if "%~1"=="--connect-timeout" (
        set "CONNECT_TIMEOUT=%~2"
        shift
    ) else if "%~1"=="--help" (
        echo !DOC_STRING!
        echo !USAGE_STRING!
        goto :bad_exit
    ) else if "%~1"=="-h" (
        echo !DOC_STRING!
        echo !USAGE_STRING!
        goto :bad_exit
    ) else (
        set PYTHONS=!PYTHONS! "%~1"
    )

    shift
    goto :arg-parse
)

if "%PYTHONS%"=="" (
    echo !USAGE_STRING!
    goto :bad_exit
)

for %%I in ("%WHEEL_DIR%") do set "WHEEL_DIR=%%~fI"
if not exist "%RESULTS_DIR%" mkdir "%RESULTS_DIR%"
for %%I in ("%RESULTS_DIR%") do set "RESULTS_DIR=%%~fI"

rem ==============================================================================
rem -- Run the smoke tests -------------------------------------------------------
rem ==============================================================================

pushd "%CARLA_ROOT%\PythonAPI\test" || goto :bad_exit

set /p SMOKE_TESTS=<smoke_test_list.txt

for %%P in (%PYTHONS%) do call :run_smoke_tests "%%~P"

popd

if not "%FAILED%"=="" (
    echo Smoke tests failed for:%FAILED%
    goto :bad_exit
)

echo Smoke tests passed for every Python version.
goto :good_exit

rem ==============================================================================
rem -- Run the smoke tests with one interpreter ----------------------------------
rem ==============================================================================

:run_smoke_tests
set "PYTHON=%~1"
set TAG=
rem "call" keeps cmd from stripping the quotes around the interpreter path.
for /f "usebackq delims=" %%T in (`call "%PYTHON%" -c "import sys; print('cp' + ''.join(map(str, sys.version_info[:2])))"`) do set "TAG=%%T"
if "%TAG%"=="" (
    echo Cannot run !PYTHON!.
    set "FAILED=!FAILED! %PYTHON%"
    goto :eof
)

set WHEEL_COUNT=0
set WHEEL=
for %%W in ("%WHEEL_DIR%\carla-*-%TAG%-%TAG%-*.whl") do (
    set /a WHEEL_COUNT+=1
    set "WHEEL=%%~fW"
)
if not "%WHEEL_COUNT%"=="1" (
    echo Expected exactly one !TAG! wheel in !WHEEL_DIR!, found !WHEEL_COUNT!.
    set "FAILED=!FAILED! %TAG%"
    goto :eof
)

for %%W in ("%WHEEL%") do echo Running smoke tests for %TAG% ^(!PYTHON!^) with %%~nxW.

if exist test-results.xml del /q test-results.xml
set RESULT=0
"%PYTHON%" -m pip install --quiet --force-reinstall --no-deps "%WHEEL%" || set RESULT=1
if "%RESULT%"=="0" "%PYTHON%" -m pip install --quiet -r ..\carla\requirements.txt -r requirements.txt || set RESULT=1
if "%RESULT%"=="0" "%PYTHON%" test_connection.py -p %PORT% --timeout=%CONNECT_TIMEOUT% || set RESULT=1
if "%RESULT%"=="0" "%PYTHON%" -m nose2 -v -c smoke\unittest.cfg -X %SMOKE_TESTS% || set RESULT=1

if exist test-results.xml move /y test-results.xml "%RESULTS_DIR%\smoke-%TAG%.xml" >nul
if not "%RESULT%"=="0" set "FAILED=!FAILED! %TAG%"
goto :eof

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
rem ============================================================================

:good_exit
    endlocal
    exit /b 0

:bad_exit
    endlocal
    exit /b 1
