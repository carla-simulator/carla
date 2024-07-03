@echo off
set CONTENT_URL=%1
set CONTENT_ARCHIVE_PATH=%2
set CONTENT_PATH=%3
echo Downloading Carla Content from "%CONTENT_URL%" to "%CONTENT_ARCHIVE_PATH%".
powershell -Command "(New-Object System.Net.WebClient).DownloadFile('%CONTENT_URL%', '%CONTENT_ARCHIVE_PATH%')"
if %errorlevel% neq 0 (
  echo ERROR, failed to download Carla Content.
  exit /b %errorlevel%
)
echo Extracting Carla Content from "%CONTENT_ARCHIVE_PATH%" to "%CONTENT_PATH%".
powershell -Command "Expand-Archive '%CONTENT_ARCHIVE_PATH%' -DestinationPath '%CONTENT_PATH%'"
if %errorlevel% neq 0 (
  echo ERROR, failed to extract Carla Content.
)
