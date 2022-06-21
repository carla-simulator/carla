@echo off
setlocal

rem BAT script that downloads and installs all Plugins
rem Run it through a cmd with the x64 Visual C++ Toolset enabled.

set LOCAL_PATH=%~dp0
set "FILE_N=    -[%~n0]:"

rem Print batch params (debug purpose)
echo %FILE_N% [Batch params]: %*

rem ============================================================================
rem -- Parse arguments ---------------------------------------------------------
rem ============================================================================

set RELEASE=false

:arg-parse
if not "%1"=="" (
    if "%1"=="--release" (
        set RELEASE=true
        shift
    )
    shift
    goto :arg-parse
)

rem ============================================================================
rem -- Parse arguments ---------------------------------------------------------
rem ============================================================================

if not exist "%ROOT_PATH%Plugins" (
    if %RELEASE% == true (
        echo Cloning for release...
        call git clone --depth=1 --recursive https://github.com/carla-simulator/carla-plugins.git "%ROOT_PATH%Plugins"
    ) else (
        echo Cloning for build...
        call git clone --recursive https://github.com/carla-simulator/carla-plugins.git "%ROOT_PATH%Plugins"
    )
    if %errorlevel% neq 0 goto error_git
) else (
    goto already
)
goto success

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
rem ============================================================================

:success
    echo.
    echo %FILE_N% "Plugins" has been successfully installed in "%ROOT_PATH%Plugins!"
    goto good_exit

:already
    echo.
    echo %FILE_N% "Plugins" already exists in "%ROOT_PATH%Plugins!"
    goto good_exit

:error_git
    echo.
    echo %FILE_N% [GIT ERROR] An error ocurred while executing the git.
    echo %FILE_N% [GIT ERROR] Possible causes:
    echo %FILE_N%              - Make sure "git" is installed.
    echo %FILE_N%              - Make sure it is available on your Windows "path".
    goto bad_exit

:good_exit
    echo %FILE_N% Exiting...
    endlocal
    exit /b 0

:bad_exit
    echo %FILE_N% Exiting with error...
    endlocal
    exit /b %errorlevel%
