#!/bin/bash

# Entry point of the carla-map-and-lidar-demo-<distro>-<rmw> Docker image (see
# run_map_and_lidar_demo.sh). Launches the demo helpers and stops them all together when
# the first one exits or the container receives INT/TERM.

set -uo pipefail

CARLA_HOST="${CARLA_HOST:-localhost}"
CARLA_PORT="${CARLA_PORT:-2000}"
WAYPOINT_DISTANCE="${WAYPOINT_DISTANCE:-2.0}"
MAP_ONLY="${MAP_ONLY:-0}"
DEMO_ROLE_NAME="${DEMO_ROLE_NAME:-hero_carla_map_and_lidar_demo}"

pids=()

# A container killed before ros2_native.py can run its finally block may leave
# its actors behind.  The stack uses an owner-qualified hero role, so this is
# safe to run before every launch and cannot delete another client's ``hero``.
python3 /opt/carla/cleanup.py --host "${CARLA_HOST}" --port "${CARLA_PORT}" \
    --role-name "${DEMO_ROLE_NAME}"

python3 /opt/carla/map_to_markers.py --waypoint-distance "${WAYPOINT_DISTANCE}" &
pids+=($!)

if [ "${MAP_ONLY}" != "1" ]; then
    python3 /opt/carla/ego_tf_broadcaster.py --host "${CARLA_HOST}" --port "${CARLA_PORT}" \
        --role-name "${DEMO_ROLE_NAME}" --frame hero &
    pids+=($!)
    # Request synchronous operation, but ros2_native.py only takes ownership
    # when the world is currently asynchronous.  If generate_traffic.py is
    # already the synchronous master, the controller follows its clock.
    python3 /opt/carla/ros2_native.py --sync --file /opt/carla/stack.json --host "${CARLA_HOST}" --port "${CARLA_PORT}" &
    pids+=($!)
fi

# Stop the helpers with SIGTERM: background jobs of a non-interactive shell
# start with SIGINT ignored, so python never installs its KeyboardInterrupt
# handler and a SIGINT would be dropped. SIGTERM reaches all of them and lets
# ros2_native.py run its cleanup (destroy sensors and vehicle, restore world
# settings). Send it only once: a second signal would land inside that
# cleanup and abort it, leaking the actors.
stopped=0
stop() {
    if [ "${stopped}" = "0" ]; then
        stopped=1
        kill -TERM "${pids[@]}" 2>/dev/null || true
    fi
}
trap stop INT TERM

wait -n || true
echo "[demo] A helper exited, stopping the stack..."
stop
wait || true

# ros2_native.py tracks and destroys only the actors it created.  The next
# launch removes leftovers by the private role above if an unclean stop ever
# prevented this normal cleanup path from running.
