FROM carla-prerequisites:latest

ARG GIT_BRANCH

USER ue4

RUN cd /home/ue4 && \
  if [ -z ${GIT_BRANCH+x} ]; then git clone --depth 1 https://github.com/carla-simulator/carla.git; \
  else git clone --depth 1 --branch $GIT_BRANCH https://github.com/carla-simulator/carla.git; fi

COPY --chown=ue4:ue4 Package.sh /home/ue4/carla/Util/BuildTools/Package.sh

RUN cd /home/ue4/carla && ./Update.sh
RUN cd /home/ue4/carla && make CarlaUE4Editor
RUN cd /home/ue4/carla && make PythonAPI
RUN cd /home/ue4/carla && make build.utils
RUN chmod 777 /home/ue4/carla/Util/BuildTools/Package.sh
RUN bash
RUN cd /home/ue4/carla && make package
RUN cd /home/ue4/carla && rm -r /home/ue4/carla/Dist

WORKDIR /home/ue4/carla
