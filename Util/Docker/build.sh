#!/bin/bash

set -e

DOC_STRING="Build the CARLA UE5 development docker image."

USAGE_STRING=$(cat <<- END
Usage: $0 [options]

User and group options:

    --user UID:GID       Set host UID and GID for the container (default: current user)
    --docker-gid GID     Set GID of the Docker group (default: $(getent group docker | cut -d: -f3))

Ubuntu distribution:

    --ubuntu-distro DISTRO   Specify ubuntu distro (default: 22.04).

Build options:

    --force-rebuild      Force rebuild images with no cache

Other commands:

    -h, --help           Show this help message and exit
END
)

UBUNTU_DISTRO=22.04

HOST_UID=$(id -u)
HOST_GID=$(id -g)
DOCKER_GID=$(getent group docker | cut -d: -f3)

FORCE_REBUILD=

OPTS=`getopt -o h --long help,ubuntu-distro:,user:,docker-gid:,force-rebuild -n 'parse-options' -- "$@"`

eval set -- "$OPTS"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --ubuntu-distro )
      UBUNTU_DISTRO="$2";
      shift 2 ;;
    --user )
      IFS=':' read -r HOST_UID HOST_GID <<< "$2"
      shift 2 ;;
    --docker-gid)
      DOCKER_GID="$2"
      shift 2 ;;
    --force-rebuild )
      FORCE_REBUILD=true
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

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CARLA_ROOT=${SCRIPT_DIR}/../..

# Copy python requirements into the Docker build context
rm -rf ${SCRIPT_DIR}/.tmp && mkdir ${SCRIPT_DIR}/.tmp
cp ${CARLA_ROOT}/requirements.txt ${SCRIPT_DIR}/.tmp/requirements.txt
cp ${CARLA_ROOT}/PythonAPI/examples/requirements.txt ${SCRIPT_DIR}/.tmp/examples_requirements.txt
cp ${CARLA_ROOT}/PythonAPI/util/requirements.txt ${SCRIPT_DIR}/.tmp/util_requirements.txt

# Build base image
echo "Building base image carla-base:ue5-${UBUNTU_DISTRO}"
docker build ${FORCE_REBUILD:+--no-cache} \
  --build-arg UBUNTU_DISTRO=${UBUNTU_DISTRO} \
  -t carla-base:ue5-${UBUNTU_DISTRO} \
  -f ${SCRIPT_DIR}/Base.Dockerfile ${SCRIPT_DIR}

# Build development image
if [ "$FORCE_REBUILD" = true ]; then
  echo "Removing existing volume carla-development-ue5-${UBUNTU_DISTRO}"
  docker volume rm -f carla-development-ue5-${UBUNTU_DISTRO} 2>/dev/null || true
fi
echo "Ensuring volume carla-development-ue5-${UBUNTU_DISTRO} exists"
docker volume create carla-development-ue5-${UBUNTU_DISTRO}

echo "Building development image carla-development:ue5-${UBUNTU_DISTRO} with user ${HOST_UID}:${HOST_GID}"
docker build ${FORCE_REBUILD:+--no-cache} \
  --build-arg UBUNTU_DISTRO=${UBUNTU_DISTRO} \
  --build-arg UID=${HOST_UID} \
  --build-arg GID=${HOST_GID} \
  --build-arg DOCKER_GID=${DOCKER_GID} \
  -t carla-development:ue5-${UBUNTU_DISTRO} \
  -f ${SCRIPT_DIR}/Development.Dockerfile ${SCRIPT_DIR}
