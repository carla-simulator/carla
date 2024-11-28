#! /bin/bash

if [ -f "crosswalksmgs.obj" ]; then
    rm crosswalksmgs.obj
fi

# check if the FBX file exist
if [ -f "FBX2OBJ" ]; then
    if [ -f "$1.fbx" ]; then
        # prepare to load .SO from current folder
        export LD_LIBRARY_PATH="./"
        # convert FBX to OBJ
        chmod +x FBX2OBJ
        ./FBX2OBJ "$1.fbx" "$1.obj"
    fi
fi



# check if the 'crosswalksmgs.obj' file exist
if [ -f "crosswalksmgs.obj" ]; then
    # join both OBJ
    python addOBJ.py "$1.obj" crosswalksmgs.obj
fi

if [ -f "$1.obj" ]; then
    if [ -f "RecastBuilder" ]; then
        # calculate the BIN file (result is same name .BIN)
        chmod +x RecastBuilder
        ./RecastBuilder "$1.obj"
    fi
fi
