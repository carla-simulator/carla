REM @echo off
setlocal

rem BAT script that downloads and installs a ready to use
rem x64 xerces-c build for CARLA (carla.org).
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
rem -- Local Variables ---------------------------------------------------------
rem ============================================================================

set SQLITE_BASE_NAME=sqlite-amalgamation-3340100
set SQLITE_ZIP=%SQLITE_BASE_NAME%.zip
set SQLITE_ZIP_DIR=%BUILD_DIR%%SQLITE_ZIP%
set SQLITE_REPO=https://www.sqlite.org/2021/%SQLITE_ZIP%

set SQLITE_SRC_DIR=%BUILD_DIR%sqlite3-src
set SQLITE_INSTALL_DIR=%BUILD_DIR%sqlite3-install

set SQLITE_INCLUDE_DIR=%SQLITE_INSTALL_DIR%\include
set SQLITE_BIN_DIR=%SQLITE_INSTALL_DIR%\bin
set SQLITE_LIB_DIR=%SQLITE_INSTALL_DIR%\lib

if exist "%SQLITE_INSTALL_DIR%" (
    goto already_build
)

echo %FILE_N% Retrieving %SQLITE_BASE_NAME%.
powershell -Command "(New-Object System.Net.WebClient).DownloadFile('%SQLITE_REPO%', '%SQLITE_ZIP_DIR%')"
if %errorlevel% neq 0 goto error_download

echo %FILE_N% Extracting Sqlite3 from "%SQLITE_ZIP%".
powershell -Command "Expand-Archive '%SQLITE_ZIP_DIR%' -DestinationPath '%BUILD_DIR%'"
move %BUILD_DIR%%SQLITE_BASE_NAME% %SQLITE_SRC_DIR%

cd %SQLITE_SRC_DIR%

call cl /MD /MP sqlite3.c shell.c -Fesqlite.exe
call cl /MD /MP /c /EHsc sqlite3.c
call lib sqlite3.obj

mkdir %SQLITE_INCLUDE_DIR%
mkdir %SQLITE_BIN_DIR%
mkdir %SQLITE_LIB_DIR%

move sqlite.exe %SQLITE_BIN_DIR%
move sqlite3.h %SQLITE_INCLUDE_DIR%
move sqlite3.lib %SQLITE_LIB_DIR%

del %SQLITE_ZIP_DIR%

goto success

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
rem ============================================================================

:help
    echo %FILE_N% Download and install a Sqlite3.
    echo "Usage: %FILE_N% [-h^|--help] [--build-dir] [--zlib-install-dir]"
    goto eof

:success
    echo.
    echo %FILE_N% Sqlite3 has been successfully installed in "%SQLITE_INSTALL_DIR%"!
    goto good_exit

:already_build
    echo %FILE_N% A Sqlite3 installation already exists.
    echo %FILE_N% Delete "%SQLITE_INSTALL_DIR%" if you want to force a rebuild.
    goto good_exit

:error_download
    echo.
    echo %FILE_N% [DOWNLOAD ERROR] An error ocurred while downloading Sqlite3.
    echo %FILE_N% [DOWNLOAD ERROR] Possible causes:
    echo %FILE_N%              - Make sure that the following url is valid:
    echo %FILE_N% "%SQLITE_REPO%"
    echo %FILE_N% [DOWNLOAD ERROR] Workaround:
    echo %FILE_N%              - Download the Sqlite3's source code and
    echo %FILE_N%                extract the content in
    echo %FILE_N%                "%SQLITE_SRC_DIR%"
    echo %FILE_N%                And re-run the setup script.
    goto bad_exit

:error_extracting
    echo.
    echo %FILE_N% [EXTRACTING ERROR] An error ocurred while extracting the zip.
    echo %FILE_N% [EXTRACTING ERROR] Workaround:
    echo %FILE_N%              - Download the Sqlite3's source code and
    echo %FILE_N%                extract the content manually in
    echo %FILE_N%                "%SQLITE_SRC_DIR%"
    echo %FILE_N%                And re-run the setup script.
    goto bad_exit

:error_compiling
    echo.
    echo %FILE_N% [COMPILING ERROR] An error ocurred while compiling with cl.exe.
    echo %FILE_N%              Possible causes:
    echo %FILE_N%              - Make sure you have Visual Studio installed.
    echo %FILE_N%              - Make sure you have the "x64 Visual C++ Toolset" in your path.
    echo %FILE_N%                For example, using the "Visual Studio x64 Native Tools Command Prompt",
    echo %FILE_N%                or the "vcvarsall.bat".
    goto bad_exit

:error_generating_lib
    echo.
    echo %FILE_N% [NMAKE ERROR] An error ocurred while compiling and installing using nmake.
    goto bad_exit

:good_exit
    echo %FILE_N% Exiting...
    rem A return value used for checking for errors
    endlocal & set install_Sqlite3=%SQLITE_INSTALL_DIR%
    exit /b 0

:bad_exit
    if exist "%SQLITE_INSTALL_DIR%" rd /s /q "%SQLITE_INSTALL_DIR%"
    echo %FILE_N% Exiting with error...
    endlocal
    exit /b %errorlevel%
