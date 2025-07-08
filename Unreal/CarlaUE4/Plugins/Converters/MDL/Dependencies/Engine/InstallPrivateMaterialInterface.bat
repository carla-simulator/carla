:: This script copies some of the private material interface from the engine
:: into the MDL plugin to facilitate some advanced material and material instance
:: creation. They are patched to work properly within the context of the MDL plugin.

@echo off

setlocal

if not defined UE4_ROOT (
    echo Error: UE4_ROOT environment variable that points to the base Unreal Engine folder is not defined
    exit /B -1
)

set MAT_DIR=%UE4_ROOT%\Engine\Source\Runtime\Engine\Private\Materials
set DEST_MAT_DIR=%~dp0..\..\Source\MDL\Private\Engine

:: If the files already exist don't update them
if not exist %MAT_DIR% (
    echo Error: missing Unreal Engine private materials folder: %MAT_DIR%
    exit /B -1
)

if not exist %DEST_MAT_DIR% (
    mkdir %DEST_MAT_DIR%
)
pushd %DEST_MAT_DIR%

:: If the files already exist don't update them
if exist HLSLMaterialTranslator.cpp (
    goto finished
)

:: Copy the private engine material files to this folder
copy /Y %MAT_DIR%\HLSLMaterialTranslator.cpp        HLSLMaterialTranslator.cpp > nul
copy /Y %MAT_DIR%\HLSLMaterialTranslator.h          HLSLMaterialTranslator.h > nul
copy /Y %MAT_DIR%\MaterialInstanceSupport.h         MaterialInstanceSupport.h > nul
copy /Y %MAT_DIR%\MaterialUniformExpressions.cpp    MaterialUniformExpressions.cpp > nul
copy /Y %MAT_DIR%\MaterialUniformExpressions.h      MaterialUniformExpressions.h > nul

:: Apply the patch to the private engine material files
call git apply --whitespace=nowarn %~dp0HLSLMaterialTranslator.cpp.patch
call git apply --whitespace=nowarn %~dp0HLSLMaterialTranslator.h.patch
call git apply --whitespace=nowarn %~dp0MaterialInstanceSupport.h.patch
call git apply --whitespace=nowarn %~dp0MaterialUniformExpressions.cpp.patch
call git apply --whitespace=nowarn %~dp0MaterialUniformExpressions.h.patch

:finished

popd