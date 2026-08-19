#!/usr/bin/env bash
#
# run_carla_autoware.sh -- orchestrate CARLA (UE5.8, native ROS2) + Autoware.
#
# Modes:
#   classical : TIER IV-validated classic stack
#               ros2 launch autoware_launch e2e_simulator.launch.xml
#                    vehicle_model:=sample_vehicle sensor_model:=awsim_sensor_kit map_path:=<dir>
#   e2e       : VAD end-to-end planner (autoware_universe e2e/autoware_tensorrt_vad).
#               Primary form uses the OPEN PR autowarefoundation/autoware_launch#1685 glue
#               (use_e2e_planning:=true e2e_planning_type:=vad). If that glue is not present
#               in the installed workspace, falls back to launching
#               autoware_tensorrt_vad vad_carla_tiny.launch.xml directly with
#               sensing/localization/perception disabled.
#
# Start order (each process logged separately, PIDs recorded for teardown):
#   0. (zenoh only) zenoh router: ros2 run rmw_zenoh_cpp rmw_zenohd
#   1. CARLA server:  -ros2 -rmw=<rmw> [-ros-domain-id=N] [-RenderOffScreen]
#   2. town loader (one-shot, non-ticking PythonAPI client)
#   3. PythonAPI/examples/autoware_demo.py  <-- the SINGLE ticking client
#   4. (e2e only) spawn_vad_rig.py (six VAD cameras, never ticks)
#   5. (e2e only) ros2 launch e2e_state_publishers.launch.py (ground-truth localization glue)
#   6. Autoware launch (classical or e2e form)
#
# Teardown (INT/TERM/EXIT): kills ONLY the exact process groups it started
# (each child is a setsid group leader; PIDs stored in <log-dir>/carla_autoware.pids).
# Never pkills by name. stop_all.sh reads the same pidfile.
#
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../../../.." && pwd)"

# ---------------------------------------------------------------- defaults --
MODE=""
TOWN="Town10HD_Opt"
RMW="cyclonedds"
MAP_PATH=""
CARLA_ROOT_ARG="${CARLA_ROOT:-}"
AUTOWARE_WS="${AUTOWARE_WS:-$HOME/autoware}"
DOMAIN_ID=""
DRY_RUN=false
WITH_DISPLAY=false
LOG_DIR="$SCRIPT_DIR/logs"
RPC_PORT=2000
CARLA_HOST=127.0.0.1

usage() {
    cat <<EOF
Usage: $(basename "$0") --mode classical|e2e [options]

  --mode classical|e2e   (required) which Autoware stack to launch
  --town NAME            CARLA town to load (default: Town10HD_Opt)
  --rmw NAME             cyclonedds|fastdds|zenoh (default: cyclonedds; CycloneDDS is
                         Autoware's recommended RMW. zenoh additionally starts a router.)
  --map-path DIR         dir containing pointcloud_map.pcd, lanelet2_map.osm,
                         map_projector_info.yaml
                         (default: ../map_tools/maps/<town-without-_Opt>)
  --carla-root DIR       packaged CARLA root containing CarlaUnreal.sh
                         (default: \$CARLA_ROOT env; falls back to editor -game via \$UE_ROOT)
  --autoware-ws DIR      Autoware colcon workspace (default: \$AUTOWARE_WS or ~/autoware)
  --domain-id N          ROS domain id (passed as -ros-domain-id=N and ROS_DOMAIN_ID)
                         (--ros-domain-id is accepted as an alias)
  --carla-rpc-port N     CARLA RPC port (passed as -carla-rpc-port=N; default: 2000)
  --log-dir DIR          per-process logs + pidfile (default: <this dir>/logs)
  --with-display         do NOT pass -RenderOffScreen to the CARLA server
  --dry-run              print every command that would run; execute nothing;
                         preflight failures downgrade to warnings
  -h | --help            this text
EOF
}

# ------------------------------------------------------------ arg parsing --
while [[ $# -gt 0 ]]; do
    case "$1" in
        --mode)         MODE="$2"; shift 2 ;;
        --town)         TOWN="$2"; shift 2 ;;
        --rmw)          RMW="$2"; shift 2 ;;
        --map-path)     MAP_PATH="$2"; shift 2 ;;
        --carla-root)   CARLA_ROOT_ARG="$2"; shift 2 ;;
        --autoware-ws)  AUTOWARE_WS="$2"; shift 2 ;;
        --domain-id|--ros-domain-id) DOMAIN_ID="$2"; shift 2 ;;
        --carla-rpc-port) RPC_PORT="$2"; shift 2 ;;
        --log-dir)      LOG_DIR="$2"; shift 2 ;;
        --with-display) WITH_DISPLAY=true; shift ;;
        --dry-run)      DRY_RUN=true; shift ;;
        -h|--help)      usage; exit 0 ;;
        *) echo "ERROR: unknown option: $1" >&2; usage >&2; exit 2 ;;
    esac
done

[[ "$MODE" == "classical" || "$MODE" == "e2e" ]] \
    || { echo "ERROR: --mode must be 'classical' or 'e2e' (got: '${MODE:-<unset>}')" >&2; exit 2; }

case "$RMW" in
    cyclonedds) RMW_IMPL=rmw_cyclonedds_cpp ;;
    fastdds)    RMW_IMPL=rmw_fastrtps_cpp ;;
    zenoh)      RMW_IMPL=rmw_zenoh_cpp ;;
    *) echo "ERROR: --rmw must be cyclonedds|fastdds|zenoh (got: '$RMW')" >&2; exit 2 ;;
esac

[[ "$RPC_PORT" =~ ^[0-9]+$ ]] \
    || { echo "ERROR: --carla-rpc-port must be a number (got: '$RPC_PORT')" >&2; exit 2; }
[[ -z "$DOMAIN_ID" || "$DOMAIN_ID" =~ ^[0-9]+$ ]] \
    || { echo "ERROR: --domain-id must be a number (got: '$DOMAIN_ID')" >&2; exit 2; }

# Prebuilt autoware-contents maps are named Town01..Town10HD (no _Opt suffix).
TOWN_BASE="${TOWN%_Opt}"
[[ -n "$MAP_PATH" ]] || MAP_PATH="$SCRIPT_DIR/../map_tools/maps/$TOWN_BASE"

PIDFILE="$LOG_DIR/carla_autoware.pids"

# ---------------------------------------------------------------- helpers --
log()  { echo "[run_carla_autoware] $*"; }
warn() { echo "[run_carla_autoware] WARNING: $*" >&2; }
die()  { echo "[run_carla_autoware] ERROR: $*" >&2; exit 1; }

# Preflight failure: fatal normally, warning under --dry-run.
preflight_fail() {
    if $DRY_RUN; then warn "(dry-run, continuing) $*"; else die "$*"; fi
}

# Start one managed background process.
#   start_proc <name> <command string (run via bash -c, setsid group leader)>
# Logs to $LOG_DIR/<name>.log; appends "<name> <pid>" to $PIDFILE.
start_proc() {
    local name="$1" cmd="$2" logfile pid
    logfile="$LOG_DIR/$name.log"
    if $DRY_RUN; then
        echo "[dry-run] start '$name'  (log: $logfile)"
        echo "[dry-run]   setsid bash -c '$cmd'"
        return 0
    fi
    setsid bash -c "$cmd" >"$logfile" 2>&1 &
    pid=$!
    echo "$name $pid" >>"$PIDFILE"
    log "started '$name' (pid/pgid $pid), log: $logfile"
}

# Run a one-shot foreground command (still printed under --dry-run).
run_fg() {
    local name="$1" cmd="$2"
    if $DRY_RUN; then
        echo "[dry-run] run '$name' (foreground, one-shot)"
        echo "[dry-run]   bash -c '$cmd'"
        return 0
    fi
    log "running '$name' (foreground)"
    bash -c "$cmd" 2>&1 | tee -a "$LOG_DIR/$name.log"
}

wait_for_carla_rpc() {
    if $DRY_RUN; then
        echo "[dry-run] wait for CARLA RPC port $CARLA_HOST:$RPC_PORT (up to 180 s)"
        return 0
    fi
    log "waiting for CARLA RPC port $CARLA_HOST:$RPC_PORT ..."
    local i
    for i in $(seq 1 180); do
        if (exec 3<>"/dev/tcp/$CARLA_HOST/$RPC_PORT") 2>/dev/null; then
            exec 3>&- 3<&- || true
            log "CARLA RPC port is up (after ${i}s)"
            return 0
        fi
        sleep 1
    done
    die "CARLA server did not open port $RPC_PORT within 180 s -- see $LOG_DIR/carla_server.log"
}

pause() {   # give a freshly started component time to come up
    local secs="$1" why="$2"
    if $DRY_RUN; then echo "[dry-run] sleep $secs  # $why"; else log "sleep $secs ($why)"; sleep "$secs"; fi
}

# ---------------------------------------------------------------- teardown --
teardown() {
    local status=$?
    trap - INT TERM EXIT
    $DRY_RUN && exit "$status"
    if [[ -f "$PIDFILE" ]]; then
        log "tearing down (reverse start order, exact PIDs from $PIDFILE only)"
        local lines=() line name pid
        while IFS= read -r line; do lines+=("$line"); done <"$PIDFILE"
        local idx
        for ((idx=${#lines[@]}-1; idx>=0; idx--)); do
            name="${lines[$idx]%% *}"; pid="${lines[$idx]##* }"
            if kill -0 "$pid" 2>/dev/null; then
                log "  SIGTERM -> '$name' (pgid $pid)"
                kill -TERM -- "-$pid" 2>/dev/null || kill -TERM "$pid" 2>/dev/null || true
            fi
        done
        local waited=0
        while ((waited < 15)); do
            local alive=false
            for line in "${lines[@]}"; do
                pid="${line##* }"
                kill -0 "$pid" 2>/dev/null && alive=true
            done
            $alive || break
            sleep 1; ((waited++)) || true
        done
        for ((idx=${#lines[@]}-1; idx>=0; idx--)); do
            name="${lines[$idx]%% *}"; pid="${lines[$idx]##* }"
            if kill -0 "$pid" 2>/dev/null; then
                log "  SIGKILL -> '$name' (pgid $pid) [did not exit within 15 s]"
                kill -KILL -- "-$pid" 2>/dev/null || kill -KILL "$pid" 2>/dev/null || true
            fi
        done
        rm -f "$PIDFILE"
    fi
    log "logs are under: $LOG_DIR"
    exit "$status"
}
trap teardown INT TERM EXIT

# ---------------------------------------------------------------- preflight --
$DRY_RUN && log "DRY RUN -- nothing will be executed; preflight failures become warnings"
log "mode=$MODE town=$TOWN rmw=$RMW ($RMW_IMPL) domain-id=${DOMAIN_ID:-<default>} log-dir=$LOG_DIR"

# ros2 CLI
ROS_SETUP=""
if [[ -f "$AUTOWARE_WS/install/setup.bash" ]]; then
    ROS_SETUP="source '$AUTOWARE_WS/install/setup.bash'; "
elif [[ -f /opt/ros/jazzy/setup.bash ]]; then
    ROS_SETUP="source /opt/ros/jazzy/setup.bash; "
elif [[ -f /opt/ros/humble/setup.bash ]]; then
    ROS_SETUP="source /opt/ros/humble/setup.bash; "
fi
if ! command -v ros2 >/dev/null 2>&1 && [[ -z "$ROS_SETUP" ]]; then
    preflight_fail "ros2 not on PATH and no ROS underlay found (/opt/ros/{jazzy,humble}) -- source your ROS 2 environment first"
fi

# Autoware workspace
if [[ ! -f "$AUTOWARE_WS/install/setup.bash" ]]; then
    preflight_fail "Autoware workspace not found at '$AUTOWARE_WS' (no install/setup.bash). Build one with: $SCRIPT_DIR/../install/install_autoware.sh  (or pass --autoware-ws)"
fi

# Map triplet
for f in pointcloud_map.pcd lanelet2_map.osm map_projector_info.yaml; do
    if [[ ! -f "$MAP_PATH/$f" ]]; then
        preflight_fail "map dir '$MAP_PATH' is missing '$f'. Fetch the prebuilt maps with: $SCRIPT_DIR/../map_tools/fetch_prebuilt_maps.sh $TOWN_BASE '$MAP_PATH'  (or pass --map-path)"
        break   # one message is enough in dry-run
    fi
done

# CARLA server binary: packaged CarlaUnreal.sh preferred, editor -game fallback.
# Packaged layout on this branch: Build/<cfg>/Package/Carla-*-Linux-*/Linux/CarlaUnreal.sh
SERVER_LAUNCHER=""
SERVER_KIND=""
CANDIDATES=()
if [[ -n "$CARLA_ROOT_ARG" ]]; then
    CANDIDATES+=("$CARLA_ROOT_ARG/CarlaUnreal.sh" "$CARLA_ROOT_ARG/Linux/CarlaUnreal.sh")
fi
for cand in "$REPO_ROOT"/Build/*/Package/Carla-*/Linux/CarlaUnreal.sh; do
    CANDIDATES+=("$cand")   # unmatched globs stay literal and fail -x below
done
for cand in "${CANDIDATES[@]}"; do
    if [[ -x "$cand" ]]; then
        SERVER_LAUNCHER="$cand"; SERVER_KIND="packaged"; break
    fi
done
UPROJECT="$REPO_ROOT/Unreal/CarlaUnreal/CarlaUnreal.uproject"
if [[ -z "$SERVER_LAUNCHER" ]]; then
    if [[ -n "${UE_ROOT:-}" && -x "$UE_ROOT/Engine/Binaries/Linux/UnrealEditor" && -f "$UPROJECT" ]]; then
        SERVER_KIND="editor"
    else
        preflight_fail "no packaged CarlaUnreal.sh found (searched under --carla-root '$CARLA_ROOT_ARG' and $REPO_ROOT/Build/*/Package/Carla-*/Linux/) and no usable editor fallback (need UE_ROOT set and $UPROJECT present). Package CARLA or pass --carla-root <dir-containing-CarlaUnreal.sh>"
        SERVER_KIND="editor"   # dry-run: show the editor-fallback command with \$UE_ROOT placeholder
    fi
fi

# The single ticking client (lands with the TIER IV Autoware-layer migration)
AUTOWARE_DEMO="$REPO_ROOT/PythonAPI/examples/autoware_demo.py"
if [[ ! -f "$AUTOWARE_DEMO" ]]; then
    preflight_fail "'$AUTOWARE_DEMO' not found -- migration branch not built yet. The native /vehicle/status/* + /control/command/* bridge and autoware_demo.py land with the concurrent Autoware-layer migration; rerun once it is merged."
fi

# carla python package for the helper clients
if ! python3 -c 'import carla' >/dev/null 2>&1; then
    preflight_fail "python3 cannot 'import carla' -- install the CARLA wheel (PythonAPI/carla/dist) into this environment"
fi

# e2e-only preflight: VAD model + launch glue
E2E_GLUE="pr1685"
if [[ "$MODE" == "e2e" ]]; then
    if [[ ! -d "$HOME/autoware_data/ml_models/vad/v0.1" ]]; then
        preflight_fail "VAD model not found at ~/autoware_data/ml_models/vad/v0.1 (HuggingFace AutowareFoundation/tensorrt_vad, tag v0.1). See $SCRIPT_DIR/../install/install_autoware.sh. First VAD run also builds TensorRT engines (slow, cached)."
    fi
    # PR autowarefoundation/autoware_launch#1685 adds use_e2e_planning/e2e_planning_type
    # to e2e_simulator.launch.xml. It is NOT merged upstream -- detect it in the ws.
    E2E_SIM_XML="$(find "$AUTOWARE_WS/install" -path '*autoware_launch*' -name 'e2e_simulator.launch.xml' 2>/dev/null | head -n1 || true)"
    if [[ -n "$E2E_SIM_XML" ]] && grep -q 'use_e2e_planning' "$E2E_SIM_XML" 2>/dev/null; then
        E2E_GLUE="pr1685"
        log "found PR#1685 e2e glue in $E2E_SIM_XML"
    else
        E2E_GLUE="fallback"
        warn "installed autoware_launch lacks the (unmerged) PR#1685 'use_e2e_planning' glue -- will launch autoware_tensorrt_vad vad_carla_tiny.launch.xml directly (sensing/localization/perception disabled)"
    fi
fi

# ----------------------------------------------------------- env preludes --
ENV_EXPORTS="export RMW_IMPLEMENTATION=$RMW_IMPL; "
[[ -n "$DOMAIN_ID" ]] && ENV_EXPORTS+="export ROS_DOMAIN_ID=$DOMAIN_ID; "
ROS_PRELUDE="$ROS_SETUP$ENV_EXPORTS"

if ! $DRY_RUN; then
    mkdir -p "$LOG_DIR"
    : >"$PIDFILE"
fi

# ------------------------------------------------------------ 0. zenoh rtr --
if [[ "$RMW" == "zenoh" ]]; then
    # rmw_zenoh needs its router up BEFORE any zenoh peer (CARLA server included).
    start_proc zenoh_router "${ROS_PRELUDE}exec ros2 run rmw_zenoh_cpp rmw_zenohd"
    pause 3 "let the zenoh router come up before starting peers"
fi

# ---------------------------------------------------------- 1. CARLA server --
SERVER_FLAGS="-ros2 -rmw=$RMW -carla-rpc-port=$RPC_PORT"
[[ -n "$DOMAIN_ID" ]] && SERVER_FLAGS+=" -ros-domain-id=$DOMAIN_ID"
$WITH_DISPLAY || SERVER_FLAGS+=" -RenderOffScreen"

if [[ "$SERVER_KIND" == "packaged" ]]; then
    start_proc carla_server "exec '$SERVER_LAUNCHER' $SERVER_FLAGS"
else
    start_proc carla_server "exec '${UE_ROOT:-\$UE_ROOT}/Engine/Binaries/Linux/UnrealEditor' '$UPROJECT' -game $SERVER_FLAGS"
fi
wait_for_carla_rpc

# --------------------------------------------------------- 2. load the town --
# One-shot, non-ticking client; runs BEFORE the sync-mode ticking client exists.
run_fg load_town "python3 -c \"
import carla
c = carla.Client('$CARLA_HOST', $RPC_PORT); c.set_timeout(120.0)
if not c.get_world().get_map().name.endswith('$TOWN'):
    print('loading $TOWN ...'); c.load_world('$TOWN')
else:
    print('$TOWN already loaded')
\""

# ------------------------------------- 3. autoware_demo.py (ticking client) --
# Spawns ego + native Autoware sensors (lidar XYZIRCAEDT, IMU, GNSS, TL camera)
# and ticks. It must remain the ONLY ticking client (sync mode).
start_proc autoware_demo "exec python3 '$AUTOWARE_DEMO' --host $CARLA_HOST --port $RPC_PORT"
pause 5 "let autoware_demo.py spawn the ego before attaching more sensors"

# ------------------------------------------------- 4+5. e2e-only glue procs --
if [[ "$MODE" == "e2e" ]]; then
    # Six VAD cameras (1600x900, nuScenes-style rig) on /sensing/camera/CAM_*/image_raw.
    # spawn_vad_rig.py never ticks; it attaches to the ego spawned by autoware_demo.py.
    start_proc vad_rig "exec python3 '$SCRIPT_DIR/spawn_vad_rig.py' --host $CARLA_HOST --port $RPC_PORT"
    pause 3 "let the camera rig attach and enable ROS publishing"

    # carla_state_publisher + autoware_vehicle_velocity_converter + autoware_twist2accel
    # (ground-truth /localization/kinematic_state + /localization/acceleration)
    # + image_transport republish (raw -> compressed) for the six cameras.
    start_proc e2e_state_publishers "${ROS_PRELUDE}exec ros2 launch '$SCRIPT_DIR/e2e_state_publishers.launch.py'"
fi

# ------------------------------------------------------- 6. Autoware launch --
CLASSICAL_CMD="${ROS_PRELUDE}exec ros2 launch autoware_launch e2e_simulator.launch.xml vehicle_model:=sample_vehicle sensor_model:=awsim_sensor_kit map_path:='$MAP_PATH'"
E2E_PR_CMD="${ROS_PRELUDE}exec ros2 launch autoware_launch e2e_simulator.launch.xml vehicle_model:=sample_vehicle sensor_model:=awsim_sensor_kit map_path:='$MAP_PATH' use_e2e_planning:=true e2e_planning_type:=vad"
E2E_FALLBACK_CMD="${ROS_PRELUDE}exec ros2 launch autoware_tensorrt_vad vad_carla_tiny.launch.xml sensing:=false localization:=false perception:=false"

if [[ "$MODE" == "classical" ]]; then
    start_proc autoware "$CLASSICAL_CMD"
else
    if [[ "$E2E_GLUE" == "pr1685" ]]; then
        start_proc autoware "$E2E_PR_CMD"
        if $DRY_RUN; then
            echo "[dry-run] NOTE: the form above needs the unmerged PR autowarefoundation/autoware_launch#1685."
            echo "[dry-run]       Fallback if your autoware_launch lacks 'use_e2e_planning':"
            echo "[dry-run]   setsid bash -c '$E2E_FALLBACK_CMD'"
        fi
    else
        start_proc autoware "$E2E_FALLBACK_CMD"
        if $DRY_RUN; then
            echo "[dry-run] NOTE: once PR autowarefoundation/autoware_launch#1685 (use_e2e_planning glue) is"
            echo "[dry-run]       present in the workspace, the primary form would be used instead:"
            echo "[dry-run]   setsid bash -c '$E2E_PR_CMD'"
        fi
    fi
fi

# --------------------------------------------------------------- foreground --
if $DRY_RUN; then
    echo "[dry-run] would then: wait on the 'autoware' PID; Ctrl-C / exit triggers teardown"
    echo "[dry-run] pidfile:  $PIDFILE   (stop later with: $SCRIPT_DIR/stop_all.sh --log-dir '$LOG_DIR')"
    echo "[dry-run] logs dir: $LOG_DIR"
    exit 0
fi

log "all components started. Ctrl-C stops everything. Stop from elsewhere with:"
log "  $SCRIPT_DIR/stop_all.sh --log-dir '$LOG_DIR'"
AUTOWARE_PID="$(awk '$1=="autoware"{print $2}' "$PIDFILE")"
wait "$AUTOWARE_PID"
