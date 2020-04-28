@echo off

if exist "crosswalks.obj" del crosswalks.obj

if exist "FBX2OBJ.exe" (
    if exist "%1.fbx" (
        rem convert FBX to OBJ
        FBX2OBJ.exe %1.fbx %1.obj
    )
)

if exist "%1.xodr" (
    rem parse openDRIVE crosswalks (generate crosswalks.obj)
    python get_xodr_crosswalks.py -f %1.xodr
) else (
    echo "XODR file doesn't exist, ignoring crosswalks from openDRIVE"
)

if exist "crosswalks.obj" (
    rem join both OBJ
    python addOBJ.py %1.obj crosswalks.obj
)
 
if exist "%1.obj" (
    if exist "RecastBuilder.exe" (
        rem calculate the BIN file (result is same name .BIN)
        RecastBuilder.exe %1.obj
    )
)
