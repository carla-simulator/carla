@echo off

echo Starting Content Download...
if not exist "Unreal\CarlaUnreal\Content" mkdir Unreal\CarlaUnreal\Content
start cmd /c git -C Unreal/CarlaUnreal/Content clone -b ue5-dev https://bitbucket.org/carla-simulator/carla-content.git Carla

call Util/SetupUtils/InstallPrerequisites.bat

if exist "%PROGRAMFILES%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    echo Activating "x64 Native Tools Command Prompt" terminal environment.
    call "%PROGRAMFILES%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
) else (
    echo Could not find vcvarsall.bat, aborting setup...
    exit 1
)

if exist "%CARLA_UNREAL_ENGINE_PATH%" (
    echo Found Unreal Engine 5 at "%CARLA_UNREAL_ENGINE_PATH%".
) else if exist ..\UnrealEngine5_carla (
    echo Found Unreal Engine 5 at "%cd%\..\UnrealEngine5_carla".
) else (
    echo Could not find Unreal Engine 5, downloading...
    pushd ..
    git clone ^
        -b ue5-dev-carla ^
        https://github.com/CarlaUnreal/UnrealEngine.git ^
        UnrealEngine5_carla || exit /b
    pushd UnrealEngine5_carla
    set CARLA_UNREAL_ENGINE_PATH=!cd!
    setx CARLA_UNREAL_ENGINE_PATH !cd!
    popd
    popd
    pushd ..
    pushd %CARLA_UNREAL_ENGINE_PATH%
    call Setup.bat || exit /b
    call GenerateProjectFiles.bat || exit /b
    msbuild Engine\Intermediate\ProjectFiles\UE5.vcxproj ^
        /property:Configuration="Development_Editor" ^
        /property:Platform="x64" || exit /b
    popd
    popd
)

cmake ^
    -G Ninja ^
    -S . ^
    -B Build ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DBUILD_CARLA_UNREAL=ON ^
    -DCARLA_UNREAL_ENGINE_PATH=%CARLA_UNREAL_ENGINE_PATH% || exit /b

cmake --build Build || exit /b

cmake --build Build --target carla-python-api-install

cmake --build Build --target launch || exit /b
