#!/bin/bash

set -e

DOC_STRING="Run docker containers.

This script launches development docker containers for CARLA UE5."

USAGE_STRING=$(cat <<- END
Usage: $0 [options]

Run configurations (choose one):

    --dev               Run the development docker container

User and group options:

    --user UID:GID      Set host UID and GID for the container (default: current user)
    --docker-gid GID    Set GID of the Docker group (default: $(getent group docker | cut -d: -f3))

Ubuntu distribution:

    --ubuntu-distro DISTRO   Specify ubuntu distro (default: 22.04)

Build options:

    --rebuild           Rebuild the docker image before running

Other commands:

    -h, --help          Show this help message and exit
END
)

UBUNTU_DISTRO=22.04

RUN_DEV=false

HOST_UID=$(id -u)
HOST_GID=$(id -g)
DOCKER_GID=$(getent group docker | cut -d: -f3)

REBUILD=false

OPTS=`getopt -o h --long help,ubuntu-distro:,dev,user:,docker-gid:,rebuild -n 'parse-options' -- "$@"`

eval set -- "$OPTS"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --ubuntu-distro )
      UBUNTU_DISTRO="$2";
      shift 2 ;;
    --dev )
      RUN_DEV=true
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
CARLA_ROOT=${SCRIPT_DIR}/../..

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

# Ensure CARLA_UNREAL_ENGINE_PATH is defined
if [ -z "${CARLA_UNREAL_ENGINE_PATH}" ]; then
  echo "[ERROR] \$CARLA_UNREAL_ENGINE_PATH is not set."
  echo "Please export CARLA_UNREAL_ENGINE_PATH=/absolute/path/to/UnrealEngine5_carla first."
  echo "Example: export CARLA_UNREAL_ENGINE_PATH=\"/home/username/UnrealEngine5_carla\""
  exit 1
fi

# Start the Docker container
if ${RUN_DEV}; then
  if ${REBUILD}; then
    echo "Rebuilding development image..."
    ${SCRIPT_DIR}/build.sh --ubuntu-distro ${UBUNTU_DISTRO} --user ${HOST_UID}:${HOST_GID} --docker-gid ${DOCKER_GID}
  fi

  echo "Running development container for ubuntu ${UBUNTU_DISTRO} with user ${HOST_UID}:${HOST_GID}"
  docker run \
    -it --rm \
    --name carla-development-ue5-${UBUNTU_DISTRO} \
    --workdir /workspaces/carla \
    --runtime=nvidia \
    --net=host \
    --env=NVIDIA_VISIBLE_DEVICES=all \
    --env=NVIDIA_DRIVER_CAPABILITIES=all \
    --env=CARLA_UNREAL_ENGINE_PATH=/workspaces/unreal-engine \
    --env=DISPLAY=${DISPLAY} \
    --mount source=carla-development-ue5-${UBUNTU_DISTRO},target=/home/carla \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    -v /var/run/docker.sock:/var/run/docker.sock \
    -v ${CARLA_UNREAL_ENGINE_PATH}:/workspaces/unreal-engine \
    -v ${CARLA_ROOT}:/workspaces/carla \
    carla-development:ue5-${UBUNTU_DISTRO} bash

else
  echo "No run configuration selected. Use --dev."
  echo ""
  echo "$USAGE_STRING"
fi
