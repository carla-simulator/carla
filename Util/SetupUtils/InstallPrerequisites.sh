#!/bin/bash

set -e

if [ -z "$EUID" ]; then
    EUID=$(id -u)
fi

if [ "$EUID" -ne 0 ]; then
    echo "Please run this script as root."
    exit 1
fi

echo "Installing Ubuntu Packages..."
if ! command -v retry &> /dev/null
then
    apt update
    apt-get install retry
fi
retry --until=success --times=12 --delay=300 -- apt-get update
retry --until=success --times=12 --delay=300 -- apt-get -y install \
    build-essential \
    g++-12 \
    gcc-12 \
    make \
    ninja-build \
    libvulkan1 \
    python3 \
    python3-dev \
    python3-pip \
    libpng-dev \
    libtiff5-dev \
    libjpeg-dev \
    tzdata \
    sed \
    curl \
    libtool \
    rsync \
    libxml2-dev \
    git \
    git-lfs

echo "Installing Python Packages..."
pip3 install --upgrade pip
pip3 install -r requirements.txt

echo "Cloning CARLA Content asynchronously... (see progress in ContentClone.log)"
mkdir -p Unreal/CarlaUnreal/Content
git -C Unreal/CarlaUnreal/Content clone -b ue5-dev https://bitbucket.org/carla-simulator/carla-content.git Carla &> ContentClone.log&

check_cmake_version() {
    CMAKE_VERSION="$($2 --version | grep -Eo '[0-9]+\.[0-9]+\.[0-9]+')"
    CMAKE_MINIMUM_VERSION=$1
    MAJOR="${CMAKE_VERSION%%.*}"
    REMAINDER="${CMAKE_VERSION#*.}"
    MINOR="${REMAINDER%.*}"
    REVISION="${REMAINDER#*.}"
    MINIMUM_MAJOR="${CMAKE_MINIMUM_VERSION%%.*}"
    MINIMUM_REMAINDER="${CMAKE_MINIMUM_VERSION#*.}"
    MINIMUM_MINOR="${MINIMUM_REMAINDER%.*}"

    if [ $MAJOR -gt $MINIMUM_MAJOR ] || ( [ $MAJOR -eq $MINIMUM_MAJOR ] &&
            ( [ $MINOR -gt $MINIMUM_MINOR ] || [ $MINOR -eq $MINIMUM_MINOR ])) ; then
        true
    else
        false
    fi
}

CMAKE_MINIMUM_VERSION=3.28.0
if (check_cmake_version $CMAKE_MINIMUM_VERSION cmake) || (check_cmake_version $CMAKE_MINIMUM_VERSION /opt/cmake-3.28.3-linux-x86_64/bin/cmake); then
    echo "Found CMake $CMAKE_VERSION"
else
    echo "Could not find CMake >=$CMAKE_MINIMUM_VERSION."
    echo "Installing CMake 3.28.3..."
    curl -L -O https://github.com/Kitware/CMake/releases/download/v3.28.3/cmake-3.28.3-linux-x86_64.tar.gz
    mkdir -p /opt
    tar -xzf cmake-3.28.3-linux-x86_64.tar.gz -C /opt
    if [[ ":$PATH:" != *":/opt/cmake-3.28.3-linux-x86_64/bin:"* ]]; then
        echo -e '\n#CARLA CMake 3.28.3\nPATH=/opt/cmake-3.28.3-linux-x86_64/bin:$PATH' >> ~/.bashrc
        export PATH=/opt/cmake-3.28.3-linux-x86_64/bin:$PATH
    fi
    rm -rf cmake-3.28.3-linux-x86_64.tar.gz
    echo "Installed CMake 3.28.3."
fi
