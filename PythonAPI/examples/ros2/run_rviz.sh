#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

function transfer_x11_permissions() {
    # store X11 access rights in temp file to be passed into docker container
    XAUTH=/tmp/.docker.xauth
    touch $XAUTH
    xauth nlist $DISPLAY | sed -e 's/^..../ffff/' | xauth -f $XAUTH nmerge -
}

transfer_x11_permissions
docker run \
    --rm \
    --net=host \
    --env="DISPLAY=$DISPLAY" \
    --env="XAUTHORITY=$XAUTH" \
    --env="FASTRTPS_DEFAULT_PROFILES_FILE=/config/fastrtps-profile.xml" \
    --volume="${SCRIPT_DIR}/config:/config:ro" \
    --volume="${SCRIPT_DIR}/rviz:/rviz:rw" \
    --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" \
    --volume="$XAUTH:$XAUTH" \
    osrf/ros:humble-desktop \
    ros2 run rviz2 rviz2 -d /rviz/ros2_native.rviz
