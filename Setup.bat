echo Starting Content Download...
if not exist "Unreal\CarlaUnreal\Content" mkdir Unreal\CarlaUnreal\Content
start cmd /c git -C Unreal/CarlaUnreal/Content clone -b ue5-dev https://bitbucket.org/carla-simulator/carla-content.git Carla


echo Installing Visual Studio 2022...
curl -L -O https://aka.ms/vs/17/release/vs_community.exe || exit /b
vs_Community.exe --add Microsoft.VisualStudio.Workload.NativeDesktop Microsoft.VisualStudio.Workload.NativeGame Microsoft.VisualStudio.Workload.ManagedDesktop Microsoft.VisualStudio.Component.Windows10SDK.18362  Microsoft.VisualStudio.Component.VC.CMake.Project Microsoft.Net.ComponentGroup.4.8.1.DeveloperTools Microsoft.VisualStudio.Component.VC.Llvm.Clang Microsoft.VisualStudio.Component.VC.Llvm.ClangToolset Microsoft.VisualStudio.ComponentGroup.NativeDesktop.Llvm.Clang --removeProductLang Es-es --addProductLang En-us --installWhileDownloading --passive --wait || exit /b
del vs_community.exe
echo Visual Studion 2022 Installed!!!


ninja --version 2>NUL
if errorlevel 1 (
    echo Found Ninja - FAIL
    echo Installing Ninja 1.11.1...
    echo Installing Ninja...
    curl -L -o %USERPROFILE%\AppData\Local\Microsoft\WindowsApps\ninja-win.zip https://github.com/ninja-build/ninja/releases/download/v1.11.1/ninja-win.zip || exit /b
    powershell -command "Expand-Archive $env:USERPROFILE\AppData\Local\Microsoft\WindowsApps\ninja-win.zip $env:USERPROFILE\AppData\Local\Microsoft\WindowsApps\ninja-win" || exit /b
    move %USERPROFILE%\AppData\Local\Microsoft\WindowsApps\ninja-win\ninja.exe %USERPROFILE%\AppData\Local\Microsoft\WindowsApps\ninja.exe || exit /b
    rmdir /s /q %USERPROFILE%\AppData\Local\Microsoft\WindowsApps\ninja-win
    del /f %USERPROFILE%\AppData\Local\Microsoft\WindowsApps\ninja-win.zip
    echo Ninja Installed!!!
) else (
    echo Found Ninja - OK
    ninja --version
)


python --version 2>NUL
if errorlevel 1 (
    echo Found Python - FAIL
    echo Installing Python 3.8.10...
    curl -L -O https://www.python.org/ftp/python/3.8.10/python-3.8.10-amd64.exe || exit /b
    python-3.8.10-amd64.exe /quiet AppendPath=1 || exit /b
    del python-3.8.10-amd64.exe
    echo Python 3.8.10 installed!!!
) else (
    echo Found Python - OK
    python --version
)


echo Installing Python Pacakges...
pip install --user numpy || exit /b
pip install --user -Iv setuptools==47.3.1 || exit /b
pip install --user distro || exit /b
pip install --user wheel auditwheel || exit /b
echo Python Pacakges Installed...


echo Switching to x64 Native Tools Command Prompt for VS 2022 command line...
call "%ProgramFiles%\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" || exit /b


if exist %CARLA_UNREAL_ENGINE_PATH% (
    echo Found UnrealEngine5 %CARLA_UNREAL_ENGINE_PATH% - OK
) else ( if ( exist ..\UnrealEngine5_carla
    echo Found UnrealEngine5 ..\UnrealEngine5_carla - OK
    REM TODO: Check if UnrealEngine binary file exists and if not build it
else (
    echo Found UnrealEngine5 $CARLA_UNREAL_ENGINE_PATH - FAIL
    pushd ..
    echo Cloning CARLA Unreal Engine 5...
    git clone -b ue5-dev-carla https://github.com/CarlaUnreal/UnrealEngine.git UnrealEngine5_carla || exit /b
    pushd UnrealEngine5_carla
    set /A CARLA_UNREAL_ENGINE_PATH=cd
    echo Setup CARLA Unreal Engine 5...
    call Setup.bat || exit /b
    echo GenerateProjectFiles CARLA Unreal Engine 5...
    call GenerateProjectFiles.bat || exit /b
    echo Opening Visual Studio 2022...
    msbuild Engine\Intermediate\ProjectFiles\UE5.vcxproj /property:Configuration="Development_Editor" /property:Platform="x64" || exit /b
    popd
    popd
)))


echo Configuring CARLA...
set COUNT=0
:cmake_configure_command
set /A COUNT=%COUNT%+1
cmake -G Ninja -S . -B Build -DCMAKE_BUILD_TYPE=Release -DBUILD_CARLA_UNREAL=ON -DCARLA_UNREAL_ENGINE_PATH=%CARLA_UNREAL_ENGINE_PATH% || exit /b
timeout /t 10
if errorlevel 1 if %COUNT% LSS 12 goto cmake_configure_command

echo Buiding CARLA...
set COUNT=0
:cmake_build_command
set /A COUNT=%COUNT%+1
cmake --build Build || exit /b
timeout /t 10
if errorlevel 1 if %COUNT% LSS 12 goto cmake_build_command

echo Build Succesfull :)
echo Launching Unreal Editor with CARLA...
cmake --build Build --target launch || exit /b
