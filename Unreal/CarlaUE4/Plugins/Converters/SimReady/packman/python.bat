:: Copyright 2019-2020 NVIDIA CORPORATION
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

@echo off
setlocal enableextensions

call "%~dp0\packman" init
set "PYTHONPATH=%PM_MODULE_DIR%;%PYTHONPATH%"

if not defined PYTHONNOUSERSITE (
    set PYTHONNOUSERSITE=1
)

REM For performance, default to unbuffered; however, allow overriding via
REM PYTHONUNBUFFERED=0 since PYTHONUNBUFFERED on windows can truncate output
REM when printing long strings
if not defined PYTHONUNBUFFERED (
    set PYTHONUNBUFFERED=1
)

"%PM_PYTHON%" %*