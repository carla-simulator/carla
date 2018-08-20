@echo off

setlocal

set LOCAL_PATH=%~dp0

set INSTALLERS_DIR=%LOCAL_PATH%Util\InstallersWin\
set INSTALLATION_DIR=%LOCAL_PATH%Util\Build\
set VERSION_FILE=%LOCAL_PATH%Util\ContentVersions.txt
set CONTENT_DIR=%LOCAL_PATH%Unreal\CarlaUE4\Content

:arg-parse
if not "%1"=="" (
    if "%1"=="-j" (
        set NUMBER_OF_ASYNC_JOBS=%2
        shift
    )
    if "%1"=="--boost-toolset" (
        set TOOLSET=%2
        shift
    )
    if "%1"=="-h" (
    	goto help
    )
    if "%1"=="--help" (
    	goto help
    )
    shift
    goto :arg-parse
)

if [%TOOLSET%] == [] set TOOLSET=msvc-14.1
if [%NUMBER_OF_ASYNC_JOBS%] == [] set NUMBER_OF_ASYNC_JOBS=1

echo Asynchronous jobs:  %NUMBER_OF_ASYNC_JOBS%
echo Boost toolset:      %TOOLSET%
echo Install directory:  %INSTALLATION_DIR%

if not exist "%CONTENT_DIR%" (
	echo Creating %CONTENT_DIR% folder...
	mkdir %CONTENT_DIR%
)

echo.
echo  Installing Protobuf...
call %INSTALLERS_DIR%install_proto.bat ^
	--build-dir %INSTALLATION_DIR% ^
	-j %NUMBER_OF_ASYNC_JOBS%

if not defined install_proto (
	echo.
	echo  Failed while installing Protobuf.
	goto failed
)

echo.
echo  Installing Boost...
call %INSTALLERS_DIR%install_boost.bat ^
	--build-dir %INSTALLATION_DIR% ^
	--toolset %TOOLSET% ^
	-j %NUMBER_OF_ASYNC_JOBS%

if not defined install_boost (
	echo.
	echo  Failed while installing Boost.
	goto failed
)

FOR /F "tokens=2" %%i in (%VERSION_FILE%) do (
	set HASH=%%i
)
set URL=https://drive.google.com/open?id=%HASH%

FOR /F "tokens=1 delims=:" %%i in (%VERSION_FILE%) do (
	set ASSETS_VERSION=%%i
)

goto success

:success
	echo.
	echo.
	echo  ###########
	echo  # SUCCESS #
	echo  ###########
	echo.
	echo  IMPORTANT!
	echo.
	echo  All the CARLA library dependences should be installed now.
	echo  (You can remove all "*-src" folders in %INSTALLATION_DIR% directory)
	echo.
	echo  You only need the ASSET PACK with all the meshes and textures.
	echo.
	echo  This script provides the assets for CARLA %ASSETS_VERSION%
	echo  If you want another asset version, search it in %VERSION_FILE%.
	echo  You can download the assets from here:
	echo.
	echo    %URL%
	echo.
	echo  Unzip it in the "%CONTENT_DIR%" folder.
	echo  After that, please run the "Rebuild.bat".

	goto eof

:help
	echo  --------
	echo    HELP
	echo  --------
	echo.
	echo  Commands:
	echo    -h, --help          -^> Shows this dialog.
	echo    -j ^<N^>              -^> N is the integer number of async jobs while compiling (default=1).
	echo    --boost-toolset [T] -^> Toolset corresponding to your compiler ^(default=^*^):
	echo                               Visual Studio 2013 -^> msvc-12.0
	echo                               Visual Studio 2015 -^> msvc-14.0
	echo                               Visual Studio 2017 -^> msvc-14.1 *

	goto eof

:failed
	echo.
	echo  Ok, and error ocurred, don't panic!
	echo  We have different platforms where you can find some help :)
	echo.
	echo  - First of all you can try to build CARLA manually:
	echo    http://carla.readthedocs.io/en/latest/how_to_build_on_windows/
	echo.
	echo  - If the problem persists, you can ask on our Github's "Building on Windows" issue:
	echo    https://github.com/carla-simulator/carla/issues/21
	echo.
	echo  - Or just use our Discord channel!
	echo    We'll be glad to help you there :)
	echo    https://discord.gg/42KJdRj

	goto eof

:eof
	endlocal
