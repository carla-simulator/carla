#!/bin/bash

set -e

python_path_default='python3'
python_path=$python_path_default

options=$(\
    getopt \
    -o "pypath:" \
    --long "python-path:" \
    -n 'CarlaSetup.sh' -- "$@")

eval set -- "$options"
while true; do
    case "$1" in
        -pypath|--python-path)
            python_path=$2
            shift 2
            ;;
        --)
            shift
            break
            ;;
        *)
            ;;
    esac
done

if [ -z "$EUID" ]; then
    EUID=$(id -u)
fi

if [ "$EUID" -ne 0 ]; then
    echo "Please run this script as root."
    exit 1
fi

# -- INSTALL APT PACKAGES --
echo "Installing Ubuntu Packages..."
apt-get update
apt-get -y install \
    build-essential \
    make \
    ninja-build \
    libvulkan1 \
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

if [ "$python_path" == "python3" ]; then
    apt-get -y install \
        python3 \
        python3-dev \
        python3-pip
fi

# -- INSTALL PYTHON PACKAGES --
echo "Installing Python Packages..."
$python_path -m pip install --upgrade pip
$python_path -m pip install -r requirements.txt

# -- INSTALL CMAKE --
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

    if [ -z "$CMAKE_VERSION" ]; then
        false
    else
        if [ $MAJOR -gt $MINIMUM_MAJOR ] || ([ $MAJOR -eq $MINIMUM_MAJOR ] && ([ $MINOR -gt $MINIMUM_MINOR ] || [ $MINOR -eq $MINIMUM_MINOR ])); then
            true
        else
            false
        fi
    fi
}

CMAKE_MINIMUM_VERSION=3.28.0
if (check_cmake_version $CMAKE_MINIMUM_VERSION cmake) || (check_cmake_version $CMAKE_MINIMUM_VERSION /opt/cmake-3.28.3-linux-x86_64/bin/cmake); then
    echo "Found CMake $CMAKE_MINIMUM_VERSION"
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
