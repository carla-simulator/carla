#! /bin/bash

################################################################################
# Packages a CARLA build.
################################################################################

set -e

DOC_STRING="Makes a packaged version of CARLA for distribution.
Please make sure to run Rebuild.sh before!"

USAGE_STRING="Usage: $0 [-h|--help] [--skip-packaging]"

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DO_PACKAGE=true
DO_COPY_FILES=true
DO_TARBALL=true

OPTS=`getopt -o h --long help,skip-packaging -n 'parse-options' -- "$@"`

if [ $? != 0 ] ; then echo "$USAGE_STRING" ; exit 2 ; fi

eval set -- "$OPTS"

while true; do
  case "$1" in
    --skip-packaging )
      DO_PACKAGE=false
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
  mkdir ${BUILD_FOLDER}

  ${UE4_ROOT}/Engine/Build/BatchFiles/RunUAT.sh BuildCookRun \
      -project="${PWD}/CarlaUE4.uproject" \
      -noP4 -platform=Linux \
      -clientconfig=Development -serverconfig=Development \
      -cook -compile -build -stage -pak -archive \
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

  rsync -vhr --delete --delete-excluded \
      --exclude "__pycache__" \
      --exclude "*.pyc" \
      --exclude ".*" \
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
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

echo ""
echo "****************"
echo "*** Success! ***"
echo "****************"

popd >/dev/null
