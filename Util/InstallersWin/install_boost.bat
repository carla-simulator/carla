@echo off

rem BAT script that downloads and installs a ready to use
rem boost build for CARLA (carla.org).
rem Just put it in `Util/Build` and run it.

setlocal

set LOCAL_PATH=%~dp0
set BUILD_DIR=%1
set FILE_N=---%~n0%~x0:

set B_VERSION=boost-1.64.0
set B_SRC=boost-src
set B_SRC_DIR=%BUILD_DIR%%B_SRC%
set B_INSTALL=boost-install
set B_INSTALL_DIR=%BUILD_DIR%%B_INSTALL%

set B_TOOLSET=msvc-14.1
set B_LIB_DIR=%B_INSTALL_DIR%/lib

if not exist "%B_SRC_DIR%" (
	echo %FILE_N% Cloning Boost - version "%B_VERSION%"...
	echo.
	call git clone --depth=1 -b %B_VERSION% ^
		--recurse-submodules -j8 ^
		https://github.com/boostorg/boost.git %B_SRC_DIR%
	echo.
) else (
	echo %FILE_N% Not cloning boost because already exists a folder called "%B_SRC%".
)

if exist "%B_INSTALL_DIR%" (
	goto already_build
)

cd %B_SRC_DIR%
echo %FILE_N% Generating build...
call bootstrap.bat

echo %FILE_N% Building...
b2 -j8 ^
	headers ^
	--with-system ^
	--with-date_time ^
	--build-dir=./build ^
	architecture=x86 ^
	address-model=64 ^
	toolset=%B_TOOLSET% ^
	variant=release ^
	link=static ^
	threading=multi ^
	install ^
	--prefix=%B_INSTALL_DIR% ^
	--libdir=%B_LIB_DIR% ^
	--includedir=%B_INSTALL_DIR%
cd %BUILD_DIR%

move "%B_INSTALL_DIR%/boost-1_64/boost" "%B_INSTALL_DIR%/"
rd /s /q "%B_INSTALL_DIR%/boost-1_64"

rem Remove the downloaded protobuf source because is no more needed
rem if you want to keep the source just delete the following command.
rem @rd /s /q %B_SRC_DIR% 2>nul

:success
	echo.
	echo %FILE_N% Boost has been successfully installed in %B_INSTALL_DIR%!
	goto eof

:already_build
	echo %FILE_N% Library has already been built.
	echo %FILE_N% Delete "%B_INSTALL_DIR%" if you want to force a rebuild.
	goto eof

:eof
	echo %FILE_N% Exiting...
	endlocal
	set install_boost=done
