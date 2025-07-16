set MAT_DIR=%UE4_ROOT%\Engine\Source\Runtime\Engine\Private\Materials
set DEST_MAT_DIR=%~dp0..\..\Source\MDL\Private\Engine

call p4merge %MAT_DIR%\HLSLMaterialTranslator.cpp        %DEST_MAT_DIR%\HLSLMaterialTranslator.cpp      
call p4merge %MAT_DIR%\HLSLMaterialTranslator.h          %DEST_MAT_DIR%\HLSLMaterialTranslator.h        
call p4merge %MAT_DIR%\MaterialInstanceSupport.h         %DEST_MAT_DIR%\MaterialInstanceSupport.h       
call p4merge %MAT_DIR%\MaterialUniformExpressions.cpp    %DEST_MAT_DIR%\MaterialUniformExpressions.cpp  
call p4merge %MAT_DIR%\MaterialUniformExpressions.h      %DEST_MAT_DIR%\MaterialUniformExpressions.h    
