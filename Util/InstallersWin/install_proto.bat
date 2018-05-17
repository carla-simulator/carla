@echo off

rem BAT script that downloads and installs a ready to use
rem protobuf build for CARLA (carla.org).
rem Just putit in `Util/Build` and run it through a cmd 
rem with the x64 Visual C++ Toolset enabled.

setlocal

set LOCAL_PATH=%~dp0
set BUILD_DIR=%1
set FILE_N=---%~n0%~x0:

set P_VERSION=v3.3.2
set P_SRC=protobuf-src
set P_SRC_DIR=%BUILD_DIR%%P_SRC%
set P_INSTALL=protobuf-install
set P_INSTALL_DIR=%BUILD_DIR%%P_INSTALL%

if not exist "%P_SRC_DIR%" (
	echo %FILE_N% Cloning Protobuf - version "%P_VERSION%"...
	echo.
	call git clone --depth=1 -b %P_VERSION% https://github.com/google/protobuf.git %P_SRC_DIR%
	echo.
) else (
	echo %FILE_N% Not cloning protobuf because already exists a folder called "%P_SRC%".
)

if exist "%P_INSTALL_DIR%" (
	goto already_build
)

if not exist "%P_SRC_DIR%\cmake\build" (
	echo %FILE_N% Creating "%P_SRC_DIR%\cmake\build"
	mkdir %P_SRC_DIR%\cmake\build
)

cd %P_SRC_DIR%\cmake\build

echo %FILE_N% Generating build...
cmake -G "NMake Makefiles" ^
	-DCMAKE_BUILD_TYPE=Release ^
	-Dprotobuf_BUILD_TESTS=OFF ^
	-DCMAKE_CXX_FLAGS_RELEASE=/MD ^
	-Dprotobuf_MSVC_STATIC_RUNTIME=OFF ^
	-DCMAKE_INSTALL_PREFIX=%P_INSTALL_DIR% ^
	%P_SRC_DIR%\cmake

echo %FILE_N% Building...
nmake & nmake install

rem Remove the downloaded protobuf source because is no more needed
rem if you want to keep the source just delete the following command.
rem rd /s /q %P_SRC_DIR% 2>nul

:success
	echo.
	echo %FILE_N% Protobuf has been successfully installed in %P_INSTALL_DIR%!
	goto eof

:already_build
	echo %FILE_N% Library has already been built.
	echo %FILE_N% Delete "%P_INSTALL_DIR%" if you want to force a rebuild.
	goto eof

:eof
	echo %FILE_N% Exiting...
	endlocal
	set install_proto=done
