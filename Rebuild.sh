#!/bin/bash
set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
pushd "$SCRIPT_DIR" >/dev/null

UNREAL_PROJECT_FOLDER=./Unreal/CarlaUE4
UE4_INTERMEDIATE_FOLDERS="Binaries Build Intermediate DerivedDataCache"

function fatal_error {
  echo -e "\033[0;31mERROR: $1\033[0m"
  exit 1
}

function log {
  echo -e "\033[0;33m$1\033[0m"
}

if [ ! -d "${UE4_ROOT}" ]; then
  fatal_error "UE4_ROOT is not defined, or points to a non-existant directory, please set this environment variable."
fi

# ==============================================================================
# -- Make CarlaServer ----------------------------------------------------------
# ==============================================================================

log "Making CarlaServer..."
make clean && make debug && make release

# ==============================================================================
# -- Clean up intermediate Unreal files ----------------------------------------
# ==============================================================================

pushd "$UNREAL_PROJECT_FOLDER" >/dev/null

pushd "Plugins/Carla" >/dev/null

log "Cleaning up CARLA Plugin..."
rm -Rf ${UE4_INTERMEDIATE_FOLDERS}

popd > /dev/null

log "Cleaning up CARLAUE4..."
rm -Rf ${UE4_INTERMEDIATE_FOLDERS}

popd >/dev/null

# ==============================================================================
# -- Build and launch Unreal project -------------------------------------------
# ==============================================================================

pushd "$UNREAL_PROJECT_FOLDER" >/dev/null

# This command usually fails but normally we can continue anyway.
set +e
log "Generate Unreal project files..."
${UE4_ROOT}/GenerateProjectFiles.sh -project="${PWD}/CarlaUE4.uproject" -game -engine
set -e

log "Build CarlaUE4 project..."
make CarlaUE4Editor

log "Launching UE4Editor..."
${UE4_ROOT}/Engine/Binaries/Linux/UE4Editor "${PWD}/CarlaUE4.uproject"

popd >/dev/null

# ==============================================================================
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

popd >/dev/null
