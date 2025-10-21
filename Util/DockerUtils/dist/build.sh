#! /bin/bash

# Usage: build.sh <mapname> [xodr_name] [--skip-xodr]
# By default, crosswalks are parsed from XODR and merged into the OBJ
# Use --skip-xodr flag to skip XODR parsing and use crosswalks from CarlaExporter (already in the main OBJ)

SKIP_XODR=false

# Check for --skip-xodr flag in arguments
for arg in "$@"; do
    if [ "$arg" == "--skip-xodr" ]; then
        SKIP_XODR=true
    fi
done

if [ "$SKIP_XODR" = false ]; then
    if [ -f "crosswalks.obj" ]; then
        rm crosswalks.obj
    fi
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

# Parse crosswalks from XODR unless --skip-xodr flag is provided
if [ "$SKIP_XODR" = false ]; then
    # check if the XODR file exist
    if [ -f "$1.xodr" ]; then
        # parse openDRIVE crosswalks (generate crosswalks.obj)
        python3 get_xodr_crosswalks.py -f "$1.xodr"
    else
        if [ -f "$2.xodr" ]; then
            # parse openDRIVE crosswalks (generate crosswalks.obj)
            python3 get_xodr_crosswalks.py -f "$2.xodr"
        else
            echo "XODR file doesn't exist, ignoring crosswalks from openDRIVE"
        fi
    fi

    # check if the 'crosswalks.obj' file exist
    if [ -f "crosswalks.obj" ]; then
        # join both OBJ
        python3 addOBJ.py "$1.obj" crosswalks.obj
    fi
else
    echo "Skipping XODR crosswalk parsing (--skip-xodr flag detected)"
    echo "Using crosswalks from CarlaExporter (included in main OBJ)"
fi

if [ -f "$1.obj" ]; then
    if [ -f "RecastBuilder" ]; then
        # calculate the BIN file (result is same name .BIN)
        chmod +x RecastBuilder
        ./RecastBuilder "$1.obj"
    fi
fi
