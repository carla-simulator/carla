@echo off

rem BAT script that downloads and installs a ready to use
rem boost build for CARLA (carla.org).
rem Just put it in `Util/Build` and run it.

setlocal

:arg-parse
if not "%1"=="" (
    if "%1"=="-j" (
        set NUMBER_OF_ASYNC_JOBS=%2
        shift
    )
    if "%1"=="--build-dir" (
        set BUILD_DIR=%2
        shift
    )
    if "%1"=="--toolset" (
        set B_TOOLSET=%2
        shift
    )
    shift
    goto :arg-parse
)

if [%BUILD_DIR%] == [] set BUILD_DIR=.
if [%B_TOOLSET%] == [] set B_TOOLSET=msvc-14.1
if [%NUMBER_OF_ASYNC_JOBS%] == [] set NUMBER_OF_ASYNC_JOBS=1

set LOCAL_PATH=%~dp0
set FILE_N=---%~n0%~x0:

set B_VERSION=boost-1.64.0
set B_SRC=boost-src
set B_SRC_DIR=%BUILD_DIR%%B_SRC%
set B_INSTALL=boost-install
set B_INSTALL_DIR=%BUILD_DIR%%B_INSTALL%
set B_LIB_DIR=%B_INSTALL_DIR%\lib

if exist "%B_INSTALL_DIR%" (
	goto already_build
)

if not exist "%B_SRC_DIR%" (
	echo %FILE_N% Cloning Boost - version "%B_VERSION%"...
	echo.
	call git clone --depth=1 -b %B_VERSION%^
		--recurse-submodules -j8^
		https://github.com/boostorg/boost.git %B_SRC_DIR%
	if errorlevel 1 goto error_git
	echo.
) else (
	echo %FILE_N% Not cloning boost because already exists a folder called "%B_SRC%".
)

cd %B_SRC_DIR%
if not exist "b2.exe" (
	echo %FILE_N% Generating build...
	call bootstrap.bat
)

if errorlevel 1 goto error_bootstrap

echo %FILE_N% Packing headers...
b2 headers

echo %FILE_N% Building...
b2 -j8^
	headers^
	--layout=versioned^
	--with-system^
	--with-date_time^
	--build-dir=./build^
	architecture=x86^
	address-model=64^
	toolset=%B_TOOLSET%^
	variant=release^
	link=static^
	threading=multi^
	--prefix=%B_INSTALL_DIR%^
	--libdir=%B_LIB_DIR%^
	--includedir=%B_INSTALL_DIR%^
	install

if errorlevel 1 goto error_install

cd %BUILD_DIR%

rem Remove the downloaded protobuf source because is no more needed
rem if you want to keep the source just delete the following command.
rem @rd /s /q %B_SRC_DIR%

goto success

:success
	echo.
	echo %FILE_N% Boost has been successfully installed in %B_INSTALL_DIR%!
	goto good_exit

:already_build
	echo %FILE_N% A Boost installation already exists.
	echo %FILE_N% Delete "%B_INSTALL_DIR%" if you want to force a rebuild.
	goto good_exit

:error_git
	echo.
	echo %FILE_N% [GIT ERROR] An error ocurred while executing the git.
	echo %FILE_N% [GIT ERROR] Possible causes:
	echo %FILE_N%              - Make sure "git" is installed.
	echo %FILE_N%              - Make sure it is available on your Windows "path".
	goto bad_exit

:error_bootstrap
	echo.
	echo %FILE_N% [BOOTSTRAP ERROR] An error ocurred while executing "bootstrap.bat".
	goto bad_exit

:error_install
	echo.
	echo %FILE_N% [B2 ERROR] An error ocurred while installing using "b2.exe".
	goto bad_exit

:good_exit
	echo %FILE_N% Exiting...
	endlocal
	set install_boost=done
	goto:EOF

:bad_exit
	if exist "%B_INSTALL_DIR%" rd /s /q "B_INSTALL_DIR"
	echo %FILE_N% Exiting with error...
	endlocal
	goto:EOF
