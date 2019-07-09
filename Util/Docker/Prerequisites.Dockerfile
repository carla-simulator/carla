ARG UE4_V=4.22.2
FROM adamrehn/ue4-source:${UE4_V}-opengl

USER root

ENV UE4_ROOT /home/ue4/UnrealEngine

RUN apt-get update ; \
  apt-get install -y wget software-properties-common && \
  add-apt-repository ppa:ubuntu-toolchain-r/test && \
  wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key|apt-key add - && \
  apt-add-repository "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-7 main" && \
  apt-get update ; \
  apt-get install -y build-essential \
    clang-7 \
    lld-7 \
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
    aria2 && \
  pip2 install setuptools && \
  pip3 install setuptools && \
  update-alternatives --install /usr/bin/clang++ clang++ /usr/lib/llvm-7/bin/clang++ 170 && \
  update-alternatives --install /usr/bin/clang clang /usr/lib/llvm-7/bin/clang 170

USER ue4

RUN cd $UE4_ROOT && \
  ./Setup.sh && \
  ./GenerateProjectFiles.sh && \
  make

WORKDIR /home/ue4
