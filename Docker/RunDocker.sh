#docker run -it -v ~/programing:/home --privileged --rm --gpus all -it --net=host -e DISPLAY=:0.GPU_ID -v /tmp/.X11-unix:/tmp/.X11-unix:rw carla-ubuntu-22.04 /bin/bash -vulkan
docker run -it -v ~/programing:/home carla-ubuntu-22.04 /bin/bash
