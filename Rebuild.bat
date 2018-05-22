@echo off

setlocal

set LOCAL_PATH=%~dp0
set FILE_N=---%~n0%~x0:

set LIB_FOLDER=%LOCAL_PATH%Util\Build\
set CARLA_FOLDER=%LOCAL_PATH%Unreal\CarlaUE4\

if not exist "%LIB_FOLDER%" (
	echo %FILE_N% Creating %LIB_FOLDER% folder...
	mkdir %LIB_FOLDER%
)

echo %FILE_N% Deleting intermediate folders...
for %%G in (
	"%CARLA_FOLDER%Binaries",
	"%CARLA_FOLDER%Intermediate",
	"%CARLA_FOLDER%Plugins\Carla\Binaries",
	"%CARLA_FOLDER%Plugins\Carla\Intermediate"
) do (
	if exist %%G (
		echo %FILE_N% Deleting: %%G
		rmdir /s/q %%G
	)
)

echo.
echo %FILE_N% Building carlaserver...
make clean & make

if errorlevel 1 goto compiling_error

echo.
echo %FILE_N% Launch editor...
start %CARLA_FOLDER%CarlaUE4.uproject

:success
	goto eof

:compiling_error
	echo.
	echo.
	echo %FILE_N% Error while compiling carlaserver
	goto eof

:eof
	endlocal
