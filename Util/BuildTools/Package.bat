@echo off
setlocal enabledelayedexpansion

rem don't remove next two empty lines after next
set LF=^


rem Bat script that compiles and exports the carla project (carla.org)
rem Run it through a cmd with the x64 Visual C++ Toolset enabled.
rem https://wiki.unrealengine.com/How_to_package_your_game_with_commands

set LOCAL_PATH=%~dp0
set FILE_N=-[%~n0]:

rem Print batch params (debug purpose)
echo %FILE_N% [Batch params]: %*

rem Measure overall execution time of packaging
call :get_current_time_in_seconds T_START_OVERALL

rem ==============================================================================
rem -- Parse arguments -----------------------------------------------------------
rem ==============================================================================

set DOC_STRING="Makes a packaged version of CARLA for distribution."
set USAGE_STRING="Usage: %FILE_N% [-h|--help] [--config={Debug,Development,Shipping}] [--no-packaging] [--no-zip] [--clean] [--clean-intermediate] [--target-archive]"

set DO_PACKAGE=true
set DO_COPY_FILES=true
set DO_TARBALL=true
set DO_CLEAN=false
set PACKAGES=Carla
set PACKAGE_CONFIG=Shipping
set USE_CARSIM=false
set SINGLE_PACKAGE=false
set MEASURE_TIME=true

:arg-parse
if not "%1"=="" (
    if "%1"=="--clean" (
        set DO_CLEAN=true
        set DO_TARBALL=false
        set DO_PACKAGE=false
        set DO_COPY_FILES=false
        set MEASURE_TIME=false
    )

    if "%1"=="--config" (
        set PACKAGE_CONFIG=%2
        shift
    )

    if "%1"=="--clean-intermediate" (
        set DO_CLEAN=true
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
        set PACKAGES=%*
        shift
    )

    if "%1"=="--target-archive" (
        set SINGLE_PACKAGE=true
        set TARGET_ARCHIVE=%2
        shift
    )

    if "%1"=="--carsim" (
        set USE_CARSIM=true
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
call :get_git_repository_version
if not defined REPOSITORY_TAG goto error_carla_version
set CARLA_VERSION=!REPOSITORY_TAG!

set BUILD_FOLDER=%INSTALLATION_DIR%UE4Carla/%CARLA_VERSION%/

set DESTINATION_ZIP=%INSTALLATION_DIR%UE4Carla/CARLA_%CARLA_VERSION%.zip
set SOURCE=!BUILD_FOLDER!WindowsNoEditor/

rem ============================================================================
rem -- Create Carla package ----------------------------------------------------
rem ============================================================================

call :get_current_time_in_seconds T_START_DO_PACKAGE
if %DO_PACKAGE%==true (

    if %USE_CARSIM% == true (
        python %ROOT_PATH%Util/BuildTools/enable_carsim_to_uproject.py -f="%ROOT_PATH%Unreal/CarlaUE4/CarlaUE4.uproject" -e
        echo CarSim ON > "%ROOT_PATH%Unreal/CarlaUE4/Config/CarSimConfig.ini"
    ) else (
        python %ROOT_PATH%Util/BuildTools/enable_carsim_to_uproject.py -f="%ROOT_PATH%Unreal/CarlaUE4/CarlaUE4.uproject"
        echo CarSim OFF > "%ROOT_PATH%Unreal/CarlaUE4/Config/CarSimConfig.ini"
    )

    if not exist "!BUILD_FOLDER!" mkdir "!BUILD_FOLDER!"

    call "%UE4_ROOT%\Engine\Build\BatchFiles\Build.bat"^
        CarlaUE4Editor^
        Win64^
        Development^
        -WaitMutex^
        -FromMsBuild^
        "%ROOT_PATH%Unreal/CarlaUE4/CarlaUE4.uproject"

    if errorlevel 1 goto error_build_editor

    echo "%UE4_ROOT%\Engine\Build\BatchFiles\Build.bat"^
        CarlaUE4^
        Win64^
        %PACKAGE_CONFIG%^
        -WaitMutex^
        -FromMsBuild^
        "%ROOT_PATH%Unreal/CarlaUE4/CarlaUE4.uproject"
    call "%UE4_ROOT%\Engine\Build\BatchFiles\Build.bat"^
        CarlaUE4^
        Win64^
        %PACKAGE_CONFIG%^
        -WaitMutex^
        -FromMsBuild^
        "%ROOT_PATH%Unreal/CarlaUE4/CarlaUE4.uproject"

    if errorlevel 1 goto error_build

    echo "%UE4_ROOT%\Engine\Build\BatchFiles\RunUAT.bat"^
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
        -clientconfig=%PACKAGE_CONFIG%

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
        -clientconfig=%PACKAGE_CONFIG%

    if errorlevel 1 goto error_runUAT

    call :cook_tagged_materials Carla !SOURCE!
)
call :get_current_time_in_seconds T_END_DO_PACKAGE
set /A ELAPSED_TIME=!T_END_DO_PACKAGE! - !T_START_DO_PACKAGE!
if %MEASURE_TIME%==true if %DO_PACKAGE%==true echo %FILE_N% [TIME]: Building and cooking Carla took !ELAPSED_TIME! seconds.

rem ==============================================================================
rem -- Adding extra files to package ---------------------------------------------
rem ==============================================================================

call :get_current_time_in_seconds T_START_DO_COPY_FILES
if %DO_COPY_FILES%==true (
    echo "%FILE_N% Adding extra files to package..."

    set XCOPY_FROM=%ROOT_PATH:/=\%
    set XCOPY_TO=%SOURCE:/=\%

    echo f | xcopy /y "!XCOPY_FROM!LICENSE"                                         "!XCOPY_TO!LICENSE"
    echo f | xcopy /y "!XCOPY_FROM!CHANGELOG.md"                                    "!XCOPY_TO!CHANGELOG"
    echo f | xcopy /y "!XCOPY_FROM!Docs\release_readme.md"                          "!XCOPY_TO!README"
    echo f | xcopy /y "!XCOPY_FROM!Util\Docker\Release.Dockerfile"                  "!XCOPY_TO!Dockerfile"
    echo f | xcopy /y "!XCOPY_FROM!PythonAPI\carla\dist\*.whl"                      "!XCOPY_TO!PythonAPI\carla\dist\"
    echo d | xcopy /y /s "!XCOPY_FROM!Co-Simulation"                                "!XCOPY_TO!Co-Simulation"
    echo d | xcopy /y /s "!XCOPY_FROM!PythonAPI\carla\agents"                       "!XCOPY_TO!PythonAPI\carla\agents"
    echo f | xcopy /y "!XCOPY_FROM!PythonAPI\carla\scene_layout.py"                 "!XCOPY_TO!PythonAPI\carla\"
    echo f | xcopy /y "!XCOPY_FROM!PythonAPI\carla\requirements.txt"                "!XCOPY_TO!PythonAPI\carla\"
    echo f | xcopy /y "!XCOPY_FROM!PythonAPI\examples\*.py"                         "!XCOPY_TO!PythonAPI\examples\"
    echo f | xcopy /y "!XCOPY_FROM!PythonAPI\examples\requirements.txt"             "!XCOPY_TO!PythonAPI\examples\"
    echo f | xcopy /y "!XCOPY_FROM!PythonAPI\examples\cosmos_aov.yaml"              "!XCOPY_TO!PythonAPI\examples\"
    echo d | xcopy /y /s "!XCOPY_FROM!PythonAPI\examples\nvidia"                    "!XCOPY_TO!PythonAPI\examples\nvidia"
    echo f | xcopy /y "!XCOPY_FROM!PythonAPI\util\*.py"                             "!XCOPY_TO!PythonAPI\util\"
    echo d | xcopy /y /s "!XCOPY_FROM!PythonAPI\util\opendrive"                     "!XCOPY_TO!PythonAPI\util\opendrive"
    echo f | xcopy /y "!XCOPY_FROM!PythonAPI\util\requirements.txt"                 "!XCOPY_TO!PythonAPI\util\"
    echo f | xcopy /y "!XCOPY_FROM!Unreal\CarlaUE4\Content\Carla\HDMaps\*.pcd"      "!XCOPY_TO!HDMaps\"
    echo f | xcopy /y "!XCOPY_FROM!Unreal\CarlaUE4\Content\Carla\HDMaps\Readme.md"  "!XCOPY_TO!HDMaps\README"
    if exist "!XCOPY_FROM!Plugins" (
        echo d | xcopy /y /s "!XCOPY_FROM!Plugins"                                  "!XCOPY_TO!Plugins"
    )
)
call :get_current_time_in_seconds T_END_DO_COPY_FILES
set /A ELAPSED_TIME=!T_END_DO_COPY_FILES! - !T_START_DO_COPY_FILES!
if %MEASURE_TIME%==true if %DO_COPY_FILES%==true echo %FILE_N% [TIME]: Copying extra files to Carla package took !ELAPSED_TIME! seconds.

rem ==============================================================================
rem -- Zip the project -----------------------------------------------------------
rem ==============================================================================

call :get_current_time_in_seconds T_START_DO_TARBALL
if %DO_PACKAGE%==true if %DO_TARBALL%==true (
    set SRC_PATH=%SOURCE:/=\%

    echo %FILE_N% Building package...

    if exist "!SRC_PATH!Manifest_NonUFSFiles_Win64.txt" del /Q "!SRC_PATH!Manifest_NonUFSFiles_Win64.txt"
    if exist "!SRC_PATH!Manifest_DebugFiles_Win64.txt" del /Q "!SRC_PATH!Manifest_DebugFiles_Win64.txt"
    if exist "!SRC_PATH!Manifest_UFSFiles_Win64.txt" del /Q "!SRC_PATH!Manifest_UFSFiles_Win64.txt"
    if exist "!SRC_PATH!CarlaUE4/Saved" rmdir /S /Q "!SRC_PATH!CarlaUE4/Saved"
    if exist "!SRC_PATH!Engine/Saved" rmdir /S /Q "!SRC_PATH!Engine/Saved"

    call :zip_dir !SRC_PATH! !DESTINATION_ZIP!
)
call :get_current_time_in_seconds T_END_DO_TARBALL
set /A ELAPSED_TIME=!T_END_DO_TARBALL! - !T_START_DO_TARBALL!
if %MEASURE_TIME%==true if %DO_PACKAGE%==true if %DO_TARBALL%==true echo %FILE_N% [TIME]: Zipping the project took !ELAPSED_TIME! seconds.

rem ==============================================================================
rem -- Remove intermediate files -------------------------------------------------
rem ==============================================================================

call :get_current_time_in_seconds T_START_DO_CLEAN
if %DO_CLEAN%==true (
    echo %FILE_N% Removing intermediate build.
    rmdir /S /Q "!BUILD_FOLDER!"
)
call :get_current_time_in_seconds T_END_DO_CLEAN
set /A ELAPSED_TIME=!T_END_DO_CLEAN! - !T_START_DO_CLEAN!
if %MEASURE_TIME%==true if %DO_CLEAN%==true echo %FILE_N% [TIME]: Cleaning up took !ELAPSED_TIME! seconds.

rem ==============================================================================
rem -- Cook other packages -------------------------------------------------------
rem ==============================================================================

call :get_current_time_in_seconds T_START_PACKAGES

rem Set some file locations
set CARLAUE4_ROOT_FOLDER=%ROOT_PATH%Unreal/CarlaUE4
set PACKAGE_PATH_FILE=%CARLAUE4_ROOT_FOLDER%/Content/PackagePath.txt
set MAP_LIST_FILE=%CARLAUE4_ROOT_FOLDER%/Content/MapPaths.txt

rem get the packages to cook from the arguments whole string
rem (to support multiple packages)
echo Parsing packages...
if not "%PACKAGES%" == "Carla" (
    set ARGUMENTS=%PACKAGES:--=!LF!%
    for /f "tokens=*" %%i in ("!ARGUMENTS!") do (
        set a=%%i
        if "!a:~0,9!" == "packages=" (
            set RESULT=!a:~9!
        ) else (
            if "!a:~0,9!" == "packages " (
                set RESULT=!a:~9!
            )
        )
    )
) else (
    set RESULT=%PACKAGES%
)
rem through all maps to cook (parameter)
set PACKAGES=%RESULT:,=!LF!%
set PACKAGES=!PACKAGES: =!
set SINGLE_PACKAGE_DIR=%INSTALLATION_DIR%UE4Carla/Temp_SinglePackage_%CARLA_VERSION%
for /f "tokens=* delims=" %%i in ("!PACKAGES!") do (
    call :get_current_time_in_seconds T_START_PACKAGE

    set PACKAGE_NAME=%%i

    if not !PACKAGE_NAME! == Carla (
        echo Preparing environment for cooking '!PACKAGE_NAME!'.

        set BUILD_FOLDER=%INSTALLATION_DIR%UE4Carla/!PACKAGE_NAME!_%CARLA_VERSION%
        set PACKAGE_PATH=%CARLAUE4_ROOT_FOLDER%/Content/!PACKAGE_NAME!

        if not exist "!BUILD_FOLDER!" mkdir "!BUILD_FOLDER!"

        echo Cooking package '!PACKAGE_NAME!'...

        pushd "%CARLAUE4_ROOT_FOLDER%"

        echo   - prepare
        REM # Prepare cooking of package
        echo Prepare cooking of package: !PACKAGE_NAME!
        call "%UE4_ROOT%/Engine/Binaries/Win64/UE4Editor.exe "^
        "%CARLAUE4_ROOT_FOLDER%/CarlaUE4.uproject"^
        -run=PrepareAssetsForCooking^
        -PackageName=!PACKAGE_NAME!^
        -OnlyPrepareMaps=false

        set /p PACKAGE_FILE=<%PACKAGE_PATH_FILE%
        set /p MAPS_TO_COOK=<%MAP_LIST_FILE%

        echo   - cook
        for /f "tokens=*" %%a in (%MAP_LIST_FILE%) do (
            REM # Cook maps
            echo Cooking: %%a
            call "%UE4_ROOT%/Engine/Binaries/Win64/UE4Editor.exe "^
            "%CARLAUE4_ROOT_FOLDER%/CarlaUE4.uproject"^
            -run=cook^
            -map="%%a"^
            -targetplatform="WindowsNoEditor"^
            -OutputDir="!BUILD_FOLDER!"^
            -iterate^
            -cooksinglepackage^
        )

        REM remove the props folder if exist
        set PROPS_MAP_FOLDER="%PACKAGE_PATH%/Maps/PropsMap"
        if exist "%PROPS_MAP_FOLDER%" (
        rmdir /S /Q "%PROPS_MAP_FOLDER%"
        )

        popd

        echo Copying files to '!PACKAGE_NAME!'...

        pushd "!BUILD_FOLDER!"

        set SUBST_PATH=!BUILD_FOLDER!\CarlaUE4

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

            REM # copy the traffic manager map file
            set SRC=!BASE_CONTENT!!MAP_FOLDER!\TM\!MAP_NAME!.bin
            set TRG=!BUILD_FOLDER!\CarlaUE4\Content\!MAP_FOLDER!\TM\
            if exist "!SRC!" (
                mkdir "!TRG!"
                copy "!SRC!" "!TRG!"
            )
        )

        rmdir /S /Q "!BUILD_FOLDER!\CarlaUE4\Metadata"
        rmdir /S /Q "!BUILD_FOLDER!\CarlaUE4\Plugins"
        REM del "!BUILD_FOLDER!\CarlaUE4\Content\!PACKAGE_NAME!/Maps/!PROPS_MAP_NAME!"
        del "!BUILD_FOLDER!\CarlaUE4\AssetRegistry.bin"

        popd

        if %SINGLE_PACKAGE%==true (
            if %DO_CLEAN%==true (
                if not exist "!SINGLE_PACKAGE_DIR:/=\!" (
                    move "!BUILD_FOLDER:/=\!" "!SINGLE_PACKAGE_DIR:/=\!"
                ) else (
                    robocopy /move /e /nfl /ndl /njh /njs /np "!BUILD_FOLDER:/=\!" "!SINGLE_PACKAGE_DIR:/=\!"
                    if exist "!BUILD_FOLDER:/=\!" rmdir /S /Q "!BUILD_FOLDER:/=\!"
                )
            ) else (
                robocopy /e /nfl /ndl /njh /njs /np "!BUILD_FOLDER:/=\!" "!SINGLE_PACKAGE_DIR:/=\!"
            )
        )
    )
    call :get_current_time_in_seconds T_END_PACKAGE
    set /A ELAPSED_TIME=!T_END_PACKAGE! - !T_START_PACKAGE!
    if %MEASURE_TIME%==true echo %FILE_N% [TIME]: Cooking package '!PACKAGE_NAME!' took !ELAPSED_TIME! seconds.
)

rem Create tagged materials for the instance segmentation for all packages at once
if not "!RESULT!" == "Carla" (
    call :cook_tagged_materials "!RESULT!" _
)

rem Zipping and cleaning up for all packages
if %SINGLE_PACKAGE%==true (
    set SRC=!SINGLE_PACKAGE_DIR:/=\!
    if %DO_TARBALL%==true (
        echo Packaging '%TARGET_ARCHIVE%'...
        set DESTINATION_ZIP=%INSTALLATION_DIR%UE4Carla/%TARGET_ARCHIVE%_%CARLA_VERSION%.zip
        call :zip_dir !SRC! !DESTINATION_ZIP!
    )
    if %DO_CLEAN%==true (
        echo %FILE_N% Removing intermediate build.
        rmdir /S /Q "!SRC!"
    )
) else (
    for /f "tokens=* delims=" %%i in ("!PACKAGES!") do (
        set PACKAGE_NAME=%%i
        if not !PACKAGE_NAME! == Carla (
            set SRC=%INSTALLATION_DIR%UE4Carla/!PACKAGE_NAME!_%CARLA_VERSION%
            if %DO_TARBALL%==true (
                echo Packaging '!PACKAGE_NAME!'...
                set DESTINATION_ZIP=%INSTALLATION_DIR%UE4Carla/!PACKAGE_NAME!_%CARLA_VERSION%.zip
                call :zip_dir !SRC! !DESTINATION_ZIP!
            )

            if %DO_CLEAN%==true (
                echo %FILE_N% Removing intermediate build.
                rmdir /S /Q "!SRC:/=\!"
            )
        )
    )
)

call :get_current_time_in_seconds T_END_PACKAGES
set /A ELAPSED_TIME=!T_END_PACKAGES! - !T_START_PACKAGES!
if %MEASURE_TIME%==true echo %FILE_N% [TIME]: Cooking all other packages took !ELAPSED_TIME! seconds.

rem ============================================================================

call :get_current_time_in_seconds T_END_OVERALL
set /A ELAPSED_TIME=!T_END_OVERALL! - !T_START_OVERALL!
if %MEASURE_TIME%==true echo %FILE_N% [TIME]: Overall packaging took !ELAPSED_TIME! seconds.

goto success

rem ============================================================================
rem -- Helper functions --------------------------------------------------------
rem ============================================================================

:get_git_repository_version
    %ROOT_PATH:/=\%Util\BuildTools\Environment.bat %*

:get_current_time_in_seconds
    for /f %%a in ('powershell -command "[int]((Get-Date -UFormat '%%s') -split ',.')[0]"') do set %1=%%a
    goto :eof

:zip_dir
    set ZD_SOURCE=%~1
    set ZD_SOURCE=!ZD_SOURCE:/=\!
    set ZD_DST_ZIP=%~2
    set ZD_DST_ZIP=!ZD_DST_ZIP:/=\!

    pushd "!ZD_SOURCE!"
    if exist "%ProgramW6432%/7-Zip/7z.exe" (
        "%ProgramW6432%/7-Zip/7z.exe" a "!ZD_DST_ZIP!" . -tzip -mmt -mx5
    ) else (
        rem https://docs.microsoft.com/en-us/powershell/module/microsoft.powershell.archive/compress-archive?view=powershell-6
        powershell -command "& { Compress-Archive -Update -Path * -CompressionLevel Fastest -DestinationPath '!ZD_DST_ZIP!' }"
    )
    popd

    if errorlevel 1 goto bad_exit
    echo ZIP created at !DESTINATION_ZIP!

    goto :eof

:cook_tagged_materials
    rem Measure duration of this function call
    call :get_current_time_in_seconds T_START_COOK_TAGGED_MATS

    rem Read input parameters.
    set CUR_PACKAGES_CSV=%~1
    set CUR_PACKAGES_CSV=%CUR_PACKAGES_CSV: =%
    set CUR_PACKAGES_UE=%CUR_PACKAGES_CSV:,=+%
    set RESULT_BUILD_DIR=%2
    set REGISTRIES_SUBDIR=CarlaUE4/Plugins/Carla/Content/PostProcessingMaterials/TaggedMaterials
    set TEMP_BUILD_DIR=%INSTALLATION_DIR%UE4Carla/%CARLA_VERSION%_TaggedMaterials

    rem Call commandlet to build TaggedMaterialsRegistries for the current package(s).
    rem Temporary maps containing the TaggedMaterialsRegistries will be created,
    rem that can be cooked to cook the registries.
    echo.
    echo Generate TaggedMaterialsRegistry for package(s) '!CUR_PACKAGES_CSV!'...
    if %SINGLE_PACKAGE%==true (
        set TARGET_ARCHIVE_OPTION=-TargetArchive="%TARGET_ARCHIVE%"
    ) else (
        set TARGET_ARCHIVE_OPTION=
    )
    call "%UE4_ROOT%/Engine/Binaries/Win64/UE4Editor.exe "^
    "%ROOT_PATH%Unreal/CarlaUE4/CarlaUE4.uproject"^
    -run=GenerateTaggedMaterialsRegistry^
    -PackageNames="!CUR_PACKAGES_UE!" !TARGET_ARCHIVE_OPTION!^
    -NoShaderCompile

    rem Construct string for all maps to cook all at once
    if %SINGLE_PACKAGE%==true (
        set MAPS_COOKING_STR=/Carla/PostProcessingMaterials/TaggedMaterials/TaggedMaterials_%TARGET_ARCHIVE%_Map
    ) else (
        for %%a in (!CUR_PACKAGES_CSV!) do (
            set MAP_PATH=/Carla/PostProcessingMaterials/TaggedMaterials/TaggedMaterials_%%a_Map
            if defined MAPS_COOKING_STR (
                set MAPS_COOKING_STR=!MAPS_COOKING_STR!+!MAP_PATH!
            ) else (
                set MAPS_COOKING_STR=!MAP_PATH!
            )
        )
    )

    rem Cook the temporary map(s).
    echo Cook TaggedMaterialsRegistries
    call "%UE4_ROOT%/Engine/Binaries/Win64/UE4Editor.exe "^
    "%ROOT_PATH%Unreal/CarlaUE4/CarlaUE4.uproject"^
    -run=cook^
    -map="!MAPS_COOKING_STR!"^
    -targetplatform="WindowsNoEditor"^
    -OutputDir="!TEMP_BUILD_DIR!"^
    -iterate^
    -cooksinglepackage^

    rem Move/Copy the generated files to the correct packages (zipping happens later on)
    echo Copy cooked TaggedMaterialsRegistries
    if "!CUR_PACKAGES_CSV!" == "Carla" (
        rem For Carla package, we only move the files to target dir
        set SRC=!TEMP_BUILD_DIR!/!REGISTRIES_SUBDIR!/TaggedMaterials_Carla
        set TRG=!RESULT_BUILD_DIR!!REGISTRIES_SUBDIR!
        if not exist "!TRG:/=\!" mkdir "!TRG:/=\!"
        move "!SRC:/=\!.*" "!TRG:/=\!"
    ) else if %SINGLE_PACKAGE%==true (
        rem Move the monolithic TaggedMaterialsRegistry to the single package dir
        set SRC=!TEMP_BUILD_DIR!/!REGISTRIES_SUBDIR!/TaggedMaterials_%TARGET_ARCHIVE%
        set TRG=!SINGLE_PACKAGE_DIR!/!REGISTRIES_SUBDIR!
        if exist "!SRC!.uasset" (
            if not exist "!TRG:/=\!" mkdir "!TRG:/=\!"
            move "!SRC:/=\!.*" "!TRG:/=\!"
        )
    ) else (
        rem For other packages, we move/copy the package-related files to their corresponding dirs
        for %%a in (!CUR_PACKAGES_CSV!) do (
            set CUR_PACKAGE=%%a
            set SRC=!TEMP_BUILD_DIR!/!REGISTRIES_SUBDIR!/TaggedMaterials_!CUR_PACKAGE!
            if exist "!SRC:/=\!.uasset" (
                set TRG=%INSTALLATION_DIR%UE4Carla/!CUR_PACKAGE!_%CARLA_VERSION%/!REGISTRIES_SUBDIR!
                if not exist "!TRG:/=\!" mkdir "!TRG:/=\!"
                move "!SRC:/=\!.*" "!TRG:/=\!"
            )
        )
    )
    rem Delete the temporary files.
    rmdir /S /Q "!TEMP_BUILD_DIR:/=\!"

    call :get_current_time_in_seconds T_END_COOK_TAGGED_MATS
    set /A ELAPSED_TIME=!T_END_COOK_TAGGED_MATS! - !T_START_COOK_TAGGED_MATS!
    if %MEASURE_TIME%==true echo %FILE_N% [TIME]: Generating and cooking of TaggedMaterialsRegistry for package(s) '!CUR_PACKAGES_CSV!' took !ELAPSED_TIME! seconds.

    goto :eof


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
