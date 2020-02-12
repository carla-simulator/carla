@echo off
setlocal

rem BAT script that downloads and installs a ready to use
rem boost build for CARLA (carla.org).

set LOCAL_PATH=%~dp0
set "FILE_N=    -[%~n0]:"

rem Print batch params (debug purpose)
echo %FILE_N% [Batch params]: %*

rem ============================================================================
rem -- Parse arguments ---------------------------------------------------------
rem ============================================================================

:arg-parse
if not "%1"=="" (
    if "%1"=="-j" (
        set NUMBER_OF_ASYNC_JOBS=%~2
    )
    if "%1"=="--build-dir" (
        set BUILD_DIR=%~2
    )
    if "%1"=="--toolset" (
        set TOOLSET=%~2
    )
    if "%1"=="--version" (
        set BOOST_VERSION=%~2
    )
    if "%1"=="-v" (
        set BOOST_VERSION=%~2
    )
    if "%1"=="-h" (
        goto help
    )
    if "%1"=="--help" (
        goto help
    )
    shift
    goto :arg-parse
)

if [%BOOST_VERSION%] == [] (
    echo %FILE_N% You must specify a boost version using [-v^|--version]
    goto bad_exit
)

rem If not set set the build dir to the current dir
if [%BUILD_DIR%] == [] set BUILD_DIR=%~dp0

rem If not defined, use Visual Studio 2017 as tool set
if [%TOOLSET%] == [] set TOOLSET=msvc-14.1

rem If is not set, set the number of parallel jobs to the number of CPU threads
if [%NUMBER_OF_ASYNC_JOBS%] == [] set NUMBER_OF_ASYNC_JOBS=%NUMBER_OF_PROCESSORS%

rem ============================================================================
rem -- Local Variables ---------------------------------------------------------
rem ============================================================================

set BOOST_BASENAME=boost-%BOOST_VERSION%

set BOOST_TEMP_FOLDER=boost_%BOOST_VERSION:.=_%
set BOOST_TEMP_FILE=%BOOST_TEMP_FOLDER%.zip
set BOOST_TEMP_FILE_DIR=%BUILD_DIR%%BOOST_TEMP_FILE%

set BOOST_REPO=https://dl.bintray.com/boostorg/release/%BOOST_VERSION%/source/%BOOST_TEMP_FILE%
set BOOST_SRC_DIR=%BUILD_DIR%%BOOST_BASENAME%-source
set BOOST_INSTALL_DIR=%BUILD_DIR%%BOOST_BASENAME%-install
set BOOST_LIB_DIR=%BOOST_INSTALL_DIR%\lib

rem ============================================================================
rem -- Get Boost ---------------------------------------------------------------
rem ============================================================================

if exist "%BOOST_INSTALL_DIR%" (
    goto already_build
)

if not exist "%BOOST_SRC_DIR%" (
    if not exist "%BOOST_TEMP_FILE_DIR%" (
        echo %FILE_N% Retrieving boost.
        powershell -Command "Start-BitsTransfer -Source '%BOOST_REPO%' -Destination '%BOOST_TEMP_FILE_DIR%'"
    )
    if %errorlevel% neq 0 goto error_download
    echo %FILE_N% Extracting boost from "%BOOST_TEMP_FILE%", this can take a while...
    if exist "%ProgramW6432%/7-Zip/7z.exe" (
        "%ProgramW6432%/7-Zip/7z.exe" x "%BOOST_TEMP_FILE_DIR%" -o"%BUILD_DIR%" -y
    ) else (
        powershell -Command "Expand-Archive '%BOOST_TEMP_FILE_DIR%' -DestinationPath '%BUILD_DIR%' -Force"
    )
    echo %FILE_N% Removing "%BOOST_TEMP_FILE%"
    del "%BOOST_TEMP_FILE_DIR:/=\%"
    rename "%BUILD_DIR%%BOOST_TEMP_FOLDER%" "%BOOST_BASENAME%-source"
) else (
    echo %FILE_N% Not downloading boost because already exists the folder "%BOOST_SRC_DIR%".
)

cd "%BOOST_SRC_DIR%"
if not exist "b2.exe" (
    echo %FILE_N% Generating build...
    call bootstrap.bat
)

if %errorlevel% neq 0 goto error_bootstrap

rem This fix some kind of issue installing headers of boost < 1.67, not installing correctly
rem echo %FILE_N% Packing headers...
rem b2 headers link=static

echo %FILE_N% Building...
b2 -j%NUMBER_OF_ASYNC_JOBS%^
    headers^
    --layout=versioned^
    --build-dir=.\build^
    --with-system^
    --with-filesystem^
    --with-python^
    --with-date_time^
    architecture=x86^
    address-model=64^
    toolset=%TOOLSET%^
    variant=release^
    link=static^
    runtime-link=shared^
    threading=multi^
    --prefix="%BOOST_INSTALL_DIR%"^
    --libdir="%BOOST_LIB_DIR%"^
    --includedir="%BOOST_INSTALL_DIR%"^
    install
if %errorlevel% neq 0 goto error_install

for /d %%i in ("%BOOST_INSTALL_DIR%\boost*") do rename "%%i" include
goto success

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
rem ============================================================================

:help
    echo %FILE_N% Download and install a boost version.
    echo "Usage: %FILE_N% [-h^|--help] [-v^|--version] [--toolset] [--build-dir] [-j]"
    goto eof

:success
    echo.
    echo %FILE_N% Boost has been successfully installed in "%BOOST_INSTALL_DIR%"!
    goto good_exit

:already_build
    echo %FILE_N% A Boost installation already exists.
    echo %FILE_N% Delete "%BOOST_INSTALL_DIR%" if you want to force a rebuild.
    goto good_exit

:error_download
    echo.
    echo %FILE_N% [GIT ERROR] An error ocurred while downloading boost.
    echo %FILE_N% [GIT ERROR] Possible causes:
    echo %FILE_N%              - Make sure that the following url is valid:
    echo %FILE_N% "%BOOST_REPO%"
    echo %FILE_N% [GIT ERROR] Workaround:
    echo %FILE_N%              - Download the source code of boost "%BOOST_VERSION%" and
    echo %FILE_N%                extract the content of "%BOOST_TEMP_FOLDER%" in
    echo %FILE_N%                "%BOOST_SRC_DIR%"
    goto bad_exit

:error_bootstrap
    echo.
    echo %FILE_N% [BOOTSTRAP ERROR] An error ocurred while executing "bootstrap.bat".
    goto bad_exit

:error_install
    echo.
    echo %FILE_N% [B2 ERROR] An error ocurred while installing using "b2.exe".
    goto bad_exit

:good_exit
    echo %FILE_N% Exiting...
    endlocal
    rem A return value used for checking for errors
    set install_boost=done
    exit /b 0

:bad_exit
    if exist "%BOOST_INSTALL_DIR%" rd /s /q "%BOOST_INSTALL_DIR%"
    echo %FILE_N% Exiting with error...
    endlocal
    exit /b %errorlevel%
