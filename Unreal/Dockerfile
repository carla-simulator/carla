FROM ubuntu:16.04

ARG DEBIAN_FRONTEND=noninteractive

# General build tooling
RUN apt-get update && apt-get install -y mesa-utils \
                                         cmake \
                                         git \
                                         gdb \
                                         dos2unix \
                                         software-properties-common \
                                         build-essential \
                                         sudo \
                                         python \
                                         python-dev \
                                         python-setuptools \
                                         python-pip \
                                         tzdata \
                                         curl \
                                         wget \
                                         unzip \
                                         sed \
                                         autoconf \
                                         alsa-utils

# Extracted from UE4/Engine/Build/BatchFiles/Linux
RUN apt-get update && apt-get install -y mono-xbuild \
                                         mono-dmcs \
                                         libmono-microsoft-build-tasks-v4.0-4.0-cil \
                                         libmono-system-data-datasetextensions4.0-cil \
                                         libmono-system-web-extensions4.0-cil \
                                         libmono-system-management4.0-cil \
                                         libmono-system-xml-linq4.0-cil \
                                         libmono-corlib4.5-cil \
                                         libmono-windowsbase4.0-cil \
                                         libmono-system-io-compression4.0-cil \
                                         libmono-system-io-compression-filesystem4.0-cil \
                                         libmono-system-runtime4.0-cil \
                                         mono-devel \
                                         clang-3.8 \
                                         llvm

# Nvidia driver
RUN add-apt-repository -y ppa:graphics-drivers/ppa
RUN apt-get update && apt-get install -y nvidia-396

# Carla requirements
RUN add-apt-repository ppa:ubuntu-toolchain-r/test
RUN apt-get update && apt-get install -y clang-5.0 \
                                         lld-5.0 \
                                         libtool \
                                         g++-7 \
                                         ninja-build
RUN update-alternatives --install /usr/bin/clang++ clang++ /usr/lib/llvm-5.0/bin/clang++ 101
RUN update-alternatives --install /usr/bin/clang clang /usr/lib/llvm-5.0/bin/clang 101

# Install newer cmake (carla requires it)
ADD https://cmake.org/files/v3.10/cmake-3.10.2-Linux-x86_64.sh /cmake.sh
RUN mkdir /opt/cmake
RUN sh /cmake.sh --prefix=/opt/cmake --skip-license
RUN ln -s /opt/cmake/bin/cmake /usr/local/bin/cmake

# Unreal
ADD UE4 /UE4
WORKDIR /UE4
ENV UE4_ROOT /UE4

# Parts taken from: https://github.com/tweakmy/ue4-in-docker/
# and https://docs.docker.com/develop/develop-images/dockerfile_best-practices/#user
RUN addgroup --gid 1000 unreal
RUN useradd -ms /bin/bash --gid 1000 --uid 1000 unreal
RUN adduser unreal sudo
RUN perl -p -i.bak -e 's/%sudo\s+ALL=\(ALL:ALL\)\s+ALL/%sudo ALL=(ALL) NOPASSWD:ALL/g' /etc/sudoers

# Ok and now go...
RUN ./GenerateProjectFiles.sh
RUN chown -R unreal:unreal /UE4
USER unreal
RUN make
