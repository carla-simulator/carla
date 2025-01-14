ARG UBUNTU_DISTRO="22.04"

FROM ubuntu:${UBUNTU_DISTRO}

RUN packages='libsdl2-2.0 xserver-xorg libvulkan1 libomp5' \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y $packages \
    && rm -rf /var/lib/apt/lists/*

# Install the `xdg-user-dir` tool so the Unreal Engine can use it to locate the user's Documents directory
RUN packages='xdg-user-dirs' \
    && apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y $packages \
    && rm -rf /var/lib/apt/lists/*

RUN useradd -m carla

COPY --chown=carla:carla . /home/carla

USER carla
WORKDIR /home/carla

ENV SDL_VIDEODRIVER="x11"

# You can also run CARLA in offscreen mode with -RenderOffScreen
CMD ["/bin/bash", "CarlaUnreal.sh"]
