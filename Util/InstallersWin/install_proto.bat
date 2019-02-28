@echo off
setlocal

rem BAT script that downloads and installs a ready to use
rem protobuf build for CARLA (carla.org).
rem Just putit in `Util/Build` and run it through a cmd
rem with the x64 Visual C++ Toolset enabled.

rem Print batch params (debug purpose)
echo %FILE_N% [Batch params]: %*

rem ============================================================================
rem -- Parse arguments ---------------------------------------------------------
rem ============================================================================

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
    shift
    goto :arg-parse
)

if [%BUILD_DIR%] == [] set BUILD_DIR=%~dp0
if [%NUMBER_OF_ASYNC_JOBS%] == [] set NUMBER_OF_ASYNC_JOBS=1

set LOCAL_PATH=%~dp0
set FILE_N=---%~n0%~x0:

set P_VERSION=v3.3.2
set P_SRC=protobuf-src
set P_SRC_DIR=%BUILD_DIR%%P_SRC%
set P_INSTALL=protobuf-install
set P_INSTALL_DIR=%BUILD_DIR%%P_INSTALL%

if exist "%P_INSTALL_DIR%" (
	goto already_build
)

if not exist "%P_SRC_DIR%" (
	echo %FILE_N% Cloning Protobuf - version "%P_VERSION%"...
	echo.
	call git clone --depth=1 -b %P_VERSION%^
		--recurse-submodules -j8^
		https://github.com/google/protobuf.git %P_SRC_DIR%
	if errorlevel 1 goto error_git
	echo.

) else (
	echo %FILE_N% Not cloning protobuf because already exists a folder called "%P_SRC%".
)

if not exist "%P_SRC_DIR%\cmake\build" (
	echo %FILE_N% Creating "%P_SRC_DIR%\cmake\build"
	mkdir %P_SRC_DIR%\cmake\build
)

cd %P_SRC_DIR%\cmake\build

echo %FILE_N% Generating build...
cmake -G "NMake Makefiles"^
	-DCMAKE_BUILD_TYPE=Release^
	-Dprotobuf_BUILD_TESTS=OFF^
	-DCMAKE_CXX_FLAGS_RELEASE=/MD^
	-Dprotobuf_MSVC_STATIC_RUNTIME=OFF^
	-DCMAKE_INSTALL_PREFIX=%P_INSTALL_DIR%^
	%P_SRC_DIR%\cmake

if errorlevel 1 goto error_cmake

echo %FILE_N% Building...
nmake & nmake install

if errorlevel 1 goto error_install

rem Remove the downloaded protobuf source because is no more needed
rem if you want to keep the source just delete the following command.
rem rd /s /q %P_SRC_DIR%

goto success

:success
	echo.
	echo %FILE_N% Protobuf has been successfully installed in %P_INSTALL_DIR%!
	goto good_exit

:already_build
	echo %FILE_N% A Protobuf installation already exists.
	echo %FILE_N% Delete "%P_INSTALL_DIR%" if you want to force a rebuild.
	goto good_exit

:error_git
	echo.
	echo %FILE_N% [GIT ERROR] An error ocurred while executing the git.
	echo %FILE_N% [GIT ERROR] Possible causes:
	echo %FILE_N%              - Make sure "git" is installed.
	echo %FILE_N%              - Make sure it is available on your Windows "path".
	goto bad_exit

:error_cmake
	echo.
	echo %FILE_N% [CMAKE ERROR] An error ocurred while executing the cmake.
	echo %FILE_N% [CMAKE ERROR] Possible causes:
	echo %FILE_N%                - Make sure "CMake" is installed.
	echo %FILE_N%                - Make sure it is available on your Windows "path".
	goto bad_exit

:error_install
	echo.
	echo %FILE_N% [NMAKE ERROR] An error ocurred while installing using NMake.
	echo %FILE_N% [NMAKE ERROR] Possible causes:
	echo %FILE_N%                - Make sure you have Visual Studio installed.
	echo %FILE_N%                - Make sure you have the "x64 Visual C++ Toolset" in your path.
	echo %FILE_N%                  For example using the "Visual Studio x64 Native Tools Command Prompt",
	echo %FILE_N%                  or the "vcvarsall.bat".
	goto bad_exit

:good_exit
	echo %FILE_N% Exiting...
	endlocal
	set install_proto=done
	goto:EOF

:bad_exit
	if exist "%B_INSTALL_DIR%" rd /s /q "B_INSTALL_DIR"
	echo %FILE_N% Exiting with error...
	endlocal
	goto:EOF
