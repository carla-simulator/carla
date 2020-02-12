@echo off
setlocal enabledelayedexpansion

rem Bat script that compiles and exports the carla project (carla.org)
rem Run it through a cmd with the x64 Visual C++ Toolset enabled.
rem https://wiki.unrealengine.com/How_to_package_your_game_with_commands

set LOCAL_PATH=%~dp0
set "FILE_N=-[%~n0]:"

rem Print batch params (debug purpose)
echo %FILE_N% [Batch params]: %*

rem ==============================================================================
rem -- Parse arguments -----------------------------------------------------------
rem ==============================================================================

set DOC_STRING="Makes a packaged version of CARLA for distribution."
set USAGE_STRING="Usage: %FILE_N% [-h|--help] [--no-packaging] [--zip] [--clean]"

set DO_PACKAGE=true
set DO_COPY_FILES=true

set DO_TARBALL=false
set DO_CLEAN=false

set UE_VERSION=4.22

:arg-parse
if not "%1"=="" (
    if "%1"=="--clean" (
        set DO_CLEAN=true
        set DO_TARBALL=false
        set DO_PACKAGE=false
        set DO_COPY_FILES=false
    )

    if "%1"=="--zip" (
        set DO_TARBALL=true
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

for /f "usebackq tokens=2*" %%A in (`reg query %KEY_NAME% /v %VALUE_NAME% /reg:64`) do set UE4_ROOT=%%B
if not defined UE4_ROOT goto error_unreal_no_found

rem Set packaging paths
rem
for /f %%i in ('git describe --tags --dirty --always') do set CARLA_VERSION=%%i
if not defined CARLA_VERSION goto error_carla_version

set BUILD_FOLDER=%INSTALLATION_DIR%UE4Carla/%CARLA_VERSION%/
if not exist "%BUILD_FOLDER%" mkdir "%BUILD_FOLDER%"

set DESTINATION_ZIP="%BUILD_FOLDER%../CARLA_%CARLA_VERSION%.zip"
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

    if errorlevel 1 goto error_build_editor

    call "%UE4_ROOT%\Engine\Build\BatchFiles\Build.bat"^
        CarlaUE4^
        Win64^
        Development^
        -WaitMutex^
        -FromMsBuild^
        "%ROOT_PATH%Unreal/CarlaUE4/CarlaUE4.uproject"

    if errorlevel 1 goto error_build

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

    if errorlevel 1 goto error_runUAT
)

rem ==============================================================================
rem -- Adding extra files to package ---------------------------------------------
rem ==============================================================================

if %DO_COPY_FILES%==true (
    echo "%FILE_N% Adding extra files to package..."

    set XCOPY_FROM=%ROOT_PATH:/=\%
    set XCOPY_TO=%SOURCE:/=\%

    echo f | xcopy /y "!XCOPY_FROM!LICENSE"                                         "!XCOPY_TO!LICENSE"
    echo f | xcopy /y "!XCOPY_FROM!CHANGELOG.md"                                    "!XCOPY_TO!CHANGELOG"
    echo f | xcopy /y "!XCOPY_FROM!Docs\release_readme.md"                          "!XCOPY_TO!README"
    echo f | xcopy /y "!XCOPY_FROM!Util\Docker\Release.Dockerfile"                  "!XCOPY_TO!Dockerfile"
    echo f | xcopy /y "!XCOPY_FROM!PythonAPI\carla\dist\*.egg"                      "!XCOPY_TO!PythonAPI\carla\dist\"
    echo d | xcopy /y /s "!XCOPY_FROM!PythonAPI\carla\agents"                       "!XCOPY_TO!PythonAPI\carla\agents"
    echo f | xcopy /y "!XCOPY_FROM!PythonAPI\carla\scene_layout.py"                 "!XCOPY_TO!PythonAPI\carla\"
    echo f | xcopy /y "!XCOPY_FROM!PythonAPI\carla\requirements.txt"                "!XCOPY_TO!PythonAPI\carla\"
    echo f | xcopy /y "!XCOPY_FROM!PythonAPI\examples\*.py"                         "!XCOPY_TO!PythonAPI\examples\"
    echo f | xcopy /y "!XCOPY_FROM!PythonAPI\examples\requirements.txt"             "!XCOPY_TO!PythonAPI\examples\"
    echo f | xcopy /y "!XCOPY_FROM!PythonAPI\util\*.py"                             "!XCOPY_TO!PythonAPI\util\"
    echo f | xcopy /y "!XCOPY_FROM!PythonAPI\util\requirements.txt"                 "!XCOPY_TO!PythonAPI\util\"
    echo f | xcopy /y "!XCOPY_FROM!Unreal\CarlaUE4\Content\Carla\HDMaps\*.pcd"      "!XCOPY_TO!HDMaps\"
    echo f | xcopy /y "!XCOPY_FROM!Unreal\CarlaUE4\Content\Carla\HDMaps\Readme.md"  "!XCOPY_TO!HDMaps\README"

)

rem ==============================================================================
rem -- Zip the project -----------------------------------------------------------
rem ==============================================================================

if %DO_TARBALL%==true (
    echo "%FILE_N% Building package..."

    set DST_ZIP=%DESTINATION_ZIP:/=\%
    set SRC_PATH=%SOURCE:/=\%

    if exist "!SRC_PATH!Manifest_NonUFSFiles_Win64.txt" del /Q "!SRC_PATH!Manifest_NonUFSFiles_Win64.txt"
    if exist "!SRC_PATH!Manifest_DebugFiles_Win64.txt" del /Q "!SRC_PATH!Manifest_DebugFiles_Win64.txt"
    if exist "!SRC_PATH!Manifest_UFSFiles_Win64.txt" del /Q "!SRC_PATH!Manifest_UFSFiles_Win64.txt"

    if exist "!SRC_PATH!CarlaUE4/Saved" rmdir /S /Q "!SRC_PATH!CarlaUE4/Saved"
    if exist "!SRC_PATH!Engine/Saved" rmdir /S /Q "!SRC_PATH!Engine/Saved"

    pushd "!SRC_PATH!"
        rem https://docs.microsoft.com/en-us/powershell/module/microsoft.powershell.archive/compress-archive?view=powershell-6
        rem If memory problem happens, then probably you need more memory for the archiving. Use:
        rem    powershell -command "Set-Item WSMan:\localhost\Shell\MaxMemoryPerShellMB 1000000"
        powershell -command "& { Compress-Archive -Path * -CompressionLevel Fastest -DestinationPath '!DST_ZIP!' }"
    popd
)

rem ==============================================================================
rem -- Remove intermediate files -------------------------------------------------
rem ==============================================================================

if %DO_CLEAN%==true (
    echo %FILE_N% Removing intermediate build.

    rmdir /S /Q "%BUILD_FOLDER%"
    goto :eof
)

goto success

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
rem ============================================================================

:success
    echo.
    if %DO_PACKAGE%==true echo %FILE_N% Carla project successful exported to "%BUILD_FOLDER:/=\%"!
    if %DO_TARBALL%==true echo %FILE_N% Compress carla project exported to "%DESTINATION_ZIP%"!
    goto good_exit

:error_carla_version
    echo.
    echo %FILE_N% [ERROR] Carla Version is not set
    goto bad_exit

:error_unreal_no_found
    echo.
    echo %FILE_N% [ERROR] Unreal Engine %UE_VERSION% not detected
    goto bad_exit

:error_build_editor
    echo.
    echo %FILE_N% [ERROR] There was a problem while building the CarlaUE4Editor.
    echo           [ERROR] Please read the screen log for more information.
    goto bad_exit

:error_build
    echo.
    echo %FILE_N% [ERROR] There was a problem while building the CarlaUE4.
    echo           [ERROR] Please read the screen log for more information.
    goto bad_exit

:error_runUAT
    echo.
    echo %FILE_N% [ERROR] There was a problem while packaging Unreal project.
    echo           [ERROR] Please read the screen log for more information.
    goto bad_exit

:good_exit
    endlocal
    exit /b 0

:bad_exit
    endlocal
    exit /b 1
