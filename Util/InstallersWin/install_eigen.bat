REM @echo off
setlocal

rem BAT script that downloads and installs a ready to use
rem x64 eigen for CARLA (carla.org).
rem Run it through a cmd with the x64 Visual C++ Toolset enabled.

set LOCAL_PATH=%~dp0
set FILE_N=    -[%~n0]:

rem Print batch params (debug purpose)
echo %FILE_N% [Batch params]: %*

rem ============================================================================
rem -- Parse arguments ---------------------------------------------------------
rem ============================================================================

:arg-parse
if not "%1"=="" (
    if "%1"=="--build-dir" (
        set BUILD_DIR=%~dpn2
        shift
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

rem If not set set the build dir to the current dir
if "%BUILD_DIR%" == "" set BUILD_DIR=%~dp0
if not "%BUILD_DIR:~-1%"=="\" set BUILD_DIR=%BUILD_DIR%\

rem ============================================================================
rem -- Get Eigen (CARLA dependency) -------------------------------------------
rem ============================================================================

set EIGEN_VERSION=3.3.7
set EIGEN_REPO=https://gitlab.com/libeigen/eigen/-/archive/%EIGEN_VERSION%/eigen-%EIGEN_VERSION%.zip
set EIGEN_BASENAME=eigen-%EIGEN_VERSION%

set EIGEN_SRC_DIR=%BUILD_DIR%%EIGEN_BASENAME%
set EIGEN_INSTALL_DIR=%BUILD_DIR%eigen-install
set EIGEN_INCLUDE=%EIGEN_INSTALL_DIR%\include
set EIGEN_TEMP_FILE=eigen-%EIGEN_VERSION%.zip
set EIGEN_TEMP_FILE_DIR=%BUILD_DIR%eigen-%EIGEN_VERSION%.zip

if not exist "%EIGEN_SRC_DIR%" (
    if not exist "%EIGEN_TEMP_FILE_DIR%" (
        echo %FILE_N% Retrieving %EIGEN_TEMP_FILE_DIR%.
        powershell -Command "(New-Object System.Net.WebClient).DownloadFile('%EIGEN_REPO%', '%EIGEN_TEMP_FILE_DIR%')"
    )
    if %errorlevel% neq 0 goto error_download_eigen
    rem Extract the downloaded library
    echo %FILE_N% Extracting eigen from "%EIGEN_TEMP_FILE%".
    powershell -Command "Expand-Archive '%EIGEN_TEMP_FILE_DIR%' -DestinationPath '%BUILD_DIR%'"
    if %errorlevel% neq 0 goto error_extracting
    echo %EIGEN_SRC_DIR%

    del %EIGEN_TEMP_FILE_DIR%

)

if not exist "%EIGEN_INSTALL_DIR%" (
    mkdir %EIGEN_INSTALL_DIR%
    mkdir %EIGEN_INCLUDE%
    mkdir %EIGEN_INCLUDE%\unsupported
    mkdir %EIGEN_INCLUDE%\Eigen
)

xcopy /q /Y /S /I /d "%EIGEN_SRC_DIR%\Eigen" "%EIGEN_INCLUDE%\Eigen"
xcopy /q /Y /S /I /d "%EIGEN_SRC_DIR%\unsupported\Eigen" "%EIGEN_INCLUDE%\unsupported\Eigen"
copy "%BUILD_DIR%..\Util\Patches\Eigen3.3.7\Macros.h" "%EIGEN_INCLUDE%\Eigen\src\Core\util\Macros.h"
copy "%BUILD_DIR%..\Util\Patches\Eigen3.3.7\VectorBlock.h" "%EIGEN_INCLUDE%\Eigen\src\Core\VectorBlock.h"

goto success

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
rem ============================================================================

:help
    echo %FILE_N% Download and install the Eigen library.
    echo "Usage: %FILE_N% [-h^|--help] [--build-dir]"
    goto eof

:success
    echo.
    echo %FILE_N% Eigen has been successfully installed in "%EIGEN_INSTALL_DIR%"!
    goto good_exit

:already_build
    echo %FILE_N% An Eigen installation already exists.
    echo %FILE_N% Delete "%EIGEN_INSTALL_DIR%" if you want to force a rebuild.
    goto good_exit

:error_download_eigen
    echo.
    echo %FILE_N% [DOWNLOAD ERROR] An error ocurred while downloading Eigen.
    echo %FILE_N% [DOWNLOAD ERROR] Possible causes:
    echo %FILE_N%              - Make sure that the following url is valid:
    echo %FILE_N% "%EIGEN_REPO%"
    echo %FILE_N% [DOWNLOAD ERROR] Workaround:
    echo %FILE_N%              - Download the Eigen source code and
    echo %FILE_N%                extract the content in
    echo %FILE_N%                "%EIGEN_SRC_DIR%"
    echo %FILE_N%                And re-run the setup script.
    goto bad_exit


:error_extracting
    echo.
    echo %FILE_N% [EXTRACTING ERROR] An error ocurred while extracting the zip.
    echo %FILE_N% [EXTRACTING ERROR] Workaround:
    echo %FILE_N%              - Download the Eigen source code and
    echo %FILE_N%                extract the content manually in
    echo %FILE_N%                "%EIGEN_SRC_DIR%"
    echo %FILE_N%                And re-run the setup script.
    goto bad_exit


:good_exit
    echo %FILE_N% Exiting...
    rem A return value used for checking for errors
    endlocal & set install_eigen=%EIGEN_INSTALL_DIR%
    exit /b 0

:bad_exit
    if exist "%EIGEN_INSTALL_DIR%" rd /s /q "%EIGEN_INSTALL_DIR%"
    echo %FILE_N% Exiting with error...
    endlocal
    exit /b %errorlevel%
