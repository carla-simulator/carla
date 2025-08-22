REM @echo off
setlocal

rem BAT script that downloads and installs a ready to use
rem x64 chrono for CARLA (carla.org).
rem Run it through a cmd with the x64 Visual C++ Toolset enabled.

set LOCAL_PATH=%~dp0
set FILE_N=    -[%~n0]:

rem Print batch params (debug purpose)
echo %FILE_N% [Batch params]: %*

rem ============================================================================
rem -- Parse arguments ---------------------------------------------------------
rem ============================================================================

set CHRONO_PATH=

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
    if "%1"=="--generator" (
        set GENERATOR=%2
        shift
    )
    if "%1"=="--chrono-path" (
        set CHRONO_PATH=%2
        shift
    )
    shift
    goto :arg-parse
)

rem If not set set the build dir to the current dir
if "%BUILD_DIR%" == "" set BUILD_DIR=%~dp0
if not "%BUILD_DIR:~-1%"=="\" set BUILD_DIR=%BUILD_DIR%\
if %GENERATOR% == "" set GENERATOR="Visual Studio 17 2022"

rem ============================================================================
rem -- Get/Use Eigen (Chrono dependency) ---------------------------------------
rem ============================================================================

set EIGEN_INSTALL_DIR=%BUILD_DIR%eigen-install
set EIGEN_INCLUDE=%EIGEN_INSTALL_DIR%\include

if not exist "%EIGEN_INSTALL_DIR%" (
    echo %FILE_N% Eigen not found, installing via install_eigen.bat...
    call "%LOCAL_PATH%install_eigen.bat" --build-dir "%BUILD_DIR%"
    if %errorlevel% neq 0 (
        echo %FILE_N% [ERROR] Failed to install Eigen.
        goto bad_exit
    )
)

if not exist "%EIGEN_INCLUDE%" (
    echo %FILE_N% [ERROR] Eigen include directory not found at: %EIGEN_INCLUDE%
    goto bad_exit
)

for %%i in ("%EIGEN_INCLUDE%") do set EIGEN_INCLUDE_ABS=%%~fi

rem ============================================================================
rem -- Get Chrono -------------------------------------------
rem ============================================================================

set CHRONO_VERSION=6.0.0
@REM set CHRONO_VERSION=develop
set CHRONO_REPO=https://github.com/projectchrono/chrono.git
set CHRONO_BASENAME=chrono

set CHRONO_INSTALL_DIR=%BUILD_DIR%chrono-install

if not "%CHRONO_PATH%"=="" (
    echo %FILE_N% Using user-provided Chrono at: %CHRONO_PATH%
    set CHRONO_SRC_DIR=%CHRONO_PATH%
) else (
    set CHRONO_SRC_DIR=%BUILD_DIR%%CHRONO_BASENAME%-src
)

set CHRONO_BUILD_DIR=%CHRONO_SRC_DIR%\build

if not exist %CHRONO_INSTALL_DIR% (
    if "%CHRONO_PATH%"=="" (
        echo %FILE_N% Retrieving Chrono.
        call git clone --depth 1 --branch %CHRONO_VERSION% %CHRONO_REPO% %CHRONO_SRC_DIR%
    ) else (
        echo %FILE_N% Using existing Chrono source at: %CHRONO_PATH%
    )

    mkdir %CHRONO_BUILD_DIR%
    mkdir %CHRONO_INSTALL_DIR%

    cd "%CHRONO_BUILD_DIR%"

    echo.%GENERATOR% | findstr /C:"Visual Studio" >nul && (
        set PLATFORM=-A x64
    ) || (
        set PLATFORM=
    )

    echo %FILE_N% Compiling Chrono.
    cmake -G %GENERATOR% %PLATFORM%^
        -DCMAKE_BUILD_TYPE=Release^
        -DCMAKE_CXX_FLAGS_RELEASE="/MD /MP /O2 /Ob2 /DNDEBUG"^
        -DEIGEN3_INCLUDE_DIR="%EIGEN_INCLUDE_ABS%"^
        -DCMAKE_INSTALL_PREFIX="%CHRONO_INSTALL_DIR%"^
        -DENABLE_MODULE_VEHICLE=ON^
        %CHRONO_SRC_DIR%

    echo %FILE_N% Building...
    cmake --build . --config Release --target install --parallel %NUMBER_OF_PROCESSORS%

)

goto success

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
rem ============================================================================

:help
    echo %FILE_N% Download and install a the Chrono library.
    echo "Usage: %FILE_N% [-h^|--help] [--build-dir] [--zlib-install-dir] [--chrono-path]"
    goto eof

:success
    echo.
    echo %FILE_N% Chrono has been successfully installed in "%CHRONO_INSTALL_DIR%"!
    goto good_exit

:already_build
    echo %FILE_N% A chrono installation already exists.
    echo %FILE_N% Delete "%CHRONO_INSTALL_DIR%" if you want to force a rebuild.
    goto good_exit

:error_download_eigen
    echo.
    echo %FILE_N% [DOWNLOAD ERROR] An error ocurred while downloading xerces.
    echo %FILE_N% [DOWNLOAD ERROR] Possible causes:
    echo %FILE_N%              - Make sure that the following url is valid:
    echo %FILE_N% "%EIGEN_REPO%"
    echo %FILE_N% [DOWNLOAD ERROR] Workaround:
    echo %FILE_N%              - Download the xerces's source code and
    echo %FILE_N%                extract the content in
    echo %FILE_N%                "%EIGEN_SRC_DIR%"
    echo %FILE_N%                And re-run the setup script.
    goto bad_exit

:error_download_chrono
    echo.
    echo %FILE_N% [DOWNLOAD ERROR] An error ocurred while downloading xerces.
    echo %FILE_N% [DOWNLOAD ERROR] Possible causes:
    echo %FILE_N%              - Make sure that the following url is valid:
    echo %FILE_N% "%XERCESC_REPO%"
    echo %FILE_N% [DOWNLOAD ERROR] Workaround:
    echo %FILE_N%              - Download the xerces's source code and
    echo %FILE_N%                extract the content in
    echo %FILE_N%                "%EIGEN_SRC_DIR%"
    echo %FILE_N%                And re-run the setup script.
    goto bad_exit

:error_extracting
    echo.
    echo %FILE_N% [EXTRACTING ERROR] An error ocurred while extracting the zip.
    echo %FILE_N% [EXTRACTING ERROR] Workaround:
    echo %FILE_N%              - Download the xerces's source code and
    echo %FILE_N%                extract the content manually in
    echo %FILE_N%                "%EIGEN_SRC_DIR%"
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
    endlocal & set install_chrono=%CHRONO_INSTALL_DIR%
    exit /b 0

:bad_exit
    if exist "%CHRONO_INSTALL_DIR%" rd /s /q "%CHRONO_INSTALL_DIR%"
    echo %FILE_N% Exiting with error...
    endlocal
    exit /b %errorlevel%
