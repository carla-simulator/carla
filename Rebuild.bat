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
echo %FILE_N% Launching Unreal Editor...
start %CARLA_FOLDER%CarlaUE4.uproject

:success
	goto eof

:error_git
	echo %FILE_N% [GIT ERROR] An error ocurred while executing the git.
	echo %FILE_N% [GIT ERROR] Possible causes:
	echo %FILE_N%              - Make sure "git" is installed.
	echo %FILE_N%              - Make sure it is available on your Windows "path".
	goto bad_exit

:compiling_error
	echo.
	echo.
	echo %FILE_N% Error while compiling carlaserver
	goto eof

:eof
	endlocal
