FROM carla-prerequisites:latest

USER ue4

RUN cd /home/ue4 && \
  git clone --depth=1 https://github.com/carla-simulator/carla.git && \
  cd /home/ue4/ue4 && \
  ./Update.sh && \
  make CarlaUE4Editor

WORKDIR /home/ue4/carla
