@echo off

setlocal

set LOCAL_PATH=%~dp0

set INSTALLERS_DIR=%LOCAL_PATH%Util\InstallersWin\
set INSTALLATION_DIR=%LOCAL_PATH%Util\Build\
set VERSION_FILE=%LOCAL_PATH%Util\ContentVersions.txt
set CONTENT_DIR=%LOCAL_PATH%Unreal\CarlaUE4\Content

if not exist "%CONTENT_DIR%" (
	echo Creating %CONTENT_DIR% folder...
	mkdir %CONTENT_DIR%
)

echo.
echo  Installing Protobuf...
call %INSTALLERS_DIR%install_proto.bat %INSTALLATION_DIR%

if not %install_proto%==done (
	echo  Failed while installing Protobuf.
	goto failed
)

echo.
echo  Installing Boost...
call %INSTALLERS_DIR%install_boost.bat %INSTALLATION_DIR%

if not %install_boost%==done (
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

:success
	echo.
	echo.
	echo  #############################
	echo  #         IMPORTANT         #
	echo  #############################
	echo.
	echo  All the CARLA library dependences should be installed now.
	echo  You only need the ASSET PACK with all the meshes and textures.
	echo.
	echo  This script provides the optimal assets version %ASSETS_VERSION%
	echo  If you want another asset version, search it in %VERSION_FILE%.
	echo  You can download the assets from here:
	echo.
	echo    %URL%
	echo.
	echo  Unzip it in the "%CONTENT_DIR%" folder.
	echo  After that, please run the "Rebuild.bat".

	goto eof

:failed
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
	echo    https://discord.gg/vNVHXfb

	goto eof

:eof
	endlocal
