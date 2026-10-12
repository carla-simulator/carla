#!/usr/bin/env bash
# Drive CARLA's shipped Autoware integration on UE 5.8 (ue58-dev).
#
#   autoware.sh probe                 what the CARLA side offers, and is it built for it
#   autoware.sh topics                the DDS contract: what CARLA publishes / subscribes
#   autoware.sh sensors               Autoware-specific blueprints on a RUNNING server
#   autoware.sh maps   --town NAME    generate lanelet2 + pointcloud map artifacts
#   autoware.sh demo   [args...]      run autoware_demo.py (the CARLA-side driver)
#   autoware.sh stack  --mode e2e     run run_carla_autoware.sh (DRY RUN unless --go)
#
# CARLA and Autoware talk over DDS with NO bridge process: the server publishes
# vehicle status and sensors, and subscribes to Autoware's control commands. That
# only exists if the server was BUILT with -DENABLE_ROS2=ON and STARTED with the
# single-dash -ros2 flag (see run-carla-ue58-server).
set -uo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh" >/dev/null
set +e

AV="${CARLA_UE58_ROOT}/PythonAPI/examples/av_stacks/autoware"
DEMO="${CARLA_UE58_ROOT}/PythonAPI/examples/autoware_demo.py"
LAUNCH="${AV}/run/run_carla_autoware.sh"

die(){ echo "[autoware] ERROR $*" >&2; exit 1; }
say(){ echo "[autoware] $*"; }

cmd_probe() {
  say "checkout   ${CARLA_UE58_ROOT} (branch $(carla_ue58_branch))"
  if carla_ue58_configured; then
    local r2; r2="$(carla_ue58_cmake_opt ENABLE_ROS2)"
    say "ENABLE_ROS2 = ${r2:-<unset>}"
    case "${r2}" in
      ON|on|1|TRUE|True) ;;
      *) say "  WARNING without ROS 2 compiled in there is no DDS interface at all" ;;
    esac
  else
    say "tree not configured — run cmake --preset ${CARLA_PRESET}"
  fi

  # The native pieces, straight from the source tree: these are what make the
  # integration bridge-free.
  say "native ROS 2 pieces for Autoware:"
  local ros2="${CARLA_UE58_ROOT}/LibCarla/source/carla/ros2"
  for f in publishers/AutowareGNSSPublisher.cpp \
           publishers/AutowareVehicleStatusPublisher.cpp \
           subscribers/AutowareControlSubscriber.cpp \
           AutowareSteeringCompensation.h; do
    [ -f "${ros2}/${f}" ] && say "  present ${f}" || say "  MISSING ${f}"
  done

  say "shipped integration:"
  for f in "${DEMO}" "${LAUNCH}" "${AV}/run/spawn_vad_rig.py" \
           "${AV}/map_tools/generate_map_artifacts.py" "${AV}/install/install_autoware.sh"; do
    [ -e "${f}" ] && say "  present ${f#${CARLA_UE58_ROOT}/}" \
                  || say "  MISSING ${f#${CARLA_UE58_ROOT}/}"
  done

  say "Autoware itself (external):"
  local aws="${AUTOWARE_WS:-${HOME}/autoware}"
  [ -d "${aws}/install" ] && say "  source workspace ${aws}" \
                          || say "  no source workspace at ${aws}"
  if command -v docker >/dev/null 2>&1; then
    local img; img="$(docker images --format '{{.Repository}}:{{.Tag}}' 2>/dev/null | grep -m1 autoware)"
    [ -n "${img}" ] && say "  docker image ${img}" || say "  no local autoware image"
  fi
  say "map artifacts:"
  local any=0
  for d in "${AV}/map_tools/maps"/*; do
    [ -d "${d}" ] || continue
    if [ -f "${d}/lanelet2_map.osm" ] && [ -f "${d}/pointcloud_map.pcd" ]; then
      say "  $(basename "${d}") ready"; any=1
    else
      say "  $(basename "${d}") INCOMPLETE"
    fi
  done
  [ "${any}" -eq 1 ] || say "  none generated yet (autoware.sh maps --town Town10HD_Opt)"
}

cmd_topics() {
  # Read from the publishers/subscriber constructors rather than restating them,
  # so this cannot drift from the build you actually have.
  local ros2="${CARLA_UE58_ROOT}/LibCarla/source/carla/ros2"
  echo "CARLA -> Autoware (published by the server):"
  grep -hoE '"rt/[a-z_/]+"' "${ros2}/publishers/AutowareVehicleStatusPublisher.cpp" 2>/dev/null \
    | tr -d '"' | sort -u | sed 's/^/  /'
  echo "  (+ the GNSS publisher on the sensor's own base topic, from sensor.other.autoware_gnss)"
  echo
  echo "Autoware -> CARLA (subscribed by the server, drives the ego):"
  grep -hoE '"rt/[a-z_/]+"' "${ros2}/subscribers/AutowareControlSubscriber.cpp" 2>/dev/null \
    | tr -d '"' | sort -u | sed 's/^/  /'
  echo
  echo "QoS, from the same constructors:"
  echo "  vehicle status  RELIABLE / VOLATILE / KEEP_LAST 1"
  echo "  control command RELIABLE / TRANSIENT_LOCAL / KEEP_LAST 1 (latched: a late"
  echo "                  joiner still sees the last command)"
  echo
  echo "A ROS 2 node sees these without the 'rt/' prefix (rt/vehicle/status ->"
  echo "/vehicle/status). Nothing appears unless the server runs with -ros2."
}

cmd_sensors() {
  "${PYTHON}" - "$@" <<'PY'
import os, sys
try:
    import carla
except Exception as exc:
    sys.exit(f"[autoware] cannot import carla: {exc}")
host = os.environ.get("CARLA_HOST", "127.0.0.1")
port = int(os.environ.get("CARLA_PORT", 2000))
try:
    c = carla.Client(host, port); c.set_timeout(20.0)
    lib = c.get_world().get_blueprint_library()
    print(f"[autoware] server {c.get_server_version()} at {host}:{port}")
except Exception:
    sys.exit(f"[autoware] no server at {host}:{port} — start one with run-carla-ue58-server")
# The two blueprints that exist only for the Autoware port, plus the sensors the
# reference rig actually spawns.
wanted = ["sensor.other.autoware_gnss", "sensor.other.vehicle_status",
          "sensor.other.gnss", "sensor.other.imu",
          "sensor.lidar.ray_cast", "sensor.camera.rgb"]
for bid in wanted:
    try:
        bp = lib.find(bid)
    except Exception:
        print(f"  MISSING {bid}")
        continue
    attrs = sorted(a.id for a in bp)
    ros = [a for a in attrs if a.startswith("ros")]
    print(f"  {bid:36} {len(attrs)} attrs   ros: {', '.join(ros) or 'none'}")
print("\n[autoware] ros_name sets the topic segment; ros_topic_name overrides the")
print("[autoware] whole topic exactly — that is how the rig lands on Autoware's names.")
PY
}

cmd_maps() {
  local gen="${AV}/map_tools/generate_map_artifacts.py"
  [ -f "${gen}" ] || die "missing ${gen}"
  say "Autoware does not read CARLA's .xodr; it needs lanelet2_map.osm +"
  say "pointcloud_map.pcd + map_projector_info.yaml. Generating those now."
  say "running: ${PYTHON} ${gen} $*"
  "${PYTHON}" "${gen}" "$@"
}

cmd_demo() {
  [ -f "${DEMO}" ] || die "missing ${DEMO}"
  say "autoware_demo.py is the CARLA-side driver: it owns world settings and the"
  say "ego, ticks the world, and spawns the sensor rig. Run it BEFORE the stack."
  say "running: ${PYTHON} ${DEMO} $*"
  "${PYTHON}" "${DEMO}" "$@"
}

cmd_stack() {
  [ -f "${LAUNCH}" ] || die "missing ${LAUNCH}"
  local go=0 args=()
  for a in "$@"; do
    case "${a}" in
      --go) go=1 ;;
      *) args+=("${a}") ;;
    esac
  done
  if [ "${go}" -eq 0 ]; then
    say "DRY RUN (pass --go to actually launch). This starts long-lived processes"
    say "and, with --stack docker, a container; that is why it is opt-in."
    args+=(--dry-run)
  fi
  say "running: bash ${LAUNCH} ${args[*]}"
  bash "${LAUNCH}" "${args[@]}"
}

MODE="${1:-}"; shift 2>/dev/null
case "${MODE}" in
  probe)   cmd_probe "$@" ;;
  topics)  cmd_topics "$@" ;;
  sensors) cmd_sensors "$@" ;;
  maps)    cmd_maps "$@" ;;
  demo)    cmd_demo "$@" ;;
  stack)   cmd_stack "$@" ;;
  *) sed -n '2,15p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//' ; exit 2 ;;
esac
