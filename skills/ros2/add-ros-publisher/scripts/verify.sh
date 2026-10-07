#!/usr/bin/env bash
# Verify a new publisher/subscriber: build LibCarla with ROS 2 and run the
# LibCarla test suite (test_ros2_middleware.cpp exercises the middleware layer
# with a fake middleware; test_type_hash.cpp guards the type registrations).
#
# A publisher that only exists in LibCarla is NOT proven by this: the UE4 plugin
# call site is compiled by `make CarlaUE4Editor ARGS="--ros2"` (build-carla-ue4
# ROS2=1), and the topic is only proven by a subscriber (visualize-ros-rviz).
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

echo "[verify] DONE — LibCarla built with ROS 2 and its tests passed."
echo "[verify] Still to do for a publisher: rebuild the plugin that calls it"
echo "[verify]   (build-carla-ue4 ROS2=1, step 06) and confirm the topic on the"
echo "[verify]   wire (visualize-ros-rviz: topics / hz)."
