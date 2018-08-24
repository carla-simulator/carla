@echo off
setlocal

set LOCAL_PATH=%~dp0
set "FILE_N=-[%~n0]:"

rem ============================================================================
rem -- Parse arguments ---------------------------------------------------------
rem ============================================================================


set DOC_STRING=Build LibCarla.
set "USAGE_STRING=Usage: %FILE_N% [-h^|--help] [--build] [--rebuild] [--launch] [--clean]"

set BUILD_CARLAUE4=false
set BUILD_CARLAUE4_EDITOR=false

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

    if "%1"=="--build-editor" (
        set BUILD_CARLAUE4_EDITOR=true
    )

    if "%1"=="--launch" (
        set LAUNCH_UE4_EDITOR=true
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
            if %BUILD_CARLAUE4_EDITOR% == false (
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

    echo %FILE_N% cleaning "%CARLA_FOLDER%Saved"
    if exist "%CARLA_FOLDER%Saved" rmdir /S /Q "%CARLA_FOLDER%Saved"

    echo %FILE_N% cleaning "%CARLA_FOLDER%Build"
    if exist "%CARLA_FOLDER%Build" rmdir /S /Q "%CARLA_FOLDER%Build"

    echo %FILE_N% cleaning "%CARLA_FOLDER%Binaries"
    if exist "%CARLA_FOLDER%Binaries" rmdir /S /Q "%CARLA_FOLDER%Binaries"

    echo %FILE_N% cleaning "%CARLA_FOLDER%Intermediate"
    if exist "%CARLA_FOLDER%Intermediate" rmdir /S /Q "%CARLA_FOLDER%Intermediate"

    echo %FILE_N% cleaning "%CARLA_FOLDER%Plugins\Carla\Binaries"
    if exist "%CARLA_FOLDER%Plugins\Carla\Binaries" rmdir /S /Q "%CARLA_FOLDER%Plugins\Carla\Binaries"

    echo %FILE_N% cleaning "%CARLA_FOLDER%Plugins\Carla\Intermediate"
    if exist "%CARLA_FOLDER%Plugins\Carla\Intermediate" rmdir /S /Q "%CARLA_FOLDER%Plugins\Carla\Intermediate"
)

if %BUILD_CARLAUE4% == true (
    echo %FILE_N% Builing and starting Carla...

    call MsBuild.exe "%CARLA_FOLDER%CarlaUE4.sln" /m /p:configuration=Development /p:platform=Win64
    if errorlevel 1 goto error_build

    start "" "%CARLA_FOLDER%Binaries\Win64\CarlaUE4.exe"
)

if %BUILD_CARLAUE4_EDITOR% == true (
    echo %FILE_N% Builing and starting Carla...

    call MsBuild.exe "%CARLA_FOLDER%CarlaUE4.sln" /m /p:configuration="Development Editor" /p:platform=Win64
    if errorlevel 1 goto error_build

    call "%CARLA_FOLDER%CarlaUE4.uproject"
)

if %LAUNCH_UE4_EDITOR% == true (
    echo %FILE_N% Launching Unreal Editor...
    call "%CARLA_FOLDER%CarlaUE4.uproject"
)

goto good_exit

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
rem ============================================================================

:error_build
    echo.
    echo %FILE_N% There was a problem building Carla.
    echo %FILE_N% Please read the screen log for more information.
    goto bad_exit

:good_exit
    endlocal
    exit /b 0

:bad_exit
    endlocal
    exit /b %errorlevel%