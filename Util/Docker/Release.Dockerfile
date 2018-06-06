# Make sure drivers are >= 390
# sudo docker run -p 2000-2002:2000-2002 --runtime=nvidia -e NVIDIA_VISIBLE_DEVICES=ID carla:latest ./CarlaUE4.sh
# /Game/Maps/Town01 -benchmark -carla-server -fps=10 -world-port=2000 -windowed -ResX=100 -ResY=100 -carla-no-hud

FROM nvidia/opengl:1.0-glvnd-runtime-ubuntu16.04


RUN apt-get update; apt-get install -y libsdl2-2.0

RUN useradd -m carla
USER carla
ENV HOME /home/carla
COPY . /home/carla

USER root
RUN chown -R carla:carla /home/carla

RUN apt-get -y install sudo
RUN echo "carla:carla" | chpasswd && adduser carla sudo

USER carla
WORKDIR /home/carla


ENV SDL_VIDEODRIVER=offscreen

CMD /bin/bash CarlaUE4.sh -carla-server
