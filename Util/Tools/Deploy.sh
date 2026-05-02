#! /bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# ==============================================================================
# -- Set up environment --------------------------------------------------------
# ==============================================================================

REPLACE_LATEST=false
AWS_COPY="aws s3 cp"
ENDPOINT="https://s3.us-east-005.backblazeb2.com"
SUMMARY_OUTPUT_PATH=

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DOC_STRING="Upload latest build to S3."

USAGE_STRING="Usage: $0 [-h|--help] [--replace-latest] [--docker-push] [--dry-run] [--summary-output SUMMARY_OUTPUT]"

OPTS=`getopt -o h --long help,replace-latest,docker-push,dry-run,summary-output: -n 'parse-options' -- "$@"`

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
      # DOCKER="echo ${DOCKER}";
      # UNTAR="echo ${UNTAR}";
      shift ;;
    --summary-output )
      SUMMARY_OUTPUT_PATH="$2";
      shift 2 ;;
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
# REPOSITORY_TAG=$(get_git_repository_version)

# Set the package name and path to be deployed
# TODO: Currently hardcoded. Replace with dynamic versioning in the future
PACKAGE_NAME=Carla-0.10.0-Linux-Shipping.tar.gz
PACKAGE_PATH=${SCRIPT_DIR}/../../Build/Release/Package/${PACKAGE_NAME}

log "Using package ${PACKAGE_PATH}."

# Check if the package exists before continuing
if [ ! -f ${PACKAGE_PATH} ]; then
  fatal_error "Package not found, please build a package"
fi

# Generate the nightly deploy filename based on the latest git commit date and hash
NIGHTLY_DEPLOY_NAME=$(git log --pretty=format:'%cd_%h' --date=format:'%Y%m%d' -n 1).tar.gz
LATEST_DEPLOY_NAME="CARLA_UE5_Latest.tar.gz"

# Backblaze bucket paths
S3_PREFIX=s3://carla-releases/Linux
URL_PREFIX=${ENDPOINT}/carla-releases/Linux

NIGHTLY_DEPLOY_URI=${S3_PREFIX}/Dev/${NIGHTLY_DEPLOY_NAME}
LATEST_DEPLOY_URI=${S3_PREFIX}/Dev/${LATEST_DEPLOY_NAME}

# ==============================================================================
# -- Upload --------------------------------------------------------------------
# ==============================================================================

log "Uploading ${PACKAGE_NAME} as ${NIGHTLY_DEPLOY_NAME}."

${AWS_COPY} ${PACKAGE_PATH} ${NIGHTLY_DEPLOY_URI} --endpoint-url ${ENDPOINT}
log "Nightly build uploaded to ${NIGHTLY_DEPLOY_URI}."

# ==============================================================================
# -- Replace Latest ------------------------------------------------------------
# ==============================================================================

# Update latest with the nightly build if desired.
if ${REPLACE_LATEST} ; then

  log "Updating latest build with nightly build"

  ${AWS_COPY} ${NIGHTLY_DEPLOY_URI} ${LATEST_DEPLOY_URI} --endpoint-url ${ENDPOINT}
  log "Latest build uploaded to ${LATEST_DEPLOY_URI}."

fi

# ==============================================================================
# -- Docker build and push -----------------------------------------------------
# ==============================================================================

# if ${DOCKER_PUSH} ; then

#   DOCKER_BUILD_FOLDER=${CARLA_BUILD_FOLDER}/${REPOSITORY_TAG}.Docker
#   DOCKER_NAME=carlasim/carla:${DOCKER_TAG}

#   mkdir -p ${DOCKER_BUILD_FOLDER}

#   ${UNTAR} ${LATEST_PACKAGE_PATH} -C ${DOCKER_BUILD_FOLDER}/

#   pushd "${DOCKER_BUILD_FOLDER}" >/dev/null

#   log "Building Docker image ${DOCKER_NAME}."

#   ${DOCKER} build -t ${DOCKER_NAME} -f Dockerfile .

#   log "Pushing Docker image."

#   ${DOCKER} push ${DOCKER_NAME}

#   popd >/dev/null

# fi;

# ==============================================================================
# -- Summary output ------------------------------------------------------------
# ==============================================================================

if [[ -n "$SUMMARY_OUTPUT_PATH" ]]; then
  {
    echo "package_uri=${URL_PREFIX}/Dev/${NIGHTLY_DEPLOY_NAME}"
  } >> "$SUMMARY_OUTPUT_PATH"
fi

# ==============================================================================
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

log "Success!"
