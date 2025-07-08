#!/bin/bash

# This script copies some of the private material interface from the engine
# into the MDL plugin to facilitate some advanced material and material instance
# creation. They are patched to work properly within the context of the MDL plugin.

set -e

if [ ! -d $UE4_ROOT ]; then
    echo "Error: UE4_ROOT environment variable that points to the base Unreal Engine folder is not defined"
    exit -1
fi

SCRIPT_DIR=$(dirname "$(realpath "$0")")
MAT_DIR=$UE4_ROOT/Engine/Source/Runtime/Engine/Private/Materials
DEST_MAT_DIR=$SCRIPT_DIR/../../Source/MDL/Private/Engine

# If the files already exist don't update them
if [ ! -d $MAT_DIR ]; then
    echo "Error: missing Unreal Engine private materials folder: $MAT_DIR"
    exit -1
fi

if [ ! -d $DEST_MAT_DIR ]; then
    mkdir $DEST_MAT_DIR
fi
pushd $DEST_MAT_DIR > /dev/null

# If the files already exist and are newer than the patch files don't update them
if [ -f HLSLMaterialTranslator.cpp ]; then
    if [ HLSLMaterialTranslator.cpp -nt $SCRIPT_DIR/HLSLMaterialTranslator.cpp.patch ]; then
        echo "Success: HLSLMaterialTranslator.cpp newer than $SCRIPT_DIR/HLSLMaterialTranslator.cpp.patch"
        echo "         Continuing without installing and patching Engine Material source"
        popd > /dev/null
        exit 0
    fi
fi

# Copy the private engine material files to this folder
cp -f $MAT_DIR/HLSLMaterialTranslator.cpp        HLSLMaterialTranslator.cpp > /dev/null
cp -f $MAT_DIR/HLSLMaterialTranslator.h          HLSLMaterialTranslator.h > /dev/null
cp -f $MAT_DIR/MaterialInstanceSupport.h         MaterialInstanceSupport.h > /dev/null
cp -f $MAT_DIR/MaterialUniformExpressions.cpp    MaterialUniformExpressions.cpp > /dev/null
cp -f $MAT_DIR/MaterialUniformExpressions.h      MaterialUniformExpressions.h > /dev/null

# Apply the patch to the private engine material files
git apply --whitespace=nowarn $SCRIPT_DIR/HLSLMaterialTranslator.cpp.patch
git apply --whitespace=nowarn $SCRIPT_DIR/HLSLMaterialTranslator.h.patch
git apply --whitespace=nowarn $SCRIPT_DIR/MaterialInstanceSupport.h.patch
git apply --whitespace=nowarn $SCRIPT_DIR/MaterialUniformExpressions.cpp.patch
git apply --whitespace=nowarn $SCRIPT_DIR/MaterialUniformExpressions.h.patch

popd > /dev/null
