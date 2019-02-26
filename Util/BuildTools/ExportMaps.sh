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

DOC_STRING="Build and packs CarlaUE4's ExportedMaps"

USAGE_STRING="Usage: $0 [-h|--help] [-d|--dir] <outdir> [-f|--file] <filename>"

OUTPUT_DIRECTORY=""
FILE_NAME=""
MAP_PATH=""

OPTS=`getopt -o h --long help,dir:,file:,map: -n 'parse-options' -- "$@"`

if [ $? != 0 ] ; then echo "$USAGE_STRING" ; exit 2; fi

eval set -- "$OPTS"

while true; do
  case "$1" in
    --dir )
      OUTPUT_DIRECTORY="$2"
      shift 2;;
    --file )
      FILE_NAME="$2"
      shift 2;;
    --map )
      MAP_PATH="$2"
      shift 2;;
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
  OUTPUT_DIRECTORY="${PWD}/ExportedMaps"
fi
if [ -z "${FILE_NAME}" ]; then
  FILE_NAME="CookedExportedMaps"
fi



# ==============================================================================
# -- Package project -----------------------------------------------------------
# ==============================================================================

REPOSITORY_TAG=$(get_carla_version)

BUILD_FOLDER=${OUTPUT_DIRECTORY}

log "Packaging user content from version '$REPOSITORY_TAG'."

#rm -Rf ${BUILD_FOLDER}
mkdir -p ${BUILD_FOLDER}

pushd "${CARLAUE4_ROOT_FOLDER}" > /dev/null

log "Current project directory: '${PWD}'"

MAP_LIST=""
if [ -z "${MAP_PATH}" ]; then
  for filepath in `find ${PWD}/Content/ -type f -name "*.umap"`; do
    if [[ $filepath == *"/ExportedMaps/"* ]]; then
      filepath="/Game/"${filepath#"${PWD}/Content/"}
      if [ -z "${MAP_LIST}" ]; then
        MAP_LIST=$filepath
      else
        MAP_LIST=$MAP_LIST+$filepath
      fi
    fi
  done
else
  MAP_LIST=${MAP_PATH}
fi

${UE4_ROOT}/Engine/Binaries/Linux/UE4Editor "${PWD}/CarlaUE4.uproject" -run=cook -map=${MAP_LIST} -cooksinglepackage -targetplatform="LinuxNoEditor" -OutputDir="${BUILD_FOLDER}/Cooked"


popd >/dev/null

#if [[ ! -d ${BUILD_FOLDER}/LinuxNoEditor ]] ; then
#  fatal_error "Failed to package the project!"
#fi

# ==============================================================================
# -- Zip the project -----------------------------------------------------------
# ==============================================================================

DESTINATION=${BUILD_FOLDER}/${FILE_NAME}.tar.gz
SOURCE=${BUILD_FOLDER}/Cooked

pushd "${SOURCE}" >/dev/null

log "Packaging build."

rm -Rf ./Engine
rm -Rf ./CarlaUE4/Metadata
rm -Rf ./CarlaUE4/Plugins
rm ./CarlaUE4/AssetRegistry.bin

tar -czvf ${DESTINATION} *

popd > /dev/null

# ==============================================================================
# -- Remove intermediate files and return everything to normal------------------
# ==============================================================================


log "Removing intermediate build."

rm -Rf ${BUILD_FOLDER}/Cooked

# ==============================================================================
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

log "ExportedMaps created at ${DESTINATION}"
log "Success!"