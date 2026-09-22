#!/bin/bash

# Build a CARLA Python wheel inside the same Ubuntu/Python generation as the
# ROS Docker runtime. A wheel built on a newer host can import locally yet
# require a newer glibc than the ROS image provides.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../../../.." && pwd)"

DISTRO="humble"
JOBS="$(nproc)"
UE_ROOT="${CARLA_UNREAL_ENGINE_PATH:-${UE_ROOT:-}}"

usage() {
    cat <<EOF
Usage: $0 [--distro=<humble|jazzy>] [--ue-root=<path>] [--jobs=<N>]

Builds the CARLA wheel in the matching development container and writes it to:
  Build/ros2-wheel-<distro>/PythonAPI/dist/

The wheel can then be used without an explicit --wheel argument by
run_map_and_lidar_demo.sh. --ue-root defaults to CARLA_UNREAL_ENGINE_PATH or
UE_ROOT and is mounted read-only into the container.
EOF
    exit 1
}

for arg in "$@"; do
    case "$arg" in
        --distro=*) DISTRO="${arg#*=}" ;;
        --ue-root=*) UE_ROOT="${arg#*=}" ;;
        --jobs=*) JOBS="${arg#*=}" ;;
        --help|-h) usage ;;
        *) echo "Unknown argument: $arg"; usage ;;
    esac
done

case "$DISTRO" in
    humble) IMAGE="carla-development:ue5-22.04"; PYTHON_TAG="cp310" ;;
    jazzy)  IMAGE="carla-development:ue5-24.04"; PYTHON_TAG="cp312" ;;
    *) echo "Unsupported distro '${DISTRO}'. Supported values: humble, jazzy"; exit 1 ;;
esac

if [ -z "$UE_ROOT" ] || [ ! -d "$UE_ROOT" ]; then
    echo "A valid Unreal Engine path is required. Pass --ue-root=<path> or export CARLA_UNREAL_ENGINE_PATH."
    exit 1
fi
if ! [[ "$JOBS" =~ ^[1-9][0-9]*$ ]]; then
    echo "Invalid --jobs value '${JOBS}'."
    exit 1
fi

BUILD_DIR="Build/ros2-wheel-${DISTRO}"
echo "[wheel] Building ${PYTHON_TAG} target-runtime wheel with ${IMAGE}..."
docker run --rm \
    --user "$(id -u):$(id -g)" \
    --volume "${REPO_ROOT}:/workspace" \
    --volume "${UE_ROOT}:/opt/UnrealEngine:ro" \
    --workdir /workspace \
    "${IMAGE}" \
    bash -lc "cmake -S /workspace -B /workspace/${BUILD_DIR} -G Ninja \\
      -DCMAKE_TOOLCHAIN_FILE=/workspace/CMake/Toolchain.cmake \\
      -DCMAKE_BUILD_TYPE=Release \\
      -DCARLA_UNREAL_ENGINE_PATH=/opt/UnrealEngine \\
      -DPython3_EXECUTABLE=/usr/bin/python3 \\
      -DBUILD_CARLA_SERVER=OFF \\
      -DBUILD_EXAMPLES=OFF \\
      -DBUILD_LIBCARLA_TESTS=OFF \\
      -DENABLE_ROS2=OFF && \\
      cmake --build /workspace/${BUILD_DIR} --target carla-python-api -- -j${JOBS}"

WHEELS=("${REPO_ROOT}/${BUILD_DIR}/PythonAPI/dist/carla-"*"-${PYTHON_TAG}-"*.whl)
if [ "${#WHEELS[@]}" -ne 1 ] || [ ! -f "${WHEELS[0]}" ]; then
    echo "Wheel build completed but did not produce exactly one ${PYTHON_TAG} wheel in ${BUILD_DIR}/PythonAPI/dist."
    exit 1
fi
echo "[wheel] Built ${WHEELS[0]}"
