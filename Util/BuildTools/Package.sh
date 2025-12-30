#! /bin/bash

# Measure overall execution time of packaging
T_START_OVERALL=$(date +%s)

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DOC_STRING="Makes a packaged version of CARLA and other content packages ready for distribution."

USAGE_STRING="Usage: $0 [-h|--help] [--config={Debug,Development,Shipping}] [--no-zip] [--clean-intermediate] [--packages=Name1,Name2,...] [--target-archive=] [--archive-sufix=]"

PACKAGES="Carla"
DO_TARBALL=true
DO_CLEAN_INTERMEDIATE=false
PROPS_MAP_NAME=PropsMap
PACKAGE_CONFIG=Shipping
USE_CARSIM=false
SINGLE_PACKAGE=false
ARCHIVE_SUFIX=""

OPTS=`getopt -o h --long help,config:,no-zip,clean-intermediate,carsim,packages:,python-version,target-archive:,archive-sufix:, -n 'parse-options' -- "$@"`

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
    --archive-sufix )
      ARCHIVE_SUFIX="$2"
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

if [[ ${ARCHIVE_SUFIX} != "" ]] ; then
  RELEASE_BUILD_FOLDER=${CARLA_DIST_FOLDER}/CARLA_${PACKAGE_CONFIG}_${REPOSITORY_TAG}_${ARCHIVE_SUFIX}
else
  RELEASE_BUILD_FOLDER=${CARLA_DIST_FOLDER}/CARLA_${PACKAGE_CONFIG}_${REPOSITORY_TAG}
fi

if [[ ${PACKAGE_CONFIG} == "Shipping" ]] ; then
  RELEASE_PACKAGE_PATH=${CARLA_DIST_FOLDER}/CARLA_${REPOSITORY_TAG}
else
  RELEASE_PACKAGE_PATH=${CARLA_DIST_FOLDER}/CARLA_${PACKAGE_CONFIG}_${REPOSITORY_TAG}
fi

if [[ ${ARCHIVE_SUFIX} != "" ]] ; then
  RELEASE_PACKAGE_PATH=${RELEASE_PACKAGE_PATH}_${ARCHIVE_SUFIX}
fi

RELEASE_PACKAGE_PATH=${RELEASE_PACKAGE_PATH}.tar.gz

log "Packaging version '${REPOSITORY_TAG}' (${PACKAGE_CONFIG})."

# ============================================================================
# -- Helper functions --------------------------------------------------------
# ============================================================================
function copy_dir_fast {
  CF_SRC=$1
  CF_DEST=$2
  CF_SRC_FS=$(stat -c "%d" "${CF_SRC}")
  CF_DEST_FS=$(stat -c "%d" "${CF_DEST}")
  if [[ ${CF_SRC_FS} == ${CF_DEST_FS} ]] ; then
    cp -rlan "${CF_SRC}"/* "${CF_DEST}"
  else
    cp -ran "${CF_SRC}"/* "${CF_DEST}"
  fi
}

function cook_tagged_materials {
  # Measure duration of this function call
  T_START_COOK_TAGGED_MATS=$(date +%s)

  # Read parameters.
  CUR_PACKAGES=("${@}")
  CUR_PACKAGES_UE=$(IFS=+ ; echo "${CUR_PACKAGES[*]}")
  REGISTRIES_SUBDIR=CarlaUE4/Plugins/Carla/Content/PostProcessingMaterials/TaggedMaterials
  TEMP_BUILD_DIR=${CARLA_DIST_FOLDER}/${REPOSITORY_TAG}_TaggedMaterials

  # Call commandlet to build TaggedMaterialsRegistries for the current package(s).
  # Temporary maps containing the TaggedMaterialsRegistries will be created,
  # that can be cooked to cook the registries.
  log "Generate TaggedMaterialsRegistry for package(s) '${CUR_PACKAGES_UE//+/,}'..."
  if ${SINGLE_PACKAGE} ; then
    TARGET_ARCHIVE_OPTION=-TargetArchive="${TARGET_ARCHIVE}"
  else
    TARGET_ARCHIVE_OPTION=
  fi
  ${UE4_ROOT}/Engine/Binaries/Linux/UE4Editor "${CARLAUE4_ROOT_FOLDER}/CarlaUE4.uproject" \
  -run=GenerateTaggedMaterialsRegistry -PackageNames=${CUR_PACKAGES_UE} ${TARGET_ARCHIVE_OPTION} \
  -NoShaderCompile

  # Construct string for all maps to cook all at once
  if ${SINGLE_PACKAGE} ; then
    MAPS_COOKING_STR=/Carla/PostProcessingMaterials/TaggedMaterials/TaggedMaterials_${TARGET_ARCHIVE}_Map
  else
    for CUR_PACKAGE in "${CUR_PACKAGES[@]}" ; do
      MAP_PATH=/Carla/PostProcessingMaterials/TaggedMaterials/TaggedMaterials_${CUR_PACKAGE}_Map
      if [[ -z ${MAPS_COOKING_STR+x} ]] ; then
        MAPS_COOKING_STR=${MAP_PATH}
      else
        MAPS_COOKING_STR=${MAPS_COOKING_STR}+${MAP_PATH}
      fi
    done
  fi

  # Cook the temporary map(s).
  log "Cook TaggedMaterialsRegistries"
  ${UE4_ROOT}/Engine/Binaries/Linux/UE4Editor "${CARLAUE4_ROOT_FOLDER}/CarlaUE4.uproject" \
  -run=cook -map="${MAPS_COOKING_STR}" \
  -targetplatform="LinuxNoEditor" -OutputDir="${TEMP_BUILD_DIR}" -iterate -cooksinglepackage

  # Move/Copy the generated files to the correct packages (zipping happens later on)
  log "Copy cooked TaggedMaterialsRegistries"
  if [[ ${CUR_PACKAGES_UE} == Carla ]] ; then
    # For Carla package, we only move the files to target dir
    SRC=${TEMP_BUILD_DIR}/${REGISTRIES_SUBDIR}/TaggedMaterials_Carla
    TRG=${RELEASE_BUILD_FOLDER}/LinuxNoEditor/${REGISTRIES_SUBDIR}
    [ ! -d ${TRG} ] && mkdir -p ${TRG}
    mv ${SRC}.* ${TRG}
  elif ${SINGLE_PACKAGE} ; then
    # Move the monolithic TaggedMaterialsRegistry to the target archive
    SRC=${TEMP_BUILD_DIR}/${REGISTRIES_SUBDIR}/TaggedMaterials_${TARGET_ARCHIVE}
    SINGLE_PACKAGE_ROOT=${CARLA_DIST_FOLDER}/${TARGET_ARCHIVE}_${REPOSITORY_TAG}
    if [[ ${ARCHIVE_SUFIX} != "" ]] ; then SINGLE_PACKAGE_ROOT=${SINGLE_PACKAGE_ROOT}_${ARCHIVE_SUFIX} ; fi

    if [ -f ${SRC}.uasset ]; then
      if ! ${DO_CLEAN_INTERMEDIATE} ; then
        SINGLE_PACKAGE_TRG=${SINGLE_PACKAGE_ROOT}/${REGISTRIES_SUBDIR}/
        [ ! -d ${SINGLE_PACKAGE_TRG} ] && mkdir -p ${SINGLE_PACKAGE_TRG}
        cp ${SRC}.* ${SINGLE_PACKAGE_TRG}
      fi

      if ${DO_TARBALL} ; then
        TEMP_TAR_DIR=${CARLA_DIST_FOLDER}/${TARGET_ARCHIVE}_${REPOSITORY_TAG}_TaggedMaterials_totar
        TEMP_TAR_RESULT_DIR=${TEMP_TAR_DIR}/${REGISTRIES_SUBDIR}/
        mkdir -p ${TEMP_TAR_RESULT_DIR}
        mv ${SRC}.* ${TEMP_TAR_RESULT_DIR}

        pushd "${TEMP_TAR_DIR}" >/dev/null
        tar -rf ${SINGLE_PACKAGE_ROOT}.tar *
        popd >/dev/null
        rm -Rf ${TEMP_TAR_DIR}
      fi
    fi
  else
    # For other packages, we copy the files to the corresponding dirs
    for CUR_PACKAGE in "${CUR_PACKAGES[@]}" ; do
      SRC=${TEMP_BUILD_DIR}/${REGISTRIES_SUBDIR}/TaggedMaterials_${CUR_PACKAGE}
      if [ -f ${SRC}.uasset ]; then
        CUR_PACKAGE_ROOT=${CARLA_DIST_FOLDER}/${PACKAGE_NAME}_${REPOSITORY_TAG}
        if [[ ${ARCHIVE_SUFIX} != "" ]] ; then CUR_PACKAGE_ROOT=${CUR_PACKAGE_ROOT}_${ARCHIVE_SUFIX} ; fi

        if ! ${DO_CLEAN_INTERMEDIATE} ; then
          CUR_PACKAGE_TRG=${CUR_PACKAGE_ROOT}/${REGISTRIES_SUBDIR}/
          [ ! -d ${CUR_PACKAGE_TRG} ] && mkdir -p ${CUR_PACKAGE_TRG}
          cp ${SRC}.* ${CUR_PACKAGE_TRG}
        fi

        if ${DO_TARBALL} ; then
          TEMP_TAR_DIR=${CARLA_DIST_FOLDER}/${CUR_PACKAGE}_${REPOSITORY_TAG}_TaggedMaterials_totar
          TEMP_TAR_RESULT_DIR=${TEMP_TAR_DIR}/${REGISTRIES_SUBDIR}/
          mkdir -p ${TEMP_TAR_RESULT_DIR}
          mv ${SRC}.* ${TEMP_TAR_RESULT_DIR}

          pushd "${TEMP_TAR_DIR}" >/dev/null
          tar -rf ${CUR_PACKAGE_ROOT}.tar *
          popd >/dev/null
          rm -Rf ${TEMP_TAR_DIR}
        fi
      fi
    done
  fi

  # Delete the temporary files.
  rm -Rf ${TEMP_BUILD_DIR}

  T_END_COOK_TAGGED_MATS=$(date +%s)
  ELAPSED_TIME=$((T_END_COOK_TAGGED_MATS - T_START_COOK_TAGGED_MATS))
  log "Generating and cooking of TaggedMaterialsRegistry for package '${CUR_PACKAGES_UE//+/,}' took ${ELAPSED_TIME} seconds."
}

# ==============================================================================
# -- Cook CARLA project --------------------------------------------------------
# ==============================================================================

T_START_DO_PACKAGE=$(date +%s)
if ${DO_CARLA_RELEASE} ; then

  pushd "${CARLAUE4_ROOT_FOLDER}" >/dev/null

  if ${USE_CARSIM} ; then
    python3 ${PWD}/../../Util/BuildTools/enable_carsim_to_uproject.py -f="CarlaUE4.uproject" -e
    echo "CarSim ON" > ${PWD}/Config/CarSimConfig.ini
  else
    python3 ${PWD}/../../Util/BuildTools/enable_carsim_to_uproject.py -f="CarlaUE4.uproject"
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

  cook_tagged_materials "Carla"

  popd >/dev/null

  if [[ ! -d ${RELEASE_BUILD_FOLDER}/LinuxNoEditor ]] ; then
    fatal_error "Failed to cook the project!"
  fi

fi
T_END_DO_PACKAGE=$(date +%s)
ELAPSED_TIME=$((T_END_DO_PACKAGE - T_END_DO_PACKAGE))
if ${DO_CARLA_RELEASE}; then log "Building and cooking Carla took ${ELAPSED_TIME} seconds."; fi

# ==============================================================================
# -- Copy files (Python API, README, etc) --------------------------------------
# ==============================================================================

T_START_DO_COPY_FILES=$(date +%s)
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

  copy_if_changed "./PythonAPI/carla/dist/*.whl" "${DESTINATION}/PythonAPI/carla/dist/"
  copy_if_changed "./PythonAPI/carla/agents/" "${DESTINATION}/PythonAPI/carla/agents"
  copy_if_changed "./PythonAPI/carla/scene_layout.py" "${DESTINATION}/PythonAPI/carla/"
  copy_if_changed "./PythonAPI/carla/requirements.txt" "${DESTINATION}/PythonAPI/carla/"

  copy_if_changed "./PythonAPI/examples/*.py" "${DESTINATION}/PythonAPI/examples/"
  copy_if_changed "./PythonAPI/examples/rss/*.py" "${DESTINATION}/PythonAPI/examples/rss/"
  copy_if_changed "./PythonAPI/examples/requirements.txt" "${DESTINATION}/PythonAPI/examples/"
  copy_if_changed "./PythonAPI/examples/nvidia/" "${DESTINATION}/PythonAPI/examples/nvidia"
  copy_if_changed "./PythonAPI/examples/ros2/" "${DESTINATION}/PythonAPI/examples/ros2"

  copy_if_changed "./PythonAPI/util/*.py" "${DESTINATION}/PythonAPI/util/"
  copy_if_changed "./PythonAPI/util/opendrive/" "${DESTINATION}/PythonAPI/util/opendrive/"
  copy_if_changed "./PythonAPI/util/requirements.txt" "${DESTINATION}/PythonAPI/util/"

  copy_if_changed "./Co-Simulation/" "${DESTINATION}/Co-Simulation/"

  if [ -d "./Plugins/" ] ; then
    copy_if_changed "./Plugins/" "${DESTINATION}/Plugins/"
  fi

  if [ -d "./Unreal/CarlaUE4/Plugins/Carla/CarlaDependencies/lib" ] ; then
    cp -r "./Unreal/CarlaUE4/Plugins/Carla/CarlaDependencies/lib" "${DESTINATION}/CarlaUE4/Plugins/Carla/CarlaDependencies"
  fi

  copy_if_changed "./Unreal/CarlaUE4/Content/Carla/HDMaps/*.pcd" "${DESTINATION}/HDMaps/"
  copy_if_changed "./Unreal/CarlaUE4/Content/Carla/HDMaps/Readme.md" "${DESTINATION}/HDMaps/README"

  popd >/dev/null

fi
T_END_DO_COPY_FILES=$(date +%s)
ELAPSED_TIME=$((T_END_DO_COPY_FILES - T_END_DO_COPY_FILES))
if ${DO_CARLA_RELEASE}; then log "Copying extra files to package Carla took ${ELAPSED_TIME} seconds."; fi

# ==============================================================================
# -- Zip the project -----------------------------------------------------------
# ==============================================================================

T_START_DO_TARBALL=$(date +%s)
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
T_END_DO_TARBALL=$(date +%s)
ELAPSED_TIME=$((T_END_DO_TARBALL - T_START_DO_CLEAN))
if ${DO_CARLA_RELEASE} && ${DO_TARBALL}; then log "Archiving and compressing the project took ${ELAPSED_TIME} seconds."; fi

# ==============================================================================
# -- Remove intermediate files -------------------------------------------------
# ==============================================================================

T_START_DO_CLEAN=$(date +%s)
if ${DO_CARLA_RELEASE} && ${DO_CLEAN_INTERMEDIATE} ; then

  log "Removing intermediate build."

  rm -Rf ${RELEASE_BUILD_FOLDER}

fi
T_END_DO_CLEAN=$(date +%s)
ELAPSED_TIME=$((T_END_DO_CLEAN - T_START_DO_CLEAN))
if ${DO_CARLA_RELEASE} && ${DO_CLEAN_INTERMEDIATE}; then log "Cleaning up took ${ELAPSED_TIME} seconds."; fi

# ==============================================================================
# -- Cook other packages -------------------------------------------------------
# ==============================================================================

T_START_PACKAGES=$(date +%s)

PACKAGE_PATH_FILE=${CARLAUE4_ROOT_FOLDER}/Content/PackagePath.txt
MAP_LIST_FILE=${CARLAUE4_ROOT_FOLDER}/Content/MapPathsLinux.txt

for PACKAGE_NAME in "${PACKAGES[@]}" ; do if [[ ${PACKAGE_NAME} != "Carla" ]] ; then
  T_START_PACKAGE=$(date +%s)

  log "Preparing environment for cooking '${PACKAGE_NAME}'."

  if ${SINGLE_PACKAGE} ; then
      BUILD_FOLDER_TARGET=${CARLA_DIST_FOLDER}/${TARGET_ARCHIVE}_${REPOSITORY_TAG}
  else
      BUILD_FOLDER_TARGET=${CARLA_DIST_FOLDER}/${PACKAGE_NAME}_${REPOSITORY_TAG}
  fi

  if [[ ${ARCHIVE_SUFIX} != "" ]] ; then
    BUILD_FOLDER_TARGET=${BUILD_FOLDER_TARGET}_${ARCHIVE_SUFIX}
  fi

  if [[ ${ARCHIVE_SUFIX} != "" ]] ; then
    BUILD_FOLDER=${CARLA_DIST_FOLDER}/${PACKAGE_NAME}_${REPOSITORY_TAG}_${ARCHIVE_SUFIX}
  else
    BUILD_FOLDER=${CARLA_DIST_FOLDER}/${PACKAGE_NAME}_${REPOSITORY_TAG}
  fi

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

  if ${SINGLE_PACKAGE}; then
    [ ! -d "${BUILD_FOLDER_TARGET}" ] && mkdir -p "${BUILD_FOLDER_TARGET}"
    copy_dir_fast "${BUILD_FOLDER}" "${BUILD_FOLDER_TARGET}"
  fi
  if ${DO_TARBALL} ; then
    tar -rf ${DESTINATION} *
  fi

  popd >/dev/null

  if ${DO_CLEAN_INTERMEDIATE} ; then

    log "Removing intermediate build."

    rm -Rf ${BUILD_FOLDER}

  fi
  T_END_PACKAGE=$(date +%s)
  ELAPSED_TIME=$((T_END_PACKAGE - T_START_PACKAGE))
  log "Cooking package ${PACKAGE_NAME} took ${ELAPSED_TIME} seconds."

fi ; done

# Create tagged materials for the instance segmentation for all packages at once
if [[ ${#PACKAGES[@]} > 1 || "${PACKAGES[0]}" != "Carla" ]] ; then
  cook_tagged_materials ${PACKAGES[@]}
fi

# Compress the TAR balls
if ${DO_TARBALL} ; then
  if ${SINGLE_PACKAGE} ; then
    gzip -f ${DESTINATION}
  else
    for PACKAGE_NAME in "${PACKAGES[@]}" ; do if [[ ${PACKAGE_NAME} != "Carla" ]] ; then
      BUILD_FOLDER_TARGET=${CARLA_DIST_FOLDER}/${PACKAGE_NAME}_${REPOSITORY_TAG}
      if [[ ${ARCHIVE_SUFIX} != "" ]] ; then BUILD_FOLDER_TARGET=${BUILD_FOLDER_TARGET}_${ARCHIVE_SUFIX} ; fi
      DESTINATION=${BUILD_FOLDER_TARGET}.tar
      gzip -f ${DESTINATION}
    fi ; done
  fi
fi

T_END_PACKAGES=$(date +%s)
ELAPSED_TIME=$((T_END_PACKAGES - T_START_PACKAGES))
log "Cooking all other packages took ${ELAPSED_TIME} seconds."

# ==============================================================================
# -- Log paths of generated packages -------------------------------------------
# ==============================================================================

T_END_OVERALL=$(date +%s)
ELAPSED_TIME=$((T_END_OVERALL - T_START_OVERALL))
log "Overall packaging took ${ELAPSED_TIME} seconds."

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
