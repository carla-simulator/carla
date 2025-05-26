#!/usr/bin/env bash
set -e

###############################################################################
# Usage:
#   ./run_container.sh [--monolith]
# 
# If --monolith is supplied, the script uses:
#   - Dockerfile:  carla-ue4.dockerfile
#   - Image Name:  carla-ue4-jammy-dev
#   - Container:   carla-ue4-jammy-devcontainer
#   - Hostname:    carla-ue4-devcontainer
#   - Extra volume for UE4 root
#
# Otherwise, it defaults to the lightweight version:
#   - Dockerfile:  carla.dockerfile
#   - Image Name:  carla-jammy-dev
#   - Container:   carla-jammy-devcontainer
#   - Hostname:    carla-devcontainer
#   - Mounts UE4_ROOT:/opt/UE4.26 (external UE4 path) for building CARLA without
#     baking UE4 into the container.
###############################################################################

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
PROJECT_NAME="$(basename "$PROJECT_DIR")"

# ------------------------------------------------------------------------------
# Parse Argument
# ------------------------------------------------------------------------------
MONOLITH=0
if [[ "$1" == "--monolith" || "$1" == "-m" ]]; then
    MONOLITH=1
    echo "[INFO] Monolithic mode enabled."
else
    echo "[INFO] Using default (lightweight) mode."
fi

# ------------------------------------------------------------------------------
# Select image, container names, and Dockerfile
# ------------------------------------------------------------------------------
if [ $MONOLITH -eq 1 ]; then
    DOCKERFILE="carla-ue4-monolith.dockerfile"
    IMAGE_NAME="carla-ue4-jammy-monolith"
    CONTAINER_NAME="carla-ue4-jammy-monolith"
    HOSTNAME="carla-ue4-monolith"
else
    DOCKERFILE="carla-ue4.dockerfile"
    IMAGE_NAME="carla-ue4-jammy-dev"
    CONTAINER_NAME="carla-ue4-jammy-devcontainer"
    HOSTNAME="carla-ue4-devcontainer"
fi

# ------------------------------------------------------------------------------
# Docker / NVIDIA checks
# ------------------------------------------------------------------------------
if ! command -v docker &> /dev/null; then
    echo "Docker is not installed. Please install Docker and try again."
    exit 1
fi
 
 if ! docker info | grep -q "Runtimes:.*nvidia"; then
    echo "[WARN] NVIDIA Container Toolkit is not installed or configured."
    echo "GPU acceleration may not work."
    echo "Refer to: https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/install-guide.html"
fi

# ------------------------------------------------------------------------------
# Build the Docker image (this calls our new combined build script)
# ------------------------------------------------------------------------------
echo "[INFO] Building Docker image: $IMAGE_NAME"
bash "$SCRIPT_DIR/build_image.sh" $([ $MONOLITH -eq 1 ]  && echo "--monolith")

# ------------------------------------------------------------------------------
# X11 / GPU Setup
# ------------------------------------------------------------------------------
DISPLAY="${DISPLAY:-:0}" 
X11_SOCKET="/tmp/.X11-unix"
NVIDIA_ICD="/usr/share/vulkan/icd.d/nvidia_icd.json"

# ------------------------------------------------------------------------------
# If using the LIGHTWEIGHT container, ensure UE4_ROOT is defined
# ------------------------------------------------------------------------------
if [ $MONOLITH -eq 0 ]; then
    if [ -z "${UE4_ROOT}" ]; then
        echo "[ERROR] \$UE4_ROOT is not set. Please export UE4_ROOT=/absolute/path/to/UE4.26 first."
        echo "Example: export UE4_ROOT=\"/home/username/UnrealEngine_4.26\""
        exit 1
    fi
fi

# ------------------------------------------------------------------------------
# Start the Docker container
# ------------------------------------------------------------------------------
echo "[INFO] Starting Docker container..."

docker run -it --rm \
    --name "${CONTAINER_NAME}" \
    --hostname "${HOSTNAME}" \
    --env "DISPLAY=${DISPLAY}" \
    --volume "${X11_SOCKET}:${X11_SOCKET}" \
    --volume "${NVIDIA_ICD}:${NVIDIA_ICD}" \
    --gpus all \
    $([ $MONOLITH -eq 0 ] && echo "--volume ${PROJECT_DIR}:/workspaces/${PROJECT_NAME}") \
    $([ $MONOLITH -eq 0 ] && echo "--volume ${UE4_ROOT}:/opt/UE4.26") \
    "${IMAGE_NAME}" \
    bash
