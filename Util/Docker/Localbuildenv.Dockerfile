FROM carla-prerequisites:latest
USER root
RUN apt-key adv --fetch-keys \
    https://developer.download.nvidia.com/compute/cuda/repos/ubuntu1804/x86_64/3bf863cc.pub && \
    apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
        libsdl2-2.0 xserver-xorg libvulkan1 libomp5 --no-install-recommends && \
    apt-get install -y \
        xdg-user-dirs xdg-utils && \
    apt-get clean
USER carla
WORKDIR /home/carla/carla
