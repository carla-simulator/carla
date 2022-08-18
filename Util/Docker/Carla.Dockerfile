
# use the prerequisites image with a pre-installed
# dev toolchain and pre-build UE4 tools from source
# as environment for the build stage
FROM carla-prerequisites:latest as build-env

ARG INTERM_BUILD_OUTPUT=/home/carla/sim_out

# make sure the carla user has full
# ownership of /home/carla/carla
USER root
RUN chown -R carla:carla /home/carla

# copy local CARLA repo
USER carla
WORKDIR /home/carla/carla
RUN git config --global --add safe.directory /home/carla/carla

# TODO: think of a more sophisticated build caching mechanism to speed up CI/CD

# run the build procedure
RUN make CarlaUE4Editor && \
    make PythonAPI && \
    make build.utils && \
    make package --no-zip && \
    mkdir $INTERM_BUILD_OUTPUT && mv Dist/CARLA_Shipping_*/LinuxNoEditor $INTERM_BUILD_OUTPUT && \
    make hard-clean

# use the official NVIDIA Vulkan runtime for serving
# GPU-empowered, headless CARLA simulations
FROM nvidia/vulkan:1.1.121-cuda-10.1--ubuntu18.04 as runtime

# set up the carla user and copy the build output from previous stage
RUN useradd -m carla
WORKDIR /home/carla
COPY --from=build-env --chown=carla:carla $INTERM_BUILD_OUTPUT .

# install some packages required to run CARLA with GPU support
RUN apt-key adv --fetch-keys \
    https://developer.download.nvidia.com/compute/cuda/repos/ubuntu1804/x86_64/3bf863cc.pub && \
    apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
        libsdl2-2.0 xserver-xorg libvulkan1 libomp5 --no-install-recommends && \
    apt-get install -y \
        xdg-user-dirs xdg-utils && \
    apt-get clean

# configure CARLA to run as a headless webservice using ports 2000-2002
USER carla
EXPOSE 2000-2002
ENTRYPOINT ["/bin/bash", "CarlaUE4.sh"]
CMD ["-RenderOffScreen"]
