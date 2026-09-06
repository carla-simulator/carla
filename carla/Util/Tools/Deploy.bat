@echo off
setlocal enabledelayedexpansion

rem ==============================================================================
rem -- Set up environment --------------------------------------------------------
rem ==============================================================================

set REPLACE_LATEST=true
set AWS_COPY=aws s3 cp

rem ==============================================================================
rem -- Parse arguments -----------------------------------------------------------
rem ==============================================================================

set DOC_STRING=Upload latest build to S3

set USAGE_STRING="Usage: $0 [-h|--help] [--replace-latest] [--dry-run]"

:arg-parse
if not "%1"=="" (
    if "%1"=="--replace-latest" (
        set REPLACE_LATEST=true
    )

    if "%1"=="--dry-run" (
      set AWS_COPY=rem aws s3 cp
    )

    if "%1"=="--help" (
        echo %DOC_STRING%
        echo %USAGE_STRING%
        GOTO :eof
    )

    shift
    goto :arg-parse
)

rem Get repository version
for /f %%i in ('git describe --tags --dirty --always') do set REPOSITORY_TAG=%%i
if not defined REPOSITORY_TAG goto error_carla_version
echo REPOSITORY_TAG = !REPOSITORY_TAG!

rem Last package data
set CARLA_DIST_FOLDER=%~dp0%\Build\UE4Carla
set PACKAGE=CARLA_%REPOSITORY_TAG%.zip
set PACKAGE_PATH=%CARLA_DIST_FOLDER%\%PACKAGE%
set PACKAGE2=AdditionalMaps_%REPOSITORY_TAG%.zip
set PACKAGE_PATH2=%CARLA_DIST_FOLDER%\%PACKAGE2%

set S3_PREFIX=s3://carla-releases/Windows

set LATEST_DEPLOY_URI=!S3_PREFIX!/Dev/CARLA_Latest.zip
set LATEST_DEPLOY_URI2=!S3_PREFIX!/Dev/AdditionalMaps_Latest.zip

rem Check for TAG version
echo %REPOSITORY_TAG% | findstr /R /C:"^[0-9]*\.[0-9]*\.[0-9]*.$" 1>nul
if %errorlevel% == 0 (
  echo Detected release version with tag %REPOSITORY_TAG%
  set DEPLOY_NAME=CARLA_%REPOSITORY_TAG%.zip
  set DEPLOY_NAME2=AdditionalMaps_%REPOSITORY_TAG%.zip
) else (
  echo Detected non-release version with tag %REPOSITORY_TAG%
  set S3_PREFIX=!S3_PREFIX!/Dev
  git log --pretty=format:%%cd_%%h --date=format:%%Y%%m%%d -n 1 > tempo1234
  set /p DEPLOY_NAME= < tempo1234
  del tempo1234
  set DEPLOY_NAME=!DEPLOY_NAME!.zip
  echo deploy name = !DEPLOY_NAME!
  
  git log --pretty=format:%%h -n 1 > tempo1234
  set /p DEPLOY_NAME2= < tempo1234
  del tempo1234
  set DEPLOY_NAME2=AdditionalMaps_!DEPLOY_NAME2!.zip
  echo deploy name2 = !DEPLOY_NAME2!
)
echo Version detected: %REPOSITORY_TAG%
echo Using package %PACKAGE% as %DEPLOY_NAME%

if not exist "%PACKAGE_PATH%" (
  echo Latest package not found, please run 'make package'
  goto :bad_exit
)

rem ==============================================================================
rem -- Upload --------------------------------------------------------------------
rem ==============================================================================

set DEPLOY_URI=!S3_PREFIX!/%DEPLOY_NAME%
%AWS_COPY% %PACKAGE_PATH% %DEPLOY_URI%
echo Latest build uploaded to %DEPLOY_URI%

set DEPLOY_URI2=!S3_PREFIX!/%DEPLOY_NAME2%
%AWS_COPY% %PACKAGE_PATH2% %DEPLOY_URI2%
echo Latest build uploaded to %DEPLOY_URI2%

rem ==============================================================================
rem -- Replace Latest ------------------------------------------------------------
rem ==============================================================================

if %REPLACE_LATEST%==true (
  %AWS_COPY% %DEPLOY_URI% %LATEST_DEPLOY_URI%
  echo Latest build updated as %LATEST_DEPLOY_URI%
  %AWS_COPY% %DEPLOY_URI2% %LATEST_DEPLOY_URI2%
  echo Latest build updated as %LATEST_DEPLOY_URI2%
)

rem ==============================================================================
rem -- ...and we are done --------------------------------------------------------
rem ==============================================================================

echo Success!

:success
    echo.
    goto good_exit

:error_carla_version
    echo.
    echo %FILE_N% [ERROR] Carla Version is not set
    goto bad_exit

:good_exit
    endlocal
    exit /b 0

:bad_exit
    endlocal
    exit /b 1
