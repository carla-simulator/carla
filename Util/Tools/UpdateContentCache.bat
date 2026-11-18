@echo off
setlocal EnableDelayedExpansion

rem ==============================================================================
rem -- Set up environment --------------------------------------------------------
rem ==============================================================================

set REPO_URL=https://bitbucket.org/carla-simulator/carla-content.git
set REPO_DIR=
set BRANCH=

rem ==============================================================================
rem -- Parse arguments -----------------------------------------------------------
rem ==============================================================================

set DOC_STRING=Create or update a shallow clone of the CARLA content repository to be used as a CI cache. WARNING: any local change, untracked file or build artifact in PATH is deleted.

set USAGE_STRING=Usage: %~nx0 [-h, --help] --path PATH --branch BRANCH [--url URL]

:arg-parse
if not "%~1"=="" (
    if "%~1"=="--path" (
        set "REPO_DIR=%~2"
        shift
    ) else if "%~1"=="--branch" (
        set "BRANCH=%~2"
        shift
    ) else if "%~1"=="--url" (
        set "REPO_URL=%~2"
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

rem Both arguments are required on purpose, so the script never wipes a directory by default
if "%REPO_DIR%"=="" goto :error_arguments
if "%BRANCH%"=="" goto :error_arguments

rem ==============================================================================
rem -- Initial set up ------------------------------------------------------------
rem ==============================================================================

if not exist "%REPO_DIR%\.git" (

    rem Never touch a non-empty directory that is not a Git repository
    if exist "%REPO_DIR%" (
        dir /b /a "%REPO_DIR%" 2>nul | findstr "^" >nul && goto :error_not_empty
    )

    echo Cache not found, initializing %REPO_DIR%.

    rem Initialize in place instead of cloning, so it also works when PATH is a mount point.
    rem The content is downloaded by the fetch below; if it is interrupted,
    rem the next run simply fetches again.
    git init -q "%REPO_DIR%" || goto :bad_exit
    git -C "%REPO_DIR%" remote add -t "%BRANCH%" origin "%REPO_URL%" || goto :bad_exit

    rem Prevent automatic garbage collection in the middle of a fetch
    git -C "%REPO_DIR%" config gc.auto 0 || goto :bad_exit

    rem The external actor packages of the large maps exceed MAX_PATH
    git -C "%REPO_DIR%" config core.longpaths true || goto :bad_exit
)

pushd "%REPO_DIR%" || goto :bad_exit

rem Make sure PATH really is the content repository before deleting anything in it
set ORIGIN_URL=
for /f "delims=" %%i in ('git remote get-url origin') do set ORIGIN_URL=%%i
if not "%ORIGIN_URL%"=="%REPO_URL%" goto :error_not_content

rem ==============================================================================
rem -- Update --------------------------------------------------------------------
rem ==============================================================================

rem Remove stale lock files left behind by a cancelled job
if exist .git\*.lock del /f /q .git\*.lock

echo Fetching latest revision of %BRANCH%.

rem Fetch the latest revision and make the working tree identical to it
git fetch --depth 1 --no-tags origin "%BRANCH%" || goto :bad_exit
git reset --hard FETCH_HEAD || goto :bad_exit
git clean -ffdxq || goto :bad_exit

rem ==============================================================================
rem -- Clean up old revisions ----------------------------------------------------
rem ==============================================================================

echo Removing objects from previous revisions.

rem The reflog must be expired first, otherwise gc still considers
rem old commits reachable and frees nothing
git reflog expire --expire=now --all || goto :bad_exit
git gc --prune=now || goto :bad_exit

rem ==============================================================================
rem -- Verify --------------------------------------------------------------------
rem ==============================================================================

rem Fail if anything other than the exact origin revision remains
set GIT_STATUS=
for /f "delims=" %%i in ('git status --porcelain --ignored') do set GIT_STATUS=%%i
if defined GIT_STATUS (
    git status --short --ignored
    goto :error_not_clean
)

for /f "delims=" %%i in ('git rev-parse --short HEAD') do set CACHE_REVISION=%%i
echo Cache at revision %CACHE_REVISION%.

rem ==============================================================================
rem -- ...and we are done --------------------------------------------------------
rem ==============================================================================

echo Success!
goto :good_exit

:error_arguments
    echo %USAGE_STRING%
    echo [%~nx0]: ERROR: --path and --branch are required
    goto :bad_exit

:error_not_empty
    echo [%~nx0]: ERROR: %REPO_DIR% exists, is not empty and is not a Git repository
    goto :bad_exit

:error_not_content
    echo [%~nx0]: ERROR: %REPO_DIR% is not a clone of %REPO_URL%
    goto :bad_exit

:error_not_clean
    echo [%~nx0]: ERROR: Cache is not clean
    goto :bad_exit

:good_exit
    popd
    endlocal
    exit /b 0

:bad_exit
    popd 2>nul
    endlocal
    exit /b 1
