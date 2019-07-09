# Make sure drivers are >= 390
# docker run -p 2000-2002:2000-2002 --runtime=nvidia -e NVIDIA_VISIBLE_DEVICES=ID carla:latest ./CarlaUE4.sh
# -carla-rpc-port=2000 -carla-streaming-port=2001

FROM nvidia/opengl:1.0-glvnd-runtime-ubuntu16.04

RUN packages='libsdl2-2.0' \
	&& apt-get update && apt-get install -y $packages --no-install-recommends \
	&& rm -rf /var/lib/apt/lists/*

RUN useradd -m carla

COPY --chown=carla:carla . /home/carla

USER carla
WORKDIR /home/carla

ENV SDL_VIDEODRIVER=offscreen

CMD /bin/bash CarlaUE4.sh
