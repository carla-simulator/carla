@echo off
rem Return a usable VS 2022 x64 activation script in vs_env_bat.
rem Call with delayed expansion disabled to preserve literal exclamation marks.
rem https://github.com/microsoft/vswhere/wiki/Find-VC
set "vs_env_bat="
setlocal DisableDelayedExpansion

set "vswhere=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%vswhere%" set "vswhere=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%vswhere%" exit /b 1

for /f "usebackq delims=" %%I in (`call "%vswhere%" -latest -products * -version "[17.0,18.0)" -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    if exist "%%I\VC\Auxiliary\Build\vcvars64.bat" (
        set "vs_env_bat=%%I\VC\Auxiliary\Build\vcvars64.bat"
        goto found
    )
)
exit /b 1

:found
endlocal & set "vs_env_bat=%vs_env_bat%" & exit /b 0
