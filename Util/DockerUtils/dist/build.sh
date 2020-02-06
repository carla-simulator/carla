# convert FBX to OBJ
chmod +x FBX2OBJ
./FBX2OBJ "$1.fbx" "$1.obj"

# parse openDRIVE crosswalks (generate crosswalks.obj)
python get_xodr_crosswalks.py -f "$1.xodr"

# join both OBJ
python addOBJ.py "$1.obj" crosswalks.obj

# calculate the BIN file (result is same name .BIN)
chmod +x RecastBuilder
./RecastBuilder "$1.obj"
