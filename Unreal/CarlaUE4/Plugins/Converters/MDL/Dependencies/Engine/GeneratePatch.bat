:: This file was run from the %~dp0..\..\Source\MDL\Private\Engine folder prior to removing the modified
:: engine files from the repository
@echo off

setlocal

set MAT_DIR=%UE4_ROOT%\Engine\Source\Runtime\Engine\Private\Materials
set DEST_MAT_DIR=%~dp0..\..\Source\MDL\Private\Engine

call git diff --no-index %MAT_DIR%\HLSLMaterialTranslator.cpp        %DEST_MAT_DIR%\HLSLMaterialTranslator.cpp     > HLSLMaterialTranslator.cpp.patch
call git diff --no-index %MAT_DIR%\HLSLMaterialTranslator.h          %DEST_MAT_DIR%\HLSLMaterialTranslator.h       > HLSLMaterialTranslator.h.patch
call git diff --no-index %MAT_DIR%\MaterialInstanceSupport.h         %DEST_MAT_DIR%\MaterialInstanceSupport.h      > MaterialInstanceSupport.h.patch
call git diff --no-index %MAT_DIR%\MaterialUniformExpressions.cpp    %DEST_MAT_DIR%\MaterialUniformExpressions.cpp > MaterialUniformExpressions.cpp.patch
call git diff --no-index %MAT_DIR%\MaterialUniformExpressions.h      %DEST_MAT_DIR%\MaterialUniformExpressions.h   > MaterialUniformExpressions.h.patch

:: NOTE, the index (first two lines) needs to be removed from these .patch files (even though we're using the --no-index arg)