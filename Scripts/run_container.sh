#!/bin/bash

# Set Docker image
IMAGE="carla-0.9.15.2-jammy-dev"

# Set container name and hostname
CONTAINER_NAME=""carla-0.9.15.2-jammy-devcontainer""
HOSTNAME="carla-devcontainer"

# Get the username dynamically based on whoami
USERNAME="$(whoami)"

# Define DISPLAY and X11 socket for GUI forwarding
DISPLAY="$DISPLAY"
X11_SOCKET="/tmp/.X11-unix"

# NVIDIA ICD Config file
NVIDIA_ICD="/usr/share/vulkan/icd.d/nvidia_icd.json"

# Change to the script's directory
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Get the absolute path of the project directory (one level above the script)
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

# Target directory inside the container
# NOTE: This directory already has host credentials and helps to avoid deleting .bashrc
TARGET_DIR="/workspace"

# Check if Docker is installed
if ! command -v docker &> /dev/null; then
    echo "Docker is not installed. Please install Docker and try again."
    exit 1
fi

# Check if NVIDIA Container Toolkit is installed (optional for GPU support)
if ! docker info | grep -q "Runtimes:.*nvidia"; then
    echo "NVIDIA Container Toolkit is not installed or configured. GPU acceleration may not work."
    echo "To enable GPU support, install NVIDIA Container Toolkit: https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/install-guide.html"
fi

# Build the Docker image (equivalent to "initializeCommand" in devcontainer.json)
if [ -f "$SCRIPT_DIR/build_image.sh" ]; then
    echo "Building Docker image..."
    bash "$SCRIPT_DIR/build_image.sh" || {
        echo "Failed to build Docker image. Exiting."
        exit 1
    }
else
    echo "Initialization script '$SCRIPT_DIR/build_image.sh' not found. Skipping image build."
fi

# Run the Docker container
echo "Starting Docker container..."
docker run -it --rm \
    --name "$CONTAINER_NAME" \
    --hostname "$HOSTNAME" \
    --env "DISPLAY=$DISPLAY" \
    --volume "$X11_SOCKET:$X11_SOCKET" \
    --volume "$PROJECT_DIR:$TARGET_DIR" \
    --volume "$NVIDIA_ICD:$NVIDIA_ICD" \
    --volume "$UE4_ROOT:/opt/UE4.26" \
    --gpus all \
    "$IMAGE" bash || {
        echo "Failed to start Docker container. Exiting."
        exit 1
    }