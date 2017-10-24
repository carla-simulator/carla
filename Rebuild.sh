#!/bin/bash
set -e

PLUGIN_FOLDER=./Plugins/Carla
PLUGIN_INTERMEDIATE_FOLDERS="Binaries Intermediate"
CARLAUE4_INTERMEDIATE_FOLDERS="Binaries Build Intermediate DerivedDataCache"

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

log "Cleaning up CARLA Plugin..."
(cd ${PLUGIN_FOLDER} && rm -Rf ${PLUGIN_INTERMEDIATE_FOLDERS})
log "Cleaning up CARLAUE4..."
rm -Rf ${CARLAUE4_INTERMEDIATE_FOLDERS}

log "Making CARLA Plugin dependencies..."
(cd ${PLUGIN_FOLDER} && make clean && make debug && make release)

# This command usually fails but we can continue anyway.
set +e
log "Generate Unreal project files..."
${UE4_ROOT}/GenerateProjectFiles.sh -project="${PWD}/CarlaUE4.uproject" -game -engine
set -e

log "Build CarlaUE4 project..."
make CarlaUE4Editor

log "Launch editor..."
${UE4_ROOT}/Engine/Binaries/Linux/UE4Editor "${PWD}/CarlaUE4.uproject"
