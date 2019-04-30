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

USAGE_STRING="Usage: $0 [--help] [-d|--dir] <outdir> [-f|--file] <filename> [-p|--maps] <maps_to_cook>"

OUTPUT_DIRECTORY=""
FILE_NAME=""
PATHS_TO_COOK=""

OPTS=`getopt -o h --long help,dir:,file:,maps: -n 'parse-options' -- "$@"`

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
    --maps )
      MAP_TO_COOK="$2"
      shift 2;;
    --help )
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

REPOSITORY_TAG=$(get_git_repository_version)

BUILD_FOLDER=${OUTPUT_DIRECTORY}

log "Packaging user content from version '$REPOSITORY_TAG'."

#rm -Rf ${BUILD_FOLDER}
mkdir -p ${BUILD_FOLDER}

pushd "${CARLAUE4_ROOT_FOLDER}" > /dev/null

log "Current project directory: '${PWD}'"

${UE4_ROOT}/Engine/Binaries/Linux/UE4Editor "${PWD}/CarlaUE4.uproject" -run=cook -cooksinglepackage -Map=${MAP_TO_COOK} -targetplatform="LinuxNoEditor" -OutputDir="${BUILD_FOLDER}/Cooked"

popd >/dev/null


# ==============================================================================
# -- Zip the project -----------------------------------------------------------
# ==============================================================================

DESTINATION=${BUILD_FOLDER}/${FILE_NAME}.tar.gz
SOURCE=${BUILD_FOLDER}/Cooked

pushd "${SOURCE}" >/dev/null

log "Packaging build."

#rm -Rf ./Engine
rm -Rf ./CarlaUE4/Metadata
rm -Rf ./CarlaUE4/Plugins
#@TODO: Instead of this hardcoded path, we need to pick the ${MAPS_TO_COOK} path
#rm -Rf ./CarlaUE4/${MAPS_TO_COOK_But_instead_of_"game"_the_folder_is_content}.uexp
#rm -Rf ./CarlaUE4/${MAPS_TO_COOK_But_instead_of_"game"_the_folder_is_content}.umap
#rm -Rf ./CarlaUE4/Content/Carla/Maps/BaseMap/TEMPMAP.uexp
#rm -Rf ./CarlaUE4/Content/Carla/Maps/BaseMap/TEMPMAP.umap

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
