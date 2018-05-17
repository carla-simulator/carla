@echo off

setlocal

set LOCAL_PATH=%~dp0

set LIB_FOLDER=%LOCAL_PATH%Util\Build\
set CARLA_FOLDER=%LOCAL_PATH%Unreal\CarlaUE4\

if not exist "%LIB_FOLDER%" (
	echo Creating %LIB_FOLDER% folder...
	mkdir %LIB_FOLDER%
)

echo Deleting intermediate folders...
for %%G in (
	"%CARLA_FOLDER%Binaries",
	"%CARLA_FOLDER%Intermediate",
	"%CARLA_FOLDER%Plugins\Carla\Binaries",
	"%CARLA_FOLDER%Plugins\Carla\Intermediate"
) do (
	if exist %%G (
		echo Deleting: %%G
		rmdir /s/q %%G
	)
)

echo.
echo Building carlaserver...
make clean & make

echo.
echo Launch editor...
start %CARLA_FOLDER%CarlaUE4.uproject

endlocal
