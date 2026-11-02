#! /bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# ==============================================================================
# -- Set up environment --------------------------------------------------------
# ==============================================================================

REPLACE_LATEST=false
AWS_COPY="aws s3 cp"
ENDPOINT="https://1b808339cbe94fecce530dfada93713b.r2.cloudflarestorage.com"
SUMMARY_OUTPUT_PATH=
ADDITIONAL_MAPS=false
LATEST_PREFIX=CARLA_UE5_Latest
RELEASE_VERSION=

# ==============================================================================
# -- Parse arguments -----------------------------------------------------------
# ==============================================================================

DOC_STRING="Upload latest build to S3."

USAGE_STRING="Usage: $0 [-h|--help] [--replace-latest] [--additional-maps] [--latest-prefix PREFIX] [--release-version VERSION] [--docker-push] [--dry-run] [--summary-output SUMMARY_OUTPUT]"

OPTS=`getopt -o h --long help,replace-latest,additional-maps,latest-prefix:,release-version:,docker-push,dry-run,summary-output: -n 'parse-options' -- "$@"`

eval set -- "$OPTS"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --replace-latest )
      REPLACE_LATEST=true;
      shift ;;
    --additional-maps )
      ADDITIONAL_MAPS=true;
      shift ;;
    --latest-prefix )
      LATEST_PREFIX="$2";
      shift 2 ;;
    --release-version )
      RELEASE_VERSION="$2";
      shift 2 ;;
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

if [[ -n "${RELEASE_VERSION}" ]] && ${REPLACE_LATEST} ; then
  fatal_error "--release-version and --replace-latest cannot be combined: a release never replaces latest."
fi

PACKAGE_DIR=${SCRIPT_DIR}/../../Build/Release/Package

# Prints the single file of PACKAGE_DIR matching the glob $1, fails otherwise.
find_one_package_file () {
  local MATCHES=(${PACKAGE_DIR}/$1)
  if [ ${#MATCHES[@]} -ne 1 ] || [ ! -f "${MATCHES[0]}" ]; then
    fatal_error "Expected exactly one $1 in ${PACKAGE_DIR}, found: ${MATCHES[*]}"
  fi
  echo "${MATCHES[0]}"
}

PACKAGE_PATH=$(find_one_package_file "Carla-*-Linux-Shipping.tar.gz") || exit 1
PACKAGE_NAME=$(basename "${PACKAGE_PATH}")
log "Using package ${PACKAGE_PATH}."

# The base release metadata the package target writes: needed to build content
# packs (AdditionalMaps or your own) that install into this package.
METADATA_PATH=$(find_one_package_file "*-release-metadata.tar.gz") || exit 1
log "Using release metadata ${METADATA_PATH}."

if ${ADDITIONAL_MAPS} ; then
  ADDITIONAL_MAPS_PATH=$(find_one_package_file "AdditionalMaps-*-Linux.tar.gz") || exit 1
  log "Using additional maps pack ${ADDITIONAL_MAPS_PATH}."
fi

# A release goes to CARLA_<version>/ named after the version; a nightly goes to
# Dev/ named after the latest git commit date and hash.
if [[ -n "${RELEASE_VERSION}" ]]; then
  DEPLOY_FOLDER=CARLA_${RELEASE_VERSION}
  NIGHTLY_DEPLOY_BASENAME=CARLA_${RELEASE_VERSION}
else
  DEPLOY_FOLDER=Dev
  NIGHTLY_DEPLOY_BASENAME=$(git log --pretty=format:'%cd_%h' --date=format:'%Y%m%d' -n 1)
fi
NIGHTLY_DEPLOY_NAME=${NIGHTLY_DEPLOY_BASENAME}.tar.gz
NIGHTLY_METADATA_DEPLOY_NAME=${NIGHTLY_DEPLOY_BASENAME}_release-metadata.tar.gz
NIGHTLY_ADDITIONAL_MAPS_DEPLOY_NAME=${NIGHTLY_DEPLOY_BASENAME}_AdditionalMaps.tar.gz
# --latest-prefix gives a workflow its own set of "latest" names, so that a
# package and the pack cooked against it are never replaced one without the other.
LATEST_DEPLOY_NAME="${LATEST_PREFIX}.tar.gz"
LATEST_METADATA_DEPLOY_NAME="${LATEST_PREFIX}_release-metadata.tar.gz"
LATEST_ADDITIONAL_MAPS_DEPLOY_NAME="${LATEST_PREFIX}_AdditionalMaps.tar.gz"

# Cloudflare R2 bucket paths
S3_PREFIX=s3://carla-releases/Linux
URL_PREFIX=https://downloads.carlasim.com/Linux

NIGHTLY_DEPLOY_URI=${S3_PREFIX}/${DEPLOY_FOLDER}/${NIGHTLY_DEPLOY_NAME}
NIGHTLY_METADATA_DEPLOY_URI=${S3_PREFIX}/${DEPLOY_FOLDER}/${NIGHTLY_METADATA_DEPLOY_NAME}
NIGHTLY_ADDITIONAL_MAPS_DEPLOY_URI=${S3_PREFIX}/${DEPLOY_FOLDER}/${NIGHTLY_ADDITIONAL_MAPS_DEPLOY_NAME}
LATEST_DEPLOY_URI=${S3_PREFIX}/${DEPLOY_FOLDER}/${LATEST_DEPLOY_NAME}
LATEST_METADATA_DEPLOY_URI=${S3_PREFIX}/${DEPLOY_FOLDER}/${LATEST_METADATA_DEPLOY_NAME}
LATEST_ADDITIONAL_MAPS_DEPLOY_URI=${S3_PREFIX}/${DEPLOY_FOLDER}/${LATEST_ADDITIONAL_MAPS_DEPLOY_NAME}

# ==============================================================================
# -- Upload --------------------------------------------------------------------
# ==============================================================================

log "Uploading ${PACKAGE_NAME} as ${NIGHTLY_DEPLOY_NAME}."

${AWS_COPY} ${PACKAGE_PATH} ${NIGHTLY_DEPLOY_URI} --endpoint-url ${ENDPOINT}
log "Package uploaded to ${NIGHTLY_DEPLOY_URI}."

${AWS_COPY} ${METADATA_PATH} ${NIGHTLY_METADATA_DEPLOY_URI} --endpoint-url ${ENDPOINT}
log "Release metadata uploaded to ${NIGHTLY_METADATA_DEPLOY_URI}."

if ${ADDITIONAL_MAPS} ; then
  ${AWS_COPY} ${ADDITIONAL_MAPS_PATH} ${NIGHTLY_ADDITIONAL_MAPS_DEPLOY_URI} --endpoint-url ${ENDPOINT}
  log "Additional maps uploaded to ${NIGHTLY_ADDITIONAL_MAPS_DEPLOY_URI}."
fi

# ==============================================================================
# -- Replace Latest ------------------------------------------------------------
# ==============================================================================

# Update latest with the nightly build if desired.
if ${REPLACE_LATEST} ; then

  log "Updating latest build with nightly build"

  # Cloudflare R2 does not implement object tagging, so copy only metadata.
  ${AWS_COPY} ${NIGHTLY_DEPLOY_URI} ${LATEST_DEPLOY_URI} --endpoint-url ${ENDPOINT} --copy-props metadata-directive
  log "Latest build uploaded to ${LATEST_DEPLOY_URI}."

  # Upload the metadata again instead of copying it: it is below the multipart
  # threshold, and a single-part copy sends x-amz-tagging-directive whatever
  # --copy-props says, which R2 rejects with NotImplemented.
  ${AWS_COPY} ${METADATA_PATH} ${LATEST_METADATA_DEPLOY_URI} --endpoint-url ${ENDPOINT}
  log "Latest release metadata uploaded to ${LATEST_METADATA_DEPLOY_URI}."

  if ${ADDITIONAL_MAPS} ; then
    ${AWS_COPY} ${NIGHTLY_ADDITIONAL_MAPS_DEPLOY_URI} ${LATEST_ADDITIONAL_MAPS_DEPLOY_URI} --endpoint-url ${ENDPOINT} --copy-props metadata-directive
    log "Latest additional maps uploaded to ${LATEST_ADDITIONAL_MAPS_DEPLOY_URI}."
  fi

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
    echo "package_uri=${URL_PREFIX}/${DEPLOY_FOLDER}/${NIGHTLY_DEPLOY_NAME}"
    echo "release_metadata_uri=${URL_PREFIX}/${DEPLOY_FOLDER}/${NIGHTLY_METADATA_DEPLOY_NAME}"
    if ${ADDITIONAL_MAPS} ; then
      echo "additional_maps_uri=${URL_PREFIX}/${DEPLOY_FOLDER}/${NIGHTLY_ADDITIONAL_MAPS_DEPLOY_NAME}"
    fi
    if ${REPLACE_LATEST} ; then
      echo "latest_package_uri=${URL_PREFIX}/${DEPLOY_FOLDER}/${LATEST_DEPLOY_NAME}"
      echo "latest_release_metadata_uri=${URL_PREFIX}/${DEPLOY_FOLDER}/${LATEST_METADATA_DEPLOY_NAME}"
      if ${ADDITIONAL_MAPS} ; then
        echo "latest_additional_maps_uri=${URL_PREFIX}/${DEPLOY_FOLDER}/${LATEST_ADDITIONAL_MAPS_DEPLOY_NAME}"
      fi
    fi
  } >> "$SUMMARY_OUTPUT_PATH"
fi

# ==============================================================================
# -- ...and we are done --------------------------------------------------------
# ==============================================================================

log "Success!"
