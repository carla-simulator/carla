ARG UBUNTU_DISTRO="22.04"

FROM ubuntu:${UBUNTU_DISTRO}

ARG UBUNTU_DISTRO

# Disable interactive prompts during package installation.
ENV DEBIAN_FRONTEND=noninteractive

WORKDIR /

# ---------------------------
# Configure UTF-8 locale
# ---------------------------
# Required by CMake's archive extraction for filenames with non-ASCII characters (e.g., Boost).
RUN apt-get update && \
    apt-get install -y locales && \
    locale-gen en_US.UTF-8 && \
    rm -rf /var/lib/apt/lists/*
ENV LANG=en_US.UTF-8
ENV LC_ALL=en_US.UTF-8

COPY .tmp/requirements.txt /tmp/requirements.txt

# ---------------------------
# Install CARLA prerequisites
# ---------------------------
# https://carla.readthedocs.io/en/latest/build_linux_ue5/
USER root

# Core build tools and UE5-specific dependencies:
# - build-essential make ninja-build: core compilation tools
# - libvulkan1: Vulkan runtime needed by Unreal Engine for rendering
# - python3 python3-dev python3-pip python-is-python3: Python runtime, headers, package manager
# - autoconf libtool: required for building dependencies from source
# - wget curl rsync unzip git git-lfs: essential CLI tools used in CARLA build scripts
# - libpng-dev libtiff5-dev libjpeg-dev: image libraries for CARLA's Python API
# - tzdata sed libxml2-dev: build utilities
# - libnss3-dev libatk-bridge2.0-dev libxkbcommon-dev libgbm-dev: required by UE5 editor
# - libpango1.0-dev libasound2-dev: display and audio support for UE5
RUN apt-get update && \
    apt-get install -y \
        build-essential \
        make \
        ninja-build \
        libvulkan1 \
        python3 \
        python3-dev \
        python3-pip \
        python-is-python3 \
        autoconf \
        libtool \
        wget \
        curl \
        rsync \
        unzip \
        git \
        git-lfs \
        libpng-dev \
        libtiff5-dev \
        libjpeg-dev \
        tzdata \
        sed \
        libxml2-dev \
        libnss3-dev \
        libatk-bridge2.0-dev \
        libxkbcommon-dev \
        libgbm-dev \
        libpango1.0-dev \
        libasound2-dev \
    && rm -rf /var/lib/apt/lists/*

# Enable git lfs
RUN git lfs install

# Minimum required version is 3.28.0.
# Install a newer version manually, as Ubuntu 22.04 includes an outdated CMake from the upstream repository.
RUN curl -L -O https://github.com/Kitware/CMake/releases/download/v3.28.3/cmake-3.28.3-linux-x86_64.tar.gz \
    && mkdir -p /opt \
    && tar -xzf cmake-3.28.3-linux-x86_64.tar.gz -C /opt \
    && rm -rf cmake-3.28.3-linux-x86_64.tar.gz
ENV PATH=/opt/cmake-3.28.3-linux-x86_64/bin:$PATH

# SDL2 libraries:
# Required for Unreal Engine to interact with the display.
RUN packages='libsdl2-dev libsdl2-2.0' \
    && apt-get update \
    && apt-get install -y $packages \
    && rm -rf /var/lib/apt/lists/*

# -------------------
# Install pip packages
# -------------------
# Allow pip to install packages system-wide (required on Ubuntu 22.04 due to PEP 668).
RUN echo '[global]' > /etc/pip.conf && \
    echo 'break-system-packages = true' >> /etc/pip.conf

RUN python3 -m pip install --upgrade pip \
    && python3 -m pip install -r /tmp/requirements.txt \
    && rm /tmp/requirements.txt

USER root
