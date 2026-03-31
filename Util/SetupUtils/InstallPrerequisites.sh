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

# -- DETECT UBUNTU VERSION --
if [ -f /etc/os-release ]; then
    . /etc/os-release
    UBUNTU_VERSION_ID="${VERSION_ID}"
else
    UBUNTU_VERSION_ID="22.04"
fi

# Select libtiff package name based on Ubuntu version
if dpkg --compare-versions "$UBUNTU_VERSION_ID" ge "24.04"; then
    LIBTIFF_PKG="libtiff-dev"
else
    LIBTIFF_PKG="libtiff5-dev"
fi

# -- INSTALL APT PACKAGES --
echo "Installing Ubuntu Packages..."
sudo apt-get update
sudo apt-get -y install \
    build-essential \
    make \
    ninja-build \
    libvulkan1 \
    libpng-dev \
    "$LIBTIFF_PKG" \
    libjpeg-dev \
    tzdata \
    sed \
    curl \
    libtool \
    rsync \
    libxml2-dev \
    git \
    git-lfs \
    libnss3-dev \
    libatk-bridge2.0-dev \
    libxkbcommon-dev \
    libgbm-dev \
    libpango1.0-dev \
    libasound2-dev

if [ "$python_path" == "python3" ]; then
    sudo apt-get -y install \
        python3 \
        python3-dev \
        python3-pip
fi

# -- CONFIGURE GIT LFS --
git lfs install

# -- INSTALL PYTHON PACKAGES --
echo "Installing Python Packages..."
PIP_EXTRA_ARGS=""
if dpkg --compare-versions "$UBUNTU_VERSION_ID" ge "24.04"; then
    PIP_EXTRA_ARGS="--break-system-packages"
fi
$python_path -m pip install --upgrade pip $PIP_EXTRA_ARGS
$python_path -m pip install -r requirements.txt $PIP_EXTRA_ARGS

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
    sudo mkdir -p /opt
    sudo tar -xzf cmake-3.28.3-linux-x86_64.tar.gz -C /opt
    if [[ ":$PATH:" != *":/opt/cmake-3.28.3-linux-x86_64/bin:"* ]]; then
        echo -e '\n#CARLA CMake 3.28.3\nPATH=/opt/cmake-3.28.3-linux-x86_64/bin:$PATH' >> ~/.bashrc
        export PATH=/opt/cmake-3.28.3-linux-x86_64/bin:$PATH
    fi
    rm -rf cmake-3.28.3-linux-x86_64.tar.gz
    echo "Installed CMake 3.28.3."
fi
