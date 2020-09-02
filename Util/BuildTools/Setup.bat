@echo off
setlocal

rem BAT script that downloads and generates
rem rpclib, gtest and boost libraries for CARLA (carla.org).
rem Run it through a cmd with the x64 Visual C++ Toolset enabled.

set LOCAL_PATH=%~dp0
set FILE_N=-[%~n0]:

rem Print batch params (debug purpose)
echo %FILE_N% [Batch params]: %*

rem ============================================================================
rem -- Check for compiler ------------------------------------------------------
rem ============================================================================

where cl 1>nul
if %errorlevel% neq 0 goto error_cl

rem TODO: check for x64 and not x86 or x64_x86

rem ============================================================================
rem -- Parse arguments ---------------------------------------------------------
rem ============================================================================

set BOOST_VERSION=1.72.0
set INSTALLERS_DIR=%ROOT_PATH:/=\%Util\InstallersWin\
set VERSION_FILE=%ROOT_PATH:/=\%Util\ContentVersions.txt
set CONTENT_DIR=%ROOT_PATH:/=\%Unreal\CarlaUE4\Content\Carla\

:arg-parse
if not "%1"=="" (
    if "%1"=="-j" (
        set NUMBER_OF_ASYNC_JOBS=%2
    )
    if "%1"=="--boost-toolset" (
        set TOOLSET=%2
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

rem If not defined, use Visual Studio 2017 as tool set
if "%TOOLSET%" == "" set TOOLSET=msvc-14.1

rem If is not set, set the number of parallel jobs to the number of CPU threads
if "%NUMBER_OF_ASYNC_JOBS%" == "" set NUMBER_OF_ASYNC_JOBS=%NUMBER_OF_PROCESSORS%

rem ============================================================================
rem -- Basic info and setup ----------------------------------------------------
rem ============================================================================

set INSTALLATION_DIR=%INSTALLATION_DIR:/=\%

echo %FILE_N% Asynchronous jobs:  %NUMBER_OF_ASYNC_JOBS%
echo %FILE_N% Boost toolset:      %TOOLSET%
echo %FILE_N% Install directory:  "%INSTALLATION_DIR%"

if not exist "%CONTENT_DIR%" (
    echo %FILE_N% Creating "%CONTENT_DIR%" folder...
    mkdir "%CONTENT_DIR%"
)

if not exist "%INSTALLATION_DIR%" (
    echo %FILE_N% Creating "%INSTALLATION_DIR%" folder...
    mkdir "%INSTALLATION_DIR%"
)

rem ============================================================================
rem -- Download and install zlib -----------------------------------------------
rem ============================================================================

echo %FILE_N% Installing zlib...
call "%INSTALLERS_DIR%install_zlib.bat"^
 --build-dir "%INSTALLATION_DIR%"

if %errorlevel% neq 0 goto failed

if not defined install_zlib (
    echo %FILE_N% Failed while installing zlib.
    goto failed
) else (
    set ZLIB_INSTALL_DIR=%install_zlib%
)

rem ============================================================================
rem -- Download and install libpng ---------------------------------------------
rem ============================================================================

echo %FILE_N% Installing libpng...
call "%INSTALLERS_DIR%install_libpng.bat"^
 --build-dir "%INSTALLATION_DIR%"^
 --zlib-install-dir "%ZLIB_INSTALL_DIR%"

if %errorlevel% neq 0 goto failed

if not defined install_libpng (
    echo %FILE_N% Failed while installing libpng.
    goto failed
) else (
    set LIBPNG_INSTALL_DIR=%install_libpng%
)

rem ============================================================================
rem -- Download and install rpclib ---------------------------------------------
rem ============================================================================

echo %FILE_N% Installing rpclib...
call "%INSTALLERS_DIR%install_rpclib.bat"^
 --build-dir "%INSTALLATION_DIR%"

if %errorlevel% neq 0 goto failed

if not defined install_rpclib (
    echo %FILE_N% Failed while installing rpclib.
    goto failed
)

rem ============================================================================
rem -- Download and install Google Test ----------------------------------------
rem ============================================================================

echo %FILE_N% Installing Google Test...
call "%INSTALLERS_DIR%install_gtest.bat"^
 --build-dir "%INSTALLATION_DIR%"

if %errorlevel% neq 0 goto failed

if not defined install_gtest (

    echo %FILE_N% Failed while installing Google Test.
    goto failed
)

rem ============================================================================
rem -- Download and install Recast & Detour ------------------------------------
rem ============================================================================

echo %FILE_N% Installing "Recast & Detour"...
call "%INSTALLERS_DIR%install_recast.bat"^
 --build-dir "%INSTALLATION_DIR%"

if %errorlevel% neq 0 goto failed

if not defined install_recast (

    echo %FILE_N% Failed while installing "Recast & Detour".
    goto failed
) else (
    set RECAST_INSTALL_DIR=%install_recast:\=/%
)

rem ============================================================================
rem -- Download and install Boost ----------------------------------------------
rem ============================================================================

echo %FILE_N% Installing Boost...
call "%INSTALLERS_DIR%install_boost.bat"^
 --build-dir "%INSTALLATION_DIR%"^
 --toolset %TOOLSET%^
 --version %BOOST_VERSION%^
 -j %NUMBER_OF_ASYNC_JOBS%

if %errorlevel% neq 0 goto failed

if not defined install_boost (
    echo %FILE_N% Failed while installing Boost.
    goto failed
)

copy /Y "%INSTALLATION_DIR%..\Util\BoostFiles\rational.hpp" "%INSTALLATION_DIR%boost-%BOOST_VERSION%-install\include\boost\rational.hpp"
copy /Y "%INSTALLATION_DIR%..\Util\BoostFiles\read.hpp" "%INSTALLATION_DIR%boost-%BOOST_VERSION%-install\include\boost\geometry\io\wkt\read.hpp"

rem ============================================================================
rem -- Download and install Xercesc ----------------------------------------------
rem ============================================================================

echo %FILE_N% Installing Xercesc...
call "%INSTALLERS_DIR%install_xercesc.bat"^
 --build-dir "%INSTALLATION_DIR%"^

rem ============================================================================
rem -- Assets download URL -----------------------------------------------------
rem ============================================================================

FOR /F "usebackq tokens=1,2" %%i in ("%VERSION_FILE%") do (
    set ASSETS_VERSION=%%i
    set HASH=%%j
)
set URL=http://carla-assets.s3.amazonaws.com/%HASH%.tar.gz

rem ============================================================================
rem -- Generate CMake ----------------------------------------------------------
rem ============================================================================

for /f %%i in ('git describe --tags --dirty --always') do set carla_version=%%i
set CMAKE_INSTALLATION_DIR=%INSTALLATION_DIR:\=/%

echo %FILE_N% Creating "CMakeLists.txt.in"...

set CMAKE_CONFIG_FILE=%INSTALLATION_DIR%CMakeLists.txt.in

 >"%CMAKE_CONFIG_FILE%" echo # Automatically generated by Setup.bat
>>"%CMAKE_CONFIG_FILE%" echo set(CARLA_VERSION %carla_version%)
>>"%CMAKE_CONFIG_FILE%" echo.
>>"%CMAKE_CONFIG_FILE%" echo set(CMAKE_CXX_STANDARD 14)
>>"%CMAKE_CONFIG_FILE%" echo set(CMAKE_CXX_STANDARD_REQUIRED ON)
>>"%CMAKE_CONFIG_FILE%" echo.
>>"%CMAKE_CONFIG_FILE%" echo add_definitions(-D_WIN32_WINNT=0x0600)
>>"%CMAKE_CONFIG_FILE%" echo add_definitions(-DHAVE_SNPRINTF)
>>"%CMAKE_CONFIG_FILE%" echo STRING (REGEX REPLACE "/RTC(su|[1su])" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
>>"%CMAKE_CONFIG_FILE%" echo.
>>"%CMAKE_CONFIG_FILE%" echo add_definitions(-DBOOST_ERROR_CODE_HEADER_ONLY)
>>"%CMAKE_CONFIG_FILE%" echo add_definitions(-DLIBCARLA_IMAGE_WITH_PNG_SUPPORT)
>>"%CMAKE_CONFIG_FILE%" echo.
>>"%CMAKE_CONFIG_FILE%" echo set(BOOST_INCLUDE_PATH "%CMAKE_INSTALLATION_DIR%boost-%BOOST_VERSION%-install/include")
>>"%CMAKE_CONFIG_FILE%" echo set(BOOST_LIB_PATH "%CMAKE_INSTALLATION_DIR%boost-%BOOST_VERSION%-install/lib")
>>"%CMAKE_CONFIG_FILE%" echo.
>>"%CMAKE_CONFIG_FILE%" echo set(RPCLIB_INCLUDE_PATH "%CMAKE_INSTALLATION_DIR%rpclib-install/include")
>>"%CMAKE_CONFIG_FILE%" echo set(RPCLIB_LIB_PATH "%CMAKE_INSTALLATION_DIR%rpclib-install/lib")
>>"%CMAKE_CONFIG_FILE%" echo.
>>"%CMAKE_CONFIG_FILE%" echo if (CMAKE_BUILD_TYPE STREQUAL "Server")
>>"%CMAKE_CONFIG_FILE%" echo   # Prevent exceptions
>>"%CMAKE_CONFIG_FILE%" echo   add_compile_options(/GR-)
>>"%CMAKE_CONFIG_FILE%" echo   add_compile_options(/EHsc)
>>"%CMAKE_CONFIG_FILE%" echo   add_definitions(-DASIO_NO_EXCEPTIONS)
>>"%CMAKE_CONFIG_FILE%" echo   add_definitions(-DBOOST_NO_EXCEPTIONS)
>>"%CMAKE_CONFIG_FILE%" echo   add_definitions(-DLIBCARLA_NO_EXCEPTIONS)
>>"%CMAKE_CONFIG_FILE%" echo   add_definitions(-DPUGIXML_NO_EXCEPTIONS)
>>"%CMAKE_CONFIG_FILE%" echo   # Specific libraries for server
>>"%CMAKE_CONFIG_FILE%" echo   set(GTEST_INCLUDE_PATH "%CMAKE_INSTALLATION_DIR%gtest-install/include")
>>"%CMAKE_CONFIG_FILE%" echo   set(GTEST_LIB_PATH "%CMAKE_INSTALLATION_DIR%gtest-install/lib")
>>"%CMAKE_CONFIG_FILE%" echo elseif (CMAKE_BUILD_TYPE STREQUAL "Client")
>>"%CMAKE_CONFIG_FILE%" echo   # Specific libraries for client
>>"%CMAKE_CONFIG_FILE%" echo   set(ZLIB_INCLUDE_PATH "%ZLIB_INSTALL_DIR:\=/%/include")
>>"%CMAKE_CONFIG_FILE%" echo   set(ZLIB_LIB_PATH "%ZLIB_INSTALL_DIR:\=/%/lib")
>>"%CMAKE_CONFIG_FILE%" echo   set(LIBPNG_INCLUDE_PATH "%LIBPNG_INSTALL_DIR:\=/%/include")
>>"%CMAKE_CONFIG_FILE%" echo   set(LIBPNG_LIB_PATH "%LIBPNG_INSTALL_DIR:\=/%/lib")
>>"%CMAKE_CONFIG_FILE%" echo   set(RECAST_INCLUDE_PATH "%RECAST_INSTALL_DIR:\=/%/include")
>>"%CMAKE_CONFIG_FILE%" echo   set(RECAST_LIB_PATH "%RECAST_INSTALL_DIR:\=/%/lib")
>>"%CMAKE_CONFIG_FILE%" echo endif ()

goto success

rem ============================================================================
rem -- Messages and Errors -----------------------------------------------------
rem ============================================================================

:success
    echo %FILE_N%
    echo    ###########
    echo    # SUCCESS #
    echo    ###########
    echo.
    echo    IMPORTANT!
    echo.
    echo    All the CARLA library dependences should be installed now.
    echo    (You can remove all "*-src" folders in %INSTALLATION_DIR% directory)
    echo.
    echo    You only need the ASSET PACK with all the meshes and textures.
    echo.
    echo    This script provides the assets for CARLA %ASSETS_VERSION%
    echo    You can download the assets from here:
    echo.
    echo        %URL%
    echo.
    echo    Unzip it in the "%CONTENT_DIR%" folder.
    echo    If you want another version, search it in %VERSION_FILE%.
    echo.
    goto good_exit

:help
    echo  Download and compiles all the necessary libraries to build CARLA.
    echo.
    echo  Commands:
    echo     -h, --help          -^> Shows this dialog.
    echo     -j ^<N^>            -^> N is the integer number of async jobs while compiling (default=1).
    echo     --boost-toolset [T] -^> Toolset corresponding to your compiler ^(default=^*^):
    echo                               Visual Studio 2013 -^> msvc-12.0
    echo                               Visual Studio 2015 -^> msvc-14.0
    echo                               Visual Studio 2017 -^> msvc-14.1 *
    goto good_exit

:error_cl
    echo.
    echo %FILE_N% [ERROR] Can't find Visual Studio compiler (cl.exe).
    echo           [ERROR] Possible causes:
    echo           [ERROR]  - Make sure you use x64 (not x64_x86!)
    echo           [ERROR]  - You are not using "Visual Studio x64 Native Tools Command Prompt".
    goto failed

:failed
    echo.
    echo %FILE_N%
    echo    Ok, and error ocurred, don't panic!
    echo    We have different platforms where you can find some help :)
    echo.
    echo    - Make sure you have read the documentation:
    echo        http://carla.readthedocs.io/en/latest/how_to_build_on_windows/
    echo.
    echo    - If the problem persists, you can ask on our Github's "Building on Windows" issue:
    echo        https://github.com/carla-simulator/carla/issues/21
    echo.
    echo    - Or just use our Discord channel!
    echo        We'll be glad to help you there :)
    echo        https://discord.gg/42KJdRj
    endlocal
    exit /b %errorlevel%

:good_exit
    endlocal
    exit /b 0
