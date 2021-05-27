#! /bin/bash

# ==============================================================================
# -- Set up environment --------------------------------------------------------
# ==============================================================================

source $(dirname "$0")/Environment.sh

DOCKER="docker"
UNTAR="tar -xvzf"

REPOSITORY_TAG=$(get_git_repository_version)

LATEST_PACKAGE=CARLA_${REPOSITORY_TAG}.tar.gz
LATEST_PACKAGE_FILE=$(ls -t ${CARLA_DIST_FOLDER} | head -n 1)
LATEST_PACKAGE_PATH=${CARLA_DIST_FOLDER}/${LATEST_PACKAGE}


if [[ ${REPOSITORY_TAG} =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
  log "Detected tag ${REPOSITORY_TAG}."
  DEPLOY_NAME=CARLA_${REPOSITORY_TAG}.tar.gz
  DOCKER_TAG=${REPOSITORY_TAG}
elif [[ ${REPOSITORY_TAG} =~ ^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
  log "Detected tag ${REPOSITORY_TAG}."
  DEPLOY_NAME=CARLA_${REPOSITORY_TAG}.tar.gz
  DOCKER_TAG=${REPOSITORY_TAG}
else
  S3_PREFIX=${S3_PREFIX}/Dev
  DEPLOY_NAME=$(git log --pretty=format:'%cd_%h' --date=format:'%Y%m%d' -n 1).tar.gz
  DOCKER_TAG=latest
fi

log "Using package ${LATEST_PACKAGE} as ${DEPLOY_NAME}."

if [ ! -f ${LATEST_PACKAGE_PATH} ]; then
  fatal_error "Latest package not found, please run 'make package'."
fi



# ==============================================================================
# -- Docker build  -----------------------------------------------------
# ==============================================================================

DOCKER_BUILD_FOLDER=${CARLA_BUILD_FOLDER}/${REPOSITORY_TAG}.Docker
DOCKER_NAME=carlasim/carla-osi:${DOCKER_TAG}

mkdir -p ${DOCKER_BUILD_FOLDER}

${UNTAR} ${LATEST_PACKAGE_PATH} -C ${DOCKER_BUILD_FOLDER}/

pushd "${DOCKER_BUILD_FOLDER}" >/dev/null

log "Building Docker image ${DOCKER_NAME}."

${DOCKER} build -t ${DOCKER_NAME} -f Dockerfile .

popd >/dev/null


# ==============================================================================
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

log "Success!"
