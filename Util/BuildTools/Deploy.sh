#! /bin/bash

# ==============================================================================
# -- Set up environment --------------------------------------------------------
# ==============================================================================

source $(dirname "$0")/Environment.sh

REPLACE_LATEST=false
AWS_COPY="aws s3 cp"
UPLOAD_MAPS=true

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DOC_STRING="Upload latest build to S3."

USAGE_STRING="Usage: $0 [-h|--help] [--replace-latest] [--dry-run]"

OPTS=`getopt -o h --long help,replace-latest,dry-run -n 'parse-options' -- "$@"`

if [ $? != 0 ] ; then echo "$USAGE_STRING" ; exit 2 ; fi

eval set -- "$OPTS"

while true; do
  case "$1" in
    --replace-latest )
      REPLACE_LATEST=true;
      shift ;;
    --dry-run )
      AWS_COPY="echo ${AWS_COPY}";
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

REPOSITORY_TAG=$(get_git_repository_version)

LATEST_PACKAGE=CARLA_${REPOSITORY_TAG}.tar.gz
LATEST_PACKAGE_PATH=${CARLA_DIST_FOLDER}/${LATEST_PACKAGE}

S3_PREFIX=s3://carla-assets-internal/Releases/Linux

LATEST_DEPLOY_URI=${S3_PREFIX}/Dev/CARLA_Latest.tar.gz

if [[ ${REPOSITORY_TAG} =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
  log "Detected tag ${REPOSITORY_TAG}."
  DEPLOY_NAME=CARLA_${REPOSITORY_TAG}.tar.gz
else
  S3_PREFIX=${S3_PREFIX}/Dev
  DEPLOY_NAME=$(git log --pretty=format:'%cd_%h' --date=format:'%Y%m%d' -n 1).tar.gz
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

  mkdir -p ${CARLA_EXPORTED_MAPS_FOLDER}
  pushd "${CARLA_EXPORTED_MAPS_FOLDER}" >/dev/null

  for MAP_PACKAGE in *.tar.gz; do

    DEPLOY_MAP_NAME=$(basename "${MAP_PACKAGE}" .tar.gz)_${REPOSITORY_TAG}.tar.gz
    DEPLOY_MAP_URI=${S3_PREFIX}/${DEPLOY_MAP_NAME}

    ${AWS_COPY} ${MAP_PACKAGE} ${DEPLOY_MAP_URI}

    log "${MAP_PACKAGE} uploaded to ${DEPLOY_MAP_URI}."

  done

  popd >/dev/null

fi

# ==============================================================================
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

log "Success!"
