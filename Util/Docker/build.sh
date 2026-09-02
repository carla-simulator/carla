#!/bin/bash

set -e

DOC_STRING="Build the CARLA UE5 development docker image.

The image bundles Python 3.8 through 3.14 (each compiled from source alongside
the distro's system interpreter). Inside the container, build the
'carla-python-api-wheels' CMake target (configure with
-DCARLA_PYTHON_API_VERSIONS=\"3.8;...;3.14\") to produce a CARLA Python API wheel
for any or all of those versions."

USAGE_STRING=$(cat <<- END
Usage: $0 [options]

User and group options:

    --user UID:GID       Set host UID and GID for the container (default: current user)
    --docker-gid GID     Set GID of the Docker group (default: $(getent group docker | cut -d: -f3))

Ubuntu distribution:

    --ubuntu-distro DISTRO   Specify ubuntu distro (default: 24.04, also supports 22.04).

Build options:

    --force-rebuild      Force rebuild images with no cache

Other commands:

    -h, --help           Show this help message and exit
END
)

UBUNTU_DISTRO=24.04

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
DISTRO_DIR=${SCRIPT_DIR}/${UBUNTU_DISTRO}

if [ ! -d "${DISTRO_DIR}" ]; then
  echo "[ERROR] Unsupported --ubuntu-distro '${UBUNTU_DISTRO}'. Expected one of:"
  for d in ${SCRIPT_DIR}/*/; do
    name=$(basename "${d}")
    [ -f "${d}/Base.Dockerfile" ] && echo "  - ${name}"
  done
  exit 1
fi

CARLA_ROOT_ABS="$(cd "${CARLA_ROOT}" && pwd)"

# Build base image
echo "Building base image carla-base:ue5-${UBUNTU_DISTRO}"
docker build ${FORCE_REBUILD:+--no-cache} \
  --build-context carla-root=${CARLA_ROOT_ABS} \
  -t carla-base:ue5-${UBUNTU_DISTRO} \
  -f ${DISTRO_DIR}/Base.Dockerfile ${DISTRO_DIR}

# Build development image
if [ "$FORCE_REBUILD" = true ]; then
  echo "Removing existing volume carla-development-ue5-${UBUNTU_DISTRO}"
  docker volume rm -f carla-development-ue5-${UBUNTU_DISTRO} 2>/dev/null || true
fi
echo "Ensuring volume carla-development-ue5-${UBUNTU_DISTRO} exists"
docker volume create carla-development-ue5-${UBUNTU_DISTRO}

echo "Building development image carla-development:ue5-${UBUNTU_DISTRO} with user ${HOST_UID}:${HOST_GID}"
docker build ${FORCE_REBUILD:+--no-cache} \
  --build-context carla-root=${CARLA_ROOT_ABS} \
  --build-arg UID=${HOST_UID} \
  --build-arg GID=${HOST_GID} \
  --build-arg DOCKER_GID=${DOCKER_GID} \
  -t carla-development:ue5-${UBUNTU_DISTRO} \
  -f ${DISTRO_DIR}/Development.Dockerfile ${DISTRO_DIR}
