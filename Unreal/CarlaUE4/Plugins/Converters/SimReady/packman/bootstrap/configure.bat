:: Copyright 2019-2023 NVIDIA CORPORATION
::
:: Licensed under the Apache License, Version 2.0 (the "License");
:: you may not use this file except in compliance with the License.
:: You may obtain a copy of the License at
::
::    http://www.apache.org/licenses/LICENSE-2.0
::
:: Unless required by applicable law or agreed to in writing, software
:: distributed under the License is distributed on an "AS IS" BASIS,
:: WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
:: See the License for the specific language governing permissions and
:: limitations under the License.

set PM_PACKMAN_VERSION=7.30

:: Specify where packman command is rooted
set PM_INSTALL_PATH=%~dp0..

:: The external root may already be configured and we should do minimal work in that case
if defined PM_PACKAGES_ROOT goto ENSURE_DIR

:: If the folder isn't set we assume that the best place for it is on the drive that we are currently
:: running from
set PM_DRIVE=%CD:~0,2%

set PM_PACKAGES_ROOT=%PM_DRIVE%\packman-repo

:: We use *setx* here so that the variable is persisted in the user environment
echo Setting user environment variable PM_PACKAGES_ROOT to %PM_PACKAGES_ROOT%
setx PM_PACKAGES_ROOT %PM_PACKAGES_ROOT%
if %errorlevel% neq 0 ( goto ERROR )

:: The above doesn't work properly from a build step in VisualStudio because a separate process is
:: spawned for it so it will be lost for subsequent compilation steps - VisualStudio must
:: be launched from a new process. We catch this odd-ball case here:
if defined PM_DISABLE_VS_WARNING goto ENSURE_DIR
if not defined VSLANG goto ENSURE_DIR
echo The above is a once-per-computer operation. Unfortunately VisualStudio cannot pick up environment change
echo unless *VisualStudio is RELAUNCHED*.
echo If you are launching VisualStudio from command line or command line utility make sure
echo you have a fresh launch environment (relaunch the command line or utility).
echo If you are using 'linkPath' and referring to packages via local folder links you can safely ignore this warning.
echo You can disable this warning by setting the environment variable PM_DISABLE_VS_WARNING.
echo.

:: Check for the directory that we need. Note that mkdir will create any directories
:: that may be needed in the path
:ENSURE_DIR
if not exist "%PM_PACKAGES_ROOT%" (
	echo Creating packman packages cache at %PM_PACKAGES_ROOT%
	mkdir "%PM_PACKAGES_ROOT%"
)
if %errorlevel% neq 0 ( goto ERROR_MKDIR_PACKAGES_ROOT )

:: The Python interpreter may already be externally configured
if defined PM_PYTHON_EXT (
	set PM_PYTHON=%PM_PYTHON_EXT%
	goto PACKMAN
)

set PM_PYTHON_VERSION=3.10.17-nv1-windows-x86_64
set PM_PYTHON_BASE_DIR=%PM_PACKAGES_ROOT%\python
set PM_PYTHON_DIR=%PM_PYTHON_BASE_DIR%\%PM_PYTHON_VERSION%
set PM_PYTHON=%PM_PYTHON_DIR%\python.exe

if exist "%PM_PYTHON%" goto PACKMAN
if not exist "%PM_PYTHON_BASE_DIR%" call :CREATE_PYTHON_BASE_DIR

set PM_PYTHON_PACKAGE=python@%PM_PYTHON_VERSION%.cab
for /f "delims=" %%a in ('powershell -ExecutionPolicy ByPass -NoLogo -NoProfile -File "%~dp0\generate_temp_file_name.ps1"') do set TEMP_FILE_NAME=%%a
set TARGET=%TEMP_FILE_NAME%.zip
call "%~dp0fetch_file_from_packman_bootstrap.cmd" %PM_PYTHON_PACKAGE% "%TARGET%"
if %errorlevel% neq 0 (
    echo !!! Error fetching python from CDN !!!
    goto ERROR
)

for /f "delims=" %%a in ('powershell -ExecutionPolicy ByPass -NoLogo -NoProfile -File "%~dp0\generate_temp_folder.ps1" -parentPath "%PM_PYTHON_BASE_DIR%"') do set TEMP_FOLDER_NAME=%%a
echo Unpacking Python interpreter ...
"%SystemRoot%\system32\expand.exe" -F:* "%TARGET%" "%TEMP_FOLDER_NAME%" 1> nul
del "%TARGET%"
:: Failure during extraction to temp folder name, need to clean up and abort
if %errorlevel% neq 0 (
    echo !!! Error unpacking python !!!
    call :CLEAN_UP_TEMP_FOLDER
    goto ERROR
)

:: If python has now been installed by a concurrent process we need to clean up and then continue
if exist "%PM_PYTHON%" (
    call :CLEAN_UP_TEMP_FOLDER
    goto PACKMAN
) else (
    if exist "%PM_PYTHON_DIR%" (
        call :REMOVE_DIR "%PM_PYTHON_DIR%"
        if errorlevel 1 (
            call :CLEAN_UP_TEMP_FOLDER
            goto ERROR
        )
    )
)

:: Perform atomic move (allowing overwrite, /y)
move /y "%TEMP_FOLDER_NAME%" "%PM_PYTHON_DIR%" 1> nul
:: Verify that python.exe is now where we expect
if exist "%PM_PYTHON%" goto PACKMAN

:: Wait a second and try again (can help with access denied weirdness)
timeout /t 1 /nobreak 1> nul
move /y "%TEMP_FOLDER_NAME%" "%PM_PYTHON_DIR%" 1> nul
if %errorlevel% neq 0 (
    echo !!! Error moving python %TEMP_FOLDER_NAME% -> %PM_PYTHON_DIR% !!!
    call :CLEAN_UP_TEMP_FOLDER
    goto ERROR
)

:PACKMAN
:: The packman module may already be externally configured
if defined PM_MODULE_DIR_EXT (
	set PM_MODULE_DIR=%PM_MODULE_DIR_EXT%
) else (
    set PM_MODULE_DIR=%PM_PACKAGES_ROOT%\packman-common\%PM_PACKMAN_VERSION%
)

set PM_MODULE=%PM_MODULE_DIR%\run.py

if exist "%PM_MODULE%" goto END

:: Clean out broken PM_MODULE_DIR if it exists
if exist "%PM_MODULE_DIR%" (
    call :REMOVE_DIR "%PM_MODULE_DIR%"
    if errorlevel 1 goto :ERROR
)

set PM_MODULE_PACKAGE=packman-common@%PM_PACKMAN_VERSION%.zip
for /f "delims=" %%a in ('powershell -ExecutionPolicy ByPass -NoLogo -NoProfile -File "%~dp0\generate_temp_file_name.ps1"') do set TEMP_FILE_NAME=%%a
set TARGET=%TEMP_FILE_NAME%
call "%~dp0fetch_file_from_packman_bootstrap.cmd" %PM_MODULE_PACKAGE% "%TARGET%"
if %errorlevel% neq 0 (
    echo !!! Error fetching packman from CDN !!!
    goto ERROR
)

echo Unpacking ...
"%PM_PYTHON%" -S -s -u -E "%~dp0\install_package.py" "%TARGET%" "%PM_MODULE_DIR%"
if %errorlevel% neq 0 (
    echo !!! Error unpacking packman !!!
    goto ERROR
)

del "%TARGET%"

goto END

:ERROR_MKDIR_PACKAGES_ROOT
echo Failed to automatically create packman packages repo at %PM_PACKAGES_ROOT%.
echo Please set a location explicitly that packman has permission to write to, by issuing:
echo.
echo    setx PM_PACKAGES_ROOT {path-you-choose-for-storing-packman-packages-locally}
echo.
echo Then launch a new command console for the changes to take effect and run packman command again.
exit /B %errorlevel%

:REMOVE_DIR
rd /s /q "%~1" > nul
if exist "%~1" (
    echo !!! Error removing corrupt directory %~1 !!!
    exit /B 1
)
exit /B 0

:ERROR
echo !!! Failure while configuring local machine :( !!!
exit /B 1

:CLEAN_UP_TEMP_FOLDER
rd /S /Q "%TEMP_FOLDER_NAME%" > nul
if exist "%TEMP_FOLDER_NAME%" (
    echo !!! Error removing temporary directory %TEMP_FOLDER_NAME% !!!
    exit /B 1
)
exit /B 0

:CREATE_PYTHON_BASE_DIR
:: We ignore errors and clean error state - if two processes create the directory one will fail which is fine
md "%PM_PYTHON_BASE_DIR%" > nul 2>&1
exit /B 0

:END
