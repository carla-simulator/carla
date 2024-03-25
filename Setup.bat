curl -L -O https://aka.ms/vs/17/release/vs_community.exe
vs_Community.exe --add Microsoft.VisualStudio.Workload.NativeDesktop Microsoft.VisualStudio.Workload.NativeGame Microsoft.VisualStudio.Workload.ManagedDesktop Microsoft.VisualStudio.Component.Windows10SDK.18362  Microsoft.VisualStudio.Component.VC.CMake.Project Microsoft.Net.ComponentGroup.4.8.1.DeveloperTools Microsoft.VisualStudio.Component.VC.Llvm.Clang Microsoft.VisualStudio.Component.VC.Llvm.ClangToolset Microsoft.VisualStudio.ComponentGroup.NativeDesktop.Llvm.Clang --removeProductLang Es-es --addProductLang En-us --installWhileDownloading --passive --wait
del vs_community.exe

curl -L -o %USERPROFILE%\AppData\Local\Microsoft\WindowsApps\ninja-win.zip https://github.com/ninja-build/ninja/releases/download/v1.11.1/ninja-win.zip
powershell -command "Expand-Archive $env:USERPROFILE\AppData\Local\Microsoft\WindowsApps\ninja-win.zip $env:USERPROFILE\AppData\Local\Microsoft\WindowsApps\ninja-win"
move %USERPROFILE%\AppData\Local\Microsoft\WindowsApps\ninja-win\ninja.exe %USERPROFILE%\AppData\Local\Microsoft\WindowsApps\ninja.exe
rmdir /s /q %USERPROFILE%\AppData\Local\Microsoft\WindowsApps\ninja-win
del /f %USERPROFILE%\AppData\Local\Microsoft\WindowsApps\ninja-win.zip

mkdir Unreal/CarlaUnreal/Content
start cmd /c git -C Unreal/CarlaUnreal/Content clone -b ue5-dev https://bitbucket.org/carla-simulator/carla-content.git Carla

pushd ..
git clone -b ue5-dev-carla https://github.com/CarlaUnreal/UnrealEngine.git UnrealEngine5_carla
pushd UnrealEngine5_carla
set /A CARLA_UNREAL_ENGINE_PATH=%cd%
Setup.bat
GenerateProjectFiles.bat
msbuild UE5.sln /m /property:Configuration="Development Editor" /property:Platform="Win64"
REM start "" /B /WAIT "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\devenv.exe" UE5.sln
popd
popd

"%ProgramFiles%\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
cmake -G Ninja -S . -B Build -DCMAKE_BUILD_TYPE=Release -DBUILD_CARLA_UNREAL=ON -DCARLA_UNREAL_ENGINE_PATH=%CARLA_UNREAL_ENGINE_PATH%
cmake --build Build --target launch
