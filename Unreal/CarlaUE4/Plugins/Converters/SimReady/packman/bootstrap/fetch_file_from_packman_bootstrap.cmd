:: Copyright 2019 NVIDIA CORPORATION
::
:: Licensed under the Apache License, Version 2.0 (the "License");
:: you may not use this file except in compliance with the License.
:: You may obtain a copy of the License at
::
::    http://www.apache.org/licenses/LICENSE-2.0
::
:: Unless required by applicable law or agreed to in writing, software
:: distributed under the License is distributed on an "AS IS" BASIS,
:: WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
:: See the License for the specific language governing permissions and
:: limitations under the License.

:: You need to specify <package-name> <target-path> as input to this command
@setlocal
@set PACKAGE_NAME=%1
@set TARGET_PATH=%2

@echo Fetching %PACKAGE_NAME% ...

@powershell -ExecutionPolicy ByPass -NoLogo -NoProfile -File "%~dp0download_file_from_url.ps1" ^
    -source "https://bootstrap.packman.nvidia.com/%PACKAGE_NAME%" -output %TARGET_PATH%
:: A bug in powershell prevents the errorlevel code from being set when using the -File execution option
:: We must therefore do our own failure analysis, basically make sure the file exists:
@if not exist %TARGET_PATH% goto ERROR_DOWNLOAD_FAILED

@endlocal
@exit /b 0

:ERROR_DOWNLOAD_FAILED
@echo Failed to download file from S3
@echo Most likely because endpoint cannot be reached or file %PACKAGE_NAME% doesn't exist
@endlocal
@exit /b 1