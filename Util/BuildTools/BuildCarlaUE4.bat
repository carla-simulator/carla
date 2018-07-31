@echo off

setlocal

set LOCAL_PATH=%~dp0
set "FILE_N=-[%~n0]:"

rem ============================================================================
rem -- Parse arguments ---------------------------------------------------------
rem ============================================================================


set DOC_STRING=Build LibCarla.
set "USAGE_STRING=Usage: %FILE_N% [-h^|--help] [--build] [--rebuild] [--launch] [--clean] [--hard-clean]"

set HARD_CLEAN=false
set BUILD_CARLAUE4=false
set LAUNCH_UE4_EDITOR=false
set REMOVE_INTERMEDIATE=false

:arg-parse
if not "%1"=="" (
    if "%1"=="--rebuild" (
        set REMOVE_INTERMEDIATE=true
        set BUILD_CARLAUE4=true
    )

    if "%1"=="--build" (
        set BUILD_CARLAUE4=true
    )

    if "%1"=="--launch" (
        set LAUNCH_UE4_EDITOR=true
    )

    if "%1"=="--hard-clean" (
        set REMOVE_INTERMEDIATE=true
        set HARD_CLEAN=true
    )

    if "%1"=="--clean" (
        set REMOVE_INTERMEDIATE=true
    )

    if "%1"=="-h" (
        echo %DOC_STRING%
        echo %USAGE_STRING%
        GOTO :eof
    )

    if "%1"=="--help" (
        echo %DOC_STRING%
        echo %USAGE_STRING%
        GOTO :eof
    )

    shift
    goto :arg-parse
)

if %REMOVE_INTERMEDIATE% == false (
    if %LAUNCH_UE4_EDITOR% == false (
        if %BUILD_CARLAUE4% == false (
            if %HARD_CLEAN% == false (
                echo Nothing selected to be done.
                goto :eof
            )
        )
    )
)

rem Set the visual studio solution directory
rem

set CARLA_FOLDER=%ROOT_PATH%Unreal\CarlaUE4\
pushd "%CARLA_FOLDER%"

if %REMOVE_INTERMEDIATE% == true (
    echo.
    echo %FILE_N% cleaning "%CARLA_FOLDER%Binaries"
    if exist "%CARLA_FOLDER%Binaries" rmdir /S /Q "%CARLA_FOLDER%Binaries"

    echo %FILE_N% cleaning "%CARLA_FOLDER%Intermediate"
    if exist "%CARLA_FOLDER%Intermediate" rmdir /S /Q "%CARLA_FOLDER%Intermediate"

    echo %FILE_N% cleaning "%CARLA_FOLDER%Plugins\Carla\Binaries"
    if exist "%CARLA_FOLDER%Plugins\Carla\Binaries" rmdir /S /Q "%CARLA_FOLDER%Plugins\Carla\Binaries"

    echo %FILE_N% cleaning "%CARLA_FOLDER%Plugins\Carla\Intermediate"
    if exist "%CARLA_FOLDER%Plugins\Carla\Intermediate" rmdir /S /Q "%CARLA_FOLDER%Plugins\Carla\Intermediate"
)

if %HARD_CLEAN% == true (
    make CarlaUE4Editor
)

if %BUILD_CARLAUE4 == true (
    echo %FILE_N% Builing and launching Unreal Editor...
    call "%CARLA_FOLDER%CarlaUE4.uproject"
)

if %LAUNCH_UE4_EDITOR% == true (
    echo %FILE_N% Launching Unreal Editor...
    call "%CARLA_FOLDER%CarlaUE4.uproject"
)

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
rem ============================================================================
