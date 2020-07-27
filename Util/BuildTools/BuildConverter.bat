@echo off
setlocal enabledelayedexpansion

rem BAT script that creates the library for conversion from OSM to OpenDRIVE (carla.org).
rem Run it through a cmd with the x64 Visual C++ Toolset enabled.

set LOCAL_PATH=%~dp0
set FILE_N=-[%~n0]:

rem Print batch params (debug purpose)
echo %FILE_N% [Batch params]: %*

rem ============================================================================
rem -- Parse arguments ---------------------------------------------------------
rem ============================================================================


rem ============================================================================
rem -- Local Variables ---------------------------------------------------------
rem ============================================================================

rem Set the visual studio solution directory
rem
set CONVERTER_VSPROJECT_PATH=%INSTALLATION_DIR:/=\%converter-visualstudio\
set CONVERTER_INSTALL_PATH=%ROOT_PATH:/=\%PythonAPI\carla\dependencies\

if "%1"=="--rebuild" (
    rmdir "%CONVERTER_VSPROJECT_PATH%" /s /q
)


if not exist "%CONVERTER_VSPROJECT_PATH%" mkdir "%CONVERTER_VSPROJECT_PATH%"
cd "%CONVERTER_VSPROJECT_PATH%"

cmake -G "Visual Studio 15 2017 Win64"^
    -DCMAKE_CXX_FLAGS_RELEASE="/MD /MP"^
    -DCMAKE_INSTALL_PREFIX="%CONVERTER_INSTALL_PATH:\=/%"^
    "%ROOT_PATH%\Util\Converter"
cmake --build . --config Release --target install | findstr /V "Up-to-date:"

endlocal
exit /b 0