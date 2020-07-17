@echo off
setlocal

rem BAT script that downloads and installs Recast & Detour library
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

set RECAST_HASH=cdce4e
set RECAST_COMMIT=cdce4e1a270fdf1f3942d4485954cc5e136df1df
set RECAST_SRC=recast-%RECAST_HASH%-src
set RECAST_SRC_DIR=%BUILD_DIR%%RECAST_SRC%\
set RECAST_INSTALL=recast-%RECAST_HASH%-install
set RECAST_INSTALL_DIR=%BUILD_DIR%%RECAST_INSTALL%\
set RECAST_BUILD_DIR=%RECAST_SRC_DIR%build\
set RECAST_BASENAME=%RECAST_SRC%

if exist "%RECAST_INSTALL_DIR%" (
    goto already_build
)

if not exist "%RECAST_SRC_DIR%" (
    echo %FILE_N% Cloning "Recast & Detour"

    call git clone https://github.com/carla-simulator/recastnavigation.git "%RECAST_SRC_DIR:~0,-1%"
    cd "%RECAST_SRC_DIR%"
    call git reset --hard %RECAST_COMMIT%
    cd ..
    if %errorlevel% neq 0 goto error_git
) else (
    echo %FILE_N% Not cloning "Recast & Detour" because already exists a folder called "%RECAST_SRC%".
)

if not exist "%RECAST_BUILD_DIR%" (
    echo %FILE_N% Creating "%RECAST_BUILD_DIR%"
    mkdir "%RECAST_BUILD_DIR%"
)

cd "%RECAST_BUILD_DIR%"
echo %FILE_N% Generating build...

cmake .. -G "Visual Studio 15 2017 Win64"^
    -DCMAKE_BUILD_TYPE=Release^
    -DCMAKE_CXX_FLAGS_RELEASE="/MD /MP"^
    -DCMAKE_INSTALL_PREFIX="%RECAST_INSTALL_DIR:\=/%"^
    -DCMAKE_CXX_FLAGS=/D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING^
    "%RECAST_SRC_DIR%"
if %errorlevel%  neq 0 goto error_cmake

echo %FILE_N% Building...
cmake --build . --config Release --target install

if errorlevel  neq 0 goto error_install

rem Remove the downloaded Recast & Detour source because is no more needed
if %DEL_SRC% == true (
    rd /s /q "%RECAST_SRC_DIR%"
)

md "%RECAST_INSTALL_DIR%include\recast"
move "%RECAST_INSTALL_DIR%include\*.h" "%RECAST_INSTALL_DIR%include\recast"

goto success

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
rem ============================================================================

:success
    echo.
    echo %FILE_N% "Recast & Detour" has been successfully installed in "%RECAST_INSTALL_DIR%"!
    goto good_exit

:already_build
    echo %FILE_N% A "Recast & Detour" installation already exists.
    echo %FILE_N% Delete "%RECAST_INSTALL_DIR%" if you want to force a rebuild.
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
    endlocal & set install_recast=%RECAST_INSTALL_DIR%
    exit /b 0

:bad_exit
    if exist "%RECAST_INSTALL_DIR%" rd /s /q "%RECAST_INSTALL_DIR%"
    echo %FILE_N% Exiting with error...
    endlocal
    exit /b %errorlevel%
