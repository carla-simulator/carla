#! /bin/bash

# ==============================================================================
# -- Set up environment --------------------------------------------------------
# ==============================================================================

source $(dirname "$0")/Environment.sh

if [ ! -d "${UE4_ROOT}" ]; then
  fatal_error "UE4_ROOT is not defined, or points to a non-existant directory, please set this environment variable."
else
  log "Using Unreal Engine at '$UE4_ROOT'"
fi

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DOC_STRING="Build and launch CarlaUE4."

USAGE_STRING="Usage: $0 [-h|--help] [--build] [--rebuild] [--launch] [--clean] [--hard-clean] [--opengl]"

REMOVE_INTERMEDIATE=false
HARD_CLEAN=false
BUILD_CARLAUE4=false
LAUNCH_UE4_EDITOR=false

GDB=
RHI="-vulkan"

OPTS=`getopt -o h --long help,build,rebuild,launch,clean,hard-clean,gdb,opengl -n 'parse-options' -- "$@"`

eval set -- "$OPTS"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --gdb )
      GDB="gdb --args";
      shift ;;
    --build )
      BUILD_CARLAUE4=true;
      shift ;;
    --rebuild )
      REMOVE_INTERMEDIATE=true;
      BUILD_CARLAUE4=true;
      shift ;;
    --launch )
      LAUNCH_UE4_EDITOR=true;
      shift ;;
    --clean )
      REMOVE_INTERMEDIATE=true;
      shift ;;
    --hard-clean )
      REMOVE_INTERMEDIATE=true;
      HARD_CLEAN=true;
      shift ;;
    --opengl )
      RHI="-opengl";
      shift ;;
    -h | --help )
      echo "$DOC_STRING"
      echo "$USAGE_STRING"
      exit 1
      ;;
    * )
      shift ;;
  esac
done

if ! { ${REMOVE_INTERMEDIATE} || ${BUILD_CARLAUE4} || ${LAUNCH_UE4_EDITOR}; }; then
  fatal_error "Nothing selected to be done."
fi

pushd "${CARLAUE4_ROOT_FOLDER}" >/dev/null

# ==============================================================================
# -- Clean CarlaUE4 ------------------------------------------------------------
# ==============================================================================

if ${HARD_CLEAN} ; then

  if [ ! -f Makefile ]; then
    fatal_error "The project wasn't built before!"
  fi

  log "Doing a \"hard\" clean of the Unreal Engine project."

  make CarlaUE4Editor ARGS=-clean

fi

if ${REMOVE_INTERMEDIATE} ; then

  log "Cleaning intermediate files and folders."

  UE4_INTERMEDIATE_FOLDERS="Binaries Build Intermediate DerivedDataCache"

  rm -Rf ${UE4_INTERMEDIATE_FOLDERS}

  rm -f Makefile

  pushd "${CARLAUE4_PLUGIN_ROOT_FOLDER}" >/dev/null

  rm -Rf ${UE4_INTERMEDIATE_FOLDERS}

  popd >/dev/null

fi

# ==============================================================================
# -- Build CarlaUE4 ------------------------------------------------------------
# ==============================================================================

if ${BUILD_CARLAUE4} ; then

  if [ ! -f Makefile ]; then

    # This command fails sometimes but normally we can continue anyway.
    set +e
    log "Generate Unreal project files."
    ${UE4_ROOT}/GenerateProjectFiles.sh -project="${PWD}/CarlaUE4.uproject" -game -engine -makefiles
    set -e

  fi

  log "Build CarlaUE4 project."
  make CarlaUE4Editor

  #Providing the user with the ExportedMaps folder
  EXPORTED_MAPS="${CARLAUE4_ROOT_FOLDER}/Content/Carla/ExportedMaps"
  mkdir -p "${EXPORTED_MAPS}"


fi

# ==============================================================================
# -- Launch UE4Editor ----------------------------------------------------------
# ==============================================================================

if ${LAUNCH_UE4_EDITOR} ; then

  log "Launching UE4Editor..."
  ${GDB} ${UE4_ROOT}/Engine/Binaries/Linux/UE4Editor "${PWD}/CarlaUE4.uproject" ${RHI}

else

  log "Success!"

fi

# ==============================================================================
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

popd >/dev/null
