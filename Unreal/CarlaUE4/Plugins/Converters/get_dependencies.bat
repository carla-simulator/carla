@echo off
setlocal
pushd %~dp0

:: Remove the ThirdParty folder where all of the links are created
rmdir /s /q "%~dp0SimReady\ThirdParty" 2> NUL
rmdir /s /q "%~dp0MDL\Source\ThirdParty" 2> NUL
rmdir /s /q "%~dp0MDL\Library\mdl" 2> NUL

call "%~dp0SimReady\packman\packman" pull "%~dp0SimReady\Dependencies\packman.xml" --platform windows-x86_64
if errorlevel 1 (
    echo Error getting SimReady dependencies, aborting!
    exit /B
)

call "%~dp0SimReady\packman\packman" pull "%~dp0MDL\Dependencies\packman.xml" --platform windows-x86_64
if errorlevel 1 (
    echo Error getting MDL dependencies, aborting!
    exit /B
)

call "%~dp0MDL\Dependencies\Engine\InstallPrivateMaterialInterface.bat"
if errorlevel 1 (
    echo Error copying and patching Engine Material interface source, aborting!
    exit /B
)
