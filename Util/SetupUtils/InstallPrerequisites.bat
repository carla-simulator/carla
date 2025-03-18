@echo off

set ninja_version=1.12.1

set python_path=python
set python_version_default=3.8.10

rem https://learn.microsoft.com/en-us/visualstudio/install/workload-component-id-vs-community?view=vs-2022&preserve-view=true
set visual_studio_components=^
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
    Microsoft.VisualStudio.Component.VC.14.36.17.6.x86.x64 ^
    Microsoft.Component.PythonTools

rem -- PARSE COMMAND LINE ARGUMENTS --

:parse
    if "%1"=="" (
        goto main
    )
    echo %1 | findstr /B /C:"--python-path=" >nul
    if not errorlevel 1 (
        set python_path="%1"
        set python_path="!python_path:--python-path=!"
    ) else if "%1"=="--python-path" (
        set python_path=%2
        shift
    ) else if "%1"=="-pypath" (
        set python_path=%2
        shift
    ) else (
        echo Unknown argument "%1"
        exit /b
    )
    shift
    goto parse

rem -- MAIN --

:main

rem -- INSTALL VISUAL STUDIO --
if not exist %cd%\Temp (
    mkdir %cd%\Temp
)
pushd Temp
curl -L -O https://aka.ms/vs/17/release/vs_community.exe || exit /b
popd Temp
%cd%\Temp\vs_community.exe --add %visual_studio_components% --installWhileDownloading --passive --wait || exit /b
del %cd%\Temp\vs_community.exe
rmdir %cd%\Temp

rem -- INSTALL NINJA --
ninja --version >nul 2>nul
if errorlevel 1 (
    echo Could not find Ninja. Downloading...
    curl -L -o %USERPROFILE%\AppData\Local\Microsoft\WindowsApps\ninja-win.zip https://github.com/ninja-build/ninja/releases/download/v%ninja_version%/ninja-win.zip || exit /b
    powershell -command "Expand-Archive $env:USERPROFILE\AppData\Local\Microsoft\WindowsApps\ninja-win.zip $env:USERPROFILE\AppData\Local\Microsoft\WindowsApps\ninja-win" || exit /b
    move %USERPROFILE%\AppData\Local\Microsoft\WindowsApps\ninja-win\ninja.exe %USERPROFILE%\AppData\Local\Microsoft\WindowsApps\ninja.exe || exit /b
    rmdir /s /q %USERPROFILE%\AppData\Local\Microsoft\WindowsApps\ninja-win
    del /f %USERPROFILE%\AppData\Local\Microsoft\WindowsApps\ninja-win.zip
    echo Installed Ninja %ninja_version%.
) else (
    echo Found Ninja.
)

rem -- INSTALL PYTHON --
%python_path% -V >nul 2>nul
if errorlevel 1 (
    echo Could not find Python. Downloading...
    echo Installing Python %python_version_default%...
    curl -L -O https://www.python.org/ftp/python/%python_version_default%/python-%python_version_default%-amd64.exe || exit /b
    python-%python_version_default%-amd64.exe /passive PrependPath=1  || exit /b
    del python-%python_version_default%-amd64.exe
    set "PATH=%LocalAppData%\Programs\Python\Python38\Scripts\;%LocalAppData%\Programs\Python\Python38\;%PATH%"
    echo Installed Python %python_version_default%.
) else (
    echo Found Python.
)

rem -- INSTALL PYTHON PACKAGES --
%python_path% -m pip install --upgrade pip || exit /b
%python_path% -m pip install -r requirements.txt || exit /b
