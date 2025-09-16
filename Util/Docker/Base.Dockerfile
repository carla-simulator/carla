ARG UBUNTU_DISTRO="20.04"

FROM ubuntu:${UBUNTU_DISTRO}

ARG UBUNTU_DISTRO

# Disable interactive prompts during package installation.
ENV DEBIAN_FRONTEND=noninteractive

WORKDIR /

COPY build_scripts build_scripts
COPY requirements requirements

# ---------------------------
# Install CARLA prerequisites
# ---------------------------
# https://carla.readthedocs.io/en/latest/build_linux/#system-requirements
USER root

# Core build tools
# - build-essential cmake ninja-build
#
# Vulkan support:
# - libvulkan1: vulkan runtime needed by Unreal Engine for rendering
#
# Python environment:
# - python3, python3-dev, python3-pip: python runtime, development headers, and package manager installed at system level
# - python-is-python3: symlink `python` to `python3` for compatibility (only Ubuntu20.04 and Ubuntu22.04)
#
# Patchelf dependency:
# - autoconf: required for building patchelf from source
#
# Utility tools used in scripts:
# - wget, curl, rsync, unzip, git, git-lfs: essential CLI tools used in CARLA's build scripts.
#
# Image libraries:
# - libpng-dev, libtiff5-dev, libjpeg-dev: CARLA's Python API links to these system libraries
RUN packages='build-essential ninja-build libvulkan1 python3 python3-dev python3-pip autoconf wget curl rsync unzip git git-lfs libpng-dev libtiff5-dev libjpeg-dev' && \
  apt-get update && \
  apt-get install -y $packages && \
  if [ "$UBUNTU_DISTRO" = "22.04" ]; then \
    packages="python-is-python3" && \
    apt-get install -y $packages; \
  elif [ "$UBUNTU_DISTRO" = "20.04" ]; then \
      packages="python-is-python3" && \
    apt-get install -y $packages; \
  elif [ "$UBUNTU_DISTRO" = "18.04" ]; then \
    packages="python" && \
    apt-get install -y $packages; \
  fi && \
  rm -rf /var/lib/apt/lists/*

# Enable git lfs
RUN git lfs install

# Minimum required version is 3.14.2.
# Install a newer version manually, as older Ubuntu versions include outdated CMake versions from the upstream repository
RUN curl -L -O https://github.com/Kitware/CMake/releases/download/v3.28.3/cmake-3.28.3-linux-x86_64.tar.gz \
    && mkdir -p /opt \
    && tar -xzf cmake-3.28.3-linux-x86_64.tar.gz -C /opt \
    && rm -rf cmake-3.28.3-linux-x86_64.tar.gz
ENV PATH=/opt/cmake-3.28.3-linux-x86_64/bin:$PATH

# SDL2 libraries:
# Required for Unreal Engine to interact with the display.
# - libsdl2-dev: development headers
# - libsdl2-2.0: runtime library
RUN packages='libsdl2-dev libsdl2-2.0' \
    && apt-get update \
    && apt-get install -y $packages \
    && rm -rf /var/lib/apt/lists/*

# ---------------
# Install CPython
# ---------------
# https://devguide.python.org/getting-started/setup-building/#install-dependencies
USER root

RUN packages='build-essential gdb lcov pkg-config libbz2-dev libffi-dev libgdbm-dev libgdbm-compat-dev liblzma-dev libncurses5-dev libreadline6-dev libsqlite3-dev libssl-dev lzma lzma-dev tk-dev uuid-dev zlib1g-dev libmpdec-dev wget'; \
    apt-get update; \
    apt-get install -y $packages; \
    rm -rf /var/lib/apt/lists/*

RUN bash build_scripts/build_cpython.sh --python-version 3.8.19
RUN bash build_scripts/build_cpython.sh --python-version 3.9.19
RUN bash build_scripts/build_cpython.sh --python-version 3.10.14
RUN bash build_scripts/build_cpython.sh --python-version 3.11.12
RUN bash build_scripts/build_cpython.sh --python-version 3.12.9

# Install python libraries at system level
USER root

RUN python3.8 -m pip install -r requirements/3.8/build.txt
RUN python3.9 -m pip install -r requirements/3.9/build.txt
RUN python3.10 -m pip install -r requirements/3.10/build.txt
RUN python3.11 -m pip install -r requirements/3.11/build.txt
RUN python3.12 -m pip install -r requirements/3.12/build.txt

RUN python3.8 -m pip install -r requirements/3.8/test.txt
RUN python3.9 -m pip install -r requirements/3.9/test.txt
RUN python3.10 -m pip install -r requirements/3.10/test.txt
RUN python3.11 -m pip install -r requirements/3.11/test.txt
RUN python3.12 -m pip install -r requirements/3.12/test.txt

# Install requirements at system level as CARLA is using it for auditing the resulting wheels.
RUN SYSTEM_PYTHON_VERSION=$(python3 -V | cut -d " " -f 2 | cut -d "." -f 1,2) \
    && python3 -m pip install -r requirements/${SYSTEM_PYTHON_VERSION}/build.txt \
    && python3 -m pip install -r requirements/${SYSTEM_PYTHON_VERSION}/test.txt

# ---------------
# Install AWS CLI
# ---------------
USER root
# Pin AWS CLI to this version due to compatibility issues with custom endpoints in newer releases
ENV AWS_CLI_VERSION=2.17.15
RUN curl -sL "https://awscli.amazonaws.com/awscli-exe-linux-x86_64-${AWS_CLI_VERSION}.zip" -o "awscliv2.zip" \
    && unzip awscliv2.zip \
    && ./aws/install \
    && rm -rf aws awscliv2.zip

# ------------------
# Install Docker CLI
# ------------------
USER root
RUN install -m 0755 -d /etc/apt/keyrings \
    && curl -fsSL https://download.docker.com/linux/ubuntu/gpg -o /etc/apt/keyrings/docker.asc \
    && chmod a+r /etc/apt/keyrings/docker.asc \
    && echo \
        "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.asc] https://download.docker.com/linux/ubuntu \
        $(. /etc/os-release && echo "${UBUNTU_CODENAME:-$VERSION_CODENAME}") stable" | \
        tee /etc/apt/sources.list.d/docker.list > /dev/null

RUN packages='docker-ce-cli' \
    && apt-get update \
    && apt-get install -y $packages \
    && rm -rf /var/lib/apt/lists/*

USER root
