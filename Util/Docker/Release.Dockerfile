
# run the docker container as:
#
# sudo -E docker run --rm --gpus all -it --net=host carla:latest /bin/bash

FROM nvidia/vulkan:1.1.121-cuda-10.1--ubuntu18.04

RUN apt-key adv --fetch-keys "https://developer.download.nvidia.com/compute/cuda/repos/ubuntu1804/x86_64/3bf863cc.pub"

RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install --no-install-recommends --yes \
        libomp5 \
        libsdl2-2.0 \
        libvulkan1 \
        xserver-xorg && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/* /var/cache/apt/archives

RUN useradd -m carla

COPY --chown=carla:carla . /home/carla

USER carla
WORKDIR /home/carla

# you can also run CARLA in offscreen mode with -RenderOffScreen
# CMD /bin/bash CarlaUE4.sh -RenderOffScreen
CMD /bin/bash CarlaUE4.sh
