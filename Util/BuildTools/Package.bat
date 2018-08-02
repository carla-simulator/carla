@echo off
setlocal enabledelayedexpansion

rem Bat script that compiles and exports the carla project (carla.org)
rem Run it through a cmd with the x64 Visual C++ Toolset enabled.
rem https://wiki.unrealengine.com/How_to_package_your_game_with_commands

set LOCAL_PATH=%~dp0
set "FILE_N=-[%~n0]:"

rem ==============================================================================
rem -- Parse arguments -----------------------------------------------------------
rem ==============================================================================

set DOC_STRING="Makes a packaged version of CARLA for distribution."
set USAGE_STRING="Usage: %FILE_N% [-h|--help] [--no-packaging] [--no-zip] [--clean-intermediate]"

set DO_PACKAGE=true
set DO_COPY_FILES=true

set DO_TARBALL=true
set DO_CLEAN_INTERMEDIATE=false

set UE_VERSION=4.19

:arg-parse
if not "%1"=="" (
    if "%1"=="--clean-intermediate" (
        set DO_CLEAN_INTERMEDIATE=true
    )

    if "%1"=="--no-zip" (
        set DO_TARBALL=false
    )

    if "%1"=="--no-packaging" (
        set DO_PACKAGE=false
    )

    if "%1"=="--ue-version" (
        set UE_VERSION=%~2
        shift
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

rem Extract Unreal Engine root path
rem
set KEY_NAME="HKEY_LOCAL_MACHINE\SOFTWARE\EpicGames\Unreal Engine\%UE_VERSION%"
set VALUE_NAME=InstalledDirectory

for /f "usebackq tokens=3*" %%A in (`reg query %KEY_NAME% /v %VALUE_NAME% /reg:64`) do set UE4_ROOT=%%A %%B
if not defined UE4_ROOT goto error_unreal_no_found

rem Set packaging paths
rem
for /f %%i in ('git describe --tags --dirty --always') do set CARLA_VERSION=%%i
if not defined CARLA_VERSION goto error_carla_version

set BUILD_FOLDER=%INSTALLATION_DIR%UE4Carla/%CARLA_VERSION%/
if not exist "%BUILD_FOLDER%" mkdir "%BUILD_FOLDER%"

set DESTINATION_TAR="%BUILD_FOLDER%../CARLA_%CARLA_VERSION%.tar.gz"
set SOURCE=%BUILD_FOLDER%WindowsNoEditor/

rem ============================================================================
rem -- Create Carla package ----------------------------------------------------
rem ============================================================================

if %DO_PACKAGE%==true (
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
        -archivedirectory="%BUILD_FOLDER%"^
        -package^
        -clientconfig=Development
)

rem ==============================================================================
rem -- Adding extra files to package ---------------------------------------------
rem ==============================================================================

if %DO_COPY_FILES%==true (
    echo.
    echo "%FILE_N% Adding extra files to package..."

    set XCOPY_ROOT_FROM=%ROOT_PATH:/=\%
    set XCOPY_COPY_TO=%SOURCE:/=\%

    echo f | xcopy /y "!XCOPY_ROOT_FROM!LICENSE"                           "!XCOPY_COPY_TO!LICENSE"
    echo f | xcopy /y "!XCOPY_ROOT_FROM!CHANGELOG.md"                      "!XCOPY_COPY_TO!CHANGELOG"
    echo f | xcopy /y "!XCOPY_ROOT_FROM!Docs\release_readme.md"            "!XCOPY_COPY_TO!README"
    echo f | xcopy /y "!XCOPY_ROOT_FROM!Util\Docker\Release.Dockerfile"    "!XCOPY_COPY_TO!Dockerfile"
    echo f | xcopy /y "!XCOPY_ROOT_FROM!PythonAPI\dist\*.egg"              "!XCOPY_COPY_TO!PythonAPI\"
    echo f | xcopy /y "!XCOPY_ROOT_FROM!PythonAPI\example.py"              "!XCOPY_COPY_TO!PythonAPI\example.py"
    echo f | xcopy /y "!XCOPY_ROOT_FROM!PythonAPI\manual_control.py"       "!XCOPY_COPY_TO!PythonAPI\manual_control.py"
    echo f | xcopy /y "!XCOPY_ROOT_FROM!PythonAPI\vehicle_gallery.py"      "!XCOPY_COPY_TO!PythonAPI\vehicle_gallery.py"
)

rem ==============================================================================
rem -- Zip the project -----------------------------------------------------------
rem ==============================================================================

if %DO_TARBALL%==true (
    echo.
    echo "%FILE_N% Building package..."

    if exist %SOURCE%Manifest_NonUFSFiles_Win64.txt del /S /Q %SOURCE%Manifest_NonUFSFiles_Win64.txt
    if exist %SOURCE%Manifest_DebugFiles_Win64.txt del /S /Q %SOURCE%Manifest_DebugFiles_Win64.txt
    if exist %SOURCE%Manifest_UFSFiles_Win64.txt del /S /Q %SOURCE%Manifest_UFSFiles_Win64.txt

    if exist CarlaUE4/Saved rmdir /S /Q CarlaUE4/Saved
    if exist Engine/Saved rmdir /S /Q Engine/Saved

    pushd "%SOURCE%""
        tar -czf "%DESTINATION_TAR%" "*"
    popd
)

rem ==============================================================================
rem -- Remove intermediate files -------------------------------------------------
rem ==============================================================================

if %DO_CLEAN_INTERMEDIATE%==true (
    echo.
    echo "%FILE_N% Removing intermediate build."
    rmdir /S /Q "%BUILD_FOLDER%"
)

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
rem ============================================================================

:success
    echo.
    if %DO_PACKAGE%==true echo %FILE_N% Carla project successful exported to "%CARLA_OUTPUT_PATH%"!
    if %DO_TARBALL%==true echo %FILE_N% Compress carla project exported to "%DESTINATION_TAR%"!
    goto good_exit

:error_carla_version
    echo.
    echo Carla Version is not set
    goto bad_exit

:error_unreal_no_found
    echo.
    echo %FILE_N% Unreal Engine %UE_VERSION% not detected
    goto bad_exit

:good_exit
    endlocal
    exit /b 0

:bad_exit
    endlocal
    exit /b 1
