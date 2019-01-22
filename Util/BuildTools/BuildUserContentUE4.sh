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

DOC_STRING="Build and packs CarlaUE4's UserContent"

USAGE_STRING="Usage: $0 [-h|--help] [-d|--dir] <outdir> [-f|--file] <filename>"

OUTPUT_DIRECTORY=""
FILE_NAME=""

OPTS=`getopt -o h,d::,f --long help,dir::,file:: -n 'parse-options' -- "$@"`

if [ $? != 0 ] ; then echo "$USAGE_STRING" ; exit 2; fi

eval set -- "$OPTS"

while true; do
  case "$1" in
    --dir )
      OUTPUT_DIRECTORY="$2"
      shift ;;
    --file )
      FILE_NAME="$2"
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

if [ -z "${OUTPUT_DIRECTORY}" ]; then
  OUTPUT_DIRECTORY="${PWD}/UserContent"
fi
if [ -z "${FILE_NAME}" ]; then
  FILE_NAME="CookedUserContent"
fi

# ==============================================================================
# -- Package project -----------------------------------------------------------
# ==============================================================================

REPOSITORY_TAG=$(get_carla_version)

BUILD_FOLDER=${OUTPUT_DIRECTORY}

log "Packaging user content from version '$REPOSITORY_TAG'."

rm -Rf ${BUILD_FOLDER}
mkdir -p ${BUILD_FOLDER}

pushd "${CARLAUE4_ROOT_FOLDER}" > /dev/null

log "Current project directory: '${PWD}'"

MAP_LIST=""

for filepath in `find ${PWD}/Content/ -type f -name "*.umap"`; do
	if [[ $filepath == *"/UserContent/"* ]]; then
    filepath="/Game/"${filepath#"${PWD}/Content/"}
    if [ -z "${MAP_LIST}" ]; then
      MAP_LIST=$filepath
    else
      MAP_LIST=$MAP_LIST+$filepath
    fi
  fi
done

echo $MAP_LIST
${UE4_ROOT}/Engine/Build/BatchFiles/RunUAT.sh BuildCookRun \
    -project="${PWD}/CarlaUE4.uproject" -map=$MAP_LIST \
    -nocompileeditor -nop4 -cook \
    -archive -package -stage -nodebuginfo -build \
    -clientconfig=Development -ue4exe=UE4Editor \
    -targetplatform=Linux -CrashReporter -utf8output \
    -SkipCookingEditorContent -archivedirectory="${BUILD_FOLDER}"

popd >/dev/null

if [[ ! -d ${BUILD_FOLDER}/LinuxNoEditor ]] ; then
  fatal_error "Failed to package the project!"
fi

# ==============================================================================
# -- Zip the project -----------------------------------------------------------
# ==============================================================================

DESTINATION=${BUILD_FOLDER}/${FILE_NAME}.tar.gz
SOURCE=${BUILD_FOLDER}

pushd "${SOURCE}/LinuxNoEditor" >/dev/null

log "Packaging build."

rm -Rf ./CarlaUE4/Saved
rm -Rf ./Engine
rm ./CarlaUE4.sh
rm ./Manifest_NonUFSFiles_Linux.txt
rm ./Manifest_UFSFiles_Linux.txt
rm -Rf ./CarlaUE4/Binaries
rm -Rf ./CarlaUE4/Config
rm -Rf ./CarlaUE4/Plugins
rm ./CarlaUE4/AssetRegistry.bin
rm ./CarlaUE4/CarlaUE4.uproject

tar -czvf ${DESTINATION} *

popd > /dev/null

# ==============================================================================
# -- Remove intermediate files and return everything to normal------------------
# ==============================================================================


log "Removing intermediate build."

rm -Rf ${BUILD_FOLDER}/LinuxNoEditor

# ==============================================================================
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

log "UserContent created at ${DESTINATION}"
log "Success!"