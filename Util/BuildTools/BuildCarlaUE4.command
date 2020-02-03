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

USAGE_STRING="Usage: $0 [-h|--help] [--build] [--rebuild] [--launch] [--clean] [--hard-clean] [--[no]-xcode]"

REMOVE_INTERMEDIATE=false
HARD_CLEAN=false
BUILD_CARLAUE4=false
LAUNCH_UE4_EDITOR=false

USE_XCODE=true
if [[ -f ${CARLA_BUILD_FOLDER}/NO_XCODEBUILD ]] ; then
  USE_XCODE=false
fi

while true; do
  case "$1" in
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
    --no-xcode )
      USE_XCODE=false;
      shift ;;
    --xcode )
      USE_XCODE=true;
      shift ;;
    -h | --help )
      echo "$DOC_STRING"
      echo "$USAGE_STRING"
      exit 1
      ;;
    * )
      if [ ! -z "$1" ]; then
        echo "Bad argument: '$1'"
        echo "$USAGE_STRING"
        exit 2
      fi
      break ;;
  esac
done

if ! { ${REMOVE_INTERMEDIATE} || ${BUILD_CARLAUE4} || ${LAUNCH_UE4_EDITOR}; }; then
  fatal_error "Nothing selected to be done."
fi

pushd "${CARLAUE4_ROOT_FOLDER}" >/dev/null

# ==============================================================================
# -- Clean CarlaUE4 ------------------------------------------------------------
# ==============================================================================

if ${REMOVE_INTERMEDIATE} ; then

  log "Cleaning intermediate files and folders."

  UE4_INTERMEDIATE_FOLDERS="Binaries Build Intermediate DerivedDataCache"

  rm -Rf ${UE4_INTERMEDIATE_FOLDERS}

  pushd "${CARLAUE4_PLUGIN_ROOT_FOLDER}" >/dev/null

  rm -Rf ${UE4_INTERMEDIATE_FOLDERS}

  popd >/dev/null

fi

if ${HARD_CLEAN} ; then

  if [ ! -f Makefile ]; then
    fatal_error "The project wasn't built before!"
  fi

  log "Doing a \"hard\" clean of the Unreal Engine project."

  if ${USE_XCODE}; then
    xcodebuild -scheme CarlaUe4 clean
  else
    make CarlaUE4Editor ARGS=-clean
  fi

fi

if ${REMOVE_INTERMEDIATE} ; then

  rm -f Makefile 
  rm -rf CarlaUe4.xcworkspace

fi

# ==============================================================================
# -- Build CarlaUE4 ------------------------------------------------------------
# ==============================================================================

if ${BUILD_CARLAUE4} ; then

  if [ ! -f Makefile ]; then

    # This command fails sometimes but normally we can continue anyway.
    set +e
    log "Generate Unreal project files."

    if [[ ! -f ${UE4_ROOT}/GenerateProjectFiles.sh ]]; then
      fatal_error "No GenerateProjectFiles.sh in ${UE4_ROOT}. You can copy this from UnrealEngine source tree."
    fi

    if ${USE_XCODE}; then
      "${UE4_ROOT}/GenerateProjectFiles.sh" -project="${PWD}/CarlaUE4.uproject" -game -engine -xcode
    else
      "${UE4_ROOT}/GenerateProjectFiles.sh" -project="${PWD}/CarlaUE4.uproject" -game -engine -makefiles
      # HACK! This generates wrong targets on the Mac! Just replace Linux with Mac everywhere:
      sed -i .original -e "s/Linux/Mac/g" Makefile
    fi

    set -e

  fi

  log "Build CarlaUE4 project."
  if ${USE_XCODE}; then
    xcodebuild -scheme CarlaUE4 -target CarlaUE4Editor
  else
    make CarlaUE4Editor
  fi

fi

# ==============================================================================
# -- Launch UE4Editor ----------------------------------------------------------
# ==============================================================================

if ${LAUNCH_UE4_EDITOR} ; then

  log "Launching UE4Editor..."
  /usr/bin/open -a "${UE4_ROOT}/Engine/Binaries/Mac/UE4Editor.app" "${PWD}/CarlaUE4.uproject"

else

  log "Success!"

fi

# ==============================================================================
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

popd >/dev/null
