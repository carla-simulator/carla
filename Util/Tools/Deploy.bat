@echo off
setlocal EnableDelayedExpansion

rem ==============================================================================
rem -- Set up environment --------------------------------------------------------
rem ==============================================================================

set REPLACE_LATEST=false
set AWS_COPY=aws s3 cp
set ENDPOINT=https://1b808339cbe94fecce530dfada93713b.r2.cloudflarestorage.com
set SUMMARY_OUTPUT_PATH=

rem ==============================================================================
rem -- Parse arguments -----------------------------------------------------------
rem ==============================================================================

set DOC_STRING=Upload latest build to S3.

set USAGE_STRING=Usage: %~nx0 [-h, --help] [--replace-latest] [--dry-run] [--summary-output SUMMARY_OUTPUT]

:arg-parse
if not "%~1"=="" (
    if "%~1"=="--replace-latest" (
        set REPLACE_LATEST=true
    ) else if "%~1"=="--dry-run" (
        set AWS_COPY=echo aws s3 cp
    ) else if "%~1"=="--summary-output" (
        set "SUMMARY_OUTPUT_PATH=%~2"
        shift
    ) else if "%~1"=="--help" (
        echo !DOC_STRING!
        echo !USAGE_STRING!
        goto :bad_exit
    ) else if "%~1"=="-h" (
        echo !DOC_STRING!
        echo !USAGE_STRING!
        goto :bad_exit
    )

    shift
    goto :arg-parse
)

rem Set the package name and path to be deployed
rem TODO: Currently hardcoded. Replace with dynamic versioning in the future
set PACKAGE_NAME=Carla-0.10.0-Win64-Shipping.zip
set PACKAGE_PATH=%~dp0..\..\Build\Release\Package\%PACKAGE_NAME%

echo Using package %PACKAGE_PATH%.

rem Check if the package exists before continuing
if not exist "%PACKAGE_PATH%" goto :error_package

rem Generate the nightly deploy filename based on the latest git commit date and hash
git log --pretty=format:%%cd_%%h --date=format:%%Y%%m%%d -n 1 > "%TEMP%\carla_deploy_name" || goto :bad_exit
set /p NIGHTLY_DEPLOY_NAME= < "%TEMP%\carla_deploy_name"
del "%TEMP%\carla_deploy_name"
set NIGHTLY_DEPLOY_NAME=%NIGHTLY_DEPLOY_NAME%.zip
set LATEST_DEPLOY_NAME=CARLA_UE5_Latest.zip

rem Cloudflare R2 bucket paths
set S3_PREFIX=s3://carla-releases/Windows
set URL_PREFIX=https://downloads.carlasim.com/Windows

set NIGHTLY_DEPLOY_URI=%S3_PREFIX%/Dev/%NIGHTLY_DEPLOY_NAME%
set LATEST_DEPLOY_URI=%S3_PREFIX%/Dev/%LATEST_DEPLOY_NAME%

rem ==============================================================================
rem -- Upload --------------------------------------------------------------------
rem ==============================================================================

echo Uploading %PACKAGE_NAME% as %NIGHTLY_DEPLOY_NAME%.

%AWS_COPY% "%PACKAGE_PATH%" %NIGHTLY_DEPLOY_URI% --endpoint-url %ENDPOINT% || goto :bad_exit
echo Nightly build uploaded to %NIGHTLY_DEPLOY_URI%.

rem ==============================================================================
rem -- Replace Latest ------------------------------------------------------------
rem ==============================================================================

rem Update latest with the nightly build if desired.
if %REPLACE_LATEST%==true (
    echo Updating latest build with nightly build

    rem Cloudflare R2 does not implement object tagging, so copy only metadata.
    %AWS_COPY% %NIGHTLY_DEPLOY_URI% %LATEST_DEPLOY_URI% --endpoint-url %ENDPOINT% --copy-props metadata-directive || goto :bad_exit
    echo Latest build uploaded to %LATEST_DEPLOY_URI%.
)

rem ==============================================================================
rem -- Summary output ------------------------------------------------------------
rem ==============================================================================

if not "%SUMMARY_OUTPUT_PATH%"=="" (
    echo package_uri=%URL_PREFIX%/Dev/%NIGHTLY_DEPLOY_NAME%>> "%SUMMARY_OUTPUT_PATH%"
)

rem ==============================================================================
rem -- ...and we are done --------------------------------------------------------
rem ==============================================================================

echo Success!
goto :good_exit

:error_package
    echo [%~nx0]: ERROR: Package not found, please build a package
    goto :bad_exit

:good_exit
    endlocal
    exit /b 0

:bad_exit
    endlocal
    exit /b 1
