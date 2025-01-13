@echo off

set NINJA_VERSION=1.12.1
set PYTHON_VERSION=3.8.10
rem https://learn.microsoft.com/en-us/visualstudio/install/workload-component-id-vs-community?view=vs-2022&preserve-view=true
set VISUAL_STUDIO_COMPONENTS=^
    Microsoft.VisualStudio.Workload.NativeDesktop ^
    Microsoft.VisualStudio.Workload.NativeGame ^
    Microsoft.VisualStudio.Workload.ManagedDesktop ^
    Microsoft.VisualStudio.Component.Windows10SDK.22621 ^
    Microsoft.VisualStudio.Component.VC.CMake.Project ^
    Microsoft.Net.Component.4.8.SDK ^
    Microsoft.Net.ComponentGroup.4.8.1.DeveloperTools ^
    Microsoft.VisualStudio.Component.VC.Llvm.Clang ^
    Microsoft.VisualStudio.Component.VC.Llvm.ClangToolset ^
    Microsoft.VisualStudio.ComponentGroup.NativeDesktop.Llvm.Clang ^
    Microsoft.VisualStudio.Component.VC.14.36.17.6.x86.x64

if not exist %cd%\Temp (
    mkdir %cd%\Temp
)
pushd Temp
curl -L -O https://aka.ms/vs/17/release/vs_community.exe || exit /b
popd Temp
%cd%\Temp\vs_community.exe --add %VISUAL_STUDIO_COMPONENTS% --installWhileDownloading --passive --wait || exit /b
del %cd%\Temp\vs_community.exe
rmdir %cd%\Temp

ninja --version 2>NUL
if errorlevel 1 (
    echo Could not find ninja %NINJA_VERSION%, downloading...
    curl -L -o %USERPROFILE%\AppData\Local\Microsoft\WindowsApps\ninja-win.zip https://github.com/ninja-build/ninja/releases/download/v%NINJA_VERSION%/ninja-win.zip || exit /b
    powershell -command "Expand-Archive $env:USERPROFILE\AppData\Local\Microsoft\WindowsApps\ninja-win.zip $env:USERPROFILE\AppData\Local\Microsoft\WindowsApps\ninja-win" || exit /b
    move %USERPROFILE%\AppData\Local\Microsoft\WindowsApps\ninja-win\ninja.exe %USERPROFILE%\AppData\Local\Microsoft\WindowsApps\ninja.exe || exit /b
    rmdir /s /q %USERPROFILE%\AppData\Local\Microsoft\WindowsApps\ninja-win
    del /f %USERPROFILE%\AppData\Local\Microsoft\WindowsApps\ninja-win.zip
    echo Installed ninja %NINJA_VERSION%.
) else (
    echo Found Ninja.
)

python --version 2>NUL
if errorlevel 1 (
    echo Found Python - FAIL
    echo Installing Python %PYTHON_VERSION%...
    curl -L -O https://www.python.org/ftp/python/%PYTHON_VERSION%/python-%PYTHON_VERSION%-amd64.exe || exit /b
    python-%PYTHON_VERSION%-amd64.exe /passive PrependPath=1  || exit /b
    del python-%PYTHON_VERSION%-amd64.exe
    set "PATH=%LocalAppData%\Programs\Python\Python38\Scripts\;%LocalAppData%\Programs\Python\Python38\;%PATH%"
    echo Python %PYTHON_VERSION% installed!!!
) else (
    echo Found Python:
    python --version
)

python -m pip install --upgrade pip || exit /b
python -m pip install -r requirements.txt || exit /b
