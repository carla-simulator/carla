#!/usr/bin/env bash
# Preflight for visualize-ros-rviz. Read-only, no sudo.
# Exits non-zero only on hard blockers; WARN means a later step handles it.
# Run by the MCP check_prerequisites(name) tool.
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

echo "== CARLA checkout / demo files =="
if [ -z "${CARLA_UE4_ROOT}" ]; then
  bad "CARLA_UE4_ROOT is unset — export it, or run from inside a carla checkout"
elif [ -d "${CARLA_ROS2_EXAMPLES}" ]; then
  ok "demo dir: ${CARLA_ROS2_EXAMPLES}"
  for f in run_rviz.sh run_map_and_lidar_demo.sh rviz/ros2_native.rviz stack.json; do
    [ -e "${CARLA_ROS2_EXAMPLES}/${f}" ] && ok "  ${f}" \
      || bad "  ${f} missing — this checkout predates the native ROS 2 demo"
  done
else
  bad "no PythonAPI/examples/ros2 under ${CARLA_UE4_ROOT} — checkout too old, or CARLA_UE4_ROOT is wrong"
fi

echo "== Docker =="
if command -v docker >/dev/null 2>&1; then
  ok "docker CLI present"
  # A CLI without a reachable daemon fails every step here, so it is a blocker.
  if docker info >/dev/null 2>&1; then
    ok "docker daemon reachable"
  else
    bad "docker daemon not reachable (permission or not running) — every mode here needs it"
  fi
else
  bad "docker missing — the ROS 2 tooling runs only in containers here (no local ROS 2 is used)"
fi

echo "== Images (built on first use) =="
for img in "${CARLA_RVIZ_IMAGE}" "${CARLA_DEMO_IMAGE}"; do
  if docker image inspect "${img}" >/dev/null 2>&1; then
    ok "${img} present"
  else
    warn "${img} not built yet — the first run builds it (minutes, needs network)"
  fi
done

echo "== carla wheel for the demo image (${CARLA_WHEEL_TAG:-?}) =="
# Only the demo/stack image needs the wheel; plain topic inspection and RViz do not.
case "${ROS_DISTRO_TAG}" in
  humble|jazzy) ;;
  *) bad "ROS_DISTRO_TAG='${ROS_DISTRO_TAG}' unsupported (humble | jazzy)";;
esac
if docker image inspect "${CARLA_DEMO_IMAGE}" >/dev/null 2>&1; then
  ok "demo image already built — the wheel is baked in"
elif [ -n "${CARLA_WHEEL_TAG}" ] && \
     compgen -G "${CARLA_UE4_ROOT}/PythonAPI/carla/dist/carla-*-${CARLA_WHEEL_TAG}-*.whl" >/dev/null; then
  ok "wheel found: $(basename "$(ls -1 "${CARLA_UE4_ROOT}"/PythonAPI/carla/dist/carla-*-"${CARLA_WHEEL_TAG}"-*.whl | head -1)")"
else
  warn "no ${CARLA_WHEEL_TAG:-?} wheel in PythonAPI/carla/dist — needed only to BUILD the demo image (make PythonAPI, or pass --wheel)"
fi

echo "== Middleware =="
case "${RMW}" in
  fastdds|cyclonedds|zenoh) ok "RMW=${RMW} -> ${RMW_IMPLEMENTATION}";;
  *) bad "RMW='${RMW}' unsupported (fastdds | cyclonedds | zenoh)";;
esac
if [ "${RMW}" = "zenoh" ]; then
  pgrep -f rmw_zenohd >/dev/null 2>&1 && ok "Zenoh router (rmw_zenohd) running" \
    || warn "no rmw_zenohd — zenoh needs a router on the host BEFORE server and containers"
fi

echo "== Server + domain =="
if command -v nc >/dev/null 2>&1 && nc -z "${CARLA_HOST}" "${CARLA_PORT}" 2>/dev/null; then
  ok "CARLA RPC reachable at ${CARLA_HOST}:${CARLA_PORT}"
else
  warn "no CARLA server at ${CARLA_HOST}:${CARLA_PORT} — start one with --ros2 (run-carla-server ROS2=1)"
fi
if [ -n "${ROS_DOMAIN_ID:-}" ]; then
  if [ "${ROS_DOMAIN_ID}" -ge 0 ] 2>/dev/null && [ "${ROS_DOMAIN_ID}" -le 232 ] 2>/dev/null; then
    ok "ROS_DOMAIN_ID=${ROS_DOMAIN_ID} — the server must use the same"
  else
    bad "ROS_DOMAIN_ID='${ROS_DOMAIN_ID}' out of range 0..232"
  fi
else
  ok "ROS_DOMAIN_ID unset — default domain 0 on both sides"
fi

echo "== RViz display =="
[ -n "${DISPLAY:-}" ] && ok "DISPLAY=${DISPLAY}" \
  || warn "DISPLAY unset — the rviz mode needs an X display; topics/echo/demo work headless"
command -v xauth >/dev/null 2>&1 && ok "xauth present (X11 cookie for the container)" \
  || warn "xauth missing — the in-checkout run_rviz.sh uses it to share the display"

echo "== Result =="
[ "$rc" -eq 0 ] && echo "  preflight OK (warnings are non-blocking)" || echo "  HARD BLOCKERS present — fix FAIL items."
exit $rc
