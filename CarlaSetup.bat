@echo off
setlocal EnableDelayedExpansion

set skip_prerequisites=false
set launch=false
set interactive=false
set python_path=python
set python_root=

rem -- PARSE COMMAND LINE ARGUMENTS --

:parse
    if "%~1"=="" (
        goto main
    )
    set "argument=%~1"
    if "!argument!"=="--interactive" (
        set interactive=true
    ) else if "!argument!"=="-i" (
        set interactive=true
    ) else if "!argument!"=="--skip-prerequisites" (
        set skip_prerequisites=true
    ) else if "!argument!"=="-p" (
        set skip_prerequisites=true
    ) else if "!argument!"=="--launch" (
        set launch=true
    ) else if "!argument!"=="-l" (
        set launch=true
    ) else if "!argument:~0,14!"=="--python-root=" (
        set "python_root=!argument:~14!"
        set "python_root=!python_root:"=!"
        if not defined python_root (
            echo Missing value for --python-root.
            exit /b 1
        )
    ) else if "!argument!"=="--python-root" (
        if "%~2"=="" (
            echo Missing value for --python-root.
            exit /b 1
        )
        set "python_root=%~2"
        shift
    ) else if "!argument!"=="-pyroot" (
        if "%~2"=="" (
            echo Missing value for -pyroot.
            exit /b 1
        )
        set "python_root=%~2"
        shift
    ) else (
        echo Unknown argument "!argument!"
        exit /b 1
    )
    shift
    goto parse

rem -- MAIN --

:main

if defined python_root (
    set "python_path=%python_root%\python.exe"
)

rem -- PREREQUISITES INSTALL STEP --

if %skip_prerequisites%==false (
    echo Installing prerequisites...
    call Util\SetupUtils\InstallPrerequisites.bat --python-path "%python_path%" || exit /b
) else (
    echo Skipping prerequisites install step.
)

rem -- CLONE CONTENT --
if exist "%cd%\Unreal\CarlaUnreal\Content\Carla\" (
    echo Found CARLA content.
) else (
    echo Could not find CARLA content. Downloading...
    if not exist "%cd%\Unreal\CarlaUnreal\Content\" (
        mkdir "%cd%\Unreal\CarlaUnreal\Content" || exit /b
    )
    git ^
        -C "%cd%\Unreal\CarlaUnreal\Content" ^
        clone ^
        -b ue58-dev-carla ^
        https://bitbucket.org/carla-simulator/carla-content.git ^
        Carla ^
    || exit /b
)

rem Activate VS terminal development environment:
set "vs_env_bat="
if exist "%PROGRAMFILES%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    set "vs_env_bat=%PROGRAMFILES%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
)
if exist "%PROGRAMFILES%\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" (
    set "vs_env_bat=%PROGRAMFILES%\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
)
if exist "%PROGRAMFILES%\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" (
    set "vs_env_bat=%PROGRAMFILES%\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
)

if not "%vs_env_bat%"=="" (
    echo Activating "x64 Native Tools Command Prompt" terminal environment.
    call "%vs_env_bat%" || exit /b
) else (
    echo Could not find vcvars64.bat for VS 2022, aborting setup...
    exit 1
)

rem -- DOWNLOAD + BUILD UNREAL ENGINE --
if exist "%CARLA_UNREAL_ENGINE_PATH%" (
    echo Found Unreal Engine 5 at "%CARLA_UNREAL_ENGINE_PATH%".
) else if exist "..\UnrealEngine5_carla\" (
    for %%I in ("..\UnrealEngine5_carla") do set "CARLA_UNREAL_ENGINE_PATH=%%~fI"
    echo Found CARLA Unreal Engine at "!CARLA_UNREAL_ENGINE_PATH!". Assuming already built...
) else (
    echo Could not find CARLA Unreal Engine, downloading...
    pushd ..
    git clone ^
        -b ue58-dev-carla ^
        https://github.com/CarlaUnreal/UnrealEngine.git ^
        UnrealEngine5_carla || exit /b
    pushd UnrealEngine5_carla
    set "CARLA_UNREAL_ENGINE_PATH=!cd!"
    setx CARLA_UNREAL_ENGINE_PATH "!cd!"
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
    "-DPython_ROOT_DIR=%python_root%" ^
    "-DPython3_ROOT_DIR=%python_root%" ^
    -DCMAKE_BUILD_TYPE=Release ^
    "-DCARLA_UNREAL_ENGINE_PATH=%CARLA_UNREAL_ENGINE_PATH%" || exit /b
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
