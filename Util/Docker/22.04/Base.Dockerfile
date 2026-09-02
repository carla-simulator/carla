FROM ubuntu:22.04

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
COPY --from=carla-root Util/Docker/requirements/build.txt /tmp/build-requirements.txt
COPY --from=carla-root Util/Docker/build_scripts /tmp/build_scripts

# ---------------------------
# Install CARLA prerequisites
# ---------------------------
# https://carla.readthedocs.io/en/latest/build_linux_ue5/
USER root

# Core build tools and UE5-specific dependencies:
# - build-essential make ninja-build: core compilation tools
# - libvulkan1: Vulkan runtime needed by Unreal Engine for rendering
# - python3 python3-dev python3-pip python3-venv python-is-python3: system Python (3.10 on jammy), headers, pip, venv
# - autoconf libtool: required for building dependencies from source
# - wget curl rsync unzip git git-lfs: essential CLI tools used in CARLA build scripts
# - libpng-dev libtiff-dev libjpeg-dev: image libraries for CARLA's Python API
#   (libtiff-dev on 22.04 is a transitional metapackage that pulls libtiff5-dev)
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
        python3-venv \
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

# Install CMake from Kitware's official binary release. Ubuntu 22.04's apt ships
# CMake 3.22.1, below CARLA's current minimum (3.27.2), so the image installs a
# newer CMake to /opt and puts it ahead on PATH. This makes the 22.04 image a
# fully buildable environment (matching the 24.04 image's apt cmake 3.28.3).
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
RUN python3 -m pip install --upgrade pip \
    && python3 -m pip install -r /tmp/requirements.txt

# ---------------------------
# CPython build dependencies
# ---------------------------
# Headers and tooling required to compile the interpreters from source below.
# `patchelf` is needed by `auditwheel repair` (run by the carla-python-api-wheels
# CMake target) to vendor shared libraries into the wheel.
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
        pkg-config \
        patchelf && \
    rm -rf /var/lib/apt/lists/*

# ---------------------------
# Provision the distro's own interpreter as a wheel build target
# ---------------------------
# CARLA's Python API links against a version-specific Boost.Python target, so a
# wheel is built once per interpreter. One of the supported versions is the
# distro default `python3` (3.10 on jammy); it is provisioned in place rather
# than rebuilt from source, so there is exactly one interpreter per MAJOR.MINOR.
# Rebuilding it would leave two same-version interpreters sharing
# /usr/local/lib/python3.X/dist-packages with a version/static-vs-shared drift.
# pip refuses to modify a distro-managed interpreter unless --break-system-packages
# is passed (PEP 668). That flag exists on the distro pip where the marker is
# present (noble) and is absent where it is not enforced (jammy), so detect it
# rather than assume. The distro pip itself is NOT upgraded: it is owned by dpkg
# and an in-place `pip install --upgrade pip` cannot uninstall it (no RECORD).
RUN BSP="$(python3 -m pip install --help | grep -q -- '--break-system-packages' && echo '--break-system-packages' || true)"; \
    python3 -m pip install ${BSP} -r /tmp/build-requirements.txt

# ---------------------------
# Install the remaining supported Python versions from source
# ---------------------------
# Compile every supported version the distro does NOT provide via `make
# altinstall` (each lands at /usr/local/bin/python3.X) and install the core build
# requirements (build, scikit-build-core, auditwheel, ...) into each, so the
# carla-python-api-wheels target can produce and repair a wheel for any of them.
# numpy resolves to a per-interpreter-compatible release (1.24.x on 3.8, 2.x on
# 3.13+). The distro version (3.10 on jammy) is intentionally excluded here.
# Override the source set at build time with --build-arg PYTHON_VERSIONS="...".
ARG PYTHON_VERSIONS="3.8.19 3.9.19 3.11.9 3.12.4 3.13.1 3.14.0"
RUN set -eux; \
    for full in ${PYTHON_VERSIONS}; do \
        minor="$(echo "${full}" | cut -d. -f1,2)"; \
        bash /tmp/build_scripts/build_cpython.sh --python-version "${full}"; \
        python${minor} -m pip install --upgrade --ignore-installed pip; \
        python${minor} -m pip install -r /tmp/build-requirements.txt; \
    done

RUN rm -f /tmp/requirements.txt /tmp/build-requirements.txt && rm -rf /tmp/build_scripts

USER root
