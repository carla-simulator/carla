#!/usr/bin/env bash
# Inspect and visualise a ROS-2-enabled CARLA server from containers — no local
# ROS 2 installation involved.
#
# Modes:
#   topics                 ros2 topic list        (the fastest "is ROS alive" check)
#   info TOPIC             ros2 topic info -v     (type + publisher/subscriber count + QoS)
#   echo TOPIC [N]         ros2 topic echo --once (N messages, default 1)
#   hz TOPIC [SECONDS]     ros2 topic hz          (rate; kill after SECONDS, default 10)
#   demo [EXTRA...]        the in-checkout map+lidar demo stack (SPAWNS ACTORS)
#   rviz                   RViz2 with the bundled preset (needs a display)
#   local-env              print the exports a LOCAL ros2 install needs (eval them)
#
# Topic names: pass the ROS name (/clock), not the DDS name (rt/clock).
#
# The read-only modes (topics/info/echo/hz) run in the RViz base image, which
# carries the ros2 CLI. `demo` and `rviz` delegate to the checkout's own scripts
# so this skill never forks their logic.
#
# Knobs (env.sh): ROS_DISTRO_TAG, RMW, ROS_DOMAIN_ID, CARLA_HOST, CARLA_PORT.
set -euo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck disable=SC1091
source "${HERE}/env.sh"

MODE="${1:-topics}"; shift || true

[ -d "${CARLA_ROS2_EXAMPLES}" ] \
  || { echo "[ros] ERROR: ${CARLA_ROS2_EXAMPLES} not found — wrong CARLA_UE4_ROOT, or a checkout without the ROS 2 demo." >&2; exit 1; }
command -v docker >/dev/null 2>&1 \
  || { echo "[ros] ERROR: docker is required (the ros2 CLI runs in a container here)." >&2; exit 1; }

# Build the RViz base image via the checkout's own script if it is missing. That
# script builds the base image as a side effect of the rviz mode, so for the
# read-only modes we build it directly from the same Dockerfile.
ensure_base_image() {
  docker image inspect "${CARLA_RVIZ_IMAGE}" >/dev/null 2>&1 && return 0
  echo "[ros] building ${CARLA_RVIZ_IMAGE} (first use; minutes, needs network)..."
  # Same two build args and tag the checkout's run_rviz.sh uses, so both produce
  # the identical image (the arg is RMW_IMPLEMENTATION, not RMW).
  docker build \
    --build-arg ROS_DISTRO="${ROS_DISTRO_TAG}" \
    --build-arg RMW_IMPLEMENTATION="${RMW_IMPLEMENTATION}" \
    --file "${CARLA_ROS2_EXAMPLES}/Dockerfile" \
    --tag "${CARLA_RVIZ_IMAGE}" \
    "${CARLA_ROS2_EXAMPLES}"
}

# The RMW config files the containers expect, mounted read-only at /config.
# Without them discovery can silently fail for cyclonedds/fastdds.
rmw_env() {
  case "${RMW}" in
    cyclonedds) printf '%s' "--env=CYCLONEDDS_URI=/config/cyclonedds.xml" ;;
    fastdds)    printf '%s' "--env=FASTRTPS_DEFAULT_PROFILES_FILE=/config/fastrtps-profile.xml" ;;
    *)          printf '' ;;
  esac
}

# One-shot ros2 CLI call in the base image. --net=host so DDS discovery sees the
# server's traffic; the domain must match what the server was started with.
ros2_cli() {
  ensure_base_image
  local extra; extra="$(rmw_env)"
  docker run --rm --init --net=host \
    --env="RMW_IMPLEMENTATION=${RMW_IMPLEMENTATION}" \
    ${ROS_DOMAIN_ID:+--env=ROS_DOMAIN_ID=${ROS_DOMAIN_ID}} \
    ${extra:+"${extra}"} \
    --volume="${CARLA_ROS2_EXAMPLES}/config:/config:ro" \
    "${CARLA_RVIZ_IMAGE}" \
    "$@"
}

case "${MODE}" in
  topics)
    echo "[ros] ros2 topic list (domain ${ROS_DOMAIN_ID:-0}, ${RMW_IMPLEMENTATION})"
    # Discovery is not instant: a fresh participant needs a moment to match the
    # server's publishers, so an empty first list is not proof of absence.
    OUT="$(ros2_cli bash -lc 'sleep 3; ros2 topic list' || true)"
    printf '%s\n' "${OUT}"
    if ! printf '%s' "${OUT}" | grep -q '/clock'; then
      echo "[ros] /clock is MISSING — the server is not publishing." >&2
      echo "[ros] Check, in order: built with ROS 2 (build-carla-ue4 ROS2=1)," >&2
      echo "[ros]   started with --ros2 (run-carla-server ROS2=1)," >&2
      echo "[ros]   same ROS_DOMAIN_ID on both sides, same --rmw," >&2
      echo "[ros]   and for zenoh: a running rmw_zenohd router." >&2
      exit 1
    fi
    echo "[ros] OK: /clock present — the native interface is live."
    ;;
  info)
    [ $# -ge 1 ] || { echo "[ros] usage: ros_view.sh info /clock" >&2; exit 2; }
    ros2_cli ros2 topic info -v "$1"
    ;;
  echo)
    [ $# -ge 1 ] || { echo "[ros] usage: ros_view.sh echo /carla/map [N]" >&2; exit 2; }
    N="${2:-1}"
    # NOTE: /carla/map exists on 0.9.x only — CarlaMapPublisher was dropped in
    # 0.10.0, where this echo waits forever because nothing advertises the topic.
    # Latched topics (only /carla/map today) publish ONCE per episode, so a
    # default VOLATILE subscription waits forever for a sample that will not come.
    # Request transient_local for them, and --full-length so the OpenDRIVE string
    # is not truncated at ros2's default 128 chars. Both verified against a live
    # server.
    QOS=()
    case "$1" in
      /carla/map|rt/carla/map)
        QOS=(--qos-durability transient_local --qos-reliability reliable --full-length)
        echo "[ros] latched topic: requesting transient_local + full length" >&2
        ;;
    esac
    # --once for a single sample; a count needs the plain echo bounded by timeout,
    # because `ros2 topic echo` has no "n messages then exit" option.
    if [ "${N}" = "1" ]; then
      ros2_cli ros2 topic echo --once "${QOS[@]}" "$1"
    else
      ros2_cli bash -lc "timeout 30 ros2 topic echo ${QOS[*]} '$1' | head -n $((N * 40))"
    fi
    ;;
  local-env)
    # For driving a LOCAL ros2 install instead of these containers. Without the
    # RMW profile, discovery works and NO DATA arrives — verified; that profile
    # forces UDP-only, matching CARLA's shared-memory-built Fast DDS.
    echo "# eval \"\$(bash scripts/ros_view.sh local-env)\" in a shell that has ROS 2"
    echo "set +u   # ROS setup.bash dereferences AMENT_TRACE_SETUP_FILES unguarded"
    echo "source /opt/ros/${ROS_DISTRO_TAG}/setup.bash"
    echo "export RMW_IMPLEMENTATION=${RMW_IMPLEMENTATION}"
    case "${RMW}" in
      fastdds)    echo "export FASTRTPS_DEFAULT_PROFILES_FILE=${CARLA_ROS2_EXAMPLES}/config/fastrtps-profile.xml" ;;
      cyclonedds) echo "export CYCLONEDDS_URI=file://${CARLA_ROS2_EXAMPLES}/config/cyclonedds.xml" ;;
      zenoh)      echo "# zenoh: no profile file; start the router first: ros2 run rmw_zenoh_cpp rmw_zenohd" ;;
    esac
    [ -n "${ROS_DOMAIN_ID:-}" ] && echo "export ROS_DOMAIN_ID=${ROS_DOMAIN_ID}"
    exit 0
    ;;
  hz)
    [ $# -ge 1 ] || { echo "[ros] usage: ros_view.sh hz /carla/hero/lidar/point_cloud [SECONDS]" >&2; exit 2; }
    SEC="${2:-10}"
    ros2_cli bash -lc "timeout ${SEC} ros2 topic hz '$1' || true"
    ;;
  demo)
    # SIDE EFFECTS: this spawns a hero vehicle with four sensors on the running
    # server and drives it on autopilot. Only for an explicit request.
    echo "[ros] running the in-checkout demo stack — it SPAWNS a hero vehicle + sensors."
    exec "${CARLA_ROS2_EXAMPLES}/run_map_and_lidar_demo.sh" \
      "--distro=${ROS_DISTRO_TAG}" "--rmw=${RMW}" \
      "--host=${CARLA_HOST}" "--port=${CARLA_PORT}" "$@"
    ;;
  rviz)
    [ -n "${DISPLAY:-}" ] || { echo "[ros] ERROR: no DISPLAY — RViz needs an X display." >&2; exit 1; }
    exec "${CARLA_ROS2_EXAMPLES}/run_rviz.sh" \
      "--distro=${ROS_DISTRO_TAG}" "--rmw=${RMW}" \
      ${ROS_DOMAIN_ID:+--ros-domain-id=${ROS_DOMAIN_ID}} "$@"
    ;;
  *)
    echo "[ros] ERROR: unknown mode '${MODE}' (topics | info | echo | hz | demo | rviz | local-env)" >&2
    exit 2
    ;;
esac
