#! /bin/bash

# ==============================================================================
# -- Set up environment --------------------------------------------------------
# ==============================================================================



REPLACE_LATEST=false
DOCKER_PUSH=false
AWS_COPY="aws s3 cp"
DOCKER="docker"
UNTAR="tar -xvzf"
UPLOAD_MAPS=true
PROFILE="--profile Jenkins-CVC"
ENDPOINT="--endpoint-url=https://s3.us-east-005.backblazeb2.com/"
TEST=false


# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DOC_STRING="Upload latest build to S3."

USAGE_STRING="Usage: $0 [-h|--help] [--replace-latest] [--docker-push] [--dry-run]"

OPTS=`getopt -o h --long help,replace-latest,docker-push,dry-run,test -n 'parse-options' -- "$@"`

eval set -- "$OPTS"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --test )
      TEST=true
      shift ;;
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

source $(dirname "$0")/Environment.sh

REPOSITORY_TAG=$(get_git_repository_version)

LATEST_PACKAGE=CARLA_${REPOSITORY_TAG}.tar.gz
LATEST_PACKAGE_PATH=${CARLA_DIST_FOLDER}/${LATEST_PACKAGE}
LATEST_PACKAGE2=AdditionalMaps_${REPOSITORY_TAG}.tar.gz
LATEST_PACKAGE_PATH2=${CARLA_DIST_FOLDER}/${LATEST_PACKAGE2}

S3_PREFIX=s3://carla-releases/Linux

LATEST_DEPLOY_URI=${S3_PREFIX}/Dev/CARLA_Latest.tar.gz
LATEST_DEPLOY_URI2=${S3_PREFIX}/Dev/AdditionalMaps_Latest.tar.gz

if [[ ${REPOSITORY_TAG} =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
  log "Detected tag ${REPOSITORY_TAG}."
  DEPLOY_NAME=CARLA_${REPOSITORY_TAG}.tar.gz
  DEPLOY_NAME2=AdditionalMaps_${REPOSITORY_TAG}.tar.gz
  DOCKER_TAG=${REPOSITORY_TAG}
elif [[ ${REPOSITORY_TAG} =~ ^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
  log "Detected tag ${REPOSITORY_TAG}."
  DEPLOY_NAME=CARLA_${REPOSITORY_TAG}.tar.gz
  DEPLOY_NAME2=AdditionalMaps_${REPOSITORY_TAG}.tar.gz
  DOCKER_TAG=${REPOSITORY_TAG}
else
  S3_PREFIX=${S3_PREFIX}/Dev
  DEPLOY_NAME=$(git log --pretty=format:'%cd_%h' --date=format:'%Y%m%d' -n 1).tar.gz
  DEPLOY_NAME2=AdditionalMaps_$(git log --pretty=format:'%cd_%h' --date=format:'%Y%m%d' -n 1).tar.gz
  DOCKER_TAG=latest
fi

log "Using package ${LATEST_PACKAGE} as ${DEPLOY_NAME}."
log "Using package ${LATEST_PACKAGE2} as ${DEPLOY_NAME2}."

if [ ! -f ${LATEST_PACKAGE_PATH} ]; then
  fatal_error "Latest package not found, please run 'make package'."
fi


# ==============================================================================
# -- TEST --------------------------------------------------------------------
# ==============================================================================

if ${TEST} ; then
  LATEST_PACKAGE=test_CARLA_${REPOSITORY_TAG}.tar.gz
  LATEST_PACKAGE_PATH=./${LATEST_PACKAGE}
  LATEST_PACKAGE2=test_AdditionalMaps_${REPOSITORY_TAG}.tar.gz
  LATEST_PACKAGE_PATH2=./${LATEST_PACKAGE2}

  DEPLOY_NAME=test_CARLA_${REPOSITORY_TAG}.tar.gz
  DEPLOY_NAME2=test_AdditionalMaps_${REPOSITORY_TAG}.tar.gz

  touch ${LATEST_PACKAGE}
  touch ${LATEST_PACKAGE2}

fi

# ==============================================================================
# -- Upload --------------------------------------------------------------------
# ==============================================================================

DEPLOY_URI=${S3_PREFIX}/${DEPLOY_NAME}
DEPLOY_URI2=${S3_PREFIX}/${DEPLOY_NAME2}

${AWS_COPY} ${LATEST_PACKAGE_PATH} ${DEPLOY_URI} ${ENDPOINT} ${PROFILE}
log "Latest build uploaded to ${DEPLOY_URI}."

${AWS_COPY} ${LATEST_PACKAGE_PATH2} ${DEPLOY_URI2} ${ENDPOINT} ${PROFILE}
log "Latest build uploaded to ${DEPLOY_URI2}."

# ==============================================================================
# -- Replace Latest ------------------------------------------------------------
# ==============================================================================

if ${REPLACE_LATEST} ; then

  ${AWS_COPY} ${DEPLOY_URI} ${LATEST_DEPLOY_URI} ${ENDPOINT} ${PROFILE}
  log "Latest build uploaded to ${LATEST_DEPLOY_URI}."
  
  ${AWS_COPY} ${DEPLOY_URI2} ${LATEST_DEPLOY_URI2} ${ENDPOINT} ${PROFILE}
  log "Latest build uploaded to ${LATEST_DEPLOY_URI2}."

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
