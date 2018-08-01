@echo off
setlocal enableextensions

rem
rem https://wiki.unrealengine.com/How_to_package_your_game_with_commands
rem

set LOCAL_PATH=%~dp0
set "FILE_N=-[%~n0]:"


rem Extract Unreal Engine root path
rem
set KEY_NAME="HKEY_LOCAL_MACHINE\SOFTWARE\EpicGames\Unreal Engine\4.19"
set VALUE_NAME=InstalledDirectory

for /f %%i in ('git describe --tags --dirty --always') do set CARLA_VERSION=%%i
if not defined CARLA_VERSION goto error_CARLA_VERSION

for /f "usebackq tokens=3*" %%A in (`reg query %KEY_NAME%  /v %VALUE_NAME%`) do set UE4_ROOT=%%A%%B
if not defined UE4_ROOT goto error_unreal_no_found

set CARLA_OUTPUT_PATH=%INSTALLATION_DIR%UE4Carla/%CARLA_VERSION%
if not exist "%CARLA_OUTPUT_PATH%" mkdir "%CARLA_OUTPUT_PATH%"

rem ============================================================================
rem -- Create Carla package ----------------------------------------------------
rem ============================================================================

call "%UE4_ROOT%\Engine\Build\BatchFiles\Build.bat"^
    CarlaUE4Editor^
    Win64^
    Development^
    -WaitMutex^
    -FromMsBuild^
    "%ROOT_PATH%Unreal/CarlaUE4/CarlaUE4.uproject"

call "%UE4_ROOT%\Engine\Build\BatchFiles\Build.bat"^
    CarlaUE4^
    Win64^
    Development^
    -WaitMutex^
    -FromMsBuild^
    "%ROOT_PATH%Unreal/CarlaUE4/CarlaUE4.uproject"

call "%UE4_ROOT%\Engine\Build\BatchFiles\RunUAT.bat"^
    BuildCookRun^
    -nocompileeditor^
    -TargetPlatform=Win64^
    -Platform=Win64^
    -installed^
    -nop4^
    -project="%ROOT_PATH%Unreal/CarlaUE4/CarlaUE4.uproject"^
    -cook^
    -stage^
    -build^
    -archive^
    -archivedirectory="%CARLA_OUTPUT_PATH%"^
    -package^
    -clientconfig=Development

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
rem ============================================================================

:success
    echo.
    echo %FILE_N% Carla project successful exported to "%CARLA_OUTPUT_PATH%"!
    goto good_exit

:error_carla_version
    echo.
    echo Carla Version is not set
    goto bad_exit

:error_unreal_no_found
    echo.
    echo %FILE_N% Unreal Engine 4.19 not detected
    goto bad_exit

:good_exit
    endlocal
    exit /b 0

:bad_exit
    endlocal
    exit /b 1

