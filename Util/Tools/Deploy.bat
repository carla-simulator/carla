@echo off
setlocal EnableDelayedExpansion

rem ==============================================================================
rem -- Set up environment --------------------------------------------------------
rem ==============================================================================

set REPLACE_LATEST=false
set AWS_COPY=aws s3 cp
set ENDPOINT=https://1b808339cbe94fecce530dfada93713b.r2.cloudflarestorage.com
set SUMMARY_OUTPUT_PATH=
set ADDITIONAL_MAPS=false
set LATEST_PREFIX=CARLA_UE5_Latest
set RELEASE_VERSION=

rem ==============================================================================
rem -- Parse arguments -----------------------------------------------------------
rem ==============================================================================

set DOC_STRING=Upload latest build to S3.

set USAGE_STRING=Usage: %~nx0 [-h, --help] [--replace-latest] [--additional-maps] [--latest-prefix PREFIX] [--release-version VERSION] [--dry-run] [--summary-output SUMMARY_OUTPUT]

:arg-parse
if not "%~1"=="" (
    if "%~1"=="--replace-latest" (
        set REPLACE_LATEST=true
    ) else if "%~1"=="--additional-maps" (
        set ADDITIONAL_MAPS=true
    ) else if "%~1"=="--latest-prefix" (
        set "LATEST_PREFIX=%~2"
        shift /1
    ) else if "%~1"=="--release-version" (
        set "RELEASE_VERSION=%~2"
        shift /1
    ) else if "%~1"=="--dry-run" (
        set AWS_COPY=echo aws s3 cp
    ) else if "%~1"=="--summary-output" (
        set "SUMMARY_OUTPUT_PATH=%~2"
        shift /1
    ) else if "%~1"=="--help" (
        echo !DOC_STRING!
        echo !USAGE_STRING!
        goto :bad_exit
    ) else if "%~1"=="-h" (
        echo !DOC_STRING!
        echo !USAGE_STRING!
        goto :bad_exit
    )

    shift /1
    goto :arg-parse
)

if not "%RELEASE_VERSION%"=="" if %REPLACE_LATEST%==true (
    echo [%~nx0]: ERROR: --release-version and --replace-latest cannot be combined: a release never replaces latest.
    goto :bad_exit
)

set "PACKAGE_DIR=%~dp0..\..\Build\Release\Package"

rem Find exactly one Shipping package to be deployed
set "PACKAGE_NAME="
for /f "delims=" %%F in ('dir /b /a-d "%PACKAGE_DIR%\Carla-*-Win64-Shipping.zip" 2^>nul') do (
    if defined PACKAGE_NAME goto :error_package
    set "PACKAGE_NAME=%%F"
    set "PACKAGE_PATH=%PACKAGE_DIR%\%%F"
)
if not defined PACKAGE_NAME goto :error_package

echo Using package "%PACKAGE_PATH%".

rem The base release metadata the package target writes: needed to build content
rem packs that install into this package.
set "METADATA_NAME="
for /f "delims=" %%F in ('dir /b /a-d "%PACKAGE_DIR%\*-release-metadata.tar.gz" 2^>nul') do (
    if defined METADATA_NAME goto :error_metadata
    set "METADATA_NAME=%%F"
    set "METADATA_PATH=%PACKAGE_DIR%\%%F"
)
if not defined METADATA_NAME goto :error_metadata

echo Using release metadata "%METADATA_PATH%".

if %ADDITIONAL_MAPS%==true (
    set "ADDITIONAL_MAPS_NAME="
    for /f "delims=" %%F in ('dir /b /a-d "%PACKAGE_DIR%\AdditionalMaps-*-Win64.tar.gz" 2^>nul') do (
        if defined ADDITIONAL_MAPS_NAME goto :error_additional_maps
        set "ADDITIONAL_MAPS_NAME=%%F"
        set "ADDITIONAL_MAPS_PATH=%PACKAGE_DIR%\%%F"
    )
    if not defined ADDITIONAL_MAPS_NAME goto :error_additional_maps
    echo Using additional maps pack "!ADDITIONAL_MAPS_PATH!".
)

rem A release goes to the platform folder itself, named after the version as the
rem UE4 releases are (CARLA_<version>.zip, AdditionalMaps_<version>.tar.gz); a
rem nightly goes to Dev/ named after the latest git commit date and hash.
if not "%RELEASE_VERSION%"=="" (
    set DEPLOY_DIR=
    set NIGHTLY_DEPLOY_BASENAME=CARLA_%RELEASE_VERSION%
    set NIGHTLY_ADDITIONAL_MAPS_DEPLOY_NAME=AdditionalMaps_%RELEASE_VERSION%.tar.gz
) else (
    set DEPLOY_DIR=Dev/
    git log --pretty=format:%%cd_%%h --date=format:%%Y%%m%%d -n 1 > "%TEMP%\carla_deploy_name" || goto :bad_exit
    set /p NIGHTLY_DEPLOY_BASENAME= < "%TEMP%\carla_deploy_name"
    del "%TEMP%\carla_deploy_name"
    set NIGHTLY_ADDITIONAL_MAPS_DEPLOY_NAME=!NIGHTLY_DEPLOY_BASENAME!_AdditionalMaps.tar.gz
)
set NIGHTLY_DEPLOY_NAME=%NIGHTLY_DEPLOY_BASENAME%.zip
set NIGHTLY_METADATA_DEPLOY_NAME=%NIGHTLY_DEPLOY_BASENAME%_release-metadata.tar.gz
rem --latest-prefix gives a workflow its own set of "latest" names, so that a
rem package and the pack cooked against it are never replaced one without the other.
set LATEST_DEPLOY_NAME=%LATEST_PREFIX%.zip
set LATEST_METADATA_DEPLOY_NAME=%LATEST_PREFIX%_release-metadata.tar.gz
set LATEST_ADDITIONAL_MAPS_DEPLOY_NAME=%LATEST_PREFIX%_AdditionalMaps.tar.gz

rem Cloudflare R2 bucket paths
set S3_PREFIX=s3://carla-releases/Windows
set URL_PREFIX=https://downloads.carlasim.com/Windows

set NIGHTLY_DEPLOY_URI=%S3_PREFIX%/%DEPLOY_DIR%%NIGHTLY_DEPLOY_NAME%
set NIGHTLY_METADATA_DEPLOY_URI=%S3_PREFIX%/%DEPLOY_DIR%%NIGHTLY_METADATA_DEPLOY_NAME%
set NIGHTLY_ADDITIONAL_MAPS_DEPLOY_URI=%S3_PREFIX%/%DEPLOY_DIR%%NIGHTLY_ADDITIONAL_MAPS_DEPLOY_NAME%
set LATEST_DEPLOY_URI=%S3_PREFIX%/%DEPLOY_DIR%%LATEST_DEPLOY_NAME%
set LATEST_METADATA_DEPLOY_URI=%S3_PREFIX%/%DEPLOY_DIR%%LATEST_METADATA_DEPLOY_NAME%
set LATEST_ADDITIONAL_MAPS_DEPLOY_URI=%S3_PREFIX%/%DEPLOY_DIR%%LATEST_ADDITIONAL_MAPS_DEPLOY_NAME%

rem ==============================================================================
rem -- Upload --------------------------------------------------------------------
rem ==============================================================================

echo Uploading %PACKAGE_NAME% as %NIGHTLY_DEPLOY_NAME%.

%AWS_COPY% "%PACKAGE_PATH%" %NIGHTLY_DEPLOY_URI% --endpoint-url %ENDPOINT% || goto :bad_exit
echo Package uploaded to %NIGHTLY_DEPLOY_URI%.

%AWS_COPY% "%METADATA_PATH%" %NIGHTLY_METADATA_DEPLOY_URI% --endpoint-url %ENDPOINT% || goto :bad_exit
echo Release metadata uploaded to %NIGHTLY_METADATA_DEPLOY_URI%.

if %ADDITIONAL_MAPS%==true (
    %AWS_COPY% "%ADDITIONAL_MAPS_PATH%" %NIGHTLY_ADDITIONAL_MAPS_DEPLOY_URI% --endpoint-url %ENDPOINT% || goto :bad_exit
    echo Additional maps uploaded to %NIGHTLY_ADDITIONAL_MAPS_DEPLOY_URI%.
)

rem ==============================================================================
rem -- Replace Latest ------------------------------------------------------------
rem ==============================================================================

rem Update latest with the nightly build if desired.
if %REPLACE_LATEST%==true (
    echo Updating latest build with nightly build

    rem Cloudflare R2 does not implement object tagging, so copy only metadata.
    %AWS_COPY% %NIGHTLY_DEPLOY_URI% %LATEST_DEPLOY_URI% --endpoint-url %ENDPOINT% --copy-props metadata-directive || goto :bad_exit
    echo Latest build uploaded to %LATEST_DEPLOY_URI%.

    rem Upload the metadata again instead of copying it: it is below the multipart
    rem threshold, and a single-part copy sends x-amz-tagging-directive whatever
    rem --copy-props says, which R2 rejects with NotImplemented.
    %AWS_COPY% "%METADATA_PATH%" %LATEST_METADATA_DEPLOY_URI% --endpoint-url %ENDPOINT% || goto :bad_exit
    echo Latest release metadata uploaded to %LATEST_METADATA_DEPLOY_URI%.

    if %ADDITIONAL_MAPS%==true (
        %AWS_COPY% %NIGHTLY_ADDITIONAL_MAPS_DEPLOY_URI% %LATEST_ADDITIONAL_MAPS_DEPLOY_URI% --endpoint-url %ENDPOINT% --copy-props metadata-directive || goto :bad_exit
        echo Latest additional maps uploaded to %LATEST_ADDITIONAL_MAPS_DEPLOY_URI%.
    )
)

rem ==============================================================================
rem -- Summary output ------------------------------------------------------------
rem ==============================================================================

if not "%SUMMARY_OUTPUT_PATH%"=="" (
    echo package_uri=%URL_PREFIX%/%DEPLOY_DIR%%NIGHTLY_DEPLOY_NAME%>> "%SUMMARY_OUTPUT_PATH%"
    echo release_metadata_uri=%URL_PREFIX%/%DEPLOY_DIR%%NIGHTLY_METADATA_DEPLOY_NAME%>> "%SUMMARY_OUTPUT_PATH%"
    if %ADDITIONAL_MAPS%==true (
        echo additional_maps_uri=%URL_PREFIX%/%DEPLOY_DIR%%NIGHTLY_ADDITIONAL_MAPS_DEPLOY_NAME%>> "%SUMMARY_OUTPUT_PATH%"
    )
    if %REPLACE_LATEST%==true (
        echo latest_package_uri=%URL_PREFIX%/%DEPLOY_DIR%%LATEST_DEPLOY_NAME%>> "%SUMMARY_OUTPUT_PATH%"
        echo latest_release_metadata_uri=%URL_PREFIX%/%DEPLOY_DIR%%LATEST_METADATA_DEPLOY_NAME%>> "%SUMMARY_OUTPUT_PATH%"
        if %ADDITIONAL_MAPS%==true (
            echo latest_additional_maps_uri=%URL_PREFIX%/%DEPLOY_DIR%%LATEST_ADDITIONAL_MAPS_DEPLOY_NAME%>> "%SUMMARY_OUTPUT_PATH%"
        )
    )
)

rem ==============================================================================
rem -- ...and we are done --------------------------------------------------------
rem ==============================================================================

echo Success!
goto :good_exit

:error_package
    echo [%~nx0]: ERROR: Expected exactly one Carla-*-Win64-Shipping.zip in "%PACKAGE_DIR%".
    goto :bad_exit

:error_metadata
    echo [%~nx0]: ERROR: Expected exactly one *-release-metadata.tar.gz in "%PACKAGE_DIR%".
    goto :bad_exit

:error_additional_maps
    echo [%~nx0]: ERROR: Expected exactly one AdditionalMaps-*-Win64.tar.gz in "%PACKAGE_DIR%".
    goto :bad_exit

:good_exit
    endlocal
    exit /b 0

:bad_exit
    endlocal
    exit /b 1
