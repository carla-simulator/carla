@echo off

setlocal

set LOCAL_PATH=%~dp0
set FILE_N=%~n0%~x0

rem ============================================================================
rem -- Parse arguments ---------------------------------------------------------
rem ============================================================================

set DOC_STRING=Build LibCarla.

set "USAGE_STRING=Usage: %FILE_N% [-h^|--help] [--rebuild] [--server] [--client] [--clean]"

set REMOVE_INTERMEDIATE=false
set BUILD_SERVER=false
set BUILD_CLIENT=false

:arg-parse
if not "%1"=="" (
    if "%1"=="--rebuild" (
        set REMOVE_INTERMEDIATE=true
        set BUILD_SERVER=true
        set BUILD_CLIENT=true
    )

    if "%1"=="--server" (
        set BUILD_SERVER=true
    )

    if "%1"=="--client" (
        set BUILD_CLIENT=true
    )

    if "%1"=="--clean" (
        set REMOVE_INTERMEDIATE=true
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

if %REMOVE_INTERMEDIATE% == false (
    if %BUILD_SERVER% == false (
        if %BUILD_CLIENT% == false (
          echo Nothing selected to be done.
          goto :eof
        )
    )
)

echo.
echo.

rem Set the visual studio solution directory
rem
set ROOT_PATH=%CD%
set "FILE_N=    -[%~n0]:"
set LIBCARLA_VSPROJECT_PATH=%ROOT_PATH%\Build\libcarla-visualstudio

set LIBCARLA_SERVER_INSTALL_PATH=%ROOT_PATH%\Unreal\CarlaUE4\Plugins\Carla\CarlaDependencies
set LIBCARLA_CLIENT_INSTALL_PATH=%ROOT_PATH%\Build\libcarla-client-install

echo Visual Studio solution: %LIBCARLA_VSPROJECT_PATH%
echo LibCarla server instalation path:       %LIBCARLA_SERVER_INSTALL_PATH%
echo LibCarla client instalation path:       %LIBCARLA_CLIENT_INSTALL_PATH%
echo.

if %REMOVE_INTERMEDIATE% == true (
    echo Cleaning project solution
    rmdir /S /Q %LIBCARLA_VSPROJECT_PATH%

    echo Cleaning client instalation
    rmdir /S /Q %LIBCARLA_CLIENT_INSTALL_PATH%

    echo Cleaning server instalation
    rmdir /S /Q %LIBCARLA_SERVER_INSTALL_PATH%
    echo.
)

if not exist %LIBCARLA_VSPROJECT_PATH% mkdir %LIBCARLA_VSPROJECT_PATH%
pushd %LIBCARLA_VSPROJECT_PATH%

rem Build libcarla server
rem
if %BUILD_SERVER% == true (
    cmake -G "Visual Studio 15 2017 Win64" -DCMAKE_BUILD_TYPE=Server -DCMAKE_INSTALL_PREFIX=%LIBCARLA_SERVER_INSTALL_PATH% %ROOT_PATH%
    cmake --build . --config Release --target install
)

rem Build libcarla client
rem
if %BUILD_CLIENT% == true (
    cmake -G "Visual Studio 15 2017 Win64" -DCMAKE_BUILD_TYPE=Client -DCMAKE_INSTALL_PREFIX=%LIBCARLA_CLIENT_INSTALL_PATH% %ROOT_PATH%
    cmake --build . --config Release --target install
)

goto :eof

rem TO DELTE
echo DEBUG: LAST LINE
