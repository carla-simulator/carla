#!/usr/bin/env bash
# Launch a CARLA RPC server on this host. You point it at ONE path and it detects
# what that is (see carla_detect_target in env.sh):
#
#   package   <path>/CarlaUE4.sh — a downloaded/extracted release. Renders for
#             real, needs no UE4 and no content checkout. Headless via
#             -RenderOffScreen, or WINDOW=1 for a window.
#   dist      <path>/Dist/CARLA_*/LinuxNoEditor/CarlaUE4.sh — the same thing,
#             cooked inside a checkout by [[package-carla-ue4]].
#   editor    <path>/Unreal/CarlaUE4/CarlaUE4.uproject — a source checkout, run
#             through UE4Editor (needs UE4_ROOT + fetched content). Defaults to
#             -nullrhi, which has NO sensor images: uncooked meshes have null
#             distance fields and the real renderer SIGSEGVs on them
#             (build-carla-ue4 L17). WINDOW=1 renders with DF generation off.
#
# Where it looks:  CARLA_TARGET > CARLA_PACKAGE_ROOT > CARLA_UE4_ROOT > $PWD.
# An explicitly named path holding no CARLA is an error, not a fallback.
#   RUN_MODE=auto|package|editor   force the choice (PACKAGED=1 == package)
#   DETECT=1                       report what would run, launch nothing
#
# Usage:
#   bash run_server.sh [MAP] [RPC_PORT]
#     MAP       default /Game/Carla/Maps/Town02 (light map = fast first load;
#               uncooked modes only — the packaged build boots its cooked
#               default map; switch maps via client.load_world instead)
#     RPC_PORT  default 2000 (streaming port = RPC+1)
#     RESX/RESY window size for WINDOW=1 (default 1280x720)
#
# ROS 2 (opt-in, composes with every mode above — see references/ros2.md):
#   ROS2=1          add --ros2: the server publishes DDS topics itself. Needs a
#                   binary BUILT with ROS 2 (build-carla-ue4 ROS2=1) — on a plain
#                   build the flag is accepted and does nothing.
#   RMW=            fastdds (default) | cyclonedds | zenoh -> --rmw=; an unknown
#                   or not-compiled-in value makes the server DISABLE ROS 2.
#                   zenoh also needs a running rmw_zenohd router.
#   ROS_DOMAIN_ID=  0..232 -> --ros-domain-id=; must match the subscriber side.
#
# Runs in the FOREGROUND (blocks). Background it DETACHED from the caller:
#   setsid nohup bash run_server.sh </dev/null >server.log 2>&1 &
# The </dev/null and setsid matter: a plain `... &` leaves the server holding the
# launching shell's stdin, and when that shell exits the server dies with
# "LowLevelFatalError ... close: Bad file descriptor" then Signal 11 — which looks
# exactly like a rendering crash and is not one (verified).
# Wait for readiness by polling the RPC port, not by sleeping:
#   until nc -z 127.0.0.1 2000; do sleep 1; done
# Stop with:
#   pkill -x UE4Editor        # uncooked modes
#   pkill -x CarlaUE4-Linux-           # packaged mode (comm is TRUNCATED to 15
#                                      # chars; -x ...-Shipping matches NOTHING)
# NEVER `pkill -f CarlaUE4.uproject` — it matches (and kills) the calling
# shell itself (exit 144; see ue4-editor-python P6).
set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${HERE}/env.sh"

MAP="${1:-/Game/Carla/Maps/Town02}"
RPC_PORT="${2:-2000}"
STREAM_PORT="$((RPC_PORT + 1))"

# --- ROS 2 (opt-in) ---------------------------------------------------------
# ROS2=1 appends --ros2 [--rmw=..] [--ros-domain-id=..] to whichever binary this
# script launches, in every mode. Requires a ROS-2-BUILT binary; the flag is
# silently ignored otherwise, which is why the state is reported here.
# Word-splitting of the flag string is intended (it is a flag list, not a path).
# shellcheck disable=SC2206
ROS2_FLAGS=($(carla_ros2_flags))
if [ "${ROS2:-0}" = "1" ]; then
  echo "[server] ROS2=1 -> ${ROS2_FLAGS[*]}"
  if [ "$(carla_detect_target | cut -d" " -f1)" = "editor" ]; then
    # Source checkout only: the ini is authoritative for what got compiled in.
    # A cooked build carries whatever it was cooked with, so the ini says nothing.
    case "$(carla_ros2_ini_state)" in
      on) : ;;
      off|absent)
        echo "[server] WARN: this checkout was last built with Ros2 $(carla_ros2_ini_state)."
        echo "[server]       --ros2 will be accepted and do NOTHING: no topics will appear."
        echo "[server]       Rebuild with build-carla-ue4 ROS2=1 first."
        ;;
    esac
  fi
  if [ "${RMW}" = "zenoh" ]; then
    echo "[server] NOTE: RMW=zenoh needs a Zenoh router running (rmw_zenohd) —"
    echo "[server]       start it BEFORE the server; see references/ros2.md."
  fi
  # A domain mismatch is the most common "no topics" cause and is invisible in
  # the log, so state the effective domain the subscriber side must match.
  echo "[server] ROS domain: ${ROS_DOMAIN_ID:-<unset -> server uses \$ROS_DOMAIN_ID in its own env, else 0>}"
fi

# --- What to run -------------------------------------------------------------
# Detected, not assumed: the user names one path (CARLA_TARGET /
# CARLA_PACKAGE_ROOT / CARLA_UE4_ROOT) and env.sh works out whether it is an
# extracted release, a checkout that cooked one, or a source checkout.
# RUN_MODE forces the choice when a checkout offers both; PACKAGED=1 is kept as
# the old spelling of RUN_MODE=package.
RUN_MODE="${RUN_MODE:-auto}"
[ "${PACKAGED:-0}" = "1" ] && RUN_MODE="package"
case "${RUN_MODE}" in
  auto|package|editor) ;;
  *) echo "[server] ERROR: RUN_MODE must be auto, package or editor (got '${RUN_MODE}')." >&2; exit 2;;
esac
read -r MODE LAUNCH <<<"$(carla_detect_target "${RUN_MODE}")"

if [ "${MODE}" = "invalid" ]; then
  echo "[server] ERROR: ${LAUNCH} was named explicitly but holds no CARLA build." >&2
  echo "[server]        Expected CarlaUE4.sh, Dist/CARLA_*/LinuxNoEditor/CarlaUE4.sh," >&2
  echo "[server]        or Unreal/CarlaUE4/CarlaUE4.uproject under it." >&2
  exit 1
fi

if [ "${MODE}" = "none" ]; then
  [ "${RUN_MODE}" = "auto" ] || echo "[server] ERROR: RUN_MODE=${RUN_MODE} found no matching build." >&2
  echo "[server] ERROR: nothing runnable found. Looked for, in order:" >&2
  echo "[server]   \$CARLA_TARGET / \$CARLA_PACKAGE_ROOT / \$CARLA_UE4_ROOT / \$PWD" >&2
  echo "[server]   as  <path>/CarlaUE4.sh            (extracted release)" >&2
  echo "[server]   or  <path>/Dist/CARLA_*/LinuxNoEditor/CarlaUE4.sh  (cooked in a checkout)" >&2
  echo "[server]   or  <path>/Unreal/CarlaUE4/CarlaUE4.uproject       (source checkout)" >&2
  exit 1
fi

# --- Is the port already taken? ---------------------------------------------
# A busy RPC port means ANOTHER CARLA SERVER IS ALREADY RUNNING (or something else
# holds it). Launching anyway is the worst outcome: the new server dies with
# "bind: Address already in use" followed by "Signal 11", which reads like a
# rendering crash and sends you debugging the wrong thing entirely (verified).
# So refuse up front and state both ways out — which one is right is the user's
# call, not ours.
port_busy() {
  if command -v nc >/dev/null 2>&1; then nc -z 127.0.0.1 "$1" 2>/dev/null; return $?; fi
  if command -v ss >/dev/null 2>&1; then ss -ltn 2>/dev/null | grep -q ":$1 "; return $?; fi
  return 1   # no probe available: let the server decide
}
if [ "${DETECT:-0}" != "1" ]; then
  for p in "${RPC_PORT}" "${STREAM_PORT}"; do
    if port_busy "${p}"; then
      echo "[server] ERROR: port ${p} is already in use — another CARLA server is running." >&2
      echo "[server] Two ways out, your choice:" >&2
      echo "[server]   1. stop the running one:" >&2
      echo "[server]        pkill -x UE4Editor            # a source/editor server" >&2
      echo "[server]        pkill -x CarlaUE4-Linux-      # a packaged server (name IS truncated)" >&2
      echo "[server]      then wait for release: until ! nc -z 127.0.0.1 ${p}; do sleep 1; done" >&2
      echo "[server]   2. run this one on other ports:" >&2
      echo "[server]        bash run_server.sh '${MAP}' 3000     # streaming becomes 3001" >&2
      echo "[server]      and point clients at it with CARLA_PORT=3000" >&2
      exit 1
    fi
  done
fi

if [ "${DETECT:-0}" = "1" ]; then
  echo "[server] mode=${MODE} launch=${LAUNCH}"
  exit 0
fi

case "${MODE}" in
  package|dist)
    # A cooked build renders for real. Headless by default (-RenderOffScreen);
    # WINDOW=1 opens a window instead, which needs a display. MAP is ignored:
    # the package boots its cooked default map — switch with client.load_world.
    RENDER=(-RenderOffScreen)
    if [ "${WINDOW:-0}" = "1" ]; then
      export DISPLAY="${DISPLAY:-:1}"
      RENDER=(-windowed "-ResX=${RESX:-1280}" "-ResY=${RESY:-720}")
      echo "[server] ${MODE} ${LAUNCH} rpc=${RPC_PORT} WINDOWED on ${DISPLAY}"
    else
      echo "[server] ${MODE} ${LAUNCH} rpc=${RPC_PORT} stream=${STREAM_PORT} (-RenderOffScreen)"
    fi
    [ "${MAP}" = "/Game/Carla/Maps/Town02" ] || \
      echo "[server] NOTE: MAP is ignored for a cooked build; load it with client.load_world('${MAP##*/}')"
    exec "${LAUNCH}" \
      "${RENDER[@]}" -nosound \
      -carla-rpc-port="${RPC_PORT}" -carla-streaming-port="${STREAM_PORT}" \
      "${ROS2_FLAGS[@]}"
    ;;
  editor)
    UE4_EDITOR="${UE4_ROOT}/Engine/Binaries/Linux/UE4Editor"
    UPROJECT="${LAUNCH}/Unreal/CarlaUE4/CarlaUE4.uproject"
    [ -n "${UE4_ROOT}" ] \
      || { echo "[server] ERROR: a source checkout needs UE4_ROOT (the built UE 4.26 fork)." >&2; exit 1; }
    [ -x "${UE4_EDITOR}" ] || { echo "[server] ERROR: UE4Editor not built (build skill step 03)." >&2; exit 1; }
    [ -f "${UPROJECT}" ]   || { echo "[server] ERROR: CarlaUE4.uproject missing: ${UPROJECT}" >&2; exit 1; }

    export DISPLAY="${DISPLAY:-:1}"
    cd "${LAUNCH}/Unreal/CarlaUE4"

    if [ "${WINDOW:-0}" = "1" ]; then
      echo "[server] editor map=${MAP} rpc=${RPC_PORT} WINDOWED on ${DISPLAY} (real render, DF off, uncooked)"
      exec "${UE4_EDITOR}" "${UPROJECT}" "${MAP}" \
        -game -windowed -ResX="${RESX:-1280}" -ResY="${RESY:-720}" -nosound \
        "-ini:Engine:[/Script/Engine.RendererSettings]:r.GenerateMeshDistanceFields=False" \
        -carla-rpc-port="${RPC_PORT}" -carla-streaming-port="${STREAM_PORT}" \
        "${ROS2_FLAGS[@]}"
    else
      echo "[server] editor map=${MAP} rpc=${RPC_PORT} stream=${STREAM_PORT} (-game -nullrhi, headless, uncooked)"
      exec "${UE4_EDITOR}" "${UPROJECT}" "${MAP}" \
        -game -nullrhi -nosound \
        -carla-rpc-port="${RPC_PORT}" -carla-streaming-port="${STREAM_PORT}" \
        "${ROS2_FLAGS[@]}"
    fi
    ;;
esac
