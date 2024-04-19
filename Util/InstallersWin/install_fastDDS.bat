@echo off
setlocal

rem BAT script that downloads and installs Fast-DDS library
rem Run it through a cmd with the x64 Visual C++ Toolset enabled.

set LOCAL_PATH=%~dp0
set FILE_N=    -[%~n0]:

rem Print batch params (debug purpose)
echo %FILE_N% [Batch params]: %*

rem ============================================================================
rem -- Parse arguments ---------------------------------------------------------
rem ============================================================================

set DEL_SRC=false

:arg-parse
if not "%1"=="" (
    if "%1"=="--build-dir" (
        set BUILD_DIR=%~dpn2
        shift
    )

    if "%1"=="--delete-src" (
        set DEL_SRC=true
    )

    shift
    goto :arg-parse
)

rem If not set set the build dir to the current dir
if "%BUILD_DIR%" == "" set BUILD_DIR=%~dp0
if not "%BUILD_DIR:~-1%"=="\" set BUILD_DIR=%BUILD_DIR%\

set FASTDDS_SRC=fastDDS-src
set FASTDDS_SRC_DIR=%BUILD_DIR%%FASTDDS_SRC%\
set FASTDDS_INSTALL=fastDDS-install
set FASTDDS_INSTALL_DIR=%BUILD_DIR%%FASTDDS_INSTALL%\
set FASTDDS_BUILD_DIR=%FASTDDS_SRC_DIR%build\
set FASTDDS_BASENAME=%FASTDDS_SRC%

if exist "%FASTDDS_INSTALL_DIR%" (
    goto already_build
)

if not exist "%FASTDDS_SRC_DIR%" (
    echo %FILE_N% Cloning "Fast-DDS"

    call git clone https://github.com/eProsima/Fast-DDS.git "%FASTDDS_SRC_DIR:~0,-1%"
    call git submodule init
    call git submodule update
    if %errorlevel% neq 0 goto error_git
) else (
    echo %FILE_N% Not cloning "Fast-DDS" because already exists a folder called "%FASTDDS_SRC%".
)

echo Compiling fastCDR dependency...

if not exist "%FASTDDS_SRC_DIR%/thirdparty/fastcdr/build" (
    echo %FILE_N% Creating "%FASTDDS_SRC_DIR%/thirdparty/fastcdr/build"
    cd "%FASTDDS_SRC_DIR%/thirdparty/fastcdr"
    mkdir build
    cd ../../
)

cd "%FASTDDS_SRC_DIR%/thirdparty/fastcdr/build"
echo %FILE_N% Generating build...

cmake .. -G "Visual Studio 16 2019" -A x64^
    -DCMAKE_BUILD_TYPE=Release^
    -DCMAKE_CXX_FLAGS_RELEASE="/MD /MP"^
    -DCMAKE_INSTALL_PREFIX="%FASTDDS_INSTALL_DIR:\=/%"^
    -DCMAKE_CXX_FLAGS=/D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING
if %errorlevel%  neq 0 goto error_cmake

echo %FILE_N% Building...
cmake --build . --config Release --target install

if errorlevel  neq 0 goto error_install

cd ../../..

@REM echo Compiling asio dependency...

@REM if not exist "%FASTDDS_SRC_DIR%/thirdparty/asio/asio/build" (
@REM     echo %FILE_N% Creating "%FASTDDS_SRC_DIR%/thirdparty/asio/asio/build"
@REM     cd "%FASTDDS_SRC_DIR%/thirdparty/asio/asio"
@REM     mkdir build
@REM     cd ../../
@REM )

@REM cd "%FASTDDS_SRC_DIR%/thirdparty/asio/asio/build"
@REM echo %FILE_N% Generating build...

@REM cmake .. -G "Visual Studio 16 2019" -A x64^
@REM     -DCMAKE_BUILD_TYPE=Release^
@REM     -DCMAKE_CXX_FLAGS_RELEASE="/MD /MP"^
@REM     -DCMAKE_INSTALL_PREFIX="%FASTDDS_INSTALL_DIR:\=/%"^
@REM     -DCMAKE_CXX_FLAGS=/D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING
@REM if %errorlevel%  neq 0 goto error_cmake

@REM echo %FILE_N% Building...
@REM cmake --build . --config Release --target install

@REM if errorlevel  neq 0 goto error_install

@REM cd ../../../..

if not exist "%FASTDDS_BUILD_DIR%" (
    echo %FILE_N% Creating "%FASTDDS_BUILD_DIR%"
    mkdir "%FASTDDS_BUILD_DIR%"
)

cd "%FASTDDS_BUILD_DIR%"
echo %FILE_N% Generating build...

cmake .. -G "Visual Studio 16 2019" -A x64^
    -DCMAKE_BUILD_TYPE=Release^
    -DCMAKE_CXX_FLAGS_RELEASE="/MD /MP"^
    -DCMAKE_INSTALL_PREFIX="%FASTDDS_INSTALL_DIR:\=/%"^
    -DCMAKE_CXX_FLAGS=/D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING^
    "%FASTDDS_SRC_DIR%"
if %errorlevel%  neq 0 goto error_cmake

echo %FILE_N% Building...
cmake --build . --config Release --target install

if errorlevel  neq 0 goto error_install

rem Remove the downloaded Fast-DDS source because is no more needed
if %DEL_SRC% == true (
    rd /s /q "%FASTDDS_SRC_DIR%"
)

goto success

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
rem ============================================================================

:success
    echo.
    echo %FILE_N% "Fast-DDS" has been successfully installed in "%FASTDDS_INSTALL_DIR%"!
    goto good_exit

:already_build
    echo %FILE_N% A "Fast-DDS" installation already exists.
    echo %FILE_N% Delete "%FASTDDS_INSTALL_DIR%" if you want to force a rebuild.
    goto good_exit

:error_git
    echo.
    echo %FILE_N% [GIT ERROR] An error ocurred while executing the git.
    echo %FILE_N% [GIT ERROR] Possible causes:
    echo %FILE_N%              - Make sure "git" is installed.
    echo %FILE_N%              - Make sure it is available on your Windows "path".
    goto bad_exit

:error_cmake
    echo.
    echo %FILE_N% [CMAKE ERROR] An error ocurred while executing the cmake.
    echo %FILE_N% [CMAKE ERROR] Possible causes:
    echo %FILE_N%                - Make sure "CMake" is installed.
    echo %FILE_N%                - Make sure it is available on your Windows "path".
    goto bad_exit

:error_install
    echo.
    echo %FILE_N% [Visual Studio 16 2019 Win64 ERROR] An error ocurred while installing using Visual Studio 16 2019 Win64.
    echo %FILE_N% [Visual Studio 16 2019 Win64 ERROR] Possible causes:
    echo %FILE_N%                - Make sure you have Visual Studio installed.
    echo %FILE_N%                - Make sure you have the "x64 Visual C++ Toolset" in your path.
    echo %FILE_N%                  For example using the "Visual Studio x64 Native Tools Command Prompt",
    echo %FILE_N%                  or the "vcvarsall.bat".
    goto bad_exit

:good_exit
    echo %FILE_N% Exiting...
    endlocal & set install_recast=%FASTDDS_INSTALL_DIR%
    exit /b 0

:bad_exit
    if exist "%FASTDDS_INSTALL_DIR%" rd /s /q "%FASTDDS_INSTALL_DIR%"
    echo %FILE_N% Exiting with error...
    endlocal
    exit /b %errorlevel%
