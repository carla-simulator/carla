#! /bin/bash

################################################################################
# Packages a CARLA build.
################################################################################

set -e

DOC_STRING="Makes a packaged version of CARLA for distribution.
Please make sure to run Rebuild.sh before!"

USAGE_STRING="Usage: $0 [-h|--help] [--no-packaging] [--no-zip] [--clean-intermediate]"

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DO_PACKAGE=true
DO_COPY_FILES=true
DO_TARBALL=true
DO_CLEAN_INTERMEDIATE=false

OPTS=`getopt -o h --long help,no-packaging,no-zip,clean-intermediate -n 'parse-options' -- "$@"`

if [ $? != 0 ] ; then echo "$USAGE_STRING" ; exit 2 ; fi

eval set -- "$OPTS"

while true; do
  case "$1" in
    --no-packaging )
      DO_PACKAGE=false
      shift ;;
    --no-zip )
      DO_TARBALL=false
      shift ;;
    --clean-intermediate )
      DO_CLEAN_INTERMEDIATE=true
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

REPOSITORY_TAG=`git describe --tags --dirty --always`

echo "Packaging version '$REPOSITORY_TAG'."

UNREAL_PROJECT_FOLDER=${PWD}/Unreal/CarlaUE4
DIST_FOLDER=${PWD}/Dist
BUILD_FOLDER=${DIST_FOLDER}/${REPOSITORY_TAG}

function fatal_error {
  echo -e "\033[0;31mERROR: $1\033[0m"
  exit 1
}

function log {
  echo -e "\033[0;33m$1\033[0m"
}

# ==============================================================================
# -- Package project -----------------------------------------------------------
# ==============================================================================

if $DO_PACKAGE ; then

  pushd "$UNREAL_PROJECT_FOLDER" >/dev/null

  log "Packaging the project..."

  if [ ! -d "${UE4_ROOT}" ]; then
    fatal_error "UE4_ROOT is not defined, or points to a non-existant directory, please set this environment variable."
  fi

  rm -Rf ${BUILD_FOLDER}
  mkdir -p ${BUILD_FOLDER}

  ${UE4_ROOT}/Engine/Build/BatchFiles/RunUAT.sh BuildCookRun \
      -project="${PWD}/CarlaUE4.uproject" \
      -nocompileeditor -nop4 -cook -stage -archive -package \
      -clientconfig=Development -ue4exe=UE4Editor \
      -pak -prereqs -nodebuginfo \
      -targetplatform=Linux -build -CrashReporter -utf8output \
      -archivedirectory="${BUILD_FOLDER}"

  popd >/dev/null

fi

if [[ ! -d ${BUILD_FOLDER}/LinuxNoEditor ]] ; then
  fatal_error "Failed to package the project!"
fi

# ==============================================================================
# -- Copy files (Python server, README, etc) -----------------------------------
# ==============================================================================

if $DO_COPY_FILES ; then

  DESTINATION=${BUILD_FOLDER}/LinuxNoEditor

  log "Copying extra files..."

  cp -v ./LICENSE ${DESTINATION}/LICENSE
  cp -v ./CHANGELOG.md ${DESTINATION}/CHANGELOG
  cp -v ./Docs/release_readme.md ${DESTINATION}/README
  cp -v ./Docs/Example.CarlaSettings.ini ${DESTINATION}/Example.CarlaSettings.ini
  cp -v ./Util/Docker/Release.Dockerfile ${DESTINATION}/Dockerfile

  rsync -vhr --delete --delete-excluded \
      --exclude "*.egg-info" \
      --exclude "*.log" \
      --exclude "*.pyc" \
      --exclude ".*" \
      --exclude ".tags*" \
      --exclude "__pycache__" \
      --exclude "_benchmarks_results*" \
      --exclude "_images*" \
      --exclude "_out*" \
      PythonClient/ ${DESTINATION}/PythonClient

  echo

fi

# ==============================================================================
# -- Zip the project -----------------------------------------------------------
# ==============================================================================

if $DO_TARBALL ; then

  DESTINATION=${DIST_FOLDER}/CARLA_${REPOSITORY_TAG}.tar.gz
  SOURCE=${BUILD_FOLDER}/LinuxNoEditor

  pushd "$SOURCE" >/dev/null

  log "Packaging build..."

  rm -f ./Manifest_NonUFSFiles_Linux.txt
  rm -Rf ./CarlaUE4/Saved
  rm -Rf ./Engine/Saved

  tar -czvf ${DESTINATION} *

  popd >/dev/null

fi

# ==============================================================================
# -- Remove intermediate files -------------------------------------------------
# ==============================================================================

if $DO_CLEAN_INTERMEDIATE ; then

  log "Removing intermediate build..."

  rm -Rf ${BUILD_FOLDER}

fi

# ==============================================================================
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

if $DO_TARBALL ; then
  FINAL_PACKAGE=Dist/CARLA_${REPOSITORY_TAG}.tar.gz
else
  FINAL_PACKAGE=Dist/${REPOSITORY_TAG}
fi

echo
echo "Packaged version created at ${FINAL_PACKAGE}"
echo
echo "****************"
echo "*** Success! ***"
echo "****************"

popd >/dev/null
