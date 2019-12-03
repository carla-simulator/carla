@rem convert FBX to OBJ
FBX2OBJ.exe %1.fbx %1.obj

@rem parse openDRIVE crosswalks (generate crosswalks.obj)
python get_xodr_crosswalks.py -f %1.xodr

@rem join both OBJ
python addOBJ.py %1.obj crosswalks.obj

@rem calculate the BIN file (result is same name .BIN)
RecastBuilder.exe %1.obj
