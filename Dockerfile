FROM docker.io/ubuntu22.04:latest

WORKDIR /carla
COPY . .

ARG DEBIAN-FRONTEND=noninteractive

RUN apt update && apt upgrade -y && \
    apt-get install build-essential g++-12 cmake ninja-build \
    libvulkan1 python3 python3-dev python3-pip python3-venv \
    autoconf wget curl rsync unzip git git-lfs libpng-dev \
    libtiff5-dev libjpeg-dev

RUN apt update && apt upgrade -y

RUN git clone --depth 1 -b carla https://github.com/CarlaUnreal/UnrealEngine.git ~/UnrealEngine_4.26

RUN cd UnrealEngine_4.26 && \
    ./Setup.sh && \
    ./GenerateProjectFiles.sh && \
    make

RUN echo "export UE4_ROOT=UnrealEngine_4.26" >> ~/.bashrc && \
    source ~/.bashrc

RUN git clone -b ue4-dev https://github.com/carla-simulator/carla

RUN echo "export CARLA_UE4_ROOT=carla/carla" >> ~/.bashrc && \
    source ~/.bashrc

RUN cd carla/carla/Unreal/CarlaUE4/Content/Carla && \
    bash Update.sh

