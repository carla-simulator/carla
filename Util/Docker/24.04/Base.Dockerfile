FROM ubuntu:24.04

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

COPY --from=carla-root requirements.txt /tmp/requirements.txt
COPY build_scripts /tmp/build_scripts

# ---------------------------
# Install CARLA prerequisites
# ---------------------------
# https://carla.readthedocs.io/en/latest/build_linux_ue5/
USER root

# Core build tools and UE5-specific dependencies:
# - build-essential make ninja-build: core compilation tools
# - libvulkan1: Vulkan runtime needed by Unreal Engine for rendering
# - python3 python3-dev python3-pip python-is-python3: system Python (3.12 on noble), headers, pip
# - autoconf libtool: required for building dependencies from source
# - wget curl rsync unzip git git-lfs: essential CLI tools used in CARLA build scripts
# - libpng-dev libtiff-dev libjpeg-dev: image libraries for CARLA's Python API
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
        libtiff-dev \
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

# Install CMake from the distro's apt repository. Ubuntu 24.04 ships CMake 3.28.3,
# which clears CARLA's minimum (3.27.2) without any extra apt source or tarball.
RUN apt-get update \
    && apt-get install -y cmake \
    && rm -rf /var/lib/apt/lists/*

# SDL2 libraries:
# Required for Unreal Engine to interact with the display.
RUN packages='libsdl2-dev libsdl2-2.0' \
    && apt-get update \
    && apt-get install -y $packages \
    && rm -rf /var/lib/apt/lists/*

# -------------------
# Install pip packages
# -------------------
# Allow pip to install packages system-wide (Ubuntu 24.04 ships the PEP 668 marker;
# opt out so pip can install into the system site-packages).
RUN echo '[global]' > /etc/pip.conf && \
    echo 'break-system-packages = true' >> /etc/pip.conf

# `--ignore-installed` is required on Ubuntu 24.04: the system-shipped pip is
# installed via dpkg and lacks a RECORD file, so a normal pip self-upgrade fails
# with "Cannot uninstall pip 24.0". Layering the new pip on top sidesteps the
# uninstall step.
RUN python3 -m pip install --upgrade --ignore-installed pip \
    && python3 -m pip install -r /tmp/requirements.txt

# ---------------------------
# Install Python 3.10 from source (alongside system 3.12)
# ---------------------------
# CARLA's Python API supports building against multiple interpreters via
# -DPython_ROOT_DIR. Ubuntu 24.04's apt only provides Python 3.12, so compile
# 3.10 from source via `make altinstall` and install the shared CARLA
# requirements against it. Result: /usr/local/bin/python3.10 sits alongside
# /usr/bin/python3.12 and is selectable at configure time.
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        libbz2-dev \
        libffi-dev \
        libgdbm-dev \
        libgdbm-compat-dev \
        liblzma-dev \
        libncurses-dev \
        libreadline-dev \
        libsqlite3-dev \
        libssl-dev \
        tk-dev \
        uuid-dev \
        zlib1g-dev \
        pkg-config && \
    bash /tmp/build_scripts/build_cpython.sh --python-version 3.10.14 && \
    python3.10 -m pip install --upgrade --ignore-installed pip && \
    python3.10 -m pip install -r /tmp/requirements.txt && \
    rm -rf /var/lib/apt/lists/*

RUN rm -f /tmp/requirements.txt && rm -rf /tmp/build_scripts

USER root
