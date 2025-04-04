#! /bin/bash

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
TIME1=$(date +%s.%N)
echo "TIME1: $TIME1"

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

# ==============================================================================
# -- Cook CARLA project --------------------------------------------------------
# ==============================================================================
TIME2=$(date +%s.%N)
echo "TIME2: $TIME2"

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
TIME3=$(date +%s.%N)
echo "TIME3: $TIME3"

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

  if [ -d "./Unreal/CarlaUE4/Plugins/Carla/CarlaDependencies/lib" ] ; then
    cp -r "./Unreal/CarlaUE4/Plugins/Carla/CarlaDependencies/lib" "${DESTINATION}/CarlaUE4/Plugins/Carla/CarlaDependencies"
  fi

  copy_if_changed "./Unreal/CarlaUE4/Content/Carla/HDMaps/*.pcd" "${DESTINATION}/HDMaps/"
  copy_if_changed "./Unreal/CarlaUE4/Content/Carla/HDMaps/Readme.md" "${DESTINATION}/HDMaps/README"

  popd >/dev/null

fi

# ==============================================================================
# -- Zip the project -----------------------------------------------------------
# ==============================================================================
TIME4=$(date +%s.%N)
echo "TIME4: $TIME4"

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
TIME5=$(date +%s.%N)
echo "TIME5: $TIME5"

if ${DO_CARLA_RELEASE} && ${DO_CLEAN_INTERMEDIATE} ; then

  log "Removing intermediate build."

  rm -Rf ${RELEASE_BUILD_FOLDER}

fi

# ==============================================================================
# -- Cook other packages -------------------------------------------------------
# ==============================================================================
package_map() {

  package_name = '$1'

  log "Preparing environment for cooking '${package_name}'."

  if ${SINGLE_PACKAGE} ; then
      BUILD_FOLDER_TARGET=${CARLA_DIST_FOLDER}/${TARGET_ARCHIVE}_${REPOSITORY_TAG}
  else
      BUILD_FOLDER_TARGET=${CARLA_DIST_FOLDER}/${package_name}_${REPOSITORY_TAG}
  fi

  if [[ ${ARCHIVE_SUFIX} != "" ]] ; then
    BUILD_FOLDER_TARGET=${BUILD_FOLDER_TARGET}_${ARCHIVE_SUFIX}
  fi

  if [[ ${ARCHIVE_SUFIX} != "" ]] ; then
    BUILD_FOLDER=${CARLA_DIST_FOLDER}/${package_name}_${REPOSITORY_TAG}_${ARCHIVE_SUFIX}
  else
    BUILD_FOLDER=${CARLA_DIST_FOLDER}/${package_name}_${REPOSITORY_TAG}
  fi

  DESTINATION=${BUILD_FOLDER_TARGET}.tar
  PACKAGE_PATH=${CARLAUE4_ROOT_FOLDER}/Content/${package_name}

  mkdir -p ${BUILD_FOLDER}

  log "Cooking package '${package_name}'..."
  pushd "${CARLAUE4_ROOT_FOLDER}" > /dev/null

  # Prepare cooking of package
  ${UE4_ROOT}/Engine/Binaries/Linux/UE4Editor "${CARLAUE4_ROOT_FOLDER}/CarlaUE4.uproject" \
      -run=PrepareAssetsForCooking -PackageName=${package_name} -OnlyPrepareMaps=false

  PACKAGE_PATH_FILE=${CARLAUE4_ROOT_FOLDER}/Content/PackagePath.txt
  PACKAGE_FILE=$(<${PACKAGE_PATH_FILE})
  MAP_LIST_FILE=${CARLAUE4_ROOT_FOLDER}/Content/MapPathsLinux.txt
  MAPS_TO_COOK=$(<${MAP_LIST_FILE})

  # Cook maps in batches
  MAX_STRINGLENGTH=1000
  IFS="+" read -ra MAP_LIST <<< $MAPS_TO_COOK
  TOTAL=0
  MAP_STRING=""
  for MAP in "${MAP_LIST[@]}"; do
    if (($(($TOTAL+${#MAP})) > $MAX_STRINGLENGTH)); then
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
  rm -Rf "./CarlaUE4/Content/${package_name}/Maps/${PROPS_MAP_NAME}"
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

  echo "$package_name has finished"
}

PIDS=()

for PACKAGE_NAME in "${PACKAGES[@]}" ; do if [[ ${PACKAGE_NAME} != "Carla" ]] ; then
  package_map() $PACKAGE_NAME & 
  PID=$!
  PIDS+=($PID)
  sleep 10
done

for PID in "${PIDS[@]}"
  echo "Waiting for $PID"
  wait $PID
  echo "Done $PID"
done

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

TIME6=$(date +%s.%N)
echo "TIME6: $TIME6"

# ==============================================================================
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

log "Success!"
TOTAL_TIME=$(echo "$TIME6 - $TIME1" | bc)
TT_=$(echo "$T1/60" | bc)

T1=$(echo "$TIME2 - $TIME1" | bc)
T1_=$(echo "$T1/60" | bc)
TT1=$(echo "100*$T1/$TOTAL_TIME" | bc)
echo "Preparing_environment: $T1 $T1_ $TT1%"

T2=$(echo "$TIME3 - $TIME2" | bc)
T2_=$(echo "$T2/60" | bc)
TT2=$(echo "100*$T2/$TOTAL_TIME" | bc)
echo "Cooking: $T2 $T2_ $TT2%"

T3=$(echo "$TIME4 - $TIME3" | bc)
T3_=$(echo "$T3/60" | bc)
TT3=$(echo "100*$T3/$TOTAL_TIME" | bc)
echo "Copy_files: $T3 $T3_ $TT3%"

T4=$(echo "$TIME5 - $TIME4" | bc)
T4_=$(echo "$T4/60" | bc)
TT4=$(echo "100*$T4/$TOTAL_TIME" | bc)
echo "Zip: $T4 $T4_ $TT4%"

T5=$(echo "$TIME6 - $TIME4" | bc)
T5_=$(echo "$T5/60" | bc)
TT5=$(echo "100*$T5/$TOTAL_TIME" | bc)
echo "Packages: $T5 $T5_ $TT5%"

# T5=$(echo "$TIME6 - $TIME5" | bc)
# T5_=$(echo "$T5/60" | bc)
# TT5=$(echo "100*$T5/$TOTAL_TIME" | bc)
# echo "Remove_intermediate: $T5 $T5_ $TT5%"

# for i in "${!PACKAGE_TIMES[@]}"; do
#   PACKAGE_TIME=${PACKAGE_TIMES[$i]}
#   TP_=$(echo "$PACKAGE_TIME/60" | bc)
#   NEXT_INDEX=$((i+1))
#   NAME=${PACKAGES[$NEXT_INDEX]}
#   TP=$(echo "100*$PACKAGE_TIME/$TOTAL_TIME" | bc)
#   echo "Package_$NAME: $PACKAGE_TIME $TP_ $TP%"
# done

# T9=$(echo "$TIME10 - $TIME9" | bc)
# T9_=$(echo "$T9/60" | bc)
# TT9=$(echo "100*$T9/$TOTAL_TIME" | bc)
# echo "Tar: $T9 $T9_ $TT9%"

# T10=$(echo "$TIME11 - $TIME10" | bc)
# T10_=$(echo "$T10/60" | bc)
# TT10=$(echo "100*$T10/$TOTAL_TIME" | bc)
# echo "Logs: $T10 $T10_ $TT10%"

# for i in "${!PACKAGE_TIMES[@]}"; do
#   PACKAGE_TIME=${PACKAGE_TIMES[$i]}
#   COOKING_TIME=${COOKING_TIMES[$i]}
#   ZIPPING_TIME=${ZIPPING_TIMES[$i]}
#   TP_=$(echo "$PACKAGE_TIME/60" | bc)
#   TC_=$(echo "$COOKING_TIME/60" | bc)
#   TZ_=$(echo "$ZIPPING_TIME/60" | bc)
#   NEXT_INDEX=$((i+1))
#   NAME=${PACKAGES[$NEXT_INDEX]}
#   TP=$(echo "100*$PACKAGE_TIME/$TOTAL_TIME" | bc)
#   TC=$(echo "100*$COOKING_TIME/$TOTAL_TIME" | bc)
#   TZ=$(echo "100*$ZIPPING_TIME/$TOTAL_TIME" | bc)
#   echo "Package_$NAME: $PACKAGE_TIME $TP_ $TP%"
#   echo "Cooking_$NAME: $COOKING_TIME $TC_ $TC%"
#   echo "Zipping_$NAME: $ZIPPING_TIME $TZ_ $TZ%"
# done

echo "Total: $TOTAL_TIME $TT_ 100%"
