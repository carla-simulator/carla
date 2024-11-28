@echo off



if exist "FBX2OBJ.exe" (
    if exist "%1.fbx" (
        rem convert FBX to OBJ
        FBX2OBJ.exe %1.fbx %1.obj
    )
)


if exist "crosswalksmgs.obj" (
    rem join both OBJ
    python addOBJ.py %1.obj crosswalksmgs.obj
)
 
if exist "%1.obj" (
    if exist "RecastBuilder.exe" (
        rem calculate the BIN file (result is same name .BIN)
        RecastBuilder.exe %1.obj
    )
)
