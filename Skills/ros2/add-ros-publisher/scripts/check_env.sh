#!/usr/bin/env bash
# Preflight for add-ros-publisher. Read-only, no sudo.
# Exits non-zero only on hard blockers.
# On-camera banner: every skill run announces itself, so a terminal recording
# shows which skill is doing the work rather than just its output. The name is
# taken from the directory so it cannot drift from the skill it belongs to.
printf '\n\033[1;36m>> using skill: %s\033[0m\n' \
  "$(basename "$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)")"

set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh" >/dev/null

rc=0
ok(){   echo "  PASS $*"; }
warn(){ echo "  WARN $*"; }
bad(){  echo "  FAIL $*"; rc=1; }

echo "== CARLA checkout =="
if [ -z "${CARLA_UE4_ROOT}" ]; then
  bad "CARLA_UE4_ROOT is unset — export it, or run from inside a carla checkout"
elif [ -d "${CARLA_ROS2_SRC}" ]; then
  ok "ros2 sources at ${CARLA_ROS2_SRC}"
else
  bad "no LibCarla/source/carla/ros2 under ${CARLA_UE4_ROOT} — this branch has no native ROS 2 interface"
fi

echo "== Layer files =="
for f in "${CARLA_ROS2_DISPATCH}" "${CARLA_ROS2_DISPATCH_H}" \
         "${CARLA_ROS2_PUBLISHERS}/BasePublisher.h" "${CARLA_ROS2_PUBLISHERS}/PublisherImpl.h" \
         "${CARLA_ROS2_SUBSCRIBERS}/BaseSubscriber.h" "${CARLA_ROS2_SUBSCRIBERS}/SubscriberImpl.h" \
         "${CARLA_ROS2_MIDDLEWARE}/MiddlewareFactory.h" \
         "${CARLA_ROS2_MIDDLEWARE}/IPublisherMiddleware.h" "${CARLA_ROS2_MIDDLEWARE}/ISubscriberMiddleware.h" \
         "${CARLA_ROS2_MIDDLEWARE}/PublisherQos.h"; do
  [ -f "${f}" ] && ok "$(basename "${f}")" || bad "$(basename "${f}") missing at ${f}"
done
[ -f "${CARLA_MIDDLEWARE_TEST}" ] && ok "test_ros2_middleware.cpp" \
  || warn "test_ros2_middleware.cpp missing — no middleware-level test to extend"

echo "== Existing publishers / subscribers =="
if [ -d "${CARLA_ROS2_PUBLISHERS}" ]; then
  ok "$(find "${CARLA_ROS2_PUBLISHERS}" -name 'Carla*Publisher.h' | wc -l) publisher(s), $(find "${CARLA_ROS2_SUBSCRIBERS}" -name '*Subscriber.h' 2>/dev/null | wc -l) subscriber(s)"
fi
# The CMake target globs sources, so a NEW .cpp needs no build-file edit — but a
# new subdirectory does. Say so before someone adds one.
if [ -f "${CARLA_UE4_ROOT}/LibCarla/cmake/ros2/CMakeLists.txt" ]; then
  ok "LibCarla/cmake/ros2/CMakeLists.txt present (globs *.cpp: new files in EXISTING dirs need no edit)"
else
  bad "LibCarla/cmake/ros2/CMakeLists.txt missing — cannot tell how sources are compiled"
fi

echo "== Plugin call sites (where data enters the publisher) =="
if [ -d "${CARLA_PLUGIN_SENSORS}" ]; then
  ok "UE4 sensor sources at ${CARLA_PLUGIN_SENSORS}"
  ok "$(grep -rl 'ProcessDataFrom' "${CARLA_PLUGIN_SENSORS}" 2>/dev/null | wc -l) sensor file(s) already call ProcessDataFrom*"
else
  warn "no Unreal/CarlaUE4/.../Sensor dir — a LibCarla-only change cannot be wired to real data"
fi

echo "== Build =="
command -v make >/dev/null && ok "make" || bad "make missing"
for d in fast-dds-install cyclone-dds-install zenoh-install; do
  [ -d "${CARLA_UE4_ROOT}/Build/${d}" ] && ok "Build/${d}" \
    || warn "Build/${d} missing — 'make LibCarla ARGS=--ros2' builds it from source first (long, one-off)"
done

echo "== Result =="
[ "$rc" -eq 0 ] && echo "  preflight OK (warnings are non-blocking)" || echo "  HARD BLOCKERS present — fix FAIL items."
exit $rc
