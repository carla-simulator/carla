echo Installing Visual Studion 2022...
curl -L -O https://aka.ms/vs/17/release/vs_community.exe
vs_Community.exe --add Microsoft.VisualStudio.Workload.NativeDesktop Microsoft.VisualStudio.Workload.NativeGame Microsoft.VisualStudio.Workload.ManagedDesktop Microsoft.VisualStudio.Component.Windows10SDK.18362  Microsoft.VisualStudio.Component.VC.CMake.Project Microsoft.Net.ComponentGroup.4.8.1.DeveloperTools Microsoft.VisualStudio.Component.VC.Llvm.Clang Microsoft.VisualStudio.Component.VC.Llvm.ClangToolset Microsoft.VisualStudio.ComponentGroup.NativeDesktop.Llvm.Clang --removeProductLang Es-es --addProductLang En-us --installWhileDownloading --passive --wait
del vs_community.exe
echo Visual Studion 2022 Installed!!!

echo Installing Ninja...
curl -L -o %USERPROFILE%\AppData\Local\Microsoft\WindowsApps\ninja-win.zip https://github.com/ninja-build/ninja/releases/download/v1.11.1/ninja-win.zip
powershell -command "Expand-Archive $env:USERPROFILE\AppData\Local\Microsoft\WindowsApps\ninja-win.zip $env:USERPROFILE\AppData\Local\Microsoft\WindowsApps\ninja-win"
move %USERPROFILE%\AppData\Local\Microsoft\WindowsApps\ninja-win\ninja.exe %USERPROFILE%\AppData\Local\Microsoft\WindowsApps\ninja.exe
rmdir /s /q %USERPROFILE%\AppData\Local\Microsoft\WindowsApps\ninja-win
del /f %USERPROFILE%\AppData\Local\Microsoft\WindowsApps\ninja-win.zip
echo Ninja Installed!!!

echo Starting Content Download...
if not exist "Unreal\CarlaUnreal\Content" mkdir Unreal\CarlaUnreal\Content
start cmd /c git -C Unreal/CarlaUnreal/Content clone -b ue5-dev https://bitbucket.org/carla-simulator/carla-content.git Carla

echo Switching to x64 Native Tools Command Prompt for VS 2022 command line...
call "%ProgramFiles%\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
pushd ..
echo Cloning CARLA Unreal Engine 5...
git clone -b ue5-dev-carla https://github.com/CarlaUnreal/UnrealEngine.git UnrealEngine5_carla
pushd UnrealEngine5_carla
set /A CARLA_UNREAL_ENGINE_PATH=%cd%
echo Setup CARLA Unreal Engine 5...
call Setup.bat
echo GenerateProjectFiles CARLA Unreal Engine 5...
call GenerateProjectFiles.bat
echo Opening Visual Studio 2022...
powershell write-host -fore Red Please Build UE5 Project following the nextlink instructions:
echo https://dev.epicgames.com/documentation/en-us/unreal-engine/building-unreal-engine-from-source#:~:text=Set%20your%20solution%20configuration%20to%20Development%20Editor.
start https://dev.epicgames.com/documentation/en-us/unreal-engine/building-unreal-engine-from-source#:~:text=Set%20your%20solution%20configuration%20to%20Development%20Editor.
start "" /B /WAIT "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\devenv.exe" UE5.sln
popd
popd

echo Configuring CARLA...
cmake -G Ninja -S . -B Build -DCMAKE_BUILD_TYPE=Release -DBUILD_CARLA_UNREAL=ON -DCARLA_UNREAL_ENGINE_PATH=%CARLA_UNREAL_ENGINE_PATH%
echo Buiding CARLA...
cmake --build Build
echo Build Succesfull :)
echo Launching Unreal Editor with CARLA...
cmake --build Build --target launch
