#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# --- Defaults ---
DISTRO="humble"

# --- Argument parsing ---
usage() {
    cat <<EOF
Usage: $0 [--distro=<distro>]

Options:
  --distro    ROS 2 distribution to use. Supported: humble, jazzy  (default: humble)

Examples:
  $0 --distro=humble
  $0 --distro=jazzy
EOF
    exit 1
}

for arg in "$@"; do
    case "$arg" in
        --distro=*) DISTRO="${arg#*=}" ;;
        --help|-h)  usage ;;
        *) echo "Unknown argument: $arg"; usage ;;
    esac
done

# --- Validate ---
case "$DISTRO" in
    humble|jazzy) ;;
    *) echo "Unsupported distro '${DISTRO}'. Supported values: humble, jazzy"; exit 1 ;;
esac

IMAGE_NAME="carla-rviz-${DISTRO}-fastdds"

# --- Build ---
function build_image() {
    echo "[RViz] Building Docker image '${IMAGE_NAME}' (distro=${DISTRO})..."
    docker build \
        --build-arg ROS_DISTRO="${DISTRO}" \
        --file "${SCRIPT_DIR}/Dockerfile" \
        --tag "${IMAGE_NAME}" \
        "${SCRIPT_DIR}"
}

if ! docker image inspect "${IMAGE_NAME}" &>/dev/null; then
    build_image
fi

# --- X11 permissions ---
XAUTH=/tmp/.docker.xauth
touch "$XAUTH"
xauth nlist "$DISPLAY" | sed -e 's/^..../ffff/' | xauth -f "$XAUTH" nmerge -

# --- Run ---
echo "[RViz] Launching RViz2 (distro=${DISTRO})..."
docker run \
    --rm \
    --net=host \
    --env="DISPLAY=$DISPLAY" \
    --env="XAUTHORITY=$XAUTH" \
    --env="RMW_IMPLEMENTATION=rmw_fastrtps_cpp" \
    --env="FASTRTPS_DEFAULT_PROFILES_FILE=/config/fastrtps-profile.xml" \
    --volume="${SCRIPT_DIR}/config:/config:ro" \
    --volume="${SCRIPT_DIR}/rviz:/rviz:rw" \
    --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" \
    --volume="$XAUTH:$XAUTH" \
    "${IMAGE_NAME}" \
    ros2 run rviz2 rviz2 -d /rviz/ros2_native.rviz
