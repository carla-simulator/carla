@echo off
setlocal EnableDelayedExpansion

set skip_prerequisites=false
set launch=false
set interactive=false
set python_path=python
set python_root=

rem -- PARSE COMMAND LINE ARGUMENTS --

:parse
    if "%1"=="" (
        goto main
    )
    if "%1"=="--interactive" (
        set interactive=true
    ) else if "%1"=="-i" (
        set interactive=true
    ) else if "%1"=="--skip-prerequisites" (
        set skip_prerequisites=true
    ) else if "%1"=="-p" (
        set skip_prerequisites=true
    ) else if "%1"=="--launch" (
        set launch=true
    ) else if "%1"=="-l" (
        set launch=true
    ) else (
        echo %1 | findstr /B /C:"--python-root=" >nul
        if not errorlevel 1 (
            set python_root="%1"
            set python_root="!python_root:--python-root=!"
        ) else if "%1"=="--python-root" (
            set python_root=%2
            shift
        ) else if "%1"=="-pyroot" (
            set python_root=%2
            shift
        ) else (
            echo Unknown argument "%1"
            exit /b
        )
    )
    shift
    goto parse

rem -- MAIN --

:main

if not "%python_root%"=="" (
    set python_path=%python_root%\python
)

rem -- PREREQUISITES INSTALL STEP --

if %skip_prerequisites%==false (
    echo Installing prerequisites...
    call Util/SetupUtils/InstallPrerequisites.bat --python-path=%python_path% || exit /b
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
    -DPython_ROOT_DIR=%python_root% ^
    -DPython3_ROOT_DIR=%python_root% ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCARLA_UNREAL_ENGINE_PATH=%CARLA_UNREAL_ENGINE_PATH% || exit /b
echo Building CARLA...
cmake --build Build || exit /b
echo Installing Python API...
cmake --build Build --target carla-python-api-install || exit /b
echo CARLA Python API build+install succeeded.

rem -- POST-BUILD STEPS --

if %launch%==true (
    echo Launching Carla Unreal Editor...
    cmake --build Build --target launch || exit /b
)
