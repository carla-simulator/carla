#docker run -it -v ~/programing:/home --privileged --rm --gpus all -it --net=host -e DISPLAY=:0.GPU_ID -v /tmp/.X11-unix:/tmp/.X11-unix:rw carla-ubuntu-22.04 /bin/bash -vulkan
CARLA_ROOT=$PWD/..
WORKSPACE=$CARLA_ROOT/..
# TODO: Add Unreal engine path as optional argument
# TODO: runthis command on start: git config --global --add safe.directory /home/xsole/programing/carlaUE5_cmake
docker run -it \
    --workdir $CARLA_ROOT \
    -e CARLA_UNREAL_ENGINE_PATH=$WORKSPACE/UnrealEngine5_carla \
    -v $WORKSPACE:$WORKSPACE \
    carla-ubuntu-22.04 \
    /bin/bash
