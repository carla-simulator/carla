#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# --- Defaults ---
DISTRO="humble"
RMW="fastdds"
HOST="localhost"
PORT="2000"
WAYPOINT_DISTANCE="2.0"
MAP_ONLY="0"
WHEEL=""
ROS_DOMAIN_ID_ARG="${ROS_DOMAIN_ID:-}"

# --- Argument parsing ---
usage() {
    cat <<EOF
Usage: $0 [--distro=<distro>] [--rmw=<middleware>] [--host=<host>] [--port=<port>]
          [--waypoint-distance=<meters>] [--wheel=<path>] [--ros-domain-id=<N>] [--map-only]

Runs the ROS2 demo stack in Docker against a CARLA server started with --ros2:
  * ros2_native.py        spawns the hero vehicle with camera/lidar/gnss/imu on autopilot
  * map_to_markers.py     converts the latched /carla/map OpenDRIVE into lane markers
  * ego_tf_broadcaster.py broadcasts the map->hero TF so map and sensors compose in RViz

Options:
  --distro              ROS 2 distribution to use. Supported: humble, jazzy  (default: humble)
  --rmw                 RMW implementation to use. Supported: fastdds, cyclonedds, zenoh  (default: fastdds)
  --host                IP of the host CARLA Simulator  (default: localhost)
  --port                TCP port of CARLA Simulator  (default: 2000)
  --waypoint-distance   Distance in meters between sampled lane points  (default: 2.0)
  --wheel               CARLA wheel built for the selected ROS Docker image
  --ros-domain-id       ROS 2 domain id (0-232). Must match the CARLA server
                        and RViz. Defaults to ROS_DOMAIN_ID when exported.
  --map-only            Only publish the map markers, do not spawn the vehicle stack

Examples:
  $0 --distro=humble --rmw=fastdds --wheel=/path/to/carla-*-cp310-*.whl
  $0 --wheel=/path/to/carla-*-cp310-*.whl --ros-domain-id=42
EOF
    exit 1
}

for arg in "$@"; do
    case "$arg" in
        --distro=*)            DISTRO="${arg#*=}" ;;
        --rmw=*)               RMW="${arg#*=}" ;;
        --host=*)              HOST="${arg#*=}" ;;
        --port=*)              PORT="${arg#*=}" ;;
        --waypoint-distance=*) WAYPOINT_DISTANCE="${arg#*=}" ;;
        --wheel=*)             WHEEL="${arg#*=}" ;;
        --ros-domain-id=*)     ROS_DOMAIN_ID_ARG="${arg#*=}" ;;
        --map-only)            MAP_ONLY="1" ;;
        --help|-h)             usage ;;
        *) echo "Unknown argument: $arg"; usage ;;
    esac
done

# --- Validate ---
case "$DISTRO" in
    humble|jazzy) ;;
    *) echo "Unsupported distro '${DISTRO}'. Supported values: humble, jazzy"; exit 1 ;;
esac

case "$RMW" in
    fastdds|cyclonedds|zenoh) ;;
    *) echo "Unsupported RMW '${RMW}'. Supported values: fastdds, cyclonedds, zenoh"; exit 1 ;;
esac

if [ -n "${ROS_DOMAIN_ID_ARG}" ]; then
    if ! [[ "${ROS_DOMAIN_ID_ARG}" =~ ^[0-9]+$ ]] || [ "${ROS_DOMAIN_ID_ARG}" -lt 0 ] || [ "${ROS_DOMAIN_ID_ARG}" -gt 232 ]; then
        echo "Invalid ROS domain id '${ROS_DOMAIN_ID_ARG}'. Must be an integer in the range 0-232."
        exit 1
    fi
fi

# Map short names to ROS RMW implementation identifiers
if [ "$RMW" = "cyclonedds" ]; then
    RMW_IMPLEMENTATION="rmw_cyclonedds_cpp"
elif [ "$RMW" = "zenoh" ]; then
    RMW_IMPLEMENTATION="rmw_zenoh_cpp"
else
    RMW_IMPLEMENTATION="rmw_fastrtps_cpp"
fi

IMAGE_NAME="carla-map-and-lidar-demo-${DISTRO}-${RMW}"

# --- Build ---
# Delegate each launch to build.sh so Docker can invalidate its cache when the
# scripts, Dockerfile, or selected runtime wheel changes.
BUILD_ARGS=(--distro="${DISTRO}" --rmw="${RMW}")
if [ -n "${WHEEL}" ]; then
    BUILD_ARGS+=(--wheel="${WHEEL}")
fi
"${SCRIPT_DIR}/map_and_lidar_demo/build.sh" "${BUILD_ARGS[@]}"

# --- RMW-specific environment variables ---
EXTRA_ENV=()
if [ "$RMW" = "cyclonedds" ]; then
    EXTRA_ENV+=(--env="CYCLONEDDS_URI=/config/cyclonedds.xml")
elif [ "$RMW" = "fastdds" ]; then
    EXTRA_ENV+=(--env="FASTRTPS_DEFAULT_PROFILES_FILE=/config/fastrtps-profile.xml")
fi
if [ -n "${ROS_DOMAIN_ID_ARG}" ]; then
    EXTRA_ENV+=(--env="ROS_DOMAIN_ID=${ROS_DOMAIN_ID_ARG}")
fi

# --- Run ---
echo "[demo] Launching (distro=${DISTRO}, rmw=${RMW}, server=${HOST}:${PORT}, ros-domain-id=${ROS_DOMAIN_ID_ARG:-default}, map-only=${MAP_ONLY})..."
# The fixed container name makes a second concurrent run fail fast instead of
# spawning a duplicate vehicle publishing on the same topics.
docker run \
    --rm \
    --init \
    --net=host \
    --name="${IMAGE_NAME}" \
    --stop-timeout=30 \
    --env="RMW_IMPLEMENTATION=${RMW_IMPLEMENTATION}" \
    --env="CARLA_HOST=${HOST}" \
    --env="CARLA_PORT=${PORT}" \
    --env="WAYPOINT_DISTANCE=${WAYPOINT_DISTANCE}" \
    --env="MAP_ONLY=${MAP_ONLY}" \
    "${EXTRA_ENV[@]}" \
    --volume="${SCRIPT_DIR}/config:/config:ro" \
    "${IMAGE_NAME}"
