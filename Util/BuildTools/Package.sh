#! /bin/bash

source $(dirname "$0")/Environment.sh

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DOC_STRING="Makes a packaged version of CARLA and other content packages ready for distribution."

USAGE_STRING="Usage: $0 [-h|--help] [--no-zip] [--clean-intermediate] [--packages=Name1,Name2,...]"

PACKAGES="Carla"
DO_TARBALL=true
DO_CLEAN_INTERMEDIATE=false

OPTS=`getopt -o h --long help,no-zip,clean-intermediate,packages: -n 'parse-options' -- "$@"`

if [ $? != 0 ] ; then echo "$USAGE_STRING" ; exit 2 ; fi

eval set -- "$OPTS"

while true; do
  case "$1" in
    --no-zip )
      DO_TARBALL=false
      shift ;;
    --clean-intermediate )
      DO_CLEAN_INTERMEDIATE=true
      shift ;;
    --packages )
      PACKAGES="$2"
      shift 2 ;;
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
# -- Prepare environment -------------------------------------------------------
# ==============================================================================

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

RELEASE_BUILD_FOLDER=${CARLA_DIST_FOLDER}/CARLA_${REPOSITORY_TAG}

log "Packaging version '${REPOSITORY_TAG}'."

# ==============================================================================
# -- Cook CARLA project --------------------------------------------------------
# ==============================================================================

if ${DO_CARLA_RELEASE} ; then

  pushd "${CARLAUE4_ROOT_FOLDER}" >/dev/null

  log "Cooking CARLA project."

  rm -Rf ${RELEASE_BUILD_FOLDER}
  mkdir -p ${RELEASE_BUILD_FOLDER}

  ${UE4_ROOT}/Engine/Build/BatchFiles/RunUAT.sh BuildCookRun \
      -project="${PWD}/CarlaUE4.uproject" \
      -nocompileeditor -nop4 -cook -stage -archive -package \
      -clientconfig=Development -ue4exe=UE4Editor \
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

  copy_if_changed "./PythonAPI/carla/dist/*.egg" "${DESTINATION}/PythonAPI/carla/dist/"
  copy_if_changed "./PythonAPI/carla/agents/" "${DESTINATION}/PythonAPI/carla/agents"
  copy_if_changed "./PythonAPI/carla/scene_layout.py" "${DESTINATION}/PythonAPI/carla/"
  copy_if_changed "./PythonAPI/carla/requirements.txt" "${DESTINATION}/PythonAPI/carla/"

  copy_if_changed "./PythonAPI/examples/*.py" "${DESTINATION}/PythonAPI/examples/"
  copy_if_changed "./PythonAPI/examples/requirements.txt" "${DESTINATION}/PythonAPI/examples/"

  copy_if_changed "./PythonAPI/util/*.py" "${DESTINATION}/PythonAPI/util/"
  copy_if_changed "./PythonAPI/util/requirements.txt" "${DESTINATION}/PythonAPI/util/"

  copy_if_changed "./Unreal/CarlaUE4/Content/Carla/HDMaps/*.pcd" "${DESTINATION}/HDMaps/"
  copy_if_changed "./Unreal/CarlaUE4/Content/Carla/HDMaps/Readme.md" "${DESTINATION}/HDMaps/README"

  popd >/dev/null

fi

# ==============================================================================
# -- Zip the project -----------------------------------------------------------
# ==============================================================================

if ${DO_CARLA_RELEASE} && ${DO_TARBALL} ; then

  DESTINATION=${CARLA_DIST_FOLDER}/CARLA_${REPOSITORY_TAG}.tar.gz
  SOURCE=${RELEASE_BUILD_FOLDER}/LinuxNoEditor

  pushd "${SOURCE}" >/dev/null

  log "Packaging CARLA release."

  rm -f ./Manifest_NonUFSFiles_Linux.txt
  rm -f ./Manifest_UFSFiles_Linux.txt
  rm -Rf ./CarlaUE4/Saved
  rm -Rf ./Engine/Saved

  tar -czvf ${DESTINATION} *

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

for PACKAGE_NAME in "${PACKAGES[@]}" ; do if [[ ${PACKAGE_NAME} != "Carla" ]] ; then

  BUILD_FOLDER=${CARLA_DIST_FOLDER}/${PACKAGE_NAME}_${REPOSITORY_TAG}

  DESTINATION=${BUILD_FOLDER}.tar.gz

  mkdir -p ${BUILD_FOLDER}

  log "Preparing environment for cooking '${PACKAGE_NAME}'."

  fatal_error "TODO: Call command-let here."

  pushd "${CARLAUE4_ROOT_FOLDER}" > /dev/null

  log "Cooking '${PACKAGE_NAME}'."

  ${UE4_ROOT}/Engine/Binaries/Linux/UE4Editor "${PWD}/CarlaUE4.uproject" \
      -run=cook -cooksinglepackage -targetplatform="LinuxNoEditor" \
      -OutputDir="${BUILD_FOLDER}" \
      -Map=${MAP_TO_COOK}

  popd >/dev/null

  if ${DO_TARBALL} ; then

    pushd "${BUILD_FOLDER}" > /dev/null

    log "Packaging '${PACKAGE_NAME}'."

    rm -Rf ./CarlaUE4/Metadata
    rm -Rf ./CarlaUE4/Plugins
    rm ./CarlaUE4/AssetRegistry.bin

    fatal_error "TODO: Remove intermediate maps here."

    tar -czvf ${DESTINATION} *

    popd >/dev/null

  fi

  if ${DO_CLEAN_INTERMEDIATE} ; then
    log "Removing intermediate build."
    rm -Rf ${BUILD_FOLDER}
  fi

fi ; done

# ==============================================================================
# -- Log paths of generated packages -------------------------------------------
# ==============================================================================

for PACKAGE_NAME in "${PACKAGES[@]}" ; do if [[ ${PACKAGE_NAME} != "Carla" ]] ; then
  FINAL_PACKAGE=${CARLA_DIST_FOLDER}/${PACKAGE_NAME}_${REPOSITORY_TAG}.tar.gz
  log "Package '${PACKAGE_NAME}' created at ${FINAL_PACKAGE}"
fi ; done

if ${DO_CARLA_RELEASE} ; then
  if ${DO_TARBALL} ; then
    FINAL_PACKAGE=${CARLA_DIST_FOLDER}/CARLA_${REPOSITORY_TAG}.tar.gz
  else
    FINAL_PACKAGE=${RELEASE_BUILD_FOLDER}
  fi
  log "CARLA release created at ${FINAL_PACKAGE}"
fi

# ==============================================================================
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

log "Success!"
