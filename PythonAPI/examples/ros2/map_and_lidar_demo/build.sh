#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EXAMPLES_DIR="$(dirname "${SCRIPT_DIR}")"

# --- Defaults ---
DISTRO="humble"
RMW="fastdds"
WHEEL=""

# --- Argument parsing ---
usage() {
    cat <<EOF
Usage: $0 [--distro=<distro>] [--rmw=<middleware>] [--wheel=<path>]

Builds the carla-map-and-lidar-demo-<distro>-<rmw> Docker image: the RViz image
extended with the carla Python wheel and the demo helpers (ros2_native.py,
map_to_markers.py, ego_tf_broadcaster.py) installed.

Options:
  --distro    ROS 2 distribution to use. Supported: humble, jazzy  (default: humble)
  --rmw       RMW implementation to use. Supported: fastdds, cyclonedds, zenoh  (default: fastdds)
  --wheel     Path to the carla wheel to install. If omitted, use the
              target-runtime wheel in Build/ros2-wheel-<distro>/PythonAPI/dist.

Examples:
  $0 --distro=humble --rmw=fastdds \\
      --wheel=/absolute/path/to/carla-0.10.0-cp310-cp310-linux_x86_64.whl
EOF
    exit 1
}

for arg in "$@"; do
    case "$arg" in
        --distro=*) DISTRO="${arg#*=}" ;;
        --rmw=*)    RMW="${arg#*=}" ;;
        --wheel=*)  WHEEL="${arg#*=}" ;;
        --help|-h)  usage ;;
        *) echo "Unknown argument: $arg"; usage ;;
    esac
done

# --- Validate ---
case "$DISTRO" in
    humble) PYTHON_TAG="cp310" ;;
    jazzy)  PYTHON_TAG="cp312" ;;
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

BASE_IMAGE="carla-rviz-${DISTRO}-${RMW}"
IMAGE_NAME="carla-map-and-lidar-demo-${DISTRO}-${RMW}"

# --- Locate the carla wheel ---
if [ -z "$WHEEL" ]; then
    # A wheel built on the host may require a newer glibc than the ROS image.
    # Only select the explicitly target-runtime build; Docker validates the
    # import below before it tags an image.
    shopt -s nullglob
    WHEELS=("${EXAMPLES_DIR}"/../../../Build/ros2-wheel-${DISTRO}/PythonAPI/dist/carla-*-${PYTHON_TAG}-*.whl)
    shopt -u nullglob
    if [ "${#WHEELS[@]}" -eq 1 ]; then
        WHEEL="${WHEELS[0]}"
    elif [ "${#WHEELS[@]}" -gt 1 ]; then
        echo "Found multiple target-runtime carla ${PYTHON_TAG} wheels; pass the intended one with --wheel."
        printf '  %s\n' "${WHEELS[@]}"
        exit 1
    fi
fi
if [ -z "$WHEEL" ] || [ ! -f "$WHEEL" ]; then
    echo "No target-runtime carla ${PYTHON_TAG} wheel found."
    echo "Pass a wheel built for the selected ROS image with --wheel=<path>."
    exit 1
fi

# Always invoke the build: Docker cache makes an unchanged build cheap, while
# source or Dockerfile changes must not silently reuse a stale demo image.
echo "[demo] Building base Docker image '${BASE_IMAGE}' (distro=${DISTRO}, rmw=${RMW})..."
docker build \
    --build-arg ROS_DISTRO="${DISTRO}" \
    --build-arg RMW_IMPLEMENTATION="${RMW_IMPLEMENTATION}" \
    --file "${EXAMPLES_DIR}/Dockerfile" \
    --tag "${BASE_IMAGE}" \
    "${EXAMPLES_DIR}"

# --- Build the demo image from a staged context ---
BUILD_DIR="$(mktemp -d)"
trap 'rm -rf "${BUILD_DIR}"' EXIT

cp "${SCRIPT_DIR}/Dockerfile" "${BUILD_DIR}/Dockerfile"
cp "${EXAMPLES_DIR}/ros2_native.py" \
   "${EXAMPLES_DIR}/stack.json" \
   "${SCRIPT_DIR}/map_to_markers.py" \
   "${SCRIPT_DIR}/ego_tf_broadcaster.py" \
   "${SCRIPT_DIR}/cleanup.py" \
   "${SCRIPT_DIR}/launcher.sh" \
   "${BUILD_DIR}/"
cp "$WHEEL" "${BUILD_DIR}/"

echo "[demo] Building Docker image '${IMAGE_NAME}' (wheel=$(basename "$WHEEL"))..."
docker build \
    --build-arg BASE_IMAGE="${BASE_IMAGE}" \
    --build-arg CARLA_VERSION="0.10.0" \
    --tag "${IMAGE_NAME}" \
    "${BUILD_DIR}"
