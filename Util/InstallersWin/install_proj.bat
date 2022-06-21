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

set PROJ_BASE_NAME=proj-7.2.1
set PROJ_ZIP=%PROJ_BASE_NAME%.tar.gz
set PROJ_ZIP_DIR=%BUILD_DIR%%PROJ_ZIP%
set PROJ_REPO=https://download.osgeo.org/proj/%PROJ_ZIP%

set SQLITE_INSTALL_DIR=%BUILD_DIR%sqlite3-install
set SQLITE_INCLUDE_DIR=%SQLITE_INSTALL_DIR%\include
set SQLITE_BIN=%SQLITE_INSTALL_DIR%\bin\sqlite.exe
set SQLITE_LIB=%SQLITE_INSTALL_DIR%\lib\sqlite3.lib

set PROJ_SRC_DIR=%BUILD_DIR%proj-src
set PROJ_BUILD_DIR=%PROJ_SRC_DIR%\build
set PROJ_INSTALL_DIR=%BUILD_DIR%proj-install

set PROJ_INCLUDE_DIR=%PROJ_INSTALL_DIR%\include
set PROJ_BIN_DIR=%PROJ_INSTALL_DIR%\bin
set PROJ_LIB_DIR=%PROJ_INSTALL_DIR%\lib

if exist "%PROJ_INSTALL_DIR%" (
    goto already_build
)

echo %FILE_N% Retrieving %PROJ_BASE_NAME%.
powershell -Command "(New-Object System.Net.WebClient).DownloadFile('%PROJ_REPO%', '%PROJ_ZIP_DIR%')"
if %errorlevel% neq 0 goto error_download

cd %BUILD_DIR%
echo %FILE_N% Extracting PROJ from "%PROJ_ZIP%".
call tar -xzf %PROJ_ZIP%
move %BUILD_DIR%%PROJ_BASE_NAME% %PROJ_SRC_DIR%

mkdir %PROJ_BUILD_DIR%
cd %PROJ_BUILD_DIR%

cmake .. -G "Visual Studio 16 2019" -A x64^
    -DCMAKE_CXX_FLAGS_RELEASE="/MD /MP"^
    -DCMAKE_CXX_FLAGS="/MD /MP"^
    -DSQLITE3_INCLUDE_DIR=%SQLITE_INCLUDE_DIR% -DSQLITE3_LIBRARY=%SQLITE_LIB%^
    -DEXE_SQLITE3=%SQLITE_BIN%^
    -DENABLE_TIFF=OFF -DENABLE_CURL=OFF -DBUILD_SHARED_LIBS=OFF -DBUILD_PROJSYNC=OFF^
    -DCMAKE_BUILD_TYPE=Release -DBUILD_PROJINFO=OFF^
    -DBUILD_CCT=OFF -DBUILD_CS2CS=OFF -DBUILD_GEOD=OFF -DBUILD_GIE=OFF^
    -DBUILD_PROJ=OFF -DBUILD_TESTING=OFF^
    -DCMAKE_INSTALL_PREFIX=%PROJ_INSTALL_DIR%
if %errorlevel% neq 0 goto error_cmake

cmake --build . --config Release --target install 

del %PROJ_ZIP_DIR%

goto success

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
rem ============================================================================

:help
    echo %FILE_N% Download and install a PROJ.
    echo "Usage: %FILE_N% [-h^|--help] [--build-dir] [--zlib-install-dir]"
    goto eof

:success
    echo.
    echo %FILE_N% PROJ has been successfully installed in "%PROJ_INSTALL_DIR%"!
    goto good_exit

:already_build
    echo %FILE_N% A PROJ installation already exists.
    echo %FILE_N% Delete "%PROJ_INSTALL_DIR%" if you want to force a rebuild.
    goto good_exit

:error_download
    echo.
    echo %FILE_N% [DOWNLOAD ERROR] An error ocurred while downloading PROJ.
    echo %FILE_N% [DOWNLOAD ERROR] Possible causes:
    echo %FILE_N%              - Make sure that the following url is valid:
    echo %FILE_N% "%PROJ_REPO%"
    echo %FILE_N% [DOWNLOAD ERROR] Workaround:
    echo %FILE_N%              - Download the PROJ's source code and
    echo %FILE_N%                extract the content in
    echo %FILE_N%                "%PROJ_SRC_DIR%"
    echo %FILE_N%                And re-run the setup script.
    goto bad_exit

:error_extracting
    echo.
    echo %FILE_N% [EXTRACTING ERROR] An error ocurred while extracting the zip.
    echo %FILE_N% [EXTRACTING ERROR] Workaround:
    echo %FILE_N%              - Download the PROJ's source code and
    echo %FILE_N%                extract the content manually in
    echo %FILE_N%                "%PROJ_SRC_DIR%"
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
    endlocal & set install_proj=%PROJ_INSTALL_DIR%
    exit /b 0

:bad_exit
    if exist "%PROJ_INSTALL_DIR%" rd /s /q "%PROJ_INSTALL_DIR%"
    echo %FILE_N% Exiting with error...
    endlocal
    exit /b %errorlevel%
