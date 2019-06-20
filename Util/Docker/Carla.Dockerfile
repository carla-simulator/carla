FROM carla-prerequisites:latest

USER ue4

RUN cd /home/ue4 && \
  git clone --depth=1 https://github.com/carla-simulator/carla.git && \
  cd /home/ue4/carla && \
  ./Update.sh && \
  make CarlaUE4Editor && \
  make package && \
  rm -r /home/ue4/carla/Dist

WORKDIR /home/ue4/carla
