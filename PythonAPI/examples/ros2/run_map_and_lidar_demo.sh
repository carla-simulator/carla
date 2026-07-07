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

# --- Argument parsing ---
usage() {
    cat <<EOF
Usage: $0 [--distro=<distro>] [--rmw=<middleware>] [--host=<host>] [--port=<port>]
          [--waypoint-distance=<meters>] [--map-only]

Runs the ROS2 demo stack in Docker against a CARLA server started with --ros2:
  * ros2_native.py        spawns the hero vehicle with camera/lidar/gnss/imu on autopilot
  * map_to_markers.py     converts the latched /carla/map OpenDRIVE into lane markers

Options:
  --distro              ROS 2 distribution to use. Supported: humble, jazzy  (default: humble)
  --rmw                 RMW implementation to use. Supported: fastdds, cyclonedds, zenoh  (default: fastdds)
  --host                IP of the host CARLA Simulator  (default: localhost)
  --port                TCP port of CARLA Simulator  (default: 2000)
  --waypoint-distance   Distance in meters between sampled lane points  (default: 2.0)
  --map-only            Only publish the map markers, do not spawn the vehicle stack

Examples:
  $0
  $0 --port=3654
  $0 --distro=jazzy --rmw=cyclonedds --map-only
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
if ! docker image inspect "${IMAGE_NAME}" &>/dev/null; then
    "${SCRIPT_DIR}/map_and_lidar_demo/build.sh" --distro="${DISTRO}" --rmw="${RMW}"
fi

# --- RMW-specific environment variables ---
EXTRA_ENV=()
if [ "$RMW" = "cyclonedds" ]; then
    EXTRA_ENV+=(--env="CYCLONEDDS_URI=/config/cyclonedds.xml")
elif [ "$RMW" = "fastdds" ]; then
    EXTRA_ENV+=(--env="FASTRTPS_DEFAULT_PROFILES_FILE=/config/fastrtps-profile.xml")
fi

# --- Run ---
echo "[demo] Launching (distro=${DISTRO}, rmw=${RMW}, server=${HOST}:${PORT}, map-only=${MAP_ONLY})..."
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
