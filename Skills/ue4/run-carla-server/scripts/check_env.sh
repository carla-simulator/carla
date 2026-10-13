#!/usr/bin/env bash
# Preflight for run-carla-server. Read-only; exits non-zero only on hard
# blockers. Run by the MCP check_prerequisites(name) tool.
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

echo "== What will be run (detected) =="
read -r MODE LAUNCH <<<"$(carla_detect_target)"
case "${MODE}" in
  package) ok "extracted release -> ${LAUNCH} (no UE4 needed, renders for real)";;
  dist)    ok "cooked package in a checkout -> ${LAUNCH} (no UE4 needed, renders for real)";;
  editor)  ok "source checkout -> ${LAUNCH} (launches UE4Editor; needs UE4_ROOT)";;
  invalid) bad "${LAUNCH} was named explicitly (CARLA_TARGET/CARLA_PACKAGE_ROOT) but holds no CARLA build";;
  none)    bad "nothing runnable found — set CARLA_TARGET to an extracted release or a checkout";;
esac

# Only the editor path needs the engine and the raw content tree; a cooked build
# carries everything it needs, which is why these are checked per-mode.
if [ "${MODE}" = "editor" ]; then
  echo "== Editor requirements =="
  [ -x "${UE4_ROOT}/Engine/Binaries/Linux/UE4Editor" ] \
    && ok "UE4Editor built" || bad "UE4Editor missing — run build-carla-ue4 step 03"
  CONTENT="${LAUNCH}/Unreal/CarlaUE4/Content/Carla"
  # -L: Content/Carla may be a symlink to a shared content checkout.
  if [ -d "${CONTENT}/.git" ] && [ -n "$(find -L "${CONTENT}" -mindepth 1 -maxdepth 1 ! -name '.git' -print -quit 2>/dev/null)" ]; then
    ok "Content/Carla populated (maps available)"
  else
    bad "Content/Carla missing/incomplete — run build-carla-ue4 step 05"
  fi
  PKG="$(ls -1dt "${LAUNCH}"/Dist/CARLA_*/LinuxNoEditor 2>/dev/null | head -1)"
  [ -n "${PKG}" ] && ok "a cooked package also exists: ${PKG}" \
    || warn "no cooked package in this checkout — camera/lidar images need one (package-carla-ue4), or use WINDOW=1"
fi

echo "== ROS 2 native interface (ROS2=${ROS2}) =="
if [ "${ROS2}" != "1" ]; then
  ok "ROS 2 off (set ROS2=1 to publish DDS topics; needs a ROS-2-built binary)"
else
  case "$(carla_ros2_ini_state)" in
    on)  ok "checkout built with Ros2 ON — uncooked modes will publish";;
    off) bad "checkout built with Ros2 OFF — --ros2 does nothing; rebuild with build-carla-ue4 ROS2=1";;
    absent) warn "OptionalModules.ini absent — cannot tell if ROS 2 is compiled in (a Dist/ package carries its own state)";;
  esac
  case "${RMW}" in
    ""|fastdds|cyclonedds|zenoh) ok "RMW=${RMW:-<server default: fastdds>}";;
    *) bad "RMW='${RMW}' is not a valid middleware (fastdds | cyclonedds | zenoh) — the server DISABLES ROS 2 on an unknown value";;
  esac
  # The domain must match on the subscriber side or topics are invisible.
  if [ -n "${ROS_DOMAIN_ID:-}" ]; then
    if [ "${ROS_DOMAIN_ID}" -ge 0 ] 2>/dev/null && [ "${ROS_DOMAIN_ID}" -le 232 ] 2>/dev/null; then
      ok "ROS_DOMAIN_ID=${ROS_DOMAIN_ID} (subscribers must use the same)"
    else
      bad "ROS_DOMAIN_ID='${ROS_DOMAIN_ID}' out of range 0..232 — the server logs an error and falls back to the default domain"
    fi
  else
    ok "ROS_DOMAIN_ID unset — default domain 0 on both sides"
  fi
  if [ "${RMW}" = "zenoh" ]; then
    pgrep -f rmw_zenohd >/dev/null 2>&1 && ok "Zenoh router (rmw_zenohd) running" \
      || warn "no rmw_zenohd process — start the Zenoh router before the server (references/ros2.md)"
  fi
  # Consuming the topics needs ROS 2 or the demo images; the server itself does not.
  command -v ros2 >/dev/null 2>&1 && ok "ros2 CLI available for verification" \
    || warn "no ros2 CLI here — verify topics from a ROS 2 container (visualize-ros-rviz)"
fi

echo "== Ports / display =="
if command -v nc >/dev/null 2>&1 && nc -z 127.0.0.1 2000 2>/dev/null; then
  warn "port 2000 is in use — ANOTHER CARLA SERVER IS ALREADY RUNNING"
  warn "  either stop it:  pkill -x UE4Editor  (editor)  /  pkill -x CarlaUE4-Linux-  (packaged)"
  warn "                   then: until ! nc -z 127.0.0.1 2000; do sleep 1; done"
  warn "  or run on other ports:  bash scripts/run_server.sh <MAP> 3000  (+ CARLA_PORT=3000 for clients)"
  warn "  launching anyway fails with 'bind: Address already in use' then Signal 11 — run_server.sh refuses first"
else
  ok "default RPC port 2000 free"
fi
[ -n "${DISPLAY:-}" ] && ok "DISPLAY=${DISPLAY} (WINDOW=1 usable)" \
  || warn "DISPLAY unset — WINDOW=1 will default to :1; headless modes unaffected"

echo "== Result =="
[ "$rc" -eq 0 ] && echo "  preflight OK (warnings are non-blocking)" || echo "  HARD BLOCKERS present — fix FAIL items."
exit $rc
