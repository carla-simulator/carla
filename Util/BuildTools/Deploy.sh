#! /bin/bash

# ==============================================================================
# -- Set up environment --------------------------------------------------------
# ==============================================================================

source $(dirname "$0")/Environment.sh

REPLACE_LATEST=false
DOCKER_PUSH=false
AWS_COPY="aws s3 cp"
DOCKER="docker"
UNTAR="tar -xvzf"
UPLOAD_MAPS=true

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DOC_STRING="Upload latest build to S3."

USAGE_STRING="Usage: $0 [-h|--help] [--replace-latest] [--docker-push] [--dry-run]"

OPTS=`getopt -o h --long help,replace-latest,docker-push,dry-run -n 'parse-options' -- "$@"`

eval set -- "$OPTS"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --replace-latest )
      REPLACE_LATEST=true;
      shift ;;
    --docker-push )
      DOCKER_PUSH=true;
      shift ;;
    --dry-run )
      AWS_COPY="echo ${AWS_COPY}";
      DOCKER="echo ${DOCKER}";
      UNTAR="echo ${UNTAR}";
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

REPOSITORY_TAG=$(get_git_repository_version)

LATEST_PACKAGE=CARLA_${REPOSITORY_TAG}.tar.gz
LATEST_PACKAGE_PATH=${CARLA_DIST_FOLDER}/${LATEST_PACKAGE}

S3_PREFIX=s3://carla-releases/Linux

LATEST_DEPLOY_URI=${S3_PREFIX}/Dev/CARLA_Latest.tar.gz

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
# -- Upload --------------------------------------------------------------------
# ==============================================================================

DEPLOY_URI=${S3_PREFIX}/${DEPLOY_NAME}

${AWS_COPY} ${LATEST_PACKAGE_PATH} ${DEPLOY_URI}

log "Latest build uploaded to ${DEPLOY_URI}."

if ${REPLACE_LATEST} ; then

  ${AWS_COPY} ${DEPLOY_URI} ${LATEST_DEPLOY_URI}

  log "Latest build uploaded to ${LATEST_DEPLOY_URI}."

fi

if ${UPLOAD_MAPS} ; then

  mkdir -p ${CARLA_DIST_FOLDER}

  pushd "${CARLA_DIST_FOLDER}" >/dev/null

  for MAP_PACKAGE in *_${REPOSITORY_TAG}.tar.gz ; do if [[ ${MAP_PACKAGE} != ${LATEST_PACKAGE} ]] ; then

    DEPLOY_MAP_URI=${S3_PREFIX}/${MAP_PACKAGE}

    ${AWS_COPY} ${MAP_PACKAGE} ${DEPLOY_MAP_URI}

    log "${MAP_PACKAGE} uploaded to ${DEPLOY_MAP_URI}."

  fi ; done

  popd >/dev/null

fi

# ==============================================================================
# -- Docker build and push -----------------------------------------------------
# ==============================================================================

if ${DOCKER_PUSH} ; then

  DOCKER_BUILD_FOLDER=${CARLA_BUILD_FOLDER}/${REPOSITORY_TAG}.Docker
  DOCKER_NAME=carlasim/carla:${DOCKER_TAG}

  mkdir -p ${DOCKER_BUILD_FOLDER}

  ${UNTAR} ${LATEST_PACKAGE_PATH} -C ${DOCKER_BUILD_FOLDER}/

  pushd "${DOCKER_BUILD_FOLDER}" >/dev/null

  log "Building Docker image ${DOCKER_NAME}."

  ${DOCKER} build -t ${DOCKER_NAME} -f Dockerfile .

  log "Pushing Docker image."

  ${DOCKER} push ${DOCKER_NAME}

  popd >/dev/null

fi;

# ==============================================================================
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

log "Success!"
