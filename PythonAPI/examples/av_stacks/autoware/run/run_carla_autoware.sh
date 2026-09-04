#!/usr/bin/env bash
#
# run_carla_autoware.sh -- orchestrate CARLA (UE5.8, native ROS2) + Autoware.
#
# Modes:
#   classical : TIER IV-validated classic stack
#               ros2 launch autoware_launch e2e_simulator.launch.xml
#                    vehicle_model:=sample_vehicle sensor_model:=awsim_sensor_kit
#                    perception_mode:=lidar rviz:=false map_path:=<dir>
#               Runs either from a source workspace (--stack source) or from the
#               official Autoware docker image (--stack docker).
#   e2e       : VAD end-to-end planner (autoware_universe e2e/autoware_tensorrt_vad).
#               Source workspace only. Primary form uses the OPEN PR
#               autowarefoundation/autoware_launch#1685 glue
#               (use_e2e_planning:=true e2e_planning_type:=vad). If that glue is not
#               present in the installed workspace, falls back to launching
#               autoware_tensorrt_vad vad_carla_tiny.launch.xml directly with
#               sensing/localization/perception disabled.
#
# DDS topology (validated 2026-08 on Town10, full classical stack):
#   - The SIMULATOR runs Fast DDS (-rmw=fastdds) with a generated profile that
#     whitelists ONLY the docker bridge IP (useBuiltinTransports=false).
#     -rmw=cyclonedds is also validated (fragmented-receive fix b9c33737a);
#     fastdds remains the default for its longer validation history.
#   - The AUTOWARE side runs CycloneDDS pinned to the docker bridge interface
#     via a generated cyclonedds.xml (MaxAutoParticipantIndex=300, 65500B max
#     message size, 10-64MB socket buffers -- the official docker image's own
#     cyclonedds.xml pins 'lo', which breaks discovery; it MUST be overridden).
#   - The kernel MUST allow those buffers: net.core.rmem_max/wmem_max = 64MB,
#     persisted (see /etc/sysctl.d note below). At the stock 4MB cap the lidar
#     PointCloud2 stream over reliable writers overruns the sockets (hundreds
#     of thousands of RcvbufErrors), reliable-writer retransmits stall every
#     participant for seconds at a time, and the stack MRM-stops the vehicle.
#     Buffer size is fixed at socket creation: after raising the sysctl, every
#     DDS process (INCLUDING the simulator) must be restarted to benefit.
#   - Mixed-RMW is fine: DDS vendors interoperate over UDPv4.
#   - NEVER bind DDS to a WiFi/DHCP interface -- rotating IPs poison DDS
#     locators. The docker bridge has a stable IP; both configs pin it.
#   Both XML files are generated at runtime into <log-dir>/dds/ from the
#   auto-detected bridge (override with --bridge-if / --docker-network).
#
# Start order (each process logged separately, PIDs recorded for teardown):
#   0. (zenoh only) zenoh router: ros2 run rmw_zenoh_cpp rmw_zenohd
#   1. CARLA server:  -ros2 -rmw=<rmw> -ros-domain-id=N [-RenderOffScreen]
#   2. town loader (one-shot, non-ticking PythonAPI client)
#   3. PythonAPI/examples/autoware_demo.py  <-- the SINGLE ticking client
#   4. (e2e only) spawn_vad_rig.py (six VAD cameras, never ticks)
#   5. (e2e only) ros2 launch e2e_state_publishers.launch.py (ground-truth localization glue)
#   6. Autoware launch (classical: source ws or docker container; e2e: source ws)
#      -- classical mode first applies the CARLA-specific config overrides
#         (NDT convergence threshold 2.3 -> 1.0, stop_check_enabled -> false)
#   7. (classical, unless --no-auto) post-launch automation: wait for the stack,
#      auto-initialize localization from GNSS, and with --goal also publish the
#      goal pose and engage autonomous mode.
#
# Teardown (INT/TERM/EXIT): kills ONLY the exact process groups it started
# (each child is a setsid group leader; PIDs stored in <log-dir>/carla_autoware.pids)
# and `docker rm -f`s ONLY the containers it created (names stored in
# <log-dir>/carla_autoware.containers). Never pkills by name -- see stop_all.sh
# for why that is load-bearing. stop_all.sh reads the same two files.
#
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../../../../.." && pwd)"

# Hostile-environment scrub: an activated conda/virtualenv breaks the stack in
# two ways -- '#!/usr/bin/env python3' nodes resolve to the env's python (wrong
# minor version for rclpy's C extension: "No module named rclpy._rclpy_pybind11"),
# and CMake/colcon prefer the env python outright. Strip the env markers and
# any conda dirs from PATH so every child starts clean.
unset CONDA_PREFIX CONDA_DEFAULT_ENV CONDA_EXE CONDA_PYTHON_EXE CONDA_SHLVL \
      VIRTUAL_ENV PYTHONPATH PYTHONHOME 2>/dev/null || true
ORIG_PATH="$PATH"   # kept only to locate a python that can 'import carla'
PATH="$(printf '%s' "$PATH" | tr ':' '\n' | grep -vi -e 'conda' -e 'miniforge' | paste -sd: || true)"
export PATH

# ---------------------------------------------------------------- defaults --
MODE=""
TOWN="Town10HD_Opt"
RMW="fastdds"                 # SIMULATOR-side RMW: fastdds (default) or cyclonedds, both validated.
STACK="auto"                  # auto|source|docker : how to run Autoware (classical)
MAP_PATH=""
SERVER_ARGS=""                # extra simulator flags, appended last (see --server-args)
CARLA_ROOT_ARG="${CARLA_ROOT:-}"
AUTOWARE_WS="${AUTOWARE_WS:-$HOME/autoware}"
DOMAIN_ID=42
DRY_RUN=false
WITH_DISPLAY=false
WITH_RVIZ=false
NO_AUTO=false
NO_GATES=false
NO_RECOVER=false
GOAL=""
SPAWN_INDEX=""                # autoware_demo.py --spawn_index passthrough (e2e default: 52)
LOG_DIR="$SCRIPT_DIR/logs"
RPC_PORT=2000
CARLA_HOST=127.0.0.1
IMAGE=""                      # docker stack image (default: auto-detect local ghcr image)
CONTAINER_NAME="carla-autoware"
SERVER_PREF="auto"            # auto: packaged if found, editor fallback; editor: force editor -game
BRIDGE_IF=""                  # docker bridge interface for DDS (default: auto-detect)
BRIDGE_IP=""
DOCKER_NETWORK=""             # detect the bridge from this docker network instead

usage() {
    cat <<EOF
Usage: $(basename "$0") --mode classical|e2e [options]

  --mode classical|e2e   (required) which Autoware stack to launch
  --town NAME            CARLA town to load (default: Town10HD_Opt)
  --stack auto|source|docker
                         classical only: run Autoware from the source workspace or
                         the official docker image (default: auto -- source ws if
                         present, else local docker image). e2e is always source.
  --image IMG            docker stack: image to use (default: newest local
                         ghcr.io/autowarefoundation/autoware:universe* image)
  --container-name NAME  docker stack: container name (default: carla-autoware).
                         Must not collide with an existing container; this script
                         never touches containers it did not create.
  --rmw NAME             SIMULATOR RMW: fastdds|cyclonedds|zenoh (default: fastdds).
                         fastdds and cyclonedds are both validated on the sim
                         side; the Autoware side always runs cyclonedds
                         (zenoh: zenoh) with a generated config.
  --map-path DIR         dir containing pointcloud_map.pcd, lanelet2_map.osm,
                         map_projector_info.yaml
                         (default: ../map_tools/maps/<town-without-_Opt>)
  --carla-root DIR       packaged CARLA root containing CarlaUnreal.sh
                         (default: \$CARLA_ROOT env; falls back to editor -game via \$UE_ROOT)
  --server auto|editor   auto (default): use a packaged build if found;
                         editor: force the editor -game path (\$UE_ROOT) — use
                         when the package is stale vs. recent simulator fixes
  --autoware-ws DIR      Autoware colcon workspace (default: \$AUTOWARE_WS or ~/autoware)
  --domain-id N          ROS domain id (default: 42; passed as -ros-domain-id=N and
                         ROS_DOMAIN_ID; --ros-domain-id is accepted as an alias)
  --carla-rpc-port N     CARLA RPC port (passed as -carla-rpc-port=N; default: 2000)
  --bridge-if NAME       docker bridge interface to pin DDS to (default: auto-detect
                         an UP br-*/docker0 interface with an IPv4 address)
  --docker-network NAME  auto-detect the bridge from this docker network instead
  --goal "X,Y,YAW"       classical: drive to this goal after startup. CARLA
                         coordinates (m, m, deg); converted to Autoware map frame
                         (x_map = x, y_map = -y, yaw_map = -yaw) automatically.
  --spawn-index N        spawn the ego at this spawn point index (passed to
                         autoware_demo.py as --spawn_index). classical default:
                         the demo's own default. e2e default: 52 -- a spawn on
                         the Town10 outer ring, which loops; VAD has NO route
                         input (fixed LANE_FOLLOW command), so a road that ends
                         in a T-junction wedges the car at the dead end.
  --no-auto              skip post-launch automation entirely (classical:
                         localization init / goal / engage; e2e: auto-engage)
  --no-recover           e2e: the drive keeper only logs collisions/wedges,
                         never teleport-recovers the ego
  --no-gates             classical: skip the pre-engage safety gates (ground-truth
                         localization check + distortion-corrector health). The
                         gates exist because engaging on a diverged pose drives
                         the car into things -- only skip them knowingly.
  --with-rviz            also start RViz (docker stack: separate container with
                         DISPLAY passthrough; source stack: local rviz2)
  --log-dir DIR          per-process logs + pidfile (default: <this dir>/logs)
  --with-display         do NOT pass -RenderOffScreen to the CARLA server
  --server-args "FLAGS"  extra flags appended verbatim to the simulator's own
                         command line, after every flag this script sets
                         itself. E.g. "-log -carla-streaming-port=2001".
                         Whitespace-split with shell-style quoting (quote a
                         value to keep a space in it); never evaluated --
                         \$, ;, \` etc. reach the simulator's argv literally.
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
        --stack)        STACK="$2"; shift 2 ;;
        --image)        IMAGE="$2"; shift 2 ;;
        --container-name) CONTAINER_NAME="$2"; shift 2 ;;
        --rmw)          RMW="$2"; shift 2 ;;
        --map-path)     MAP_PATH="$2"; shift 2 ;;
        --carla-root)   CARLA_ROOT_ARG="$2"; shift 2 ;;
        --server)       SERVER_PREF="$2"; shift 2 ;;
        --autoware-ws)  AUTOWARE_WS="$2"; shift 2 ;;
        --domain-id|--ros-domain-id) DOMAIN_ID="$2"; shift 2 ;;
        --carla-rpc-port) RPC_PORT="$2"; shift 2 ;;
        --bridge-if)    BRIDGE_IF="$2"; shift 2 ;;
        --docker-network) DOCKER_NETWORK="$2"; shift 2 ;;
        --goal)         GOAL="$2"; shift 2 ;;
        --spawn-index)  SPAWN_INDEX="$2"; shift 2 ;;
        --no-auto)      NO_AUTO=true; shift ;;
        --no-gates)     NO_GATES=true; shift ;;
        --no-recover)   NO_RECOVER=true; shift ;;
        --with-rviz)    WITH_RVIZ=true; shift ;;
        --log-dir)      LOG_DIR="$2"; shift 2 ;;
        --with-display) WITH_DISPLAY=true; shift ;;
        --server-args)  SERVER_ARGS="$2"; shift 2 ;;
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

case "$STACK" in auto|source|docker) ;; *)
    echo "ERROR: --stack must be auto|source|docker (got: '$STACK')" >&2; exit 2 ;;
esac

case "$SERVER_PREF" in auto|editor) ;; *)
    echo "ERROR: --server must be auto|editor (got: '$SERVER_PREF')" >&2; exit 2 ;;
esac

[[ "$RPC_PORT" =~ ^[0-9]+$ ]] \
    || { echo "ERROR: --carla-rpc-port must be a number (got: '$RPC_PORT')" >&2; exit 2; }
[[ "$DOMAIN_ID" =~ ^[0-9]+$ ]] \
    || { echo "ERROR: --domain-id must be a number (got: '$DOMAIN_ID')" >&2; exit 2; }
[[ -z "$GOAL" || "$GOAL" =~ ^-?[0-9.]+,-?[0-9.]+,-?[0-9.]+$ ]] \
    || { echo "ERROR: --goal must be \"X,Y,YAW\" (CARLA meters,meters,degrees; got: '$GOAL')" >&2; exit 2; }
[[ -z "$SPAWN_INDEX" || "$SPAWN_INDEX" =~ ^[0-9]+$ ]] \
    || { echo "ERROR: --spawn-index must be a number (got: '$SPAWN_INDEX')" >&2; exit 2; }

# Prebuilt autoware-contents maps are named Town01..Town10HD (no _Opt suffix).
TOWN_BASE="${TOWN%_Opt}"
[[ -n "$MAP_PATH" ]] || MAP_PATH="$SCRIPT_DIR/../map_tools/maps/$TOWN_BASE"

PIDFILE="$LOG_DIR/carla_autoware.pids"
CONTAINERFILE="$LOG_DIR/carla_autoware.containers"
DDS_DIR="$LOG_DIR/dds"
FASTDDS_PROFILE="$DDS_DIR/fastdds_profile.xml"
CYCLONE_XML="$DDS_DIR/cyclonedds.xml"

# ---------------------------------------------------------------- helpers --
log()  { echo "[run_carla_autoware] $*"; }
warn() { echo "[run_carla_autoware] WARNING: $*" >&2; }
die()  { echo "[run_carla_autoware] ERROR: $*" >&2; exit 1; }

have() { command -v "$1" >/dev/null 2>&1; }

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

# Start one managed docker container (recorded for teardown; this script only
# ever removes containers it created itself).
#   start_container <name> <docker run args...>
start_container() {
    local name="$1"; shift
    if $DRY_RUN; then
        echo "[dry-run] docker run -d --name '$name' $*"
        return 0
    fi
    docker run -d --name "$name" "$@" >/dev/null
    echo "$name" >>"$CONTAINERFILE"
    log "started container '$name'"
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
    # Containers WE created (killing the docker-exec client above does not stop
    # the in-container launch; docker rm -f does, cleanly, without pkill).
    if [[ -f "$CONTAINERFILE" ]]; then
        local c
        while IFS= read -r c; do
            [[ -n "$c" ]] || continue
            log "  docker rm -f -> '$c'"
            docker rm -f "$c" >/dev/null 2>&1 || true
        done <"$CONTAINERFILE"
        rm -f "$CONTAINERFILE"
    fi
    log "logs are under: $LOG_DIR"
    exit "$status"
}
trap teardown INT TERM EXIT

# ------------------------------------------------- DDS bridge auto-detect --
# DDS must be pinned to a STABLE interface. WiFi/DHCP interfaces rotate IPs,
# which poisons DDS locators mid-session. The docker bridge is stable, so both
# sides pin it: fastdds (sim) whitelists its IP, cyclonedds (Autoware) binds
# its interface name.
detect_docker_bridge() {
    # 1. explicit interface
    if [[ -n "$BRIDGE_IF" && -z "$BRIDGE_IP" ]]; then
        BRIDGE_IP="$(ip -o -4 addr show dev "$BRIDGE_IF" 2>/dev/null | awk '{print $4}' | cut -d/ -f1 | head -1)"
        [[ -n "$BRIDGE_IP" ]] || preflight_fail "--bridge-if '$BRIDGE_IF' has no IPv4 address"
        return 0
    fi
    [[ -n "$BRIDGE_IF" ]] && return 0
    # 2. explicit docker network -> gateway IP + bridge interface name
    if [[ -n "$DOCKER_NETWORK" ]] && have docker; then
        local net_id
        if net_id="$(docker network inspect -f '{{.Id}}' "$DOCKER_NETWORK" 2>/dev/null)"; then
            BRIDGE_IP="$(docker network inspect -f '{{(index .IPAM.Config 0).Gateway}}' "$DOCKER_NETWORK" 2>/dev/null || true)"
            BRIDGE_IF="$(docker network inspect -f '{{index .Options "com.docker.network.bridge.name"}}' "$DOCKER_NETWORK" 2>/dev/null || true)"
            if [[ -z "$BRIDGE_IF" || "$BRIDGE_IF" == "<no value>" ]]; then
                if [[ "$DOCKER_NETWORK" == "bridge" ]]; then BRIDGE_IF="docker0"; else BRIDGE_IF="br-${net_id:0:12}"; fi
            fi
            [[ -n "$BRIDGE_IP" && -n "$BRIDGE_IF" ]] && return 0
        fi
        preflight_fail "could not resolve docker network '$DOCKER_NETWORK' to a bridge interface"
    fi
    # 3. fallback: scan for a docker bridge; prefer UP br-*, then UP docker0,
    #    then any bridge with an IPv4 (warn: a DOWN bridge cannot carry traffic).
    local line ifname state addr best_down_if="" best_down_ip=""
    while IFS= read -r line; do
        ifname="$(awk '{print $1}' <<<"$line")"
        state="$(awk '{print $2}' <<<"$line")"
        addr="$(awk '{for(i=3;i<=NF;i++) if ($i ~ /^[0-9]+\./) {print $i; exit}}' <<<"$line" | cut -d/ -f1)"
        [[ -n "$addr" ]] || continue
        case "$ifname" in br-*|docker0) ;; *) continue ;; esac
        if [[ "$state" == "UP" || "$state" == "UNKNOWN" ]]; then
            BRIDGE_IF="$ifname"; BRIDGE_IP="$addr"
            [[ "$ifname" == br-* ]] && return 0   # keep scanning only to prefer br-* over docker0
        elif [[ -z "$best_down_if" ]]; then
            best_down_if="$ifname"; best_down_ip="$addr"
        fi
    done < <(ip -br addr 2>/dev/null)
    [[ -n "$BRIDGE_IF" ]] && return 0
    if [[ -n "$best_down_if" ]]; then
        BRIDGE_IF="$best_down_if"; BRIDGE_IP="$best_down_ip"
        warn "only DOWN docker bridge '$BRIDGE_IF' found -- DDS cannot flow over a DOWN interface. Bring it up first: sudo ip link set $BRIDGE_IF up"
        return 0
    fi
    preflight_fail "no docker bridge interface found (need an UP br-*/docker0 with an IPv4; is the docker daemon running?). Override with --bridge-if / --docker-network."
    BRIDGE_IF="<bridge-if>"; BRIDGE_IP="<bridge-ip>"   # dry-run placeholders
}

# ------------------------------------------------- DDS config generation --
generate_dds_configs() {
    if $DRY_RUN; then
        echo "[dry-run] generate $FASTDDS_PROFILE  (UDPv4 whitelist: $BRIDGE_IP, useBuiltinTransports=false)"
        echo "[dry-run] generate $CYCLONE_XML  (NetworkInterface: $BRIDGE_IF, MaxAutoParticipantIndex=300, MaxMessageSize=65500B, rx buffers 10-64MB)"
        return 0
    fi
    mkdir -p "$DDS_DIR"
    # Sim side (Fast DDS): UDPv4 only, whitelisted to the docker bridge IP.
    cat >"$FASTDDS_PROFILE" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!-- Generated by run_carla_autoware.sh (do not edit; regenerated each run).
     Pins the CARLA server's Fast DDS to the docker bridge ($BRIDGE_IF), the
     only stable interface. Never whitelist a WiFi/DHCP address here. -->
<profiles xmlns="http://www.eprosima.com/XMLSchemas/fastRTPS_Profiles">
    <transport_descriptors>
        <transport_descriptor>
            <transport_id>udp_bridge_only</transport_id>
            <type>UDPv4</type>
            <interfaceWhiteList>
                <address>$BRIDGE_IP</address>
            </interfaceWhiteList>
        </transport_descriptor>
    </transport_descriptors>
    <participant profile_name="carla_bridge_only" is_default_profile="true">
        <rtps>
            <useBuiltinTransports>false</useBuiltinTransports>
            <userTransports>
                <transport_id>udp_bridge_only</transport_id>
            </userTransports>
        </rtps>
    </participant>
</profiles>
EOF
    # Autoware side (CycloneDDS): pinned to the bridge interface. Replaces the
    # config shipped in the official docker image (which pins 'lo' and breaks
    # discovery with the simulator).
    # MaxAutoParticipantIndex must cover the full stack (63+ nodes).
    # 10-64MB socket buffers are REQUIRED (validated 2026-08): at 4MB the
    # 20Hz lidar PointCloud2 stream overruns the receive sockets and
    # reliable-writer retransmit stalls freeze the whole stack. The kernel
    # rmem_max/wmem_max must be raised to match (checked below).
    cat >"$CYCLONE_XML" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!-- Generated by run_carla_autoware.sh (do not edit; regenerated each run). -->
<CycloneDDS xmlns="https://cdds.io/config">
    <Domain Id="any">
        <General>
            <Interfaces>
                <NetworkInterface name="$BRIDGE_IF" priority="default" multicast="default"/>
            </Interfaces>
            <AllowMulticast>default</AllowMulticast>
            <MaxMessageSize>65500B</MaxMessageSize>
        </General>
        <Discovery>
            <MaxAutoParticipantIndex>300</MaxAutoParticipantIndex>
        </Discovery>
        <Internal>
            <SocketReceiveBufferSize min="10MB" max="64MB"/>
            <Watermarks>
                <WhcHigh>500kB</WhcHigh>
            </Watermarks>
        </Internal>
    </Domain>
</CycloneDDS>
EOF
    log "generated DDS configs in $DDS_DIR (bridge: $BRIDGE_IF / $BRIDGE_IP)"
}

# ------------------------------------- CARLA-specific Autoware overrides --
# Container/workspace config changes the validated Town10 run needed:
#   1. ndt_scan_matcher.param.yaml:
#      converged_param_nearest_voxel_transformation_likelihood 2.3 -> 1.0
#      (UE4-era prebuilt pcd maps score low against UE5.8 geometry; regenerating
#      the pcd with map_tools removes the need for this).
#   2. tier4_localization_launch pose_twist_estimator.launch.xml:
#      stop_check_enabled -> false (the sim runs below real time; the
#      stopped-vehicle check never passes and initialization hangs).
#   3. diagnostics/autoware-carla.yaml: the /autoware/localization/state and
#      /adapi/mrm_request/delegate diag units get timeout 30.0 (validated
#      2026-08). These ADAPI topics publish at a low rate; at sub-realtime sim
#      speed the stock 3.0 s staleness window flaps ERROR and the MRM pulses
#      EMERGENCY_STOP, freezing the car mid-drive for no real reason.
#   4. planning/preset/default_preset.yaml: launch_traffic_light_module ->
#      false. The generated lanelet2 maps do not yet carry usable traffic
#      light regulatory elements + camera ROI projection is unverified; with
#      the module on, the car can wait forever at a light it cannot see.
OVERRIDE_SCRIPT="$LOG_DIR/apply_carla_overrides.sh"

write_override_script() {
    if $DRY_RUN; then
        echo "[dry-run] write $OVERRIDE_SCRIPT (NDT convergence likelihood -> 1.0; stop_check_enabled -> false; ADAPI diag timeouts -> 30.0; launch_traffic_light_module -> false; idempotent)"
        return 0
    fi
    # A generated file (fed to bash via stdin / docker exec -i) sidesteps the
    # quoting of nested bash -c strings entirely.
    cat >"$OVERRIDE_SCRIPT" <<'EOF'
#!/usr/bin/env bash
# Generated by run_carla_autoware.sh -- CARLA-specific Autoware overrides.
# Idempotent: re-running after the values are already patched is a no-op.
set -e
root="${OVERRIDE_ROOT:-/opt/autoware}"
ndt="$(find -L "$root" -name ndt_scan_matcher.param.yaml 2>/dev/null | head -1)"
if [ -n "$ndt" ]; then
    sed -E -i --follow-symlinks 's/(converged_param_nearest_voxel_transformation_likelihood:)[[:space:]]*[0-9.]+/\1 1.0/' "$ndt"
    echo "patched: $ndt (NDT convergence likelihood -> 1.0)"
else
    echo "WARNING: ndt_scan_matcher.param.yaml not found under $root" >&2
fi
ptw="$(find -L "$root" -path '*tier4_localization_launch*' -name pose_twist_estimator.launch.xml 2>/dev/null | head -1)"
if [ -n "$ptw" ]; then
    sed -E -i --follow-symlinks 's/(stop_check_enabled"[[:space:]]*(default|value)=")[^"]*(")/\1false\3/g' "$ptw"
    echo "patched: $ptw (stop_check_enabled -> false)"
else
    echo "WARNING: pose_twist_estimator.launch.xml not found under $root" >&2
fi
# ADAPI diag units publish at a low rate; at sub-realtime sim speed the stock
# 3.0 s staleness window flaps ERROR -> MRM EMERGENCY_STOP pulses. Bump ONLY
# the two ADAPI units to 30.0 (the sensor-rate units keep their 3.0).
diag="$(find -L "$root" -path '*autoware_launch*' -name autoware-carla.yaml 2>/dev/null | head -1)"
if [ -n "$diag" ]; then
    python3 - "$diag" <<'PYEOF'
import re, sys
path = sys.argv[1]
lines = open(path).read().splitlines(keepends=True)
targets = {"/autoware/localization/state", "/adapi/mrm_request/delegate"}
current = None
for i, ln in enumerate(lines):
    m = re.match(r"\s*- path: (\S+)", ln)
    if m:
        current = m.group(1)
    elif current in targets and re.match(r"\s*timeout:", ln):
        lines[i] = re.sub(r"timeout:\s*[0-9.]+", "timeout: 30.0", ln)
        current = None
open(path, "w").write("".join(lines))
print(f"patched: {path} (ADAPI diag timeouts -> 30.0)")
PYEOF
else
    echo "WARNING: diagnostics/autoware-carla.yaml not found under $root (older autoware_launch? MRM may flap at sub-realtime speed)" >&2
fi
# Generated lanelet2 maps have no usable traffic-light regulatory elements yet
# (and camera ROI projection is unverified) -- with the module on, the car can
# wait forever at a light it cannot see.
preset="$(find -L "$root" -path '*autoware_launch*' -name default_preset.yaml 2>/dev/null | head -1)"
if [ -n "$preset" ]; then
    python3 - "$preset" <<'PYEOF'
import re, sys
path = sys.argv[1]
lines = open(path).read().splitlines(keepends=True)
armed = False
for i, ln in enumerate(lines):
    if "launch_traffic_light_module" in ln:
        armed = True
    elif armed and re.match(r"\s*default:", ln):
        lines[i] = re.sub(r'default:\s*"?\w+"?', 'default: "false"', ln)
        break
open(path, "w").write("".join(lines))
print(f"patched: {path} (launch_traffic_light_module -> false)")
PYEOF
else
    echo "WARNING: planning preset default_preset.yaml not found under $root" >&2
fi
EOF
}

apply_carla_overrides() {
    local ctx="$1" root="$2"
    write_override_script
    if [[ "$ctx" == "docker" ]]; then
        run_fg apply_overrides "docker exec -i -e OVERRIDE_ROOT='$root' '$CONTAINER_NAME' bash <'$OVERRIDE_SCRIPT'"
    else
        run_fg apply_overrides "OVERRIDE_ROOT='$root' bash '$OVERRIDE_SCRIPT'"
    fi
}

# ------------------------------------------- Autoware-side ros2 CLI shim --
# Runs a ros2 CLI command in the same context (env + DDS config) as the stack.
AW_SETUP_SNIPPET='for s in /opt/autoware/setup.bash /opt/ros/*/setup.bash; do [ -f "$s" ] && source "$s" && break; done'
aw_ros2() {
    local cmd="$1"
    if [[ "$STACK" == "docker" && "$MODE" == "classical" ]]; then
        bash -c "docker exec '$CONTAINER_NAME' bash -c '$AW_SETUP_SNIPPET; $cmd'"
    else
        bash -c "${STACK_PRELUDE}$cmd"
    fi
}

# ---------------------------------------------------------------- preflight --
$DRY_RUN && log "DRY RUN -- nothing will be executed; preflight failures become warnings"

# Simulator RMW sanity
if [[ "$RMW" == "cyclonedds" ]]; then
    log "simulator RMW: cyclonedds (validated; fastdds is the default only by history)"
fi

# Resolve --stack auto (classical only; e2e always needs the source ws for VAD)
if [[ "$MODE" == "e2e" ]]; then
    [[ "$STACK" == "docker" ]] && die "--mode e2e requires --stack source (VAD lives in the source workspace)"
    STACK="source"
elif [[ "$STACK" == "auto" ]]; then
    if [[ -f "$AUTOWARE_WS/install/setup.bash" ]]; then
        STACK="source"
    elif have docker && docker image ls --format '{{.Repository}}:{{.Tag}}' 2>/dev/null | grep -q '^ghcr.io/autowarefoundation/autoware:universe'; then
        STACK="docker"
    else
        preflight_fail "no Autoware found: neither a source workspace ('$AUTOWARE_WS/install/setup.bash') nor a local ghcr.io/autowarefoundation/autoware:universe* docker image. Run $SCRIPT_DIR/../install/install_autoware.sh (--source or --docker) first, or pass --stack/--autoware-ws/--image."
        STACK="docker"   # dry-run: show the docker-flavored commands
    fi
fi
if [[ "$RMW" == "zenoh" && "$STACK" == "docker" ]]; then
    die "--rmw zenoh is only supported with --stack source"
fi

log "mode=$MODE town=$TOWN sim-rmw=$RMW ($RMW_IMPL) stack=$STACK domain-id=$DOMAIN_ID log-dir=$LOG_DIR"

# ros2 CLI (host side; needed for zenoh router and e2e glue nodes)
ROS_SETUP=""
if [[ -f "$AUTOWARE_WS/install/setup.bash" ]]; then
    ROS_SETUP="source '$AUTOWARE_WS/install/setup.bash'; "
elif [[ -f /opt/ros/jazzy/setup.bash ]]; then
    ROS_SETUP="source /opt/ros/jazzy/setup.bash; "
elif [[ -f /opt/ros/humble/setup.bash ]]; then
    ROS_SETUP="source /opt/ros/humble/setup.bash; "
fi
if ! command -v ros2 >/dev/null 2>&1 && [[ -z "$ROS_SETUP" ]] && [[ "$STACK" != "docker" ]]; then
    preflight_fail "ros2 not on PATH and no ROS underlay found (/opt/ros/{jazzy,humble}) -- source your ROS 2 environment first"
fi

# Autoware workspace / docker image
if [[ "$STACK" == "source" ]]; then
    if [[ ! -f "$AUTOWARE_WS/install/setup.bash" ]]; then
        preflight_fail "Autoware workspace not found at '$AUTOWARE_WS' (no install/setup.bash). Build one with: $SCRIPT_DIR/../install/install_autoware.sh  (or pass --autoware-ws / --stack docker)"
    fi
else
    have docker || preflight_fail "docker is required for --stack docker"
    if [[ -z "$IMAGE" ]]; then
        IMAGE="$(docker image ls --format '{{.Repository}}:{{.Tag}}' 2>/dev/null | grep '^ghcr.io/autowarefoundation/autoware:universe' | head -1 || true)"
        [[ -n "$IMAGE" ]] || { preflight_fail "no local ghcr.io/autowarefoundation/autoware:universe* image. Pull one with: $SCRIPT_DIR/../install/install_autoware.sh --docker  (or pass --image)"; IMAGE="ghcr.io/autowarefoundation/autoware:universe-cuda-jazzy"; }
    fi
    # Refuse to reuse an existing container: this script never touches
    # containers it did not create (a live demo may be running in one).
    if have docker && docker ps -a --format '{{.Names}}' 2>/dev/null | grep -qx "$CONTAINER_NAME"; then
        preflight_fail "a container named '$CONTAINER_NAME' already exists -- refusing to touch it. Pass a different --container-name (or remove it yourself if it is stale)."
    fi
fi

# Map triplet
for f in pointcloud_map.pcd lanelet2_map.osm map_projector_info.yaml; do
    if [[ ! -f "$MAP_PATH/$f" ]]; then
        preflight_fail "map dir '$MAP_PATH' is missing '$f'. Fetch the prebuilt maps with: $SCRIPT_DIR/../map_tools/fetch_prebuilt_maps.sh $TOWN_BASE '$MAP_PATH'  (or pass --map-path)"
        break   # one message is enough in dry-run
    fi
done

# The lanelet2 map MUST carry <MetaInfo format_version="1.0.0" map_version="1"/>;
# without it Autoware's route_handler rejects the map, planning silently never
# starts and mission_planner can segfault. Fix it in place (idempotent).
if [[ -f "$MAP_PATH/lanelet2_map.osm" ]] && ! grep -q "<MetaInfo" "$MAP_PATH/lanelet2_map.osm"; then
    if $DRY_RUN; then
        warn "'$MAP_PATH/lanelet2_map.osm' lacks the required MetaInfo element; would inject it via fetch_prebuilt_maps.sh --metainfo-only"
    else
        log "injecting missing MetaInfo element into $MAP_PATH/lanelet2_map.osm"
        "$SCRIPT_DIR/../map_tools/fetch_prebuilt_maps.sh" --metainfo-only "$MAP_PATH/lanelet2_map.osm"
    fi
fi

# DDS socket buffers: the 20Hz lidar PointCloud2 stream over reliable writers
# needs 10-64MB receive buffers (validated 2026-08). At the stock 4MB kernel
# cap the sockets overrun (RcvbufErrors), reliable-writer retransmits stall
# every DDS participant for seconds, and the stack MRM-stops the vehicle.
# Buffer size is fixed at socket creation, so raise the sysctl BEFORE starting
# anything, persist it, and restart every DDS process after changing it.
RMEM_MAX="$(cat /proc/sys/net/core/rmem_max 2>/dev/null || echo 0)"
WMEM_MAX="$(cat /proc/sys/net/core/wmem_max 2>/dev/null || echo 0)"
if [[ "$RMEM_MAX" -lt 67108864 || "$WMEM_MAX" -lt 67108864 ]]; then
    warn "net.core.rmem_max/wmem_max are $RMEM_MAX/$WMEM_MAX (<64MB): DDS sockets will overrun under lidar load and the stack will stall/MRM-stop."
    warn "Fix (and persist across reboots), then rerun:"
    warn "  sudo sh -c 'sysctl -w net.core.rmem_max=67108864 net.core.wmem_max=67108864 && printf \"net.core.rmem_max=67108864\\nnet.core.wmem_max=67108864\\n\" > /etc/sysctl.d/99-carla-dds.conf'"
fi

# CARLA server binary: packaged CarlaUnreal.sh preferred, editor -game fallback.
# Packaged layout on this branch: Build/<cfg>/Package/Carla-*-Linux-*/Linux/CarlaUnreal.sh
# --server editor skips package discovery: a stale package silently misses
# recent simulator-side fixes (the editor -game path always runs the current
# plugin binaries built into the repo tree).
SERVER_LAUNCHER=""
SERVER_KIND=""
CANDIDATES=()
if [[ "$SERVER_PREF" != "editor" ]]; then
    if [[ -n "$CARLA_ROOT_ARG" ]]; then
        CANDIDATES+=("$CARLA_ROOT_ARG/CarlaUnreal.sh" "$CARLA_ROOT_ARG/Linux/CarlaUnreal.sh")
    fi
    for cand in "$REPO_ROOT"/Build/*/Package/Carla-*/Linux/CarlaUnreal.sh; do
        CANDIDATES+=("$cand")   # unmatched globs stay literal and fail -x below
    done
fi
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
# Interpreter for the CARLA-client processes (town loader, ego spawner, VAD
# rig). The ROS side must run the scrubbed system python, but the CARLA wheel
# is often installed in a conda/venv python (matching whatever built it) --
# fall back to any python3 on the ORIGINAL path that can import carla.
CARLA_PY=""
if python3 -c 'import carla' >/dev/null 2>&1; then
    CARLA_PY="$(command -v python3)"
else
    while IFS= read -r dir; do
        [[ -x "$dir/python3" ]] || continue
        if "$dir/python3" -c 'import carla' >/dev/null 2>&1; then
            CARLA_PY="$dir/python3"
            warn "system python3 lacks the carla module; using '$CARLA_PY' for CARLA-client processes only (the Autoware stack still runs the clean system python). Install the CARLA wheel into system python to silence this."
            break
        fi
    done < <(printf '%s' "$ORIG_PATH" | tr ':' '\n')
fi
if [[ -z "$CARLA_PY" ]]; then
    preflight_fail "no python3 (scrubbed PATH or original PATH) can 'import carla' -- install the CARLA wheel (Build/*/PythonAPI/dist/*.whl) into a python environment"
    CARLA_PY="python3"   # dry-run placeholder
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

if ! $DRY_RUN; then
    mkdir -p "$LOG_DIR"
    : >"$PIDFILE"
    : >"$CONTAINERFILE"
fi

# ----------------------------------------------------- DDS configuration --
detect_docker_bridge
log "DDS bridge: $BRIDGE_IF ($BRIDGE_IP) -- fastdds whitelists the IP, cyclonedds pins the interface"
generate_dds_configs

# ----------------------------------------------------------- env preludes --
# Autoware/helper side. Validated topology: the stack runs CycloneDDS with the
# generated config regardless of the simulator RMW (mixed-RMW interop over
# UDPv4 is fine). Exception: zenoh runs zenoh end to end.
if [[ "$RMW" == "zenoh" ]]; then
    STACK_ENV="export RMW_IMPLEMENTATION=rmw_zenoh_cpp; export ROS_DOMAIN_ID=$DOMAIN_ID; "
else
    STACK_ENV="export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp; export CYCLONEDDS_URI='file://$CYCLONE_XML'; export ROS_DOMAIN_ID=$DOMAIN_ID; "
fi
STACK_PRELUDE="$ROS_SETUP$STACK_ENV"
# Source-stack runtime extras: autoware_path_optimizer links libacados.so
# from the acados prefix the installer set up (not on the default loader path).
if [[ "$STACK" == "source" && -d "$HOME/acados/lib" ]]; then
    STACK_PRELUDE+="export LD_LIBRARY_PATH=\"$HOME/acados/lib\${LD_LIBRARY_PATH:+:\$LD_LIBRARY_PATH}\"; "
fi

# Simulator side env (exported inside the server start command).
SIM_ENV=""
[[ "$RMW" == "fastdds" ]] && SIM_ENV="export FASTRTPS_DEFAULT_PROFILES_FILE='$FASTDDS_PROFILE'; "
[[ "$RMW" == "cyclonedds" ]] && SIM_ENV="export CYCLONEDDS_URI='file://$CYCLONE_XML'; "

# ------------------------------------------------------------ 0. zenoh rtr --
if [[ "$RMW" == "zenoh" ]]; then
    # rmw_zenoh needs its router up BEFORE any zenoh peer (CARLA server included).
    start_proc zenoh_router "${STACK_PRELUDE}exec ros2 run rmw_zenoh_cpp rmw_zenohd"
    pause 3 "let the zenoh router come up before starting peers"
fi

# ---------------------------------------------------------- 1. CARLA server --
SERVER_FLAGS="-ros2 -rmw=$RMW -carla-rpc-port=$RPC_PORT -ros-domain-id=$DOMAIN_ID"
$WITH_DISPLAY || SERVER_FLAGS+=" -RenderOffScreen"
if [[ -n "$SERVER_ARGS" ]]; then
    # Reject a whitespace-only value the same way an unbalanced quote is
    # rejected below, rather than letting it through to xargs/mapfile,
    # which would turn it into one spurious empty argv token.
    [[ "$SERVER_ARGS" =~ [^[:space:]] ]] \
        || die "--server-args: value is empty or whitespace-only"
    # Tokenize --server-args with xargs: it honors shell-style quoting
    # (so a value containing a space can be kept as one token) but, unlike
    # eval or an unquoted expansion, never performs variable/command
    # substitution -- a metacharacter in the input stays inert text.
    SERVER_ARGS_LINES="$(xargs -n1 printf '%s\n' <<<"$SERVER_ARGS")" \
        || die "--server-args: unbalanced quoting in '$SERVER_ARGS'"
    SERVER_ARGS_ARR=()
    mapfile -t SERVER_ARGS_ARR <<<"$SERVER_ARGS_LINES"
    # Re-quote each token with %q: start_proc() below re-parses the whole
    # command string through `bash -c`, so each token must round-trip
    # through that second parse as the single literal argument it is.
    SERVER_FLAGS+="$(printf ' %q' "${SERVER_ARGS_ARR[@]}")"
fi

if [[ "$SERVER_KIND" == "packaged" ]]; then
    start_proc carla_server "${SIM_ENV}exec '$SERVER_LAUNCHER' $SERVER_FLAGS"
else
    start_proc carla_server "${SIM_ENV}exec '${UE_ROOT:-\$UE_ROOT}/Engine/Binaries/Linux/UnrealEditor' '$UPROJECT' -game $SERVER_FLAGS"
fi
wait_for_carla_rpc

# --------------------------------------------------------- 2. load the town --
# One-shot, non-ticking client; runs BEFORE the sync-mode ticking client exists.
run_fg load_town "'$CARLA_PY' -c \"
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
# --hz_rate 20 --resync is the validated configuration (20 Hz fixed step,
# sim clock resynced on startup).
# e2e default spawn: VAD's navigation command is a fixed LANE_FOLLOW (no route
# input exists in the integration), so only looping roads sustain a demo --
# spawn 52 sits on the Town10 outer ring. A dead-end spawn (e.g. 41's road,
# which T-terminates) wedges the car at the road end.
if [[ "$MODE" == "e2e" && -z "$SPAWN_INDEX" ]]; then
    SPAWN_INDEX=52
    log "e2e: defaulting to ring spawn --spawn-index 52 (LANE_FOLLOW needs a looping road)"
fi
start_proc autoware_demo "exec '$CARLA_PY' '$AUTOWARE_DEMO' --host $CARLA_HOST --port $RPC_PORT --hz_rate 20 --resync${SPAWN_INDEX:+ --spawn_index $SPAWN_INDEX}"
pause 5 "let autoware_demo.py spawn the ego before attaching more sensors"

# ------------------------------------------------- 4+5. e2e-only glue procs --
if [[ "$MODE" == "e2e" ]]; then
    # Six VAD cameras (1600x900, nuScenes-style rig) on /sensing/camera/CAM_*/image_raw.
    # spawn_vad_rig.py never ticks; it attaches to the ego spawned by autoware_demo.py.
    start_proc vad_rig "exec '$CARLA_PY' '$SCRIPT_DIR/spawn_vad_rig.py' --host $CARLA_HOST --port $RPC_PORT"
    pause 3 "let the camera rig attach and enable ROS publishing"

    # carla_state_publisher + autoware_vehicle_velocity_converter + autoware_twist2accel
    # (ground-truth /localization/kinematic_state + /localization/acceleration)
    # + image_transport republish (raw -> compressed) for the six cameras.
    start_proc e2e_state_publishers "${STACK_PRELUDE}exec ros2 launch '$SCRIPT_DIR/e2e_state_publishers.launch.py'"

    # Drive keeper: collision sensor on the ego (the ONLY ground truth for
    # driving quality -- position/velocity alone cannot see contacts: the
    # wheel-speed VelocityReport keeps reading 4-7 m/s while the car is
    # pinned) + wedge detection + (unless --no-recover) teleport recovery so
    # the LANE_FOLLOW demo keeps looping. Never ticks.
    KEEPER_ARGS=""
    $NO_RECOVER && KEEPER_ARGS=" --no-recover"
    start_proc drive_keeper "exec '$CARLA_PY' '$SCRIPT_DIR/e2e_drive_keeper.py' --host $CARLA_HOST --port $RPC_PORT$KEEPER_ARGS"
fi

# ------------------------------------------------------- 6. Autoware launch --
# simulator_type:=carla is REQUIRED (validated 2026-08): it selects the
# autoware-carla.yaml diagnostic profile, which drops the routing/state
# staleness check. The default awsim profile flaps ERROR on low-rate ADAPI
# topics whenever the sim runs below real time, and the MRM then pulses
# EMERGENCY_STOP -- the car freezes mid-drive with nothing actually wrong.
# launch_simulator_interface:=false because this branch's simulator publishes
# the vehicle/sensor topics natively (no external interface node needed).
LAUNCH_ARGS="vehicle_model:=sample_vehicle sensor_model:=awsim_sensor_kit perception_mode:=lidar rviz:=false simulator_type:=carla launch_simulator_interface:=false"
CLASSICAL_SRC_CMD="${STACK_PRELUDE}exec ros2 launch autoware_launch e2e_simulator.launch.xml $LAUNCH_ARGS map_path:='$MAP_PATH'"
# simulator_type:=carla + launch_simulator_interface:=false for the same
# reasons as classical (CARLA diag profile; native topics need no interface
# node); the launch maps sensor_model to carla_sensor_kit for carla itself.
#
# rviz image panel: the stock autoware.rviz points its visible image panel at
# /perception/traffic_light_recognition/.../debug/rois, which does not exist
# in e2e mode (no perception stack) -- the panel stays black. Generate a copy
# repointed at the raw front VAD camera. Best-effort: if the ws config or the
# expected topic line is missing, fall back to the stock config.
E2E_RVIZ_ARG=""
if [[ "$MODE" == "e2e" ]] && ! $DRY_RUN; then
    STOCK_RVIZ="$(find "$AUTOWARE_WS/install" -path '*autoware_launch*' -name autoware.rviz 2>/dev/null | head -1)"
    if [[ -n "$STOCK_RVIZ" ]] && grep -q 'Value: /perception/traffic_light_recognition/traffic_light/debug/rois' "$STOCK_RVIZ"; then
        sed -e 's|Value: /perception/traffic_light_recognition/traffic_light/debug/rois|Value: /sensing/camera/CAM_FRONT/image_raw/image|' \
            -e 's|Name: RecognitionResultOnImage|Name: FrontCamera|' \
            "$STOCK_RVIZ" > "$LOG_DIR/vad_e2e.rviz"
        E2E_RVIZ_ARG=" rviz_config:='$LOG_DIR/vad_e2e.rviz'"
        log "e2e rviz: image panel repointed to /sensing/camera/CAM_FRONT/image_raw/image ($LOG_DIR/vad_e2e.rviz)"
    else
        warn "could not generate the e2e rviz config (stock autoware.rviz or its traffic-light image panel not found) -- rviz image panel will be black"
    fi
fi
E2E_PR_CMD="${STACK_PRELUDE}exec ros2 launch autoware_launch e2e_simulator.launch.xml vehicle_model:=sample_vehicle sensor_model:=awsim_sensor_kit simulator_type:=carla launch_simulator_interface:=false map_path:='$MAP_PATH' use_e2e_planning:=true e2e_planning_type:=vad$E2E_RVIZ_ARG"
E2E_FALLBACK_CMD="${STACK_PRELUDE}exec ros2 launch autoware_tensorrt_vad vad_carla_tiny.launch.xml sensing:=false localization:=false perception:=false"

if [[ "$MODE" == "classical" ]]; then
    if [[ "$STACK" == "docker" ]]; then
        GPU_ARGS=()
        have nvidia-smi && GPU_ARGS=(--gpus all)
        # Idle container first (host networking; DDS pinned to the bridge IF on
        # the host). The launch is a separate docker exec so the container is a
        # stable target for overrides / automation / rviz.
        start_container "$CONTAINER_NAME" \
            --network host "${GPU_ARGS[@]}" \
            -e RMW_IMPLEMENTATION=rmw_cyclonedds_cpp \
            -e CYCLONEDDS_URI=file:///dds/cyclonedds.xml \
            -e ROS_DOMAIN_ID="$DOMAIN_ID" \
            -v "$DDS_DIR":/dds:ro \
            -v "$MAP_PATH":"/maps/$TOWN_BASE":ro \
            -v "$HOME/autoware_data":/root/autoware_data \
            --entrypoint bash "$IMAGE" -c 'sleep infinity'
        apply_carla_overrides docker /opt/autoware
        start_proc autoware "exec docker exec '$CONTAINER_NAME' bash -c '$AW_SETUP_SNIPPET; exec ros2 launch autoware_launch e2e_simulator.launch.xml $LAUNCH_ARGS map_path:=/maps/$TOWN_BASE'"
        if $WITH_RVIZ; then
            # rviz MUST render on the GPU. On llvmpipe (software GL) it burns
            # 4+ cores rendering the pointcloud, starves the sim and DDS, and
            # the whole stack destabilizes (validated failure mode 2026-08).
            RVIZ_GPU_ENV=()
            have nvidia-smi && RVIZ_GPU_ENV=(-e __GLX_VENDOR_LIBRARY_NAME=nvidia -e __NV_PRIME_RENDER_OFFLOAD=1)
            # X auth: a fresh X session (e.g. after a reboot) does not authorize
            # container clients -- rviz then dies with "could not connect to
            # display". Harmless no-op if already authorized.
            if have xhost && [[ -n "${DISPLAY:-}" ]] && ! $DRY_RUN; then
                xhost +local: >/dev/null 2>&1 || warn "xhost +local: failed -- rviz may not reach the X display"
            fi
            start_container "$CONTAINER_NAME-rviz" \
                --network host "${GPU_ARGS[@]}" "${RVIZ_GPU_ENV[@]}" \
                -e RMW_IMPLEMENTATION=rmw_cyclonedds_cpp \
                -e CYCLONEDDS_URI=file:///dds/cyclonedds.xml \
                -e ROS_DOMAIN_ID="$DOMAIN_ID" \
                -e DISPLAY="${DISPLAY:-:0}" \
                -v /tmp/.X11-unix:/tmp/.X11-unix \
                -v "$DDS_DIR":/dds:ro \
                --entrypoint bash "$IMAGE" \
                -c "$AW_SETUP_SNIPPET; exec rviz2 -d /opt/autoware/autoware_launch/share/autoware_launch/rviz/autoware.rviz"
            log "rviz container started (if the window does not appear, run: xhost +local:)"
            if ! $DRY_RUN; then
                ( sleep 20
                  if docker logs "$CONTAINER_NAME-rviz" 2>&1 | grep -qi llvmpipe; then
                      warn "rviz is SOFTWARE-RENDERING (llvmpipe): it will starve the machine and destabilize the stack. Fix NVIDIA GL in containers (nvidia-container-toolkit) before relying on this session."
                  fi ) &
            fi
        fi
    else
        apply_carla_overrides source "$AUTOWARE_WS/install"
        start_proc autoware "$CLASSICAL_SRC_CMD"
        if $WITH_RVIZ; then
            # PRIME render offload: on hybrid-GPU hosts rviz otherwise lands on
            # llvmpipe/iGPU and starves the machine (see the docker path above).
            RVIZ_ENV=""
            have nvidia-smi && RVIZ_ENV="__NV_PRIME_RENDER_OFFLOAD=1 __GLX_VENDOR_LIBRARY_NAME=nvidia "
            start_proc rviz "${STACK_PRELUDE}RVIZ_CFG=\$(find '$AUTOWARE_WS/install' -path '*autoware_launch*' -name autoware.rviz 2>/dev/null | head -1); exec env ${RVIZ_ENV}rviz2 \${RVIZ_CFG:+-d \"\$RVIZ_CFG\"}"
        fi
    fi
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

# --------------------------------------- 7. classical post-launch driving --
# Sequence validated on Town10: empty InitializeLocalization auto-inits from
# GNSS; goal goes to /planning/mission_planning/goal in the MAP frame
# (map frame = CARLA x, NEGATED y, negated yaw); then change_to_autonomous.
# The resulting trajectory appears on /planning/trajectory (current Autoware;
# NOT the old /planning/scenario_planning/trajectory).
wait_for_autoware_api() {
    if $DRY_RUN; then
        echo "[dry-run] wait (up to 300 s) until 'ros2 service list' on the stack side shows /api/operation_mode/change_to_autonomous"
        return 0
    fi
    log "waiting for the Autoware ADAPI to come up (up to 300 s) ..."
    local i
    for i in $(seq 1 60); do
        # grep runs INSIDE aw_ros2: this script sets pipefail, and an outer
        # `| grep -q` exits at the (alphabetically early) match, SIGPIPEs the
        # still-writing `ros2 service list` (exit 141), and fails the pipeline
        # -- the loop then never succeeds even though the service is up.
        if aw_ros2 "ros2 service list 2>/dev/null | grep -q '/api/operation_mode/change_to_autonomous'"; then
            log "Autoware ADAPI is up (after ~$((i * 5))s)"
            return 0
        fi
        sleep 5
    done
    warn "Autoware ADAPI did not appear within 300 s -- skipping automation (see $LOG_DIR/autoware.log)"
    return 1
}

auto_step() {   # run one automation ros2 command on the stack side
    local desc="$1" cmd="$2"
    if $DRY_RUN; then
        echo "[dry-run] $desc:"
        echo "[dry-run]   (stack-side) $cmd"
        return 0
    fi
    log "$desc"
    aw_ros2 "$cmd" 2>&1 | tee -a "$LOG_DIR/automation.log"
}

# ----------------------------------------------------- pre-engage gates --
# Validated 2026-08: NEVER engage on a diverged pose. NDT can report converged
# while the believed pose sits meters from the vehicle (the car then drives
# into whatever the divergence points it at). Gate 1 compares Autoware's
# /localization/kinematic_state against CARLA ground truth (base_link is the
# REAR AXLE: CARLA's center transform is shifted back half the sample_vehicle
# wheelbase, 1.425 m). Gate 2 listens to /diagnostics for distortion-corrector
# errors (a mismatched per-point time_stamp silently disables the corrector
# and NDT then degrades at speed).
GATE1_PY="$LOG_DIR/engage_gate1.py"
GATE2_PY="$LOG_DIR/engage_gate2.py"

write_gate_scripts() {
    cat >"$GATE1_PY" <<'EOF'
#!/usr/bin/env python3
# Generated by run_carla_autoware.sh -- engage gate 1: localization truth check.
# argv: <carla-host> <carla-rpc-port>; stdin: one `ros2 topic echo --once
# /localization/kinematic_state` dump. Exit 0 pass, 1 diverged, 2 no data yet.
import math
import sys

import carla

HALF_WHEELBASE = 1.425  # sample_vehicle: Autoware base_link is the REAR AXLE

client = carla.Client(sys.argv[1], int(sys.argv[2]))
client.set_timeout(20.0)
world = client.get_world()
# A fresh client's actor registry stays empty in sync mode until it has seen
# a tick (fork issue); one wait_for_tick populates it deterministically.
try:
    world.wait_for_tick(10.0)
except RuntimeError:
    pass
vehicles = [a for a in world.get_actors() if a.type_id.startswith("vehicle.")]
if not vehicles:
    print("GATE1: no vehicle in the actor registry (yet)")
    sys.exit(2)
tf = vehicles[0].get_transform()
gt_x, gt_y, gt_yaw = tf.location.x, -tf.location.y, -math.radians(tf.rotation.yaw)
gt_x -= HALF_WHEELBASE * math.cos(gt_yaw)
gt_y -= HALF_WHEELBASE * math.sin(gt_yaw)

vals, section = {}, None
for line in sys.stdin.read().splitlines():
    s = line.strip()
    if s.startswith("position:"):
        section = "p"
    elif s.startswith("orientation:"):
        section = "o"
    elif section and ":" in s:
        k, _, v = s.partition(":")
        if k in ("x", "y", "z", "w"):
            try:
                vals[section + k] = float(v)
            except ValueError:
                pass
            if section == "o" and k == "w":
                section = None
if "px" not in vals or "ow" not in vals:
    print("GATE1: no kinematic_state sample")
    sys.exit(2)
aw_x, aw_y = vals["px"], vals["py"]
aw_yaw = math.atan2(2 * vals["ow"] * vals["oz"], 1 - 2 * vals["oz"] ** 2)
dp = math.hypot(aw_x - gt_x, aw_y - gt_y)
dyaw = abs((aw_yaw - gt_yaw + math.pi) % (2 * math.pi) - math.pi)
print(f"GATE1: truth ({gt_x:.2f}, {gt_y:.2f}) yaw {math.degrees(gt_yaw):.1f} deg"
      f" | belief ({aw_x:.2f}, {aw_y:.2f}) yaw {math.degrees(aw_yaw):.1f} deg"
      f" | delta {dp:.2f} m, {math.degrees(dyaw):.1f} deg")
sys.exit(0 if dp <= 0.5 and dyaw <= math.radians(5) else 1)
EOF
    cat >"$GATE2_PY" <<'EOF'
#!/usr/bin/env python3
# Generated by run_carla_autoware.sh -- engage gate 2: distortion corrector
# health. Listens to /diagnostics for 12 s; any ERROR-level status mentioning
# "distortion" fails the gate. Exit 0 clean, 1 errors seen.
import time

import rclpy
from diagnostic_msgs.msg import DiagnosticArray
from rclpy.node import Node

rclpy.init()
node = Node("carla_engage_gate2")
hits = []

def cb(msg):
    for status in msg.status:
        level = status.level if isinstance(status.level, int) else ord(status.level)
        if level >= 2 and "distortion" in status.name:
            hits.append(f"{status.name}: {status.message}")

node.create_subscription(DiagnosticArray, "/diagnostics", cb, 10)
deadline = time.time() + 12.0
while time.time() < deadline:
    rclpy.spin_once(node, timeout_sec=0.5)
if hits:
    print("GATE2: distortion corrector reporting errors: " + hits[0][:120])
    raise SystemExit(1)
print("GATE2: distortion corrector clean")
EOF
}

run_stack_py() {   # feed a python script (stdin) to an interpreter in the stack's env
    local file="$1"
    if [[ "$STACK" == "docker" && "$MODE" == "classical" ]]; then
        docker exec -i "$CONTAINER_NAME" bash -c "$AW_SETUP_SNIPPET; exec timeout 40 python3 -" <"$file"
    else
        bash -c "${STACK_PRELUDE}exec timeout 40 python3 -" <"$file"
    fi
}

run_engage_gates() {
    if $DRY_RUN; then
        echo "[dry-run] engage gates: compare /localization/kinematic_state vs CARLA ground truth (<=0.5 m, <=5 deg, rear-axle offset), then 12 s /diagnostics watch for distortion-corrector errors"
        return 0
    fi
    write_gate_scripts
    local attempt belief rc gate1_ok=false
    for attempt in $(seq 1 6); do
        belief="$(aw_ros2 "timeout 15 ros2 topic echo --once /localization/kinematic_state 2>/dev/null" || true)"
        rc=0
        printf '%s\n' "$belief" | "$CARLA_PY" "$GATE1_PY" "$CARLA_HOST" "$RPC_PORT" | tee -a "$LOG_DIR/automation.log" || rc=$?
        if [[ $rc -eq 0 ]]; then
            gate1_ok=true
            break
        fi
        log "gate 1 not passing yet (attempt $attempt/6), retrying in 8 s ..."
        sleep 8
    done
    if ! $gate1_ok; then
        warn "GATE 1 FAILED: Autoware's believed pose does not match CARLA ground truth. Engaging now would drive the car into things."
        return 1
    fi
    local g2rc=0
    run_stack_py "$GATE2_PY" | tee -a "$LOG_DIR/automation.log" || g2rc=$?
    if [[ $g2rc -ne 0 ]]; then
        warn "GATE 2 FAILED: pointcloud distortion corrector is reporting errors -- NDT will degrade once the car moves."
        return 1
    fi
    log "engage gates PASSED"
    return 0
}

if [[ "$MODE" == "classical" ]] && ! $NO_AUTO; then
    if wait_for_autoware_api; then
        # GNSS pose race: right after startup the initializer may not have a
        # GNSS fix yet and returns success=False -- retry, don't give up.
        if $DRY_RUN; then
            auto_step "initializing localization (empty request = auto-init from GNSS; retried up to 6x every 8 s until success=True)" \
                "ros2 service call /api/localization/initialize autoware_adapi_v1_msgs/srv/InitializeLocalization {}"
        else
            INIT_OK=false
            for _ in $(seq 1 6); do
                if aw_ros2 "ros2 service call /api/localization/initialize autoware_adapi_v1_msgs/srv/InitializeLocalization {}" 2>&1 \
                        | tee -a "$LOG_DIR/automation.log" | grep -q "success=True"; then
                    INIT_OK=true
                    log "localization initialized"
                    break
                fi
                log "localization initialize not accepted yet (GNSS pose race?), retrying in 8 s ..."
                sleep 8
            done
            $INIT_OK || warn "localization initialize never returned success=True -- continuing, but the engage gates will likely fail"
        fi
        pause 20 "let NDT localization converge"
        GATES_OK=true
        if ! $NO_GATES; then
            run_engage_gates || GATES_OK=false
        fi
        if ! $GATES_OK; then
            warn "pre-engage gates failed -- NOT sending goal / engaging. Inspect $LOG_DIR/automation.log and $LOG_DIR/autoware.log, then drive manually or re-run (--no-gates overrides, at your own risk)."
        elif [[ -n "$GOAL" ]]; then
            # CARLA -> Autoware map frame: x_map = x, y_map = -y, yaw_map = -yaw.
            read -r GX GY GQZ GQW <<<"$(python3 -c "
import math
x, y, yaw = (float(v) for v in '$GOAL'.split(','))
r = math.radians(-yaw)
print(f'{x:.3f} {-y:.3f} {math.sin(r / 2.0):.6f} {math.cos(r / 2.0):.6f}')
")"
            # NB: double quotes around the YAML -- the docker path of aw_ros2
            # wraps the command in single quotes.
            auto_step "publishing goal (CARLA '$GOAL' -> map x=$GX y=$GY qz=$GQZ qw=$GQW)" \
                "ros2 topic pub --once /planning/mission_planning/goal geometry_msgs/msg/PoseStamped \"{header: {frame_id: map}, pose: {position: {x: $GX, y: $GY, z: 0.0}, orientation: {z: $GQZ, w: $GQW}}}\""
            if $DRY_RUN; then
                echo "[dry-run] then: retry 'ros2 service call /api/operation_mode/change_to_autonomous autoware_adapi_v1_msgs/srv/ChangeOperationMode {}' (stack-side, up to 12x every 10 s until success=True)"
            else
                ENGAGED=false
                for _ in $(seq 1 12); do
                    if aw_ros2 "ros2 service call /api/operation_mode/change_to_autonomous autoware_adapi_v1_msgs/srv/ChangeOperationMode {}" 2>&1 \
                            | tee -a "$LOG_DIR/automation.log" | grep -q "success=True"; then
                        ENGAGED=true
                        log "autonomous mode engaged -- trajectory on /planning/trajectory"
                        break
                    fi
                    log "change_to_autonomous not accepted yet (route/planning not ready?), retrying in 10 s ..."
                    sleep 10
                done
                $ENGAGED || warn "could not engage autonomous mode after 12 attempts -- check /planning/mission_planning/route_state and $LOG_DIR/autoware.log"
            fi
        else
            log "no --goal given. To drive: set a goal in RViz, or re-run with --goal \"x,y,yaw\" (CARLA coords),"
            log "then engage via: ros2 service call /api/operation_mode/change_to_autonomous autoware_adapi_v1_msgs/srv/ChangeOperationMode {}"
        fi
    fi
fi

# ------------------------------------------------ 7b. e2e post-launch engage --
# e2e needs no goal (VAD's command input is a fixed LANE_FOLLOW); the only
# post-launch step is change_to_autonomous once VAD is up. The engage would
# be accepted even before VAD publishes (the vehicle_cmd_gate holds stop until
# control is valid), but waiting for the init line keeps the logs honest.
if [[ "$MODE" == "e2e" ]] && ! $NO_AUTO; then
    if wait_for_autoware_api; then
        if $DRY_RUN; then
            echo "[dry-run] wait for 'VAD model and interface initialized successfully' in $LOG_DIR/autoware.log (cached engines: ~1 min; a FIRST run builds TensorRT engines -- tens of minutes)"
            echo "[dry-run] then: retry 'ros2 service call /api/operation_mode/change_to_autonomous autoware_adapi_v1_msgs/srv/ChangeOperationMode {}' (stack-side, up to 12x every 10 s until success=True)"
        else
            log "waiting for VAD init (cached engines: ~1 min; a FIRST run builds TensorRT engines and can take tens of minutes) ..."
            VAD_OK=false
            for _ in $(seq 1 360); do
                if grep -aq "VAD model and interface initialized successfully" "$LOG_DIR/autoware.log" 2>/dev/null; then
                    VAD_OK=true
                    break
                fi
                sleep 5
            done
            $VAD_OK || warn "VAD init line not seen within 30 min -- attempting engage anyway"
            ENGAGED=false
            for _ in $(seq 1 12); do
                if aw_ros2 "ros2 service call /api/operation_mode/change_to_autonomous autoware_adapi_v1_msgs/srv/ChangeOperationMode {}" 2>&1 \
                        | tee -a "$LOG_DIR/automation.log" | grep -q "success=True"; then
                    ENGAGED=true
                    log "autonomous mode engaged -- VAD is driving (collisions/wedges/recoveries in $LOG_DIR/drive_keeper.log)"
                    break
                fi
                log "change_to_autonomous not accepted yet, retrying in 10 s ..."
                sleep 10
            done
            $ENGAGED || warn "could not engage autonomous mode after 12 attempts -- check $LOG_DIR/autoware.log, then engage manually"
        fi
    fi
fi

# --------------------------------------------------------------- foreground --
if $DRY_RUN; then
    echo "[dry-run] would then: wait on the 'autoware' PID; Ctrl-C / exit triggers teardown"
    echo "[dry-run] pidfile:    $PIDFILE   (stop later with: $SCRIPT_DIR/stop_all.sh --log-dir '$LOG_DIR')"
    echo "[dry-run] containers: $CONTAINERFILE"
    echo "[dry-run] logs dir:   $LOG_DIR"
    exit 0
fi

log "all components started. Ctrl-C stops everything. Stop from elsewhere with:"
log "  $SCRIPT_DIR/stop_all.sh --log-dir '$LOG_DIR'"
AUTOWARE_PID="$(awk '$1=="autoware"{print $2}' "$PIDFILE")"
wait "$AUTOWARE_PID"
