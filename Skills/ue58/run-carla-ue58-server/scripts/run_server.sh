#!/usr/bin/env bash
# Start, probe and stop a CARLA UE 5.8 server.
#
#   bash run_server.sh detect              what is runnable in this tree
#   bash run_server.sh game    [MAP]       editor binary in -game mode  <- the working headless server
#   bash run_server.sh package [MAP]       the packaged shipping server
#   bash run_server.sh editor              open the full editor (interactive)
#   bash run_server.sh probe               connect a client and report
#   bash run_server.sh stop                stop any CARLA server
#
# Env: PORT=2000  TM_PORT=8000  ROS2=1  RMW=fastdds|cyclonedds|zenoh  ROS_DOMAIN_ID=
#      OFFSCREEN=1 (default for game/package)  NULLRHI=1  QUALITY=Low|Epic
#      WINDOW=1 (render to a window instead)   DETACH=1 (background it)
#      TRACE=1  (keep UnrealTrace on; WINDOW=1 passes -notrace without it)
#      RES=1920x1080  FULLSCREEN=1   (both WINDOW=1 only, both opt-in)
#      EXTRA="-any -more -flags"
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh"
set +e

MODE="${1:-detect}"; shift 2>/dev/null || true
MAP="${1:-}"
PORT="${PORT:-${CARLA_PORT}}"
TM_PORT="${TM_PORT:-8000}"

# `comm` in /proc is truncated to 15 characters, so the shipping binary appears as
# 'CarlaUnreal-Lin', never 'CarlaUnreal-Linux-Shipping'. Matching the full name
# with pkill -x silently matches nothing and leaves the port held.
COMM_PKG="CarlaUnreal-Lin"
COMM_EDITOR="UnrealEditor"

port_busy() { (echo >"/dev/tcp/127.0.0.1/$1") >/dev/null 2>&1; }

preflight_ports() {
  local blocked=0
  for p in "${PORT}" "$(( PORT + 1 ))" "$(( PORT + 2 ))"; do
    if port_busy "${p}"; then
      echo "ERROR port ${p} is in use — ANOTHER CARLA SERVER IS ALREADY RUNNING" >&2
      blocked=1
    fi
  done
  if [ "${blocked}" -eq 1 ]; then
    echo "  CARLA needs the RPC port and the two streaming ports above it." >&2
    echo "  Either stop the running server:" >&2
    echo "      bash run_server.sh stop" >&2
    echo "  or run this one on different ports:" >&2
    echo "      PORT=3000 bash run_server.sh ${MODE}   (+ CARLA_PORT=3000 for clients)" >&2
    echo "  Launching anyway ends in 'bind: Address already in use' then Signal 11." >&2
    exit 4
  fi
}

common_args() {
  ARGS=(-carla-rpc-port="${PORT}" -carla-streaming-port="$(( PORT + 1 ))")
  if [ "${WINDOW:-0}" = "1" ]; then
    # Windowed mode gets -notrace by default. Measured on ue58-dev: a windowed
    # launch started right after a headless one crashed on startup with
    #   SIGSEGV at 0x00007dcfcdff1a10
    #   UE::Trace::Private::Writer_WorkerThread (Trace/Writer.cpp:1082)
    #   -> TraceAuxiliaryOnMessageCallback -> FLLMTracker::TrackAllocation
    # i.e. inside UnrealTrace's own writer thread, nothing to do with the
    # window or the RHI, with a stale UnrealTraceServer daemon still resident
    # from the previous run. -notrace stops the engine attaching to that daemon
    # and the same command then came up clean. Nobody profiling a demo needs
    # the trace stream; TRACE=1 puts it back.
    if [ "${TRACE:-0}" != "1" ]; then
      ARGS+=(-notrace)
    fi
    # RES=WxH sizes the window; FULLSCREEN=1 asks for exclusive fullscreen.
    # Neither is default: a plain WINDOW=1 keeps whatever size the engine
    # picks, and exclusive fullscreen is a poor default because an overlay
    # window (a terminal being recorded on top of the sim, say) does not
    # reliably stay above it.
    if [ -n "${RES:-}" ]; then
      case "${RES}" in
        *[xX]*) ARGS+=(-ResX="${RES%%[xX]*}" -ResY="${RES##*[xX]}") ;;
        *) echo "[run] WARNING RES='${RES}' is not WxH — ignored" ;;
      esac
    fi
    if [ "${FULLSCREEN:-0}" = "1" ]; then
      ARGS+=(-fullscreen)
    else
      ARGS+=(-windowed)
    fi
  elif [ "${NULLRHI:-0}" = "1" ]; then
    # No RHI at all. Fastest start, but do NOT spawn a camera on a -nullrhi
    # server: there is no render target, ImageUtil::ReadImageDataBegin
    # (Sensor/ImageUtil.cpp:224) dereferences null on the render thread and the
    # whole server dies with SIGSEGV at 0x58. Verified on ue58-dev.
    echo "[run] WARNING -nullrhi: spawning ANY camera sensor will CRASH this server"
    echo "[run] WARNING   (ImageUtil::ReadImageDataBegin null render target, SIGSEGV)."
    echo "[run] WARNING   Use the default -RenderOffScreen for anything with cameras."
    ARGS+=(-nullrhi)
  else
    ARGS+=(-RenderOffScreen)
  fi
  [ -n "${QUALITY:-}" ] && ARGS+=(-quality-level="${QUALITY}")
  if [ "${ROS2:-0}" = "1" ]; then
    # UE 5.8's FParse::Param strips a leading dash from the search string, so the
    # flag is -ros2 here where UE4 wanted --ros2. Current ue58-dev accepts both
    # (CarlaSettings.cpp adds an explicit --ros2 Strifind fallback); -ros2 is the
    # spelling that works on every ue58 commit.
    ARGS+=(-ros2)
    [ -n "${RMW:-}" ] && ARGS+=(-rmw="${RMW}")
    [ -n "${ROS_DOMAIN_ID:-}" ] && ARGS+=(-ros-domain-id="${ROS_DOMAIN_ID}")
  fi
  [ -n "${EXTRA:-}" ] && read -ra _x <<<"${EXTRA}" && ARGS+=("${_x[@]}")
}

launch() {
  local desc="$1"; shift
  echo "[run] ${desc}"
  echo "[run] $*"
  if [ "${DETACH:-0}" = "1" ]; then
    # A plain trailing & is not enough: the server inherits the shell's stdio,
    # and when that goes away it dies with 'close: Bad file descriptor' followed
    # by Signal 11. setsid + nohup + </dev/null detaches it properly.
    local log="${TMPDIR:-/tmp}/carla-ue58-${PORT}.log"
    setsid nohup "$@" >"${log}" 2>&1 </dev/null &
    echo "[run] detached, pid $!, log ${log}"
    wait_ready
  else
    echo "[run] foreground; Ctrl-C to stop. Add DETACH=1 to background it."
    "$@"
    echo "[run] server exited $? — NOTE the server takes SIGSEGV on SIGTERM on this"
    echo "[run] branch, so a non-zero exit here does not mean the run failed."
  fi
}

wait_ready() {
  echo -n "[run] waiting for RPC port ${PORT} "
  for _ in $(seq 1 120); do
    if port_busy "${PORT}"; then echo " up"; return 0; fi
    echo -n .
    sleep 1
  done
  echo " TIMEOUT after 120s"
  echo "[run] the first launch of a fresh build compiles shaders and can take much longer"
  return 1
}

case "${MODE}" in

detect)
  echo "== What can run here =="
  P="$(carla_ue58_package_sh)"
  [ -n "${P}" ] && echo "  package : ${P}" || echo "  package : none built (package-carla-ue58)"
  EB="${CARLA_UNREAL_ENGINE_PATH}/Engine/Binaries/Linux/UnrealEditor"
  UPROJ="${CARLA_UE58_ROOT}/Unreal/CarlaUnreal/CarlaUnreal.uproject"
  if [ -x "${EB}" ] && [ -f "${UPROJ}" ]; then
    echo "  editor  : ${EB}"
    echo "  game    : same binary with -game (recommended headless server)"
  else
    echo "  editor  : not available (engine or uproject missing)"
  fi
  # The uncooked Development game target cannot start: it is built to load COOKED
  # content and the tree has none, so it dies on the missing global shader cache.
  SGB="${CARLA_UE58_ROOT}/Unreal/CarlaUnreal/Binaries/Linux/CarlaUnreal"
  [ -x "${SGB}" ] && echo "  NOTE    : ${SGB} exists but CANNOT start ('built to load COOKED content')"
  echo
  echo "== Ports =="
  for p in "${PORT}" "$(( PORT + 1 ))" "$(( PORT + 2 ))"; do
    port_busy "${p}" && echo "  ${p} IN USE" || echo "  ${p} free"
  done
  echo
  echo "== ROS 2 =="
  R="$(carla_ue58_cmake_opt ENABLE_ROS2 2>/dev/null)"
  case "${R}" in
    ON)  echo "  built with ENABLE_ROS2=ON — add ROS2=1 to publish topics" ;;
    OFF) echo "  built with ENABLE_ROS2=OFF — -ros2 will do nothing; rebuild with ROS2=1" ;;
    *)   echo "  unknown (tree not configured)" ;;
  esac
  ;;

game)
  # This is the server mode that actually works on an uncooked tree: the editor
  # binary with -game. nurec/README.md recommends the standalone game binary,
  # which cannot start (see detect).
  preflight_ports
  EB="${CARLA_UNREAL_ENGINE_PATH}/Engine/Binaries/Linux/UnrealEditor"
  UPROJ="${CARLA_UE58_ROOT}/Unreal/CarlaUnreal/CarlaUnreal.uproject"
  [ -x "${EB}" ] || { echo "ERROR ${EB} missing — build the engine" >&2; exit 3; }
  [ -f "${UPROJ}" ] || { echo "ERROR ${UPROJ} missing" >&2; exit 3; }
  common_args
  CMD=("${EB}" "${UPROJ}")
  [ -n "${MAP}" ] && CMD+=("${MAP}")
  CMD+=(-game "-${CARLA_UNREAL_RHI:-vulkan}" "${ARGS[@]}")
  launch "editor binary in -game mode (uncooked tree, full client support)" "${CMD[@]}"
  ;;

package)
  preflight_ports
  P="$(carla_ue58_package_sh)"
  [ -n "${P}" ] || { echo "ERROR no package built — see package-carla-ue58" >&2; exit 3; }
  common_args
  CMD=("${P}")
  [ -n "${MAP}" ] && CMD+=("${MAP}")
  CMD+=("${ARGS[@]}")
  # Measured on this branch, not assumed: a client DOES connect to a packaged
  # server and get_world(), spawning and navigation all work. What is broken is
  # map DISCOVERY and switching — get_available_maps() returns [] because
  # GetAllMapNames() uses a raw FindFilesRecursive("*.umap") that cannot see
  # inside a .pak. Small maps still load by name; large ones do not.
  echo "[run] NOTE packaged server limitations (measured on ue58-dev):"
  echo "[run]   get_available_maps() returns []            (0 vs 29 in game mode)"
  echo "[run]   load_world('Town12'/'Town13') FAILS        (large/World Partition maps)"
  echo "[run]   load_world('Town_C'/'Town10HD_Opt') works  (small maps, by exact name)"
  echo "[run]   get_world/spawn/navigation all work        (contrary to the older report)"
  echo "[run]   use 'game' mode when you need map switching or discovery."
  launch "packaged shipping server" "${CMD[@]}"
  ;;

editor)
  preflight_ports
  echo "[run] opening the editor via the CMake target (interactive; needs a display)"
  echo "[run] launch args come from -DCARLA_LAUNCH_ARGS at configure time, not from here"
  cd "${CARLA_UE58_ROOT}" && cmake --build "$(carla_ue58_build_dir)" --target launch
  ;;

probe)
  "${PYTHON}" - "${CARLA_HOST}" "${PORT}" <<'PY'
import sys, time
host, port = sys.argv[1], int(sys.argv[2])
try:
    import carla
except Exception as e:
    sys.exit(f"FAIL cannot import carla ({e})")
if getattr(carla, "__file__", None) is None:
    sys.exit(f"FAIL `carla` resolved to a directory, not the client: {getattr(carla,'__path__','?')}")
c = carla.Client(host, port)
c.set_timeout(20.0)
try:
    sv, cv = c.get_server_version(), c.get_client_version()
except Exception as e:
    sys.exit(f"FAIL no server at {host}:{port} ({e})")
print(f"PASS server {sv} / client {cv}" + ("" if sv == cv else "   <-- MISMATCH"))
t0 = time.time()
try:
    w = c.get_world()
except Exception as e:
    # The packaged-server path escape lands here: create_directories throws
    # Permission denied while writing the client file cache.
    print(f"FAIL get_world() failed after {time.time()-t0:.1f}s: {e}")
    print("FAIL   if this mentions '../../../CarlaUnreal/Content' it is the packaged-server")
    print("FAIL   file-cache path escape — use 'game' mode instead of 'package'")
    sys.exit(1)
print(f"PASS get_world() in {time.time()-t0:.1f}s")
m = w.get_map()
print(f"PASS map {m.name.split('/')[-1]}  spawn points {len(m.get_spawn_points())}")
s = w.get_settings()
print(f"PASS settings sync={s.synchronous_mode} dt={s.fixed_delta_seconds}")
print(f"PASS blueprints {len(w.get_blueprint_library())}")
print(f"PASS actors {len(w.get_actors())}  frame {w.get_snapshot().frame}")
maps = [x.split('/')[-1] for x in c.get_available_maps()]
print(f"PASS available maps ({len(maps)}): {', '.join(sorted(maps))}")
PY
  exit $?
  ;;

stop)
  STOPPED=0
  for comm in "${COMM_PKG}" "${COMM_EDITOR}"; do
    if pgrep -x "${comm}" >/dev/null 2>&1; then
      echo "[run] stopping ${comm} ($(pgrep -x "${comm}" | tr '\n' ' '))"
      pkill -x "${comm}"
      STOPPED=1
    fi
  done
  [ "${STOPPED}" -eq 0 ] && echo "[run] no CARLA server process found"
  # The port is what other launches collide with, so wait on the port rather than
  # on the process.
  for _ in $(seq 1 30); do
    port_busy "${PORT}" || { echo "[run] port ${PORT} released"; exit 0; }
    sleep 1
  done
  echo "[run] port ${PORT} still held after 30s — check for a stuck process" >&2
  exit 1
  ;;

*)
  echo "usage: bash run_server.sh {detect|game [MAP]|package [MAP]|editor|probe|stop}" >&2
  exit 2
  ;;
esac
