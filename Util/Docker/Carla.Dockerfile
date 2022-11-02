FROM carla-prerequisites:latest

ARG GIT_BRANCH

USER ue4

RUN cd /home/ue4 && \
  if [ -z ${GIT_BRANCH+x} ]; then git clone --depth 1 https://github.com/carla-simulator/carla.git; \
  else git clone --depth 1 --branch $GIT_BRANCH https://github.com/carla-simulator/carla.git; fi && \
  cd /home/ue4/carla && \
  ./Update.sh && \
  make CarlaUE4Editor && \
  make PythonAPI && \
  make build.utils && \
  make package && \
  rm -r /home/ue4/carla/Dist

WORKDIR /home/ue4/carla
