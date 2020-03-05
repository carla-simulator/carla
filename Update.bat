@echo off

rem ============================================================================
rem -- Set up environment ------------------------------------------------------
rem ============================================================================

set SCRIPT_DIR=%~dp0
set CONTENT_FOLDER=%SCRIPT_DIR%Unreal/CarlaUE4/Content/Carla
set VERSION_FILE=%CONTENT_FOLDER%/.version
set CONTENT_VERSIONS=%SCRIPT_DIR%/Util/ContentVersions.txt

rem ============================================================================
rem -- Get the last version to download ----------------------------------------
rem ============================================================================

if not exist "%CONTENT_FOLDER%" mkdir "%CONTENT_FOLDER%"

for /F "delims=" %%a in (%CONTENT_VERSIONS%) do (
   set "lastLine=%%a"
)
set CONTENT_ID=%lastLine:~-16,16%
set CONTENT_LINK=http://carla-assets.s3.amazonaws.com/%CONTENT_ID%.tar.gz
if "%CONTENT_ID:~0,2%"=="20" (
  set CONTENT_FILE=%CONTENT_FOLDER%/%CONTENT_ID%.tar.gz
  set CONTENT_FILE_TAR=%CONTENT_FOLDER%/%CONTENT_ID%.tar
  echo %CONTENT_ID%
  echo %CONTENT_LINK%
) else (
  echo Error reading the latest version from ContentVersions.txt, check last line of file %CONTENT_VERSIONS%'
  goto error_download
)

rem ============================================================================
rem -- Download the content ----------------------------------------------------
rem ============================================================================

echo Downloading "%CONTENT_LINK%"...
powershell -Command "Start-BitsTransfer -Source '%CONTENT_LINK%' -Destination '%CONTENT_FOLDER%'"
if %errorlevel% neq 0 goto error_download

echo %FILE_N% Extracting content from "%CONTENT_FILE%", this can take a while...
if exist "%ProgramW6432%/7-Zip/7z.exe" (
    "%ProgramW6432%/7-Zip/7z.exe" x "%CONTENT_FILE%" -o"%CONTENT_FOLDER%" -y
    if %errorlevel% neq 0 goto error_download
    echo Deleting %CONTENT_FILE:/=\%
    del %CONTENT_FILE:/=\%
    "%ProgramW6432%/7-Zip/7z.exe" x "%CONTENT_FILE_TAR%" -o"%CONTENT_FOLDER%" -y
    if %errorlevel% neq 0 goto error_download
    echo Deleting %CONTENT_FILE_TAR:/=\%
    del %CONTENT_FILE_TAR:/=\%
) else (
    powershell -Command "Expand-Archive '%CONTENT_FILE%' -DestinationPath '%CONTENT_FOLDER%'"
    if %errorlevel% neq 0 goto error_download
    del %CONTENT_FILE%
)

goto success

:success
    echo.
    echo %FILE_N% Content has been successfully installed in "%CONTENT_FOLDER%"!
    goto good_exit

:error_download
    goto bad_exit

:good_exit
    echo %FILE_N% Exiting...
    endlocal
    exit /b 0

:bad_exit
    if exist "%CONTENT_FILE%" rd /s /q "%CONTENT_FOLDER%"
    echo %FILE_N% Exiting with error...
    endlocal
    exit /b %errorlevel%
