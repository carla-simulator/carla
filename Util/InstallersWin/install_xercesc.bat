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

set XERCESC_BASENAME=xerces-c
set XERCESC_VERSION=3.2.3

rem xerces-c-x.x.x
set XERCESC_TEMP_FOLDER=%XERCESC_BASENAME%-%XERCESC_VERSION%
rem ../xerces-c-x.x.x
set XERCESC_TEMP_FOLDER_DIR=%BUILD_DIR%%XERCESC_TEMP_FOLDER%
rem xerces-c-x.x.x-src.zip
set XERCESC_TEMP_FILE=%XERCESC_TEMP_FOLDER%-src.zip
rem ../xerces-c-x.x.x-src.zip
set XERCESC_TEMP_FILE_DIR=%BUILD_DIR%%XERCESC_TEMP_FILE%

set XERCESC_REPO=https://archive.apache.org/dist/xerces/c/3/sources/xerces-c-%XERCESC_VERSION%.zip
set XERCESC_BACKUP_REPO=https://carla-releases.s3.eu-west-3.amazonaws.com/Backup/xerces-c-%XERCESC_VERSION%.zip

rem ../xerces-c-x.x.x-source/
set XERCESC_SRC_DIR=%BUILD_DIR%%XERCESC_BASENAME%-%XERCESC_VERSION%-source\
rem ../xerces-c-x.x.x-install/
set XERCESC_INSTALL_DIR=%BUILD_DIR%%XERCESC_BASENAME%-%XERCESC_VERSION%-install\

rem ============================================================================
rem -- Get xerces --------------------------------------------------------------
rem ============================================================================

if exist "%XERCESC_INSTALL_DIR%" (
    goto already_build
)

if not exist "%XERCESC_SRC_DIR%" (
    if not exist "%XERCESC_TEMP_FILE_DIR%" (
        echo %FILE_N% Retrieving %XERCESC_BASENAME%.
        powershell -Command "(New-Object System.Net.WebClient).DownloadFile('%XERCESC_REPO%', '%XERCESC_TEMP_FILE_DIR%')"
    )
    if not exist "%XERCESC_TEMP_FILE_DIR%" (
        echo %FILE_N% Using %XERCESC_BASENAME% from backup.
        powershell -Command "(New-Object System.Net.WebClient).DownloadFile('%XERCESC_BACKUP_REPO%', '%XERCESC_TEMP_FILE_DIR%')"
    )
    if %errorlevel% neq 0 goto error_download
    rem Extract the downloaded library
    echo %FILE_N% Extracting xerces from "%XERCESC_TEMP_FILE%".
    powershell -Command "Expand-Archive '%XERCESC_TEMP_FILE_DIR%' -DestinationPath '%BUILD_DIR%'"
    if %errorlevel% neq 0 goto error_extracting

    rem Remove unnecessary files and folders
    echo %FILE_N% Removing "%XERCESC_TEMP_FILE%"
    del "%XERCESC_TEMP_FILE_DIR%"
    echo %FILE_N% Removing dir "%BUILD_DIR%manifest"
    rmdir /s/q "%BUILD_DIR%manifest"
    
    echo %FILE_N% Renaming dir %XERCESC_TEMP_FOLDER_DIR% to %XERCESC_BASENAME%-%XERCESC_VERSION%-source
    rename "%XERCESC_TEMP_FOLDER_DIR%" "%XERCESC_BASENAME%-%XERCESC_VERSION%-source"
) else (
    echo %FILE_N% Not downloading xerces because already exists the folder "%XERCESC_SRC_DIR%".
)

rem ============================================================================
rem -- Compile xerces ----------------------------------------------------------
rem ============================================================================

if not exist "%XERCESC_SRC_DIR%build" (
    echo %FILE_N% Creating "%XERCESC_SRC_DIR%build"
    mkdir "%XERCESC_SRC_DIR%build"
)

cd "%XERCESC_SRC_DIR%build"

if not exist "%XERCESC_INSTALL_DIR%lib" (
    echo %FILE_N% Creating "%XERCESC_INSTALL_DIR%lib"
    mkdir "%XERCESC_INSTALL_DIR%lib"
)

if not exist "%XERCESC_INSTALL_DIR%include" (
    echo %FILE_N% Creating "%XERCESC_INSTALL_DIR%include"
    mkdir "%XERCESC_INSTALL_DIR%include"
)

cmake .. -G "Visual Studio 16 2019" -A x64^
  -DCMAKE_INSTALL_PREFIX="%XERCESC_INSTALL_DIR:\=/%"^
  -DBUILD_SHARED_LIBS=OFF^
  "%BUILD_DIR%%XERCESC_BASENAME%-%XERCESC_VERSION%-source"
if %errorlevel% neq 0 goto error_cmake

cmake --build . --config Release --target install 

goto success

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
rem ============================================================================

:help
    echo %FILE_N% Download and install a xerces.
    echo "Usage: %FILE_N% [-h^|--help] [--build-dir] [--zlib-install-dir]"
    goto eof

:success
    echo.
    echo %FILE_N% xerces has been successfully installed in "%XERCESC_INSTALL_DIR%"!
    goto good_exit

:already_build
    echo %FILE_N% A xerces installation already exists.
    echo %FILE_N% Delete "%XERCESC_INSTALL_DIR%" if you want to force a rebuild.
    goto good_exit

:error_download
    echo.
    echo %FILE_N% [DOWNLOAD ERROR] An error ocurred while downloading xerces.
    echo %FILE_N% [DOWNLOAD ERROR] Possible causes:
    echo %FILE_N%              - Make sure that the following url is valid:
    echo %FILE_N% "%XERCESC_REPO%"
    echo %FILE_N% [DOWNLOAD ERROR] Workaround:
    echo %FILE_N%              - Download the xerces's source code and
    echo %FILE_N%                extract the content in
    echo %FILE_N%                "%XERCESC_SRC_DIR%"
    echo %FILE_N%                And re-run the setup script.
    goto bad_exit

:error_extracting
    echo.
    echo %FILE_N% [EXTRACTING ERROR] An error ocurred while extracting the zip.
    echo %FILE_N% [EXTRACTING ERROR] Workaround:
    echo %FILE_N%              - Download the xerces's source code and
    echo %FILE_N%                extract the content manually in
    echo %FILE_N%                "%XERCESC_SRC_DIR%"
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
    endlocal & set install_xerces=%XERCESC_INSTALL_DIR%
    exit /b 0

:bad_exit
    if exist "%XERCESC_INSTALL_DIR%" rd /s /q "%XERCESC_INSTALL_DIR%"
    echo %FILE_N% Exiting with error...
    endlocal
    exit /b %errorlevel%
