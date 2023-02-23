@echo off
setlocal enabledelayedexpansion 

set LIBOSMSCOUT_REPO=https://github.com/Framstag/libosmscout
set LUNASVG_REPO=https://github.com/sammycage/lunasvg

set LIBOSMSCOUT_SOURCE_PATH=%INSTALLATION_DIR:/=\%libosmscout-source\
set LIBOSMSCOUT_VSPROJECT_PATH=%INSTALLATION_DIR:/=\%libosmscout-visualstudio\

set LUNASVG_SOURCE_PATH=%INSTALLATION_DIR:/=\%lunasvg-source\
set LUNASVG_VSPROJECT_PATH=%INSTALLATION_DIR:/=\%lunasvg-visualstudio\

set OSM_RENDERER_SOURCE=%ROOT_PATH:/=\%osm-world-renderer\
set OSM_RENDERER_VSPROJECT_PATH=%INSTALLATION_DIR:/=\%%osm-world-renderer-visualstudio\

rem Installation path for server dependencies
set DEPENDENCIES_INSTALLATION_PATH=%OSM_RENDERER_SOURCE:/=\%ThirdParties\


rem ============================================================================
rem -- Download dependency manager for libosmscout -----------------------------
rem ============================================================================
set VCPKG_REPO=https://github.com/microsoft/vcpkg
set VCPKG_PATH=%INSTALLATION_DIR:/=\%vcpkg\
set VCPKG_CMAKE_TOOLCHAIN_PATH=%VCPKG_PATH:/=\%scripts\buildsystems\vcpkg.cmake

rem if not exist "%VCPKG_PATH%" git clone %VCPKG_REPO% %VCPKG_PATH%

rem .\"%VCPKG_PATH:/=\%"bootstrap-vcpkg.bat


rem ============================================================================
rem -- Download and build libosmscout ------------------------------------------
rem ============================================================================

if not exist "%LIBOSMSCOUT_SOURCE_PATH%" git clone %LIBOSMSCOUT_REPO% %LIBOSMSCOUT_SOURCE_PATH%

if not exist "%LIBOSMSCOUT_VSPROJECT_PATH%" mkdir "%LIBOSMSCOUT_VSPROJECT_PATH%"
cd "%LIBOSMSCOUT_VSPROJECT_PATH%"

cmake -G "Visual Studio 16 2019"^
    -DCMAKE_INSTALL_PREFIX="%DEPENDENCIES_INSTALLATION_PATH:\=/%"^
    -DOSMSCOUT_BUILD_TOOL_STYLEEDITOR=OFF^
    -DOSMSCOUT_BUILD_TOOL_OSMSCOUT2=OFF^
    -DOSMSCOUT_BUILD_TESTS=OFF^
    -DOSMSCOUT_BUILD_CLIENT_QT=OFF^
    -DOSMSCOUT_BUILD_DEMOS=OFF^
    "%LIBOSMSCOUT_SOURCE_PATH%"

rem -DCMAKE_CXX_FLAGS_RELEASE="/DM_PI=3.14159265358979323846"^

cmake --build . --config=Release --target install

rem ============================================================================
rem -- Download and build lunasvg ----------------------------------------------
rem ============================================================================

if not exist "%LUNASVG_SOURCE_PATH%" git clone %LUNASVG_REPO% %LUNASVG_SOURCE_PATH%

if not exist "%LUNASVG_VSPROJECT_PATH%" mkdir "%LUNASVG_VSPROJECT_PATH%"
cd "%LUNASVG_VSPROJECT_PATH%"

cmake -G "Visual Studio 16 2019" -A x64^
    -DCMAKE_INSTALL_PREFIX="%DEPENDENCIES_INSTALLATION_PATH:\=/%"^
    "%LUNASVG_SOURCE_PATH%"

cmake --build . --config Release --target install


rem ===========================================================================
rem -- Build osm-map-renderer tool --------------------------------------------
rem ===========================================================================

if not exist "%OSM_RENDERER_VSPROJECT_PATH%" mkdir "%OSM_RENDERER_VSPROJECT_PATH%"
cd "%OSM_RENDERER_VSPROJECT_PATH%"

cmake -G "Visual Studio 16 2019" -A x64^
    -DCMAKE_CXX_FLAGS_RELEASE="/std:c++17 /wd4251 /I%INSTALLATION_DIR:/=\%boost-1.80.0-install\include"^
    "%OSM_RENDERER_SOURCE%"

cmake --build . --config Release
rem cmake --build . --config Release
rem    -DOSMSCOUT_BUILD_MAP_QT=OFF^

copy "%DEPENDENCIES_INSTALLATION_PATH:/=\%"bin "%OSM_RENDERER_VSPROJECT_PATH:/=\%"Release\