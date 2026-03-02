#!/bin/bash

CLEAN_FILE=$1

if [ -z "$CLEAN_FILE" ]; then
    echo "Usage: $0 <filename.idl>"
    exit 1
fi

if [ ! -f "$CLEAN_FILE" ]; then
    echo "Provided file '$CLEAN_FILE' not a regular file"
    echo "Usage: $0 <filename.idl>"
    exit 1
fi

echo "--- Processing: $CLEAN_FILE ---"

# 1. CHECK & REPLACE: #pragma once (Smart Placement)
if ! grep -qi "#pragma once" "$CLEAN_FILE"; then
    # Find the line number of the first line that does NOT start with / or *
    # We use grep -n to get line numbers, then head -n1 to get the first match
    FIRST_CODE_LINE=$(grep -nE -v "^([[:space:]]*[/]|([[:space:]]*[*]))" "$CLEAN_FILE" | head -n1 | cut -d: -f1)

    # If the file is all comments or empty, default to line 1
    if [ -z "$FIRST_CODE_LINE" ]; then FIRST_CODE_LINE=1; fi

    # Insert at the calculated line
    sed -i "${FIRST_CODE_LINE}i #pragma once\n" "$CLEAN_FILE"
    echo "[ADDED] #pragma once inserted at line $FIRST_CODE_LINE (after comments)."
else
    echo "[SKIP]  #pragma once already present."
fi

# 2. CHECK & REPLACE: \" with '
# Count matches before replacement
#COUNT=$(grep -c '\\"' "$CLEAN_FILE")

#if [ "$COUNT" -gt 0 ]; then
    # Perform replacement using the safe hex code for single quote
#    sed -i "s/\\\\\"/\x27/g" "$CLEAN_FILE"
    
    # Verify if any are left (should be 0)
#    REMAINING=$(grep -c '\\"' "$CLEAN_FILE")
#    SUCCESS_COUNT=$((COUNT - REMAINING))
    
#    echo "[FIXED] Replaced $SUCCESS_COUNT instance(s) of escaped quotes."
    
#    if [ "$REMAINING" -gt 0 ]; then
#        echo "[WARN]  $REMAINING instances could not be replaced (check file permissions)."
#    fi
#else
#    echo "[SKIP]  No escaped double quotes (\\\") found."
#fi

echo "--- Finished $CLEAN_FILE ---"