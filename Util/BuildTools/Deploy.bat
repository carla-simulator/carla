@echo off
setlocal enabledelayedexpansion

rem ==============================================================================
rem -- Set up environment --------------------------------------------------------
rem ==============================================================================

set REPLACE_LATEST=false
set AWS_COPY=aws s3 cp
set UPLOAD_MAPS=true

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
      echo %AWS_COPY%
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

rem Last package data
set CARLA_DIST_FOLDER=%~dp0%\Build\UE4Carla
set LATEST_PACKAGE=CARLA_%REPOSITORY_TAG%.zip
set LATEST_PACKAGE_PATH=%CARLA_DIST_FOLDER%\%LATEST_PACKAGE%

set S3_PREFIX=s3://carla-releases/Windows

set LATEST_DEPLOY_URI=!S3_PREFIX!/Dev/CARLA_Latest.zip

rem Check for TAG version
echo %REPOSITORY_TAG% | findstr /R /C:"^[0-9]*\.[0-9]*\.[0-9]*.$" 1>nul
if %errorlevel% == 0 (
  echo Detected tag %REPOSITORY_TAG%
  set DEPLOY_NAME=CARLA_%REPOSITORY_TAG%.zip
) else (
  set S3_PREFIX=!S3_PREFIX!/Dev
  git log --pretty=format:%%cd_%%h --date=format:%%Y%%m%%d -n 1 > tempo1234
  set /p DEPLOY_NAME= < tempo1234
  del tempo1234
  set DEPLOY_NAME=!DEPLOY_NAME!.zip
)
echo Version detected: %REPOSITORY_TAG%
echo Using package %LATEST_PACKAGE% as %DEPLOY_NAME%

if not exist "%LATEST_PACKAGE_PATH%" (
  echo Latest package not found, please run 'make package'
  goto :bad_exit
)

rem ==============================================================================
rem -- Upload --------------------------------------------------------------------
rem ==============================================================================

set DEPLOY_URI=!S3_PREFIX!/%DEPLOY_NAME%
%AWS_COPY% %LATEST_PACKAGE_PATH% %DEPLOY_URI%
echo Latest build uploaded to %DEPLOY_URI%

if %REPLACE_LATEST%==true (
  %AWS_COPY% %DEPLOY_URI% %LATEST_DEPLOY_URI%
  echo Latest build uploaded to %LATEST_DEPLOY_URI%
)

if %UPLOAD_MAPS%==true (

  pushd "%CARLA_DIST_FOLDER%"

  for /f %%i in ('dir *_%REPOSITORY_TAG%.zip /b') do (
    if not %%i == %LATEST_PACKAGE% (
      set DEPLOY_MAP_URI=!S3_PREFIX!/%%i
      %AWS_COPY% %%i !DEPLOY_MAP_URI!
      echo %%i uploaded to !DEPLOY_MAP_URI!
    )
  )
  popd
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
