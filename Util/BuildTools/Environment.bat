@echo off
setlocal enabledelayedexpansion

rem BAT script containing utility functions for reuse across other scripts.

:get_git_repository_version
    for /f "delims=" %%b in ('git rev-parse --abbrev-ref HEAD') do set BRANCH=%%b

    echo %BRANCH% | findstr /b "ue4/" >nul
    if %errorlevel%==0 (
        set TAG=%BRANCH:ue4/=%
    ) else (
        for /f "delims=" %%v in ('git describe --tags --dirty --always') do set TAG=%%v
    )

    goto :good_exit

:good_exit
    endlocal & set REPOSITORY_TAG=%TAG%
    exit /b 0
