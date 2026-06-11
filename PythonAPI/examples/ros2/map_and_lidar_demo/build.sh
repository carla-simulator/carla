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
  --wheel     Path to the carla wheel to install (default: autodetect in PythonAPI/carla/dist)

Examples:
  $0 --distro=humble --rmw=fastdds
  $0 --distro=jazzy  --rmw=cyclonedds --wheel=/path/to/carla-0.9.16-cp312-cp312-manylinux_2_31_x86_64.whl
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
    WHEEL="$(ls "${EXAMPLES_DIR}"/../../carla/dist/carla-*-${PYTHON_TAG}-*.whl 2>/dev/null | head -n 1 || true)"
fi
if [ -z "$WHEEL" ] || [ ! -f "$WHEEL" ]; then
    echo "No carla ${PYTHON_TAG} wheel found in '${EXAMPLES_DIR}/../../carla/dist'."
    echo "Build it with 'make PythonAPI' or pass one explicitly with --wheel=<path>."
    exit 1
fi

# --- Build the base RViz image if missing ---
if ! docker image inspect "${BASE_IMAGE}" &>/dev/null; then
    echo "[demo] Building base Docker image '${BASE_IMAGE}' (distro=${DISTRO}, rmw=${RMW})..."
    docker build \
        --build-arg ROS_DISTRO="${DISTRO}" \
        --build-arg RMW_IMPLEMENTATION="${RMW_IMPLEMENTATION}" \
        --file "${EXAMPLES_DIR}/Dockerfile" \
        --tag "${BASE_IMAGE}" \
        "${EXAMPLES_DIR}"
fi

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
    --tag "${IMAGE_NAME}" \
    "${BUILD_DIR}"
