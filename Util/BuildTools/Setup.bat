@echo off
setlocal enabledelayedexpansion

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

set BOOST_VERSION=1.80.0
set INSTALLERS_DIR=%ROOT_PATH:/=\%Util\InstallersWin\
set VERSION_FILE=%ROOT_PATH:/=\%Util\ContentVersions.txt
set CONTENT_DIR=%ROOT_PATH:/=\%Unreal\CarlaUE4\Content\Carla\
set CARLA_DEPENDENCIES_FOLDER=%ROOT_PATH:/=\%Unreal\CarlaUE4\Plugins\Carla\CarlaDependencies\
set CARLA_BINARIES_FOLDER=%ROOT_PATH:/=\%Unreal\CarlaUE4\Plugins\Carla\Binaries\Win64
set CARLA_PYTHON_DEPENDENCIES=%ROOT_PATH:/=\%PythonAPI\carla\dependencies\
set USE_CHRONO=false
set USE_ROS2=false

:arg-parse
if not "%1"=="" (
    if "%1"=="-j" (
        set NUMBER_OF_ASYNC_JOBS=%2
    )
    if "%1"=="--boost-toolset" (
        set TOOLSET=%2
    )
    if "%1"=="--chrono" (
        set USE_CHRONO=true
    )
    if "%1"=="--ros2" (
        set USE_ROS2=true
    )
    if "%1" == "--generator" (
        set GENERATOR=%2
        shift
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

rem If not defined, use Visual Studio 2019 as tool set
if "%TOOLSET%" == "" set TOOLSET=msvc-14.2
if %GENERATOR% == "" set GENERATOR="Visual Studio 16 2019"

rem If is not set, set the number of parallel jobs to the number of CPU threads
if "%NUMBER_OF_ASYNC_JOBS%" == "" set NUMBER_OF_ASYNC_JOBS=%NUMBER_OF_PROCESSORS%

rem ============================================================================
rem -- Basic info and setup ----------------------------------------------------
rem ============================================================================

set INSTALLATION_DIR=%INSTALLATION_DIR:/=\%

echo %FILE_N% Asynchronous jobs:  %NUMBER_OF_ASYNC_JOBS%
echo %FILE_N% Boost toolset:      %TOOLSET%
echo %FILE_N% Generator:          %GENERATOR%
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
 --build-dir "%INSTALLATION_DIR%"^
 --generator %GENERATOR%

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
 --build-dir "%INSTALLATION_DIR%"^
 --generator %GENERATOR%

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
 --build-dir "%INSTALLATION_DIR%"^
 --generator %GENERATOR%

if %errorlevel% neq 0 goto failed

if not defined install_recast (

    echo %FILE_N% Failed while installing "Recast & Detour".
    goto failed
) else (
    set RECAST_INSTALL_DIR=%install_recast:\=/%
)

rem ============================================================================
rem -- Download and install Fast-DDS (for ROS2)---------------------------------
rem ============================================================================

if %USE_ROS2% == true (
    echo %FILE_N% Installing "Fast-DDS"...
    call "%INSTALLERS_DIR%install_fastDDS.bat"^
    --build-dir "%INSTALLATION_DIR%"

    if %errorlevel% neq 0 goto failed

    if not defined install_dds (

        echo %FILE_N% Failed while installing "Fast-DDS".
        goto failed
    ) else (
        set FASTDDS_INSTALL_DIR=%install_dds:\=/%
    )
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

rem ============================================================================
rem -- Download and install Xercesc --------------------------------------------
rem ============================================================================

echo %FILE_N% Installing Xercesc...
call "%INSTALLERS_DIR%install_xercesc.bat"^
 --build-dir "%INSTALLATION_DIR%"^
 --generator %GENERATOR%
copy %INSTALLATION_DIR%\xerces-c-3.2.3-install\lib\xerces-c_3.lib %CARLA_PYTHON_DEPENDENCIES%\lib
copy %INSTALLATION_DIR%\xerces-c-3.2.3-install\lib\xerces-c_3.lib %CARLA_DEPENDENCIES_FOLDER%\lib

rem ============================================================================
rem -- Download and install Sqlite3 --------------------------------------------
rem ============================================================================
echo %FILE_N% Installing Sqlite3
call "%INSTALLERS_DIR%install_sqlite3.bat"^
 --build-dir "%INSTALLATION_DIR%"
copy %INSTALLATION_DIR%\sqlite3-install\lib\sqlite3.lib %CARLA_PYTHON_DEPENDENCIES%\lib
copy %INSTALLATION_DIR%\sqlite3-install\lib\sqlite3.lib %CARLA_DEPENDENCIES_FOLDER%\lib

rem ============================================================================
rem -- Download and install PROJ --------------------------------------------
rem ============================================================================

echo %FILE_N% Installing PROJ
call "%INSTALLERS_DIR%install_proj.bat"^
 --build-dir "%INSTALLATION_DIR%"^
 --generator %GENERATOR%
copy %INSTALLATION_DIR%\proj-install\lib\proj.lib %CARLA_PYTHON_DEPENDENCIES%\lib
copy %INSTALLATION_DIR%\proj-install\lib\proj.lib %CARLA_DEPENDENCIES_FOLDER%\lib

rem ============================================================================
rem -- Download and install Eigen ----------------------------------------------
rem ============================================================================

echo %FILE_N% Installing Eigen
call "%INSTALLERS_DIR%install_eigen.bat"^
 --build-dir "%INSTALLATION_DIR%"
xcopy /Y /S /I "%INSTALLATION_DIR%eigen-install\include\*" "%CARLA_DEPENDENCIES_FOLDER%include\*" > NUL

rem ============================================================================
rem -- Download and install Chrono ----------------------------------------------
rem ============================================================================

if %USE_CHRONO% == true (
    echo %FILE_N% Installing Chrono...
    call "%INSTALLERS_DIR%install_chrono.bat"^
     --build-dir "%INSTALLATION_DIR%" ^
     --generator %GENERATOR%

    if not exist "%CARLA_DEPENDENCIES_FOLDER%" (
        mkdir "%CARLA_DEPENDENCIES_FOLDER%"
    )
    if not exist "%CARLA_DEPENDENCIES_FOLDER%include" (
        mkdir "%CARLA_DEPENDENCIES_FOLDER%include"
    )
    if not exist "%CARLA_DEPENDENCIES_FOLDER%lib" (
        mkdir "%CARLA_DEPENDENCIES_FOLDER%lib"
    )
    if not exist "%CARLA_DEPENDENCIES_FOLDER%dll" (
        mkdir "%CARLA_DEPENDENCIES_FOLDER%dll"
    )
    echo "%INSTALLATION_DIR%chrono-install\include\*" "%CARLA_DEPENDENCIES_FOLDER%include\*" > NUL
    xcopy /Y /S /I "%INSTALLATION_DIR%chrono-install\include\*" "%CARLA_DEPENDENCIES_FOLDER%include\*" > NUL
    copy "%INSTALLATION_DIR%chrono-install\lib\*.lib" "%CARLA_DEPENDENCIES_FOLDER%lib\*.lib" > NUL
    copy "%INSTALLATION_DIR%chrono-install\bin\*.dll" "%CARLA_DEPENDENCIES_FOLDER%dll\*.dll" > NUL
    xcopy /Y /S /I "%INSTALLATION_DIR%eigen-install\include\*" "%CARLA_DEPENDENCIES_FOLDER%include\*" > NUL
)

REM ==============================================================================
REM -- Download Fast DDS and dependencies ----------------------------------------
REM ==============================================================================

SET FASTDDS_BASENAME=fast-dds
SET FASTDDS_INSTALL_DIR=%CD%\%FASTDDS_BASENAME%-install
SET FASTDDS_INCLUDE=%FASTDDS_INSTALL_DIR%\include
SET FASTDDS_LIB=%FASTDDS_INSTALL_DIR%\lib
IF "%USE_ROS2%"=="true" (

  :build_fastdds_extension
  SET LIB_SOURCE=%1
  SET LIB_REPO=%2
  SET CMAKE_FLAGS=%3

  IF NOT EXIST "%LIB_SOURCE%" (
    mkdir "%LIB_SOURCE%"
    echo %LIB_REPO%
    git clone %LIB_REPO% %LIB_SOURCE%
    mkdir "%LIB_SOURCE%\build"
  )

  IF NOT EXIST "%FASTDDS_INSTALL_DIR%" (
    mkdir "%FASTDDS_INSTALL_DIR%"
    echo Build foonathan memory vendor
    SET FOONATHAN_MEMORY_VENDOR_BASENAME=foonathan-memory-vendor
    SET FOONATHAN_MEMORY_VENDOR_SOURCE_DIR=%CD%\%FOONATHAN_MEMORY_VENDOR_BASENAME%-source
    SET FOONATHAN_MEMORY_VENDOR_REPO="https://github.com/eProsima/foonathan_memory_vendor.git"
    SET FOONATHAN_MEMORY_VENDOR_CMAKE_FLAGS=-DBUILD_SHARED_LIBS=ON
    CALL :build_fastdds_extension "%FOONATHAN_MEMORY_VENDOR_SOURCE_DIR%" "%FOONATHAN_MEMORY_VENDOR_REPO%"
    pushd "%FOONATHAN_MEMORY_VENDOR_SOURCE_DIR%\build" >nul
    cmake -G "Ninja" ^
      -DCMAKE_INSTALL_PREFIX="%FASTDDS_INSTALL_DIR%" ^
      -DBUILD_SHARED_LIBS=ON ^
      -DCMAKE_CXX_FLAGS_RELEASE="-D_GLIBCXX_USE_CXX11_ABI=0" ^
      ..
    ninja
    ninja install
    popd >nul
    rmdir /s /q "%FOONATHAN_MEMORY_VENDOR_SOURCE_DIR%"

    echo Build fast cdr
    SET FAST_CDR_BASENAME=fast-cdr
    SET FAST_CDR_SOURCE_DIR=%CD%\%FAST_CDR_BASENAME%-source
    SET FAST_CDR_REPO="https://github.com/eProsima/Fast-CDR.git"
    CALL :build_fastdds_extension "%FAST_CDR_SOURCE_DIR%" "%FAST_CDR_REPO%"
    pushd "%FAST_CDR_SOURCE_DIR%\build" >nul
    cmake -G "Ninja" ^
      -DCMAKE_INSTALL_PREFIX="%FASTDDS_INSTALL_DIR%" ^
      -DCMAKE_CXX_FLAGS_RELEASE="-D_GLIBCXX_USE_CXX11_ABI=0" ^
      ..
    ninja
    ninja install
    popd >nul
    rmdir /s /q "%FAST_CDR_SOURCE_DIR%"

    echo Build fast dds
    SET FAST_DDS_LIB_BASENAME=fast-dds-lib
    SET FAST_DDS_LIB_SOURCE_DIR=%CD%\%FAST_DDS_LIB_BASENAME%-source
    SET FAST_DDS_LIB_REPO="https://github.com/eProsima/Fast-DDS.git"
    CALL :build_fastdds_extension "%FAST_DDS_LIB_SOURCE_DIR%" "%FAST_DDS_LIB_REPO%"
    pushd "%FAST_DDS_LIB_SOURCE_DIR%\build" >nul
    cmake -G "Ninja" ^
      -DCMAKE_INSTALL_PREFIX="%FASTDDS_INSTALL_DIR%" ^
      -DCMAKE_CXX_FLAGS=-latomic ^
      -DCMAKE_CXX_FLAGS_RELEASE="-D_GLIBCXX_USE_CXX11_ABI=0" ^
      ..
    ninja
    ninja install
    popd >nul
    rmdir /
  )
)

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
>>"%CMAKE_CONFIG_FILE%" echo   add_definitions(-DBOOST_TYPE_INDEX_FORCE_NO_RTTI_COMPATIBILITY)
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
    echo                               Visual Studio 2017 -^> msvc-14.1
    echo                               Visual Studio 2019 -^> msvc-14.2 *
    echo                               Visual Studio 2022 -^> msvc-14.3
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
