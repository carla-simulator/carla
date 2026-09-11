#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# --- Defaults ---
DISTRO="humble"
RMW="fastdds"
# Holds the value of --ros-domain-id. Named differently from ROS_DOMAIN_ID so it
# does not clobber the inherited environment variable; it defaults to that env
# var so exporting ROS_DOMAIN_ID works without the flag, and --ros-domain-id
# overrides it. The value is then forwarded into the container.
ROS_DOMAIN_ID_ARG="${ROS_DOMAIN_ID:-}"

# --- Argument parsing ---
usage() {
    cat <<EOF
Usage: $0 [--distro=<distro>] [--rmw=<middleware>] [--ros-domain-id=<N>]

Options:
  --distro          ROS 2 distribution to use. Supported: humble, jazzy  (default: humble)
  --rmw             Middleware to use. Supported: fastdds, cyclonedds, zenoh  (default: fastdds)
  --ros-domain-id   ROS 2 domain id (0-232). Must match the CARLA server's
                    --ros-domain-id. When omitted, the default domain is used.

Examples:
  $0 --distro=humble --rmw=fastdds
  $0 --distro=jazzy  --rmw=cyclonedds
  $0 --distro=humble --rmw=zenoh
  $0 --ros-domain-id=42

Note: zenoh requires a Zenoh router running on the host first, e.g.
  docker run --rm --net=host carla-rviz-${DISTRO:-humble}-zenoh ros2 run rmw_zenoh_cpp rmw_zenohd
EOF
    exit 1
}

for arg in "$@"; do
    case "$arg" in
        --distro=*)         DISTRO="${arg#*=}" ;;
        --rmw=*)            RMW="${arg#*=}" ;;
        --ros-domain-id=*)  ROS_DOMAIN_ID_ARG="${arg#*=}" ;;
        --help|-h)          usage ;;
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

if [ -n "${ROS_DOMAIN_ID_ARG}" ]; then
    if ! [[ "${ROS_DOMAIN_ID_ARG}" =~ ^[0-9]+$ ]] || [ "${ROS_DOMAIN_ID_ARG}" -lt 0 ] || [ "${ROS_DOMAIN_ID_ARG}" -gt 232 ]; then
        echo "Invalid ROS domain id '${ROS_DOMAIN_ID_ARG}'. Must be an integer in the range 0-232."
        exit 1
    fi
fi

# Map short names to ROS RMW implementation identifiers
case "$RMW" in
    cyclonedds) RMW_IMPLEMENTATION="rmw_cyclonedds_cpp" ;;
    zenoh)      RMW_IMPLEMENTATION="rmw_zenoh_cpp" ;;
    *)          RMW_IMPLEMENTATION="rmw_fastrtps_cpp" ;;
esac

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
# zenoh needs no profile file; the rmw_zenoh peer connects to the router
# (rmw_zenohd) on localhost:7447 using its own built-in defaults.
EXTRA_ENV=()
case "$RMW" in
    cyclonedds) EXTRA_ENV+=(--env="CYCLONEDDS_URI=/config/cyclonedds.xml") ;;
    fastdds)    EXTRA_ENV+=(--env="FASTRTPS_DEFAULT_PROFILES_FILE=/config/fastrtps-profile.xml") ;;
esac

# --- ROS domain id ---
# Forward ROS_DOMAIN_ID so the container discovers a CARLA server launched with
# the matching --ros-domain-id. When unset, the container uses the default domain.
if [ -n "${ROS_DOMAIN_ID_ARG}" ]; then
    EXTRA_ENV+=(--env="ROS_DOMAIN_ID=${ROS_DOMAIN_ID_ARG}")
fi

# --- Run ---
echo "[RViz] Launching RViz2 (distro=${DISTRO}, rmw=${RMW}, ros-domain-id=${ROS_DOMAIN_ID_ARG:-default})..."
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
