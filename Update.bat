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

if exist "%CONTENT_FOLDER%" rd /s /q "%CONTENT_FOLDER%"

for /F "delims=" %%a in (%CONTENT_VERSIONS%) do (
   set "lastLine=%%a"
)
set CONTENT_ID=%lastLine:~-16,16%
set CONTENT_REPO=https://bitbucket.org/carla-simulator/carla-content.git
if "%CONTENT_ID:~0,2%"=="20" (
  for /F "tokens=2 delims=_" %%a in ("%CONTENT_ID%") do set CONTENT_COMMIT=%%a
  echo %CONTENT_ID%
  echo %CONTENT_REPO%
) else (
  echo Error reading the latest version from ContentVersions.txt, check last line of file %CONTENT_VERSIONS%'
  goto error_download
)

rem ============================================================================
rem -- Clone the content -------------------------------------------------------
rem ============================================================================

rem Skip the LFS smudge until the pinned commit is checked out, otherwise the
rem blobs of the default branch get downloaded too.
set GIT_LFS_SKIP_SMUDGE=1

echo Cloning "%CONTENT_REPO%" into "%CONTENT_FOLDER%", this can take a while...
git clone "%CONTENT_REPO%" "%CONTENT_FOLDER%"
if %errorlevel% neq 0 goto error_download

echo %FILE_N% Checking out content version %CONTENT_COMMIT%...
git -C "%CONTENT_FOLDER%" checkout %CONTENT_COMMIT%
if %errorlevel% neq 0 goto error_download

set GIT_LFS_SKIP_SMUDGE=

echo %FILE_N% Downloading content files, this can take a while...
git -C "%CONTENT_FOLDER%" lfs pull
if %errorlevel% neq 0 goto error_download

echo .version>> "%CONTENT_FOLDER%/.git/info/exclude"
echo %CONTENT_ID%> "%VERSION_FILE%"

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
    if exist "%CONTENT_FOLDER%" rd /s /q "%CONTENT_FOLDER%"
    echo %FILE_N% Exiting with error...
    endlocal
    exit /b %errorlevel%
