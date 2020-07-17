@echo off
setlocal

rem BAT script that downloads and installs a ready to use
rem Google Test build for CARLA (carla.org).
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

set GT_VERSION=release-1.8.1
set GT_SRC=gtest-src
set GT_SRC_DIR=%BUILD_DIR%%GT_SRC%\
set GT_INSTALL=gtest-install
set GT_INSTALL_DIR=%BUILD_DIR%%GT_INSTALL%\
set GT_BUILD_DIR=%GT_SRC_DIR%build\

if exist "%GT_INSTALL_DIR%" (
    goto already_build
)

if not exist "%GT_SRC_DIR%" (
    echo %FILE_N% Cloning Google Test - version "%GT_VERSION%"...

    call git clone --depth=1 -b "%GT_VERSION%" https://github.com/google/googletest.git "%GT_SRC_DIR:~0,-1%"
    if %errorlevel% neq 0 goto error_git
) else (
    echo %FILE_N% Not cloning Google Test because already exists a folder called "%GT_SRC%".
)

if not exist "%GT_BUILD_DIR%" (
    echo %FILE_N% Creating "%GT_BUILD_DIR%"
    mkdir "%GT_BUILD_DIR%"
)

cd "%GT_BUILD_DIR%"
echo %FILE_N% Generating build...

cmake .. -G "Visual Studio 15 2017 Win64"^
    -DCMAKE_BUILD_TYPE=Release^
    -DCMAKE_CXX_FLAGS_RELEASE="/MD /MP"^
    -DCMAKE_INSTALL_PREFIX="%GT_INSTALL_DIR:\=/%"^
    -DCMAKE_CXX_FLAGS=/D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING^
    "%GT_SRC_DIR%"
if %errorlevel%  neq 0 goto error_cmake

echo %FILE_N% Building...
cmake --build . --config Release --target install

if errorlevel  neq 0 goto error_install

rem Remove the downloaded Google Test source because is no more needed
if %DEL_SRC% == true (
    rd /s /q "%GT_SRC_DIR%"
)

goto success

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
rem ============================================================================

:success
    echo.
    echo %FILE_N% Google Test has been successfully installed in "%GT_INSTALL_DIR%"!
    goto good_exit

:already_build
    echo %FILE_N% A Google Test installation already exists.
    echo %FILE_N% Delete "%GT_INSTALL_DIR%" if you want to force a rebuild.
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
    echo %FILE_N% [Visual Studio 15 2017 Win64 ERROR] An error ocurred while installing using Visual Studio 15 2017 Win64.
    echo %FILE_N% [Visual Studio 15 2017 Win64 ERROR] Possible causes:
    echo %FILE_N%                - Make sure you have Visual Studio installed.
    echo %FILE_N%                - Make sure you have the "x64 Visual C++ Toolset" in your path.
    echo %FILE_N%                  For example using the "Visual Studio x64 Native Tools Command Prompt",
    echo %FILE_N%                  or the "vcvarsall.bat".
    goto bad_exit

:good_exit
    echo %FILE_N% Exiting...
    endlocal
    set install_gtest=done
    exit /b 0

:bad_exit
    if exist "%GT_INSTALL_DIR%" rd /s /q "%GT_INSTALL_DIR%"
    echo %FILE_N% Exiting with error...
    endlocal
    exit /b %errorlevel%
