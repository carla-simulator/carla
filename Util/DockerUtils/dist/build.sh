#! /bin/bash

# check if the FBX file exist
if [ ! -f "$1.fbx" ]; then
    echo Nothing to do: there is no FBX file
    exit
fi

# prepare to load .SO from current folder
export LD_LIBRARY_PATH="./"
# convert FBX to OBJ
chmod +x FBX2OBJ
./FBX2OBJ "$1.fbx" "$1.obj"

# check if the OBJ file exist
if [ ! -f "$1.obj" ]; then
    echo Nothing to do: conversion from FBX to OBJ failed
    exit
fi

# check if the XODR file exist
if [ -f "$1.xodr" ]; then
    # parse openDRIVE crosswalks (generate crosswalks.obj)
    python get_xodr_crosswalks.py -f "$1.xodr"
else
    echo "XODR file doesn't exist, ignoring crosswalks from openDRIVE"
fi

# check if the 'crosswalks.obj' file exist
if [ -f "crosswalks.obj" ]; then
    # join both OBJ
    python addOBJ.py "$1.obj" crosswalks.obj
    rm crosswalks.obj
fi

# calculate the BIN file (result is same name .BIN)
chmod +x RecastBuilder
./RecastBuilder "$1.obj"

echo Success!
