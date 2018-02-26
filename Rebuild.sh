#!/bin/bash

################################################################################
# Updates CARLA content.
################################################################################

set -e

DOC_STRING="Update CARLA content to the latest version, to be run after 'git pull'."

USAGE_STRING="Usage: $0 [-h|--help] [--no-editor]"

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

LAUNCH_UE4_EDITOR=true

OPTS=`getopt -o h --long help,no-editor -n 'parse-options' -- "$@"`

if [ $? != 0 ] ; then echo "$USAGE_STRING" ; exit 2 ; fi

eval set -- "$OPTS"

while true; do
  case "$1" in
    --no-editor )
      LAUNCH_UE4_EDITOR=false;
      shift ;;
    -h | --help )
      echo "$DOC_STRING"
      echo "$USAGE_STRING"
      exit 1
      ;;
    * )
      break ;;
  esac
done

# ==============================================================================
# -- Set up environment --------------------------------------------------------
# ==============================================================================

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
else
  echo "Using Unreal Engine at '$UE4_ROOT'"
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

if $LAUNCH_UE4_EDITOR ; then
  log "Launching UE4Editor..."
  ${UE4_ROOT}/Engine/Binaries/Linux/UE4Editor "${PWD}/CarlaUE4.uproject"
else
  echo ""
  echo "****************"
  echo "*** Success! ***"
  echo "****************"
fi

popd >/dev/null

# ==============================================================================
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

popd >/dev/null
