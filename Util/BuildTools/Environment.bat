@echo off
setlocal enabledelayedexpansion

rem BAT script containing utility functions for reuse across other scripts.

:get_git_repository_version
    for /f "delims=" %%b in ('git rev-parse --abbrev-ref HEAD') do set BRANCH=%%b

    echo %BRANCH% | findstr /b "ue4/" >nul
    if %errorlevel%==0 (
        set TAG=%BRANCH:ue4/=%
    ) else (
        REM Get short commit hash
        for /f "delims=" %%c in ('git rev-parse --short HEAD') do set COMMIT=%%c

        REM Check if working directory is dirty
        git diff-index --quiet HEAD --
        if %errorlevel%==0 (
            set DIRTY=
        ) else (
            set DIRTY=-dirty
        )
        set TAG=!COMMIT!!DIRTY!
    )

    goto :good_exit

:good_exit
    endlocal & set REPOSITORY_TAG=%TAG%
    exit /b 0
