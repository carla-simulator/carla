#!/usr/bin/env bash
# Preflight for add-ros-message-type. Read-only, no sudo.
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

echo "== Files this skill edits =="
for f in "${CARLA_CDR_SERIALIZATION}" "${CARLA_CDR_TOPIC_INFO}" "${CARLA_TYPE_HASH_TEST}"; do
  [ -f "${f}" ] && ok "$(basename "${f}")" || bad "$(basename "${f}") missing at ${f}"
done
[ -d "${CARLA_ROS2_MSG_DIR}" ] && ok "types/msg/ ($(find "${CARLA_ROS2_MSG_DIR}" -name '*.h' | wc -l) existing types)" \
  || bad "types/msg/ missing"

echo "== Hash tool (Step 4) =="
if [ -x "${CARLA_HASH_TOOL}" ]; then
  ok "Util/ros2/compute_type_hash.sh present"
elif [ -f "${CARLA_HASH_TOOL}" ]; then
  warn "compute_type_hash.sh is not executable — run it with 'bash <path>'"
else
  bad "Util/ros2/compute_type_hash.sh missing — the RIHS01 hash cannot be computed without it"
fi
# The hash tool builds the .msg inside osrf/ros:jazzy-desktop.
if command -v docker >/dev/null 2>&1; then
  docker info >/dev/null 2>&1 && ok "docker daemon reachable (needed for the hash only)" \
    || bad "docker daemon unreachable — Step 4 (hash) cannot run; the rest of the workflow can"
else
  bad "docker missing — Step 4 (RIHS01 hash) needs it; no local ROS 2 is required"
fi

echo "== Verify build (Step 7) =="
command -v make >/dev/null && ok "make" || bad "make missing"
# The ros2 test target needs the ROS 2 middleware deps built at least once.
for d in fast-dds-install cyclone-dds-install zenoh-install; do
  [ -d "${CARLA_UE4_ROOT}/Build/${d}" ] && ok "Build/${d}" \
    || warn "Build/${d} missing — 'make LibCarla ARGS=--ros2' builds it from source first (long, one-off)"
done

echo "== Result =="
[ "$rc" -eq 0 ] && echo "  preflight OK (warnings are non-blocking)" || echo "  HARD BLOCKERS present — fix FAIL items."
exit $rc
