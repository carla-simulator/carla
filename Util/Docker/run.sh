#!/bin/bash

set -e

DOC_STRING="Run docker containers.

This script launches development or CI/CD docker containers."

USAGE_STRING=$(cat <<- END
Usage: $0 [options]

Run configurations (choose one):

    --dev               Run the development docker container
    --ci                Run the CI/CD docker container

User and group options:

    --user UID:GID      Set host UID and GID for the container (default: current user)
    --docker-gid GID    Set GID of the Docker group (default: $(getent group docker | cut -d: -f3))

Ubuntu distribution:

    --ubuntu-distro DISTRO   Specify ubuntu distro (default: 20.04)

Build options:

    --rebuild           Rebuild the docker image before running

Other commands:

    -h, --help          Show this help message and exit
END
)

UBUNTU_DISTRO=20.04

RUN_DEV=false
RUN_CI=false

HOST_UID=$(id -u)
HOST_GID=$(id -g)
DOCKER_GID=$(getent group docker | cut -d: -f3)

REBUILD=false

OPTS=`getopt -o h --long help,ubuntu-distro:,dev,ci,user:,docker-gid:,rebuild -n 'parse-options' -- "$@"`

eval set -- "$OPTS"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --ubuntu-distro )
      UBUNTU_DISTRO="$2";
      shift 2 ;;
    --dev )
      RUN_DEV=true
      shift ;;
    --ci )
      RUN_CI=true
      shift ;;
    --user )
      IFS=':' read -r HOST_UID HOST_GID <<< "$2"
      shift 2 ;;
    --docker-gid)
      DOCKER_GID="$2"
      shift 2 ;;
    --rebuild )
      REBUILD=true
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
CARLA_ROOT=${SCRIPT_DIR}../..

# Docker / NVIDIA checks
if ! command -v docker &> /dev/null; then
    echo "Docker is not installed. Please install Docker and try again."
    exit 1
fi
 
 if ! docker info | grep -q "Runtimes:.*nvidia"; then
    echo "[WARN] NVIDIA Container Toolkit is not installed or configured."
    echo "GPU acceleration may not work."
    echo "Refer to: https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/install-guide.html"
fi

# Ensure UE4_ROOT is defined
if [ -z "${UE4_ROOT}" ]; then
  echo "[ERROR] \$UE4_ROOT is not set. Please export UE4_ROOT=/absolute/path/to/UE4.26 first."
  echo "Example: export UE4_ROOT=\"/home/username/UnrealEngine_4.26\""
  exit 1
fi

# Start the Docker container
if ${RUN_DEV}; then
  if ${REBUILD}; then
    echo "Rebuilding development image..."
    ${SCRIPT_DIR}/build.sh --ubuntu-distro ${UBUNTU_DISTRO} --user ${HOST_UID}:${HOST_GID} --docker-gid ${DOCKER_GID} --dev
  fi

  echo "Running development container for ubuntu $UBUNTU_DISTRO with user ${HOST_UID}:${HOST_GID}"
  docker run \
    -it --rm \
    --name carla-development-ue4-${UBUNTU_DISTRO} \
    --workdir /workspaces/carla \
    --runtime=nvidia \
    --net=host \
    --env=NVIDIA_VISIBLE_DEVICES=all \
    --env=NVIDIA_DRIVER_CAPABILITIES=all \
    --env=UE4_ROOT=/workspaces/unreal-engine \
    --env=CARLA_UE4_ROOT=/workspaces/carla \
    --mount source=carla-development-ue4-${UBUNTU_DISTRO},target=/home/carla \
    --env=DISPLAY=${DISPLAY} \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    -v /var/run/docker.sock:/var/run/docker.sock \
    -v ${UE4_ROOT}:/workspaces/unreal-engine \
    -v ${SCRIPT_DIR}/../..:/workspaces/carla \
    carla-development:ue4-${UBUNTU_DISTRO} bash

elif ${RUN_CI} ; then
  if ${REBUILD}; then
    echo "Rebuilding CI/CD image..."
    ${SCRIPT_DIR}/build.sh --ubuntu-distro ${UBUNTU_DISTRO} --user ${HOST_UID}:${HOST_GID} --docker-gid ${DOCKER_GID} --ci
  fi

  echo "Running CI/CD container for ubuntu $UBUNTU_DISTRO with user ${HOST_UID}:${HOST_GID}"
  docker run \
    -it --rm \
    --name carla-builder-ue4-${UBUNTU_DISTRO} \
    --workdir /workspaces/carla \
    --net=host \
    --env=UE4_ROOT=/workspaces/unreal-engine \
    --env=CARLA_UE4_ROOT=/workspaces/carla \
    -v /var/run/docker.sock:/var/run/docker.sock \
    -v ${UE4_ROOT}:/workspaces/unreal-engine \
    -v ${SCRIPT_DIR}/../..:/workspaces/carla \
    carla-builder:ue4-${UBUNTU_DISTRO} bash

else
  echo "No run configuration selected. Use --dev or --ci."
fi
