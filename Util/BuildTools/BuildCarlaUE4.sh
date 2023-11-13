#! /bin/bash

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DOC_STRING="Build and launch CarlaUE4."

USAGE_STRING="Usage: $0 [-h|--help] [--build] [--rebuild] [--launch] [--clean] [--hard-clean] [--opengl]"

REMOVE_INTERMEDIATE=false
HARD_CLEAN=false
BUILD_CARLAUE4=false
LAUNCH_UE4_EDITOR=false
USE_CARSIM=false
USE_CHRONO=false
USE_PYTORCH=false
USE_UNITY=true
USE_ROS2=false

EDITOR_FLAGS=""
USE_HOUDINI=false

GDB=
RHI="-vulkan"

OPTS=`getopt -o h --long help,build,rebuild,launch,clean,hard-clean,gdb,opengl,carsim,pytorch,chrono,ros2,no-unity,editor-flags: -n 'parse-options' -- "$@"`

eval set -- "$OPTS"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --editor-flags )
      EDITOR_FLAGS=$2
      shift ;;
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
    --carsim )
      USE_CARSIM=true;
      shift ;;
    --chrono )
      USE_CHRONO=true
      shift ;;
    --pytorch )
      USE_PYTORCH=true;
      shift ;;
    --ros2 )
      USE_ROS2=true;
      shift ;;
    --no-unity )
      USE_UNITY=false
      shift ;;
    --with-houdini )
      USE_HOUDINI=true;
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

# ==============================================================================
# -- Set up environment --------------------------------------------------------
# ==============================================================================

source $(dirname "$0")/Environment.sh

if [ ! -d "${UE4_ROOT}" ]; then
  fatal_error "UE4_ROOT is not defined, or points to a non-existant directory, please set this environment variable."
else
  log "Using Unreal Engine at '$UE4_ROOT'"
fi

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
# -- Download Houdini Plugin for Unreal Engine ---------------------------------
# ==============================================================================

HOUDINI_PLUGIN_REPO=https://github.com/sideeffects/HoudiniEngineForUnreal.git
HOUDINI_PLUGIN_PATH=Plugins/HoudiniEngine
HOUDINI_PLUGIN_COMMIT=55b6a16cdf274389687fce3019b33e3b6e92a914
HOUDINI_PATCH=${CARLA_UTIL_FOLDER}/Patches/houdini_patch.txt
if [[ ! -d ${HOUDINI_PLUGIN_PATH} ]] ; then
  git clone ${HOUDINI_PLUGIN_REPO} ${HOUDINI_PLUGIN_PATH}
  pushd ${HOUDINI_PLUGIN_PATH} >/dev/null
  git checkout ${HOUDINI_PLUGIN_COMMIT}
  git apply ${HOUDINI_PATCH}
  popd >/dev/null
fi

# ==============================================================================
# -- Build CarlaUE4 ------------------------------------------------------------
# ==============================================================================

if ${BUILD_CARLAUE4} ; then

  OPTIONAL_MODULES_TEXT=""
  if ${USE_CARSIM} ; then
    python ${PWD}/../../Util/BuildTools/enable_carsim_to_uproject.py -f="CarlaUE4.uproject" -e
    OPTIONAL_MODULES_TEXT="CarSim ON"$'\n'"${OPTIONAL_MODULES_TEXT}"
  else
    python ${PWD}/../../Util/BuildTools/enable_carsim_to_uproject.py -f="CarlaUE4.uproject"
    OPTIONAL_MODULES_TEXT="CarSim OFF"$'\n'"${OPTIONAL_MODULES_TEXT}"
  fi
  if ${USE_CHRONO} ; then
    OPTIONAL_MODULES_TEXT="Chrono ON"$'\n'"${OPTIONAL_MODULES_TEXT}"
  else
    OPTIONAL_MODULES_TEXT="Chrono OFF"$'\n'"${OPTIONAL_MODULES_TEXT}"
  fi
  if ${USE_PYTORCH} ; then
    OPTIONAL_MODULES_TEXT="Pytorch ON"$'\n'"${OPTIONAL_MODULES_TEXT}"
  else
    OPTIONAL_MODULES_TEXT="Pytorch OFF"$'\n'"${OPTIONAL_MODULES_TEXT}"
  fi
  if ${USE_ROS2} ; then
    OPTIONAL_MODULES_TEXT="Ros2 ON"$'\n'"${OPTIONAL_MODULES_TEXT}"
  else
    OPTIONAL_MODULES_TEXT="Ros2 OFF"$'\n'"${OPTIONAL_MODULES_TEXT}"
  fi
  if ${USE_UNITY} ; then
    OPTIONAL_MODULES_TEXT="Unity ON"$'\n'"${OPTIONAL_MODULES_TEXT}"
  else
    OPTIONAL_MODULES_TEXT="Unity OFF"$'\n'"${OPTIONAL_MODULES_TEXT}"
  fi
  OPTIONAL_MODULES_TEXT="Fast_dds ON"$'\n'"${OPTIONAL_MODULES_TEXT}"
  echo ${OPTIONAL_MODULES_TEXT} > ${PWD}/Config/OptionalModules.ini

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
  ${GDB} ${UE4_ROOT}/Engine/Binaries/Linux/UE4Editor "${PWD}/CarlaUE4.uproject" ${RHI} ${EDITOR_FLAGS}

else

  log "Success!"

fi

# ==============================================================================
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

popd >/dev/null
