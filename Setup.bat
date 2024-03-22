curl -L -O https://aka.ms/vs/17/release/vs_community.exe
vs_Community.exe --add Microsoft.VisualStudio.Workload.NativeDesktop Microsoft.VisualStudio.Workload.NativeGame Microsoft.VisualStudio.Workload.ManagedDesktop Microsoft.VisualStudio.Component.Windows10SDK.18362  Microsoft.VisualStudio.Component.VC.CMake.Project Microsoft.Net.ComponentGroup.4.8.1.DeveloperTools Microsoft.VisualStudio.Component.VC.Llvm.Clang Microsoft.VisualStudio.Component.VC.Llvm.ClangToolset Microsoft.VisualStudio.ComponentGroup.NativeDesktop.Llvm.Clang --removeProductLang Es-es --addProductLang En-us --installWhileDownloading --passive
del vs_community.exe

curl -L -o %USERPROFILE%\AppData\Local\Microsoft\WindowsApps\ninja-win.zip https://github.com/ninja-build/ninja/releases/download/v1.11.1/ninja-win.zip
powershell -command "Expand-Archive $env:USERPROFILE\AppData\Local\Microsoft\WindowsApps\ninja-win.zip $env:USERPROFILE\AppData\Local\Microsoft\WindowsApps\ninja-win"
move %USERPROFILE%\AppData\Local\Microsoft\WindowsApps\ninja-win\ninja.exe %USERPROFILE%\AppData\Local\Microsoft\WindowsApps\ninja.exe
rmdir /s /q %USERPROFILE%\AppData\Local\Microsoft\WindowsApps\ninja-win
del /f %USERPROFILE%\AppData\Local\Microsoft\WindowsApps\ninja-win.zip

