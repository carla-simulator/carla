FROM carla-prerequisites:latest

ARG GIT_BRANCH=master
ARG GIT_REPO=https://github.com/carla-simulator/carla.git

USER carla
WORKDIR /home/carla

RUN cd /home/carla && \
  git clone --depth 1 --branch $GIT_BRANCH "$GIT_REPO" && \
  cd /home/carla/carla && \
  ./Update.sh && \
  make CarlaUE4Editor && \
  make PythonAPI && \
  make build.utils && \
  make package && \
  rm -r /home/carla/carla/Dist

WORKDIR /home/carla/carla
