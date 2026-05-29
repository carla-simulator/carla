#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# --- Defaults ---
DISTRO="humble"
RMW="fastdds"

# --- Argument parsing ---
usage() {
    cat <<EOF
Usage: $0 [--distro=<distro>] [--rmw=<middleware>]

Options:
  --distro    ROS 2 distribution to use. Supported: humble, jazzy  (default: humble)
  --rmw       RMW implementation to use. Supported: fastdds, cyclonedds, zenoh  (default: fastdds)

Examples:
  $0 --distro=humble --rmw=fastdds
  $0 --distro=jazzy  --rmw=cyclonedds
  $0 --distro=jazzy  --rmw=zenoh
EOF
    exit 1
}

for arg in "$@"; do
    case "$arg" in
        --distro=*) DISTRO="${arg#*=}" ;;
        --rmw=*)    RMW="${arg#*=}" ;;
        --help|-h)  usage ;;
        *) echo "Unknown argument: $arg"; usage ;;
    esac
done

# --- Validate ---
case "$DISTRO" in
    humble|jazzy) ;;
    *) echo "Unsupported distro '${DISTRO}'. Supported values: humble, jazzy"; exit 1 ;;
esac

case "$RMW" in
    fastdds|cyclonedds|zenoh) ;;
    *) echo "Unsupported RMW '${RMW}'. Supported values: fastdds, cyclonedds, zenoh"; exit 1 ;;
esac

# Map short names to ROS RMW implementation identifiers
if [ "$RMW" = "cyclonedds" ]; then
    RMW_IMPLEMENTATION="rmw_cyclonedds_cpp"
elif [ "$RMW" = "zenoh" ]; then
    RMW_IMPLEMENTATION="rmw_zenoh_cpp"
else
    RMW_IMPLEMENTATION="rmw_fastrtps_cpp"
fi

IMAGE_NAME="carla-rviz-${DISTRO}-${RMW}"

# --- Build ---
function build_image() {
    echo "[RViz] Building Docker image '${IMAGE_NAME}' (distro=${DISTRO}, rmw=${RMW})..."
    docker build \
        --build-arg ROS_DISTRO="${DISTRO}" \
        --build-arg RMW_IMPLEMENTATION="${RMW_IMPLEMENTATION}" \
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

# --- RMW-specific environment variables ---
EXTRA_ENV=()
if [ "$RMW" = "cyclonedds" ]; then
    EXTRA_ENV+=(--env="CYCLONEDDS_URI=/config/cyclonedds.xml")
elif [ "$RMW" = "fastdds" ]; then
    EXTRA_ENV+=(--env="FASTRTPS_DEFAULT_PROFILES_FILE=/config/fastrtps-profile.xml")
fi

# --- Run ---
echo "[RViz] Launching RViz2 (distro=${DISTRO}, rmw=${RMW})..."
docker run \
    --rm \
    --net=host \
    --env="DISPLAY=$DISPLAY" \
    --env="XAUTHORITY=$XAUTH" \
    --env="RMW_IMPLEMENTATION=${RMW_IMPLEMENTATION}" \
    "${EXTRA_ENV[@]}" \
    --volume="${SCRIPT_DIR}/config:/config:ro" \
    --volume="${SCRIPT_DIR}/rviz:/rviz:rw" \
    --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" \
    --volume="$XAUTH:$XAUTH" \
    "${IMAGE_NAME}" \
    ros2 run rviz2 rviz2 -d /rviz/ros2_native.rviz
