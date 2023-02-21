FROM ubuntu:18.04

USER root

ARG EPIC_USER=user
ARG EPIC_PASS=pass
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update ; \
  apt-get install -y wget software-properties-common && \
  add-apt-repository ppa:ubuntu-toolchain-r/test && \
  wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key|apt-key add - && \
  apt-add-repository "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-8 main" && \
  apt-get update ; \
  apt-get install -y build-essential \
    clang-8 \
    lld-8 \
    g++-7 \
    cmake \
    ninja-build \
    libvulkan1 \
    python \
    python-pip \
    python-dev \
    python3-dev \
    python3-pip \
    libpng-dev \
    libtiff5-dev \
    libjpeg-dev \
    tzdata \
    sed \
    curl \
    unzip \
    autoconf \
    libtool \
    rsync \
    libxml2-dev \
    git \
    aria2 && \
  pip3 install -Iv setuptools==47.3.1 && \
  pip3 install distro && \
  update-alternatives --install /usr/bin/clang++ clang++ /usr/lib/llvm-8/bin/clang++ 180 && \
  update-alternatives --install /usr/bin/clang clang /usr/lib/llvm-8/bin/clang 180

# Update SSH key search location
RUN  echo "    IdentityFile /home/carla/.ssh/ssh_key" >> /etc/ssh/ssh_config
RUN useradd -m carla
COPY --chown=carla:carla . /home/carla
USER carla
WORKDIR /home/carla
ENV UE4_ROOT /home/carla/UE4.26

# No longer can use user/pwd to validate on github. Using ssh keys.
# Reference: https://vsupalov.com/build-docker-image-clone-private-repo-ssh-key/
ARG SSH_PRIVATE_KEY
RUN mkdir /home/carla/.ssh/
RUN echo "${SSH_PRIVATE_KEY}" > /home/carla/.ssh/ssh_key
RUN chmod 400 /home/carla/.ssh/ssh_key

# make sure your domain is accepted
RUN touch /home/carla/.ssh/known_hosts
RUN ssh-keyscan github.com >> /home/carla/.ssh/known_hosts
RUN git clone --depth 1 -b carla "git@github.com:CarlaUnreal/UnrealEngine.git" ${UE4_ROOT}

# Delete the key now that we are done.
RUN rm /home/carla/.ssh/ssh_key

RUN cd $UE4_ROOT && \
  ./Setup.sh && \
  ./GenerateProjectFiles.sh && \
  make

WORKDIR /home/carla/
