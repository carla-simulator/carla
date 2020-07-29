@echo off
setlocal enabledelayedexpansion

rem Bat script that compiles and exports the carla project (carla.org)
rem Run it through a cmd with the x64 Visual C++ Toolset enabled.
rem https://wiki.unrealengine.com/How_to_package_your_game_with_commands

set LOCAL_PATH=%~dp0
set FILE_N=-[%~n0]:

rem Print batch params (debug purpose)
echo %FILE_N% [Batch params]: %*

rem ==============================================================================
rem -- Parse arguments -----------------------------------------------------------
rem ==============================================================================

set DOC_STRING="Makes a packaged version of CARLA for distribution."
set USAGE_STRING="Usage: %FILE_N% [-h|--help] [--no-packaging] [--no-zip] [--clean]"

set DO_PACKAGE=true
set DO_COPY_FILES=true
set DO_TARBALL=true
set DO_CLEAN=false
set PACKAGES=Carla


:arg-parse
if not "%1"=="" (
    if "%1"=="--clean" (
        set DO_CLEAN=true
        set DO_TARBALL=false
        set DO_PACKAGE=false
        set DO_COPY_FILES=false

    )

    if "%1"=="--no-zip" (
        set DO_TARBALL=false
    )

    if "%1"=="--no-packaging" (
        set DO_PACKAGE=false
    )

    if "%1"=="--packages" (
        set DO_PACKAGE=false
        set DO_COPY_FILES=false
        set PACKAGES=%~2
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

rem Get Unreal Engine root path
if not defined UE4_ROOT (
    set KEY_NAME="HKEY_LOCAL_MACHINE\SOFTWARE\EpicGames\Unreal Engine"
    set VALUE_NAME=InstalledDirectory
    for /f "usebackq tokens=1,2,*" %%A in (`reg query !KEY_NAME! /s /reg:64`) do (
        if "%%A" == "!VALUE_NAME!" (
            set UE4_ROOT=%%C
        )
    )
    if not defined UE4_ROOT goto error_unreal_no_found
)

rem Set packaging paths
rem
for /f %%i in ('git describe --tags --dirty --always') do set CARLA_VERSION=%%i
if not defined CARLA_VERSION goto error_carla_version

set BUILD_FOLDER=%INSTALLATION_DIR%UE4Carla/%CARLA_VERSION%/

set DESTINATION_ZIP=%INSTALLATION_DIR%UE4Carla/CARLA_%CARLA_VERSION%.zip
set SOURCE=!BUILD_FOLDER!WindowsNoEditor/

rem ============================================================================
rem -- Create Carla package ----------------------------------------------------
rem ============================================================================

if %DO_PACKAGE%==true (
    if not exist "!BUILD_FOLDER!" mkdir "!BUILD_FOLDER!"

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
        Shipping^
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
        -archivedirectory="!BUILD_FOLDER!"^
        -package^
        -clientconfig=Shipping

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
    echo f | xcopy /y /s "!XCOPY_FROM!PythonAPI\carla\data\*"                          "!XCOPY_TO!PythonAPI\carla\data\"
    echo d | xcopy /y /s "!XCOPY_FROM!Co-Simulation"                                "!XCOPY_TO!Co-Simulation"
    echo d | xcopy /y /s "!XCOPY_FROM!PythonAPI\carla\agents"                       "!XCOPY_TO!PythonAPI\carla\agents"
    echo f | xcopy /y "!XCOPY_FROM!PythonAPI\carla\scene_layout.py"                 "!XCOPY_TO!PythonAPI\carla\"
    echo f | xcopy /y "!XCOPY_FROM!PythonAPI\carla\requirements.txt"                "!XCOPY_TO!PythonAPI\carla\"
    echo f | xcopy /y "!XCOPY_FROM!PythonAPI\examples\*.py"                         "!XCOPY_TO!PythonAPI\examples\"
    echo f | xcopy /y "!XCOPY_FROM!PythonAPI\examples\requirements.txt"             "!XCOPY_TO!PythonAPI\examples\"
    echo f | xcopy /y "!XCOPY_FROM!PythonAPI\util\*.py"                             "!XCOPY_TO!PythonAPI\util\"
    echo d | xcopy /y /s "!XCOPY_FROM!PythonAPI\util\opendrive"                     "!XCOPY_TO!PythonAPI\util\opendrive"
    echo f | xcopy /y "!XCOPY_FROM!PythonAPI\util\requirements.txt"                 "!XCOPY_TO!PythonAPI\util\"
    echo f | xcopy /y "!XCOPY_FROM!Unreal\CarlaUE4\Content\Carla\HDMaps\*.pcd"      "!XCOPY_TO!HDMaps\"
    echo f | xcopy /y "!XCOPY_FROM!Unreal\CarlaUE4\Content\Carla\HDMaps\Readme.md"  "!XCOPY_TO!HDMaps\README"
)

rem ==============================================================================
rem -- Zip the project -----------------------------------------------------------
rem ==============================================================================

if %DO_TARBALL%==true (
    set SRC_PATH=%SOURCE:/=\%

    echo %FILE_N% Building package...

    if exist "!SRC_PATH!Manifest_NonUFSFiles_Win64.txt" del /Q "!SRC_PATH!Manifest_NonUFSFiles_Win64.txt"
    if exist "!SRC_PATH!Manifest_DebugFiles_Win64.txt" del /Q "!SRC_PATH!Manifest_DebugFiles_Win64.txt"
    if exist "!SRC_PATH!Manifest_UFSFiles_Win64.txt" del /Q "!SRC_PATH!Manifest_UFSFiles_Win64.txt"
    if exist "!SRC_PATH!CarlaUE4/Saved" rmdir /S /Q "!SRC_PATH!CarlaUE4/Saved"
    if exist "!SRC_PATH!Engine/Saved" rmdir /S /Q "!SRC_PATH!Engine/Saved"

    set DST_ZIP=%DESTINATION_ZIP:/=\%
    if exist "%ProgramW6432%/7-Zip/7z.exe" (
        "%ProgramW6432%/7-Zip/7z.exe" a "!DST_ZIP!" "!SRC_PATH!" -tzip -mmt -mx5
    ) else (
        pushd "!SRC_PATH!"
            rem https://docs.microsoft.com/en-us/powershell/module/microsoft.powershell.archive/compress-archive?view=powershell-6
            powershell -command "& { Compress-Archive -Path * -CompressionLevel Fastest -DestinationPath '!DST_ZIP!' }"
        popd
    )
)

rem ==============================================================================
rem -- Remove intermediate files -------------------------------------------------
rem ==============================================================================

if %DO_CLEAN%==true (
    echo %FILE_N% Removing intermediate build.
    rmdir /S /Q "!BUILD_FOLDER!"
)

rem ==============================================================================
rem -- Cook other packages -------------------------------------------------------
rem ==============================================================================

rem Set some file locations
set CARLAUE4_ROOT_FOLDER=%ROOT_PATH%Unreal/CarlaUE4
set PACKAGE_PATH_FILE=%CARLAUE4_ROOT_FOLDER%/Content/PackagePath.txt
set MAP_LIST_FILE=%CARLAUE4_ROOT_FOLDER%/Content/MapPaths.txt

rem through all maps to cook (parameter)
for %%i in (%PACKAGES%) do (

    set PACKAGE_NAME=%%i

    if not !PACKAGE_NAME! == Carla (
        echo Preparing environment for cooking '!PACKAGE_NAME!'.

        set BUILD_FOLDER=%INSTALLATION_DIR%UE4Carla/!PACKAGE_NAME!_%CARLA_VERSION%\
        set PACKAGE_PATH=%CARLAUE4_ROOT_FOLDER%/Content/!PACKAGE_NAME!

        if not exist "!BUILD_FOLDER!" mkdir "!BUILD_FOLDER!"

        echo Cooking package '!PACKAGE_NAME!'...

        pushd "%CARLAUE4_ROOT_FOLDER%"

        REM # Prepare cooking of package
        call "%UE4_ROOT%/Engine/Binaries/Win64/UE4Editor.exe "^
        "%CARLAUE4_ROOT_FOLDER%/CarlaUE4.uproject"^
        -run=PrepareAssetsForCooking^
        -PackageName=!PACKAGE_NAME!^
        -OnlyPrepareMaps=false

        set /p PACKAGE_FILE=<%PACKAGE_PATH_FILE%
        set /p MAPS_TO_COOK=<%MAP_LIST_FILE%

        REM # Cook maps
        call "%UE4_ROOT%/Engine/Binaries/Win64/UE4Editor.exe "^
        "%CARLAUE4_ROOT_FOLDER%/CarlaUE4.uproject"^
        -run=cook^
        -map="!MAPS_TO_COOK!"^
        -cooksinglepackage^
        -targetplatform="WindowsNoEditor"^
        -OutputDir="!BUILD_FOLDER!"

        REM remove the props folder if exist
        set PROPS_MAP_FOLDER="%PACKAGE_PATH%/Maps/PropsMap"
        if exist "%PROPS_MAP_FOLDER%" (
        rmdir /S /Q "%PROPS_MAP_FOLDER%"
        )

        popd

        echo Copying files to '!PACKAGE_NAME!'...

        pushd "!BUILD_FOLDER!"

        set SUBST_PATH=!BUILD_FOLDER!CarlaUE4

        REM Copy the package config file to package
        set TARGET="!SUBST_PATH!\Content\Carla\Config\"
        mkdir !TARGET:/=\!
        copy "!PACKAGE_FILE:/=\!" !TARGET:/=\!

        REM Copy some files for each map to the package
        REM MAPS_TO_COOK is read into an array as tokens separated by '+', we replace the '+' by a new line
        REM We need the blank line after this line, don't remove it
        set MAPS_TO_COOK=!MAPS_TO_COOK:+=^

        !
        set BASE_CONTENT=%INSTALLATION_DIR:/=\%..\Unreal\CarlaUE4\Content
        for /f "tokens=1 delims=+" %%a in ("!MAPS_TO_COOK!") do (

            REM Get path and name of map
            for /f %%i in ("%%a") do (
                set MAP_FOLDER=%%~pi
                set MAP_NAME=%%~ni
                REM Remove the '/Game' string
                set MAP_FOLDER=!MAP_FOLDER:~5!
            )

            REM # copy the OpenDrive file
            set SRC=!BASE_CONTENT!!MAP_FOLDER!\OpenDrive\!MAP_NAME!.xodr
            set TRG=!BUILD_FOLDER!\CarlaUE4\Content\!MAP_FOLDER!\OpenDrive\
            if exist "!SRC!" (
                mkdir "!TRG!"
                copy "!SRC!" "!TRG!"
            )

            REM # copy the navigation file
            set SRC=!BASE_CONTENT!!MAP_FOLDER!\Nav\!MAP_NAME!.bin
            set TRG=!BUILD_FOLDER!\CarlaUE4\Content\!MAP_FOLDER!\Nav\
            if exist "!SRC!" (
                mkdir "!TRG!"
                copy "!SRC!" "!TRG!"
            )
        )

        rmdir /S /Q "!BUILD_FOLDER!\CarlaUE4\Metadata"
        rmdir /S /Q "!BUILD_FOLDER!\CarlaUE4\Plugins"
        REM del "!BUILD_FOLDER!\CarlaUE4\Content\!PACKAGE_NAME!/Maps/!PROPS_MAP_NAME!"
        del "!BUILD_FOLDER!\CarlaUE4\AssetRegistry.bin"

        if %DO_TARBALL%==true (

            echo Packaging '!PACKAGE_NAME!'...

            set DESTINATION_ZIP=%INSTALLATION_DIR%UE4Carla/!PACKAGE_NAME!_%CARLA_VERSION%.zip
            set SOURCE=!BUILD_FOLDER:/=\!\
            set DST_ZIP=!DESTINATION_ZIP:/=\!

            pushd "!SOURCE!"

            if exist "%ProgramW6432%/7-Zip/7z.exe" (
                "%ProgramW6432%/7-Zip/7z.exe" a "!DST_ZIP!" . -tzip -mmt -mx5
            ) else (
                rem https://docs.microsoft.com/en-us/powershell/module/microsoft.powershell.archive/compress-archive?view=powershell-6
                powershell -command "& { Compress-Archive -Path * -CompressionLevel Fastest -DestinationPath '!DST_ZIP!' }"
            )

            popd

            if errorlevel 1 goto bad_exit
            echo ZIP created at !DST_ZIP!
        )

        popd

        if %DO_CLEAN%==true (
            echo %FILE_N% Removing intermediate build.
            rmdir /S /Q "!BUILD_FOLDER!"
        )
    )
)

rem ============================================================================

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
    echo %FILE_N% [ERROR] Unreal Engine not detected
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
