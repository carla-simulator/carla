#!/bin/bash

# --- Configuration ---
# Set the directory where your generated files are located. 
# "." means the current directory.
TARGET_DIR="."

# --- Replacement Rules ---
# Rule 1: ' BEFORE_DECLARATION' -> ' PlacementKind::BEFORE_DECLARATION'
# Rule 2: 'PlacementKindValue'  -> 'PlacementKind'
# Rule 3. 'BLABLA' (temporary type/name fixes)   -> '' (remove this)
# Rule 4. '""' (empty quotes)   -> '"' (single quotes)

echo "Starting post-processing of Fast DDS generated files in: $TARGET_DIR"

# We use find to target only relevant source/header files.
# -i is for in-place editing.
# We use a '|' as a separator in sed to handle spaces safely.
find "$TARGET_DIR" -type f \( -name "*.h" -o -name "*.hpp" -o -name "*.ipp" -o -name "*.cxx" -o -name "*.cpp" \) -print0 | xargs -0 sed -i \
    -e 's|BLABLA||g' \
    -e 's| BEFORE_DECLARATION| PlacementKind::BEFORE_DECLARATION|g' \
    -e 's|PlacementKindValue|PlacementKind|g' \
    -e 's|\"\"|"|g' \

echo "Done! Processed $(find "$TARGET_DIR" -type f \( -name "*.h" -o -name "*.hpp" -o -name "*.ipp" -o -name "*.cxx" -o -name "*.cpp" \) | wc -l) files."
