#! /bin/bash

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DOC_STRING="Makes a packaged version of CARLA and other content packages ready for distribution."

USAGE_STRING="Usage: $0 [-h|--help] [--config={Debug,Development,Shipping}] [--no-zip] [--clean-intermediate] [--packages=Name1,Name2,...] [--target-archive=]"

PACKAGES="Carla"
DO_TARBALL=true
DO_CLEAN_INTERMEDIATE=false
PROPS_MAP_NAME=PropsMap
PACKAGE_CONFIG=Shipping
USE_CARSIM=false
SINGLE_PACKAGE=false

OPTS=`getopt -o h --long help,config:,no-zip,clean-intermediate,carsim,packages:,python-version,target-archive:, -n 'parse-options' -- "$@"`

eval set -- "$OPTS"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --config )
      PACKAGE_CONFIG="$2"
      shift 2 ;;
    --no-zip )
      DO_TARBALL=false
      shift ;;
    --clean-intermediate )
      DO_CLEAN_INTERMEDIATE=true
      shift ;;
    --packages )
      PACKAGES="$2"
      shift 2 ;;
    --target-archive )
      SINGLE_PACKAGE=true
      TARGET_ARCHIVE="$2"
      shift 2 ;;
    --carsim )
      USE_CARSIM=true;
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
# -- Prepare environment -------------------------------------------------------
# ==============================================================================
source $(dirname "$0")/Environment.sh

if [ ! -d "${UE4_ROOT}" ]; then
  fatal_error "UE4_ROOT is not defined, or points to a non-existent directory, please set this environment variable."
fi

if [ ! -n "${PACKAGES}" ] ; then
  fatal_error "Nothing to be done."
fi

# Convert comma-separated string to array of unique elements.
PACKAGES="$(echo "${PACKAGES}" | tr ',' '\n' | sort -u | tr '\n' ',')"
IFS=',' read -r -a PACKAGES <<< "${PACKAGES}"

# If contains an element called "Carla".
if [[ "${PACKAGES[@]}" =~ "Carla" ]] ; then
  DO_CARLA_RELEASE=true
else
  DO_CARLA_RELEASE=false
fi

REPOSITORY_TAG=$(get_git_repository_version)

RELEASE_BUILD_FOLDER=${CARLA_DIST_FOLDER}/CARLA_${PACKAGE_CONFIG}_${REPOSITORY_TAG}

if [[ ${PACKAGE_CONFIG} == "Shipping" ]] ; then
  RELEASE_PACKAGE_PATH=${CARLA_DIST_FOLDER}/CARLA_${REPOSITORY_TAG}.tar.gz
else
  RELEASE_PACKAGE_PATH=${CARLA_DIST_FOLDER}/CARLA_${PACKAGE_CONFIG}_${REPOSITORY_TAG}.tar.gz
fi

log "Packaging version '${REPOSITORY_TAG}' (${PACKAGE_CONFIG})."

# ==============================================================================
# -- Cook CARLA project --------------------------------------------------------
# ==============================================================================

if ${DO_CARLA_RELEASE} ; then

  pushd "${CARLAUE4_ROOT_FOLDER}" >/dev/null

  if ${USE_CARSIM} ; then
    python ${PWD}/../../Util/BuildTools/enable_carsim_to_uproject.py -f="CarlaUE4.uproject" -e
    echo "CarSim ON" > ${PWD}/Config/CarSimConfig.ini
  else
    python ${PWD}/../../Util/BuildTools/enable_carsim_to_uproject.py -f="CarlaUE4.uproject"
    echo "CarSim OFF" > ${PWD}/Config/CarSimConfig.ini
  fi

  log "Cooking CARLA project."

  rm -Rf ${RELEASE_BUILD_FOLDER}
  mkdir -p ${RELEASE_BUILD_FOLDER}

  ${UE4_ROOT}/Engine/Build/BatchFiles/RunUAT.sh BuildCookRun \
      -project="${PWD}/CarlaUE4.uproject" \
      -nocompileeditor -nop4 -cook -stage -archive -package -iterate \
      -clientconfig=${PACKAGE_CONFIG} -ue4exe=UE4Editor \
      -prereqs -targetplatform=Linux -build -utf8output \
      -archivedirectory="${RELEASE_BUILD_FOLDER}"

  popd >/dev/null

  if [[ ! -d ${RELEASE_BUILD_FOLDER}/LinuxNoEditor ]] ; then
    fatal_error "Failed to cook the project!"
  fi

fi

# ==============================================================================
# -- Copy files (Python API, README, etc) --------------------------------------
# ==============================================================================

if ${DO_CARLA_RELEASE} ; then

  DESTINATION=${RELEASE_BUILD_FOLDER}/LinuxNoEditor

  log "Adding extra files to CARLA package."

  pushd ${CARLA_ROOT_FOLDER} >/dev/null

  mkdir -p "${DESTINATION}/Import"

  echo "${REPOSITORY_TAG}" > ${DESTINATION}/VERSION

  copy_if_changed "./LICENSE" "${DESTINATION}/LICENSE"
  copy_if_changed "./CHANGELOG.md" "${DESTINATION}/CHANGELOG"
  copy_if_changed "./Docs/release_readme.md" "${DESTINATION}/README"
  copy_if_changed "./Docs/python_api.md" "${DESTINATION}/PythonAPI/python_api.md"
  copy_if_changed "./Util/Docker/Release.Dockerfile" "${DESTINATION}/Dockerfile"
  copy_if_changed "./Util/ImportAssets.sh" "${DESTINATION}/ImportAssets.sh"
  copy_if_changed "./Util/DockerUtils/dist/RecastBuilder" "${DESTINATION}/Tools/"

  copy_if_changed "./PythonAPI/carla/dist/*.egg" "${DESTINATION}/PythonAPI/carla/dist/"
  copy_if_changed "./PythonAPI/carla/dist/*.whl" "${DESTINATION}/PythonAPI/carla/dist/"
  copy_if_changed "./PythonAPI/carla/agents/" "${DESTINATION}/PythonAPI/carla/agents"
  copy_if_changed "./PythonAPI/carla/scene_layout.py" "${DESTINATION}/PythonAPI/carla/"
  copy_if_changed "./PythonAPI/carla/requirements.txt" "${DESTINATION}/PythonAPI/carla/"

  copy_if_changed "./PythonAPI/examples/*.py" "${DESTINATION}/PythonAPI/examples/"
  copy_if_changed "./PythonAPI/examples/rss/*.py" "${DESTINATION}/PythonAPI/examples/rss/"
  copy_if_changed "./PythonAPI/examples/requirements.txt" "${DESTINATION}/PythonAPI/examples/"

  copy_if_changed "./PythonAPI/util/*.py" "${DESTINATION}/PythonAPI/util/"
  copy_if_changed "./PythonAPI/util/opendrive/" "${DESTINATION}/PythonAPI/util/opendrive/"
  copy_if_changed "./PythonAPI/util/requirements.txt" "${DESTINATION}/PythonAPI/util/"

  copy_if_changed "./Co-Simulation/" "${DESTINATION}/Co-Simulation/"

  if [ -d "./Plugins/" ] ; then
    copy_if_changed "./Plugins/" "${DESTINATION}/Plugins/"
  fi

  copy_if_changed "./Unreal/CarlaUE4/Content/Carla/HDMaps/*.pcd" "${DESTINATION}/HDMaps/"
  copy_if_changed "./Unreal/CarlaUE4/Content/Carla/HDMaps/Readme.md" "${DESTINATION}/HDMaps/README"

  popd >/dev/null

fi

# ==============================================================================
# -- Zip the project -----------------------------------------------------------
# ==============================================================================

if ${DO_CARLA_RELEASE} && ${DO_TARBALL} ; then

  DESTINATION=${RELEASE_PACKAGE_PATH}
  SOURCE=${RELEASE_BUILD_FOLDER}/LinuxNoEditor

  pushd "${SOURCE}" >/dev/null

  log "Packaging CARLA release."

  rm -f ./Manifest_NonUFSFiles_Linux.txt
  rm -f ./Manifest_UFSFiles_Linux.txt
  rm -Rf ./CarlaUE4/Saved
  rm -Rf ./Engine/Saved

  tar -czf ${DESTINATION} *

  popd >/dev/null

fi

# ==============================================================================
# -- Remove intermediate files -------------------------------------------------
# ==============================================================================

if ${DO_CARLA_RELEASE} && ${DO_CLEAN_INTERMEDIATE} ; then

  log "Removing intermediate build."

  rm -Rf ${RELEASE_BUILD_FOLDER}

fi

# ==============================================================================
# -- Cook other packages -------------------------------------------------------
# ==============================================================================

PACKAGE_PATH_FILE=${CARLAUE4_ROOT_FOLDER}/Content/PackagePath.txt
MAP_LIST_FILE=${CARLAUE4_ROOT_FOLDER}/Content/MapPathsLinux.txt

for PACKAGE_NAME in "${PACKAGES[@]}" ; do if [[ ${PACKAGE_NAME} != "Carla" ]] ; then

  log "Preparing environment for cooking '${PACKAGE_NAME}'."

  if ${SINGLE_PACKAGE} ; then
      BUILD_FOLDER_TARGET=${CARLA_DIST_FOLDER}/${TARGET_ARCHIVE}_${REPOSITORY_TAG}
  else
      BUILD_FOLDER_TARGET=${CARLA_DIST_FOLDER}/${PACKAGE_NAME}_${REPOSITORY_TAG}
  fi

  BUILD_FOLDER=${CARLA_DIST_FOLDER}/${PACKAGE_NAME}_${REPOSITORY_TAG}
  DESTINATION=${BUILD_FOLDER_TARGET}.tar
  PACKAGE_PATH=${CARLAUE4_ROOT_FOLDER}/Content/${PACKAGE_NAME}

  mkdir -p ${BUILD_FOLDER}

  log "Cooking package '${PACKAGE_NAME}'..."

  pushd "${CARLAUE4_ROOT_FOLDER}" > /dev/null

  # Prepare cooking of package
  ${UE4_ROOT}/Engine/Binaries/Linux/UE4Editor "${CARLAUE4_ROOT_FOLDER}/CarlaUE4.uproject" \
      -run=PrepareAssetsForCooking -PackageName=${PACKAGE_NAME} -OnlyPrepareMaps=false

  PACKAGE_FILE=$(<${PACKAGE_PATH_FILE})
  MAPS_TO_COOK=$(<${MAP_LIST_FILE})


  # Cook maps in batches
  MAX_STRINGLENGTH=1000
  IFS="+" read -ra MAP_LIST <<< $MAPS_TO_COOK
  TOTAL=0
  MAP_STRING=""
  for MAP in "${MAP_LIST[@]}"; do
    if (($(($TOTAL+${#MAP})) > $MAX_STRINGLENGTH)); then
      echo "Cooking $MAP_STRING"
      ${UE4_ROOT}/Engine/Binaries/Linux/UE4Editor "${CARLAUE4_ROOT_FOLDER}/CarlaUE4.uproject" \
          -run=cook -map="${MAP_STRING}" -cooksinglepackage -targetplatform="LinuxNoEditor" \
          -OutputDir="${BUILD_FOLDER}" -iterate
      MAP_STRING=""
      TOTAL=0
    fi
    MAP_STRING=$MAP_STRING+$MAP
    TOTAL=$(($TOTAL+${#MAP}))
  done
  if (($TOTAL > 0)); then
    ${UE4_ROOT}/Engine/Binaries/Linux/UE4Editor "${CARLAUE4_ROOT_FOLDER}/CarlaUE4.uproject" \
        -run=cook -map="${MAP_STRING}" -cooksinglepackage -targetplatform="LinuxNoEditor" \
        -OutputDir="${BUILD_FOLDER}" -iterate
  fi

  PROP_MAP_FOLDER="${PACKAGE_PATH}/Maps/${PROPS_MAP_NAME}"

  if [ -d ${PROP_MAP_FOLDER} ] ; then
    rm -Rf ${PROP_MAP_FOLDER}
  fi

  popd >/dev/null

  pushd "${BUILD_FOLDER}" > /dev/null

  SUBST_PATH="${BUILD_FOLDER}/CarlaUE4"
  SUBST_FILE="${PACKAGE_FILE/${CARLAUE4_ROOT_FOLDER}/${SUBST_PATH}}"

  # Copy the package config file to package
  mkdir -p "$(dirname ${SUBST_FILE})" && cp "${PACKAGE_FILE}" "$_"

  # Copy the OpenDRIVE .xodr files to package
  IFS='+' # set delimiter
  # MAPS_TO_COOK is read into an array as tokens separated by IFS
  read -ra ADDR <<< "$MAPS_TO_COOK"
  for i in "${ADDR[@]}"; do # access each element of array
    
    XODR_FILE_PATH="${CARLAUE4_ROOT_FOLDER}/Content${i:5}"
    MAP_NAME=${XODR_FILE_PATH##*/}
    XODR_FILE=$(find "${CARLAUE4_ROOT_FOLDER}/Content" -name "${MAP_NAME}.xodr" -print -quit)

    if [ -f "${XODR_FILE}" ] ; then

      SUBST_FILE="${XODR_FILE/${CARLAUE4_ROOT_FOLDER}/${SUBST_PATH}}"

      # Copy the package config file to package
      mkdir -p "$(dirname ${SUBST_FILE})" && cp "${XODR_FILE}" "$_"

    fi

    # binary files for navigation and traffic manager
    BIN_FILE_PATH="${CARLAUE4_ROOT_FOLDER}/Content${i:5}"
    MAP_NAME=${BIN_FILE_PATH##*/}
    find "${CARLAUE4_ROOT_FOLDER}/Content" -name "${MAP_NAME}.bin" -print0 | while read -d $'\0' BIN_FILE
    do
      if [ -f "${BIN_FILE}" ] ; then

        SUBST_FILE="${BIN_FILE/${CARLAUE4_ROOT_FOLDER}/${SUBST_PATH}}"

        # Copy the package config file to package
        mkdir -p "$(dirname ${SUBST_FILE})" && cp "${BIN_FILE}" "$_"
    fi
    done
  done

    rm -Rf "./CarlaUE4/Metadata"
    rm -Rf "./CarlaUE4/Plugins"
    rm -Rf "./CarlaUE4/Content/${PACKAGE_NAME}/Maps/${PROPS_MAP_NAME}"
    rm -f "./CarlaUE4/AssetRegistry.bin"

  if ${DO_TARBALL} ; then
  
    if ${SINGLE_PACKAGE} ; then
      tar -rf ${DESTINATION} *
    else
      tar -czf ${DESTINATION}.gz *
    fi

    popd >/dev/null

  fi

  if ${DO_CLEAN_INTERMEDIATE} ; then

    log "Removing intermediate build."

    rm -Rf ${BUILD_FOLDER}

  fi

fi ; done

# compress the TAR if it is a single package
if ${SINGLE_PACKAGE} ; then
  gzip -f ${DESTINATION}
fi

# ==============================================================================
# -- Log paths of generated packages -------------------------------------------
# ==============================================================================

# for PACKAGE_NAME in "${PACKAGES[@]}" ; do if [[ ${PACKAGE_NAME} != "Carla" ]] ; then
#   FINAL_PACKAGE=${CARLA_DIST_FOLDER}/${PACKAGE_NAME}_${REPOSITORY_TAG}.tar.gz
#   log "Package '${PACKAGE_NAME}' created at ${FINAL_PACKAGE}"
# fi ; done

if ${DO_CARLA_RELEASE} ; then
  if ${DO_TARBALL} ; then
    log "CARLA release created at ${RELEASE_PACKAGE_PATH}"
  else
    log "CARLA release created at ${RELEASE_BUILD_FOLDER}"
  fi
fi

# ==============================================================================
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

log "Success!"
