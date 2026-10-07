#!/usr/bin/env bash
# Step 7 — build LibCarla with ROS 2 and run the LibCarla test suite, which is
# what actually validates a new message type: test_type_hash.cpp checks every
# registered hash for the RIHS01_<64 hex> format and for uniqueness across types.
#
# Usage:
#   bash verify.sh              # make LibCarla ARGS="--ros2" + make check.LibCarla
#   BUILD_ONLY=1 bash verify.sh # compile only (fast loop while the code is broken)
#
# First run also builds Fast-DDS / CycloneDDS / Zenoh from source (long, cached
# in Build/*-install).
set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh"

[ -d "${CARLA_ROS2_SRC}" ] \
  || { echo "[verify] ERROR: ${CARLA_ROS2_SRC} not found — wrong CARLA_UE4_ROOT." >&2; exit 1; }

cd "${CARLA_UE4_ROOT}"

echo "[verify] make LibCarla ARGS=\"--ros2\" ..."
make LibCarla ARGS="--ros2"

if [ "${BUILD_ONLY:-0}" = "1" ]; then
  echo "[verify] BUILD_ONLY=1 — skipping the test suite."
  exit 0
fi

# The tests link the ros2 sources, so they must be built with the same flag.
echo "[verify] make check.LibCarla ARGS=\"--ros2\" ..."
make check.LibCarla ARGS="--ros2"

echo "[verify] DONE — type_hash tests passed (format + uniqueness)."
echo "[verify] A passing suite does NOT prove wire compatibility: for that,"
echo "[verify] publish the type and echo it from a ROS 2 node (visualize-ros-rviz)."
