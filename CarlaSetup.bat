@echo off
setlocal EnableDelayedExpansion

set SKIP_PREREQUISITES=false
set LAUNCH=false
set INTERACTIVE=false
set PYTHON_PATH=python
set PYTHON_ROOT=

if not "%*"=="" (
    for %%x in ("%*") do (
        if "%%~x"=="--interactive" (
            set INTERACTIVE=true
        ) else if "%%~x"=="-i" (
            set INTERACTIVE=true
        ) else if "%%~x"=="--skip-prerequisites" (
            set SKIP_PREREQUISITES=true
        ) else if "%%~x"=="-p" (
            set SKIP_PREREQUISITES=true
        ) else if "%%~x"=="--launch" (
            set LAUNCH=true
        ) else if "%%~x"=="-l" (
            set LAUNCH=true
        ) else if "%%~x"=="--python-path=" (
            set PYTHON_PATH=%%~nx
        ) else if "%%~x"=="-pypath" (
            set PYTHON_PATH=%%~nx
        ) else if "%%~x"=="--python-root=" (
            set PYTHON_ROOT=%%~nx
        ) else if "%%~x"=="-pyroot" (
            set PYTHON_ROOT=%%~nx
        ) else (
            echo Unknown argument "%%~x"
        )
    )
)

rem -- PREREQUISITES INSTALL STEP --

if %SKIP_PREREQUISITES%==false (
    echo Installing prerequisites...
    call Util/SetupUtils/InstallPrerequisites.bat || exit /b
) else (
    echo Skipping prerequisites install step.
)

rem -- CLONE CONTENT --
if exist "%cd%\Unreal\CarlaUnreal\Content" (
    echo Found CARLA content.
) else (
    echo Could not find CARLA content. Downloading...
    mkdir %cd%\Unreal\CarlaUnreal\Content
    git ^
        -C %cd%\Unreal\CarlaUnreal\Content ^
        clone ^
        -b ue5-dev ^
        https://bitbucket.org/carla-simulator/carla-content.git ^
        Carla ^
    || exit /b
)

rem Activate VS terminal development environment:
if exist "%PROGRAMFILES%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    echo Activating "x64 Native Tools Command Prompt" terminal environment.
    call "%PROGRAMFILES%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" || exit /b
) else (
    echo Could not find vcvarsall.bat, aborting setup...
    exit 1
)

rem -- DOWNLOAD + BUILD UNREAL ENGINE --
if exist "%CARLA_UNREAL_ENGINE_PATH%" (
    echo Found Unreal Engine 5 at "%CARLA_UNREAL_ENGINE_PATH%".
) else if exist ..\UnrealEngine5_carla (
    echo Found CARLA Unreal Engine at %cd%/UnrealEngine5_carla. Assuming already built...
) else (
    echo Could not find CARLA Unreal Engine, downloading...
    pushd ..
    git clone ^
        -b ue5-dev-carla ^
        https://github.com/CarlaUnreal/UnrealEngine.git ^
        UnrealEngine5_carla || exit /b
    pushd UnrealEngine5_carla
    set CARLA_UNREAL_ENGINE_PATH=!cd!
    setx CARLA_UNREAL_ENGINE_PATH !cd!
    echo Running Unreal Engine pre-build steps...
    call Setup.bat || exit /b
    call GenerateProjectFiles.bat || exit /b
    echo Building Unreal Engine 5...
    msbuild ^
        Engine\Intermediate\ProjectFiles\UE5.vcxproj ^
        /property:Configuration="Development_Editor" ^
        /property:Platform="x64" || exit /b
    popd
    popd
)

rem -- BUILD CARLA --
echo Configuring the CARLA CMake project...
cmake ^
    -G Ninja ^
    -S . ^
    -B Build ^
    --toolchain=CMake/Toolchain.cmake ^
    -DPython_ROOT_DIR=%PYTHON_ROOT% ^
    -DPython3_ROOT_DIR=%PYTHON_ROOT% ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCARLA_UNREAL_ENGINE_PATH=%CARLA_UNREAL_ENGINE_PATH% || exit /b
echo Building CARLA...
cmake --build Build || exit /b
echo Installing Python API...
cmake --build Build --target carla-python-api-install || exit /b
echo CARLA Python API build+install succeeded.

rem -- POST-BUILD STEPS --

if %LAUNCH%==true (
    echo Launching Carla Unreal Editor...
    cmake --build Build --target launch || exit /b
)
