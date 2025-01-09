#!/bin/bash

set -e

options=$(getopt -o "i" --long "interactive" -n 'CarlaSetup.sh' -- "$@")
eval set -- "$options"

interactive=0

while true; do
    case "$1" in
        -i | --interactive)
            interactive=1
            shift
            ;;
        --)
            shift
            break
            ;;
        *)
            ;;
    esac
done

if [ $interactive -eq 1 ]; then
    echo "Requesting root rights."
    sudo echo "Acquired root rights."
else
    if [ -z "$EUID" ]; then
        EUID=$(id -u)
    fi

    if [ "$EUID" -ne 0 ]; then
        echo "Please run this script as root."
        exit 1
    fi
fi

arrIN=(${GIT_LOCAL_CREDENTIALS//@/ })
GIT_LOCAL_USER=${arrIN[0]}
GIT_LOCAL_TOKEN=${arrIN[1]}
if [ -z "$GIT_LOCAL_CREDENTIALS" ]; then
    if [ $interactive -eq 1 ]; then
        echo "Warning: git credentials are not set"
    else
        echo "Git credentials are not set, can not continue setup in unatteded mode."
        exit 1
    fi
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
echo "Ubuntu Packages Installed..."

echo "Installing Python Packages..."
pip3 install --upgrade pip
pip3 install -r requirements.txt
echo "Python Packages Installed..."

echo "Cloning CARLA Content asynchronously... (see progress in ContentClone.log)"
mkdir -p Unreal/CarlaUnreal/Content
git -C Unreal/CarlaUnreal/Content clone -b ue5-dev https://bitbucket.org/carla-simulator/carla-content.git Carla &> ContentClone.log&

satisfies_minimum_version() {
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
if (satisfies_minimum_version $CMAKE_MINIMUM_VERSION cmake) || (satisfies_minimum_version $CMAKE_MINIMUM_VERSION /opt/cmake-3.28.3-linux-x86_64/bin/cmake); then
    echo "Found CMake $CMAKE_VERSION"
else
    echo "Could not find CMake $CMAKE_VERSION."
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

if [ ! -z $CARLA_UNREAL_ENGINE_PATH ] && [ -d $CARLA_UNREAL_ENGINE_PATH ]; then
    echo "Found CARLA Unreal Engine at $CARLA_UNREAL_ENGINE_PATH"
elif [ -d ../UnrealEngine5_carla ]; then
    pushd ..
    pushd UnrealEngine5_carla
    echo "Found CARLA Unreal Engine at ../UnrealEngine5_carla"
    export CARLA_UNREAL_ENGINE_PATH=$PWD
    echo -e '\n#CARLA UnrealEngine5\nexport CARLA_UNREAL_ENGINE_PATH='$CARLA_UNREAL_ENGINE_PATH >> ~/.bashrc
    popd
    popd
else
    echo "Could not find CARLA Unreal Engine."
    echo "Cloning CARLA Unreal Engine..."
    pushd ..
    if [ -z "$GIT_LOCAL_CREDENTIALS" ]
    then
        git clone -b ue5-dev-carla https://github.com/CarlaUnreal/UnrealEngine.git UnrealEngine5_carla
    else
        git clone -b ue5-dev-carla https://$GIT_LOCAL_USER:$GIT_LOCAL_TOKEN@github.com/CarlaUnreal/UnrealEngine.git UnrealEngine5_carla
    fi
    pushd UnrealEngine5_carla
    echo -e '\n#CARLA UnrealEngine5\nexport CARLA_UNREAL_ENGINE_PATH='$PWD >> ~/.bashrc
    export CARLA_UNREAL_ENGINE_PATH=$PWD
    popd
    popd
    echo "Installed CARLA Unreal Engine..."
fi
pushd ..
pushd $CARLA_UNREAL_ENGINE_PATH
echo Checking if Unreal Engine is in the most recent commit...
git fetch
if [[ $(git status) =~ "up to date" ]]; then
    echo CARLA Unreal Engine is already in the last commit
else
    echo CARLA Unreal Engine is NOT in the last commit.
    echo Running git clean...
    git clean -fdx
    echo Running git pull...
    git pull
fi
echo "Running CARLA Unreal Engine setup..."
./Setup.sh --force
echo "Creating CARLA Unreal Engine project files..."
./GenerateProjectFiles.sh
echo "Building CARLA Unreal Engine..."
make
popd
popd

echo "Configuring CARLA..."
retry --until=success --times=10 -- cmake -G Ninja -S . -B Build \
    --toolchain=$PWD/CMake/LinuxToolchain.cmake \
    -DLAUNCH_ARGS="-prefernvidia" \
    -DCMAKE_BUILD_TYPE=Release \
    -DENABLE_ROS2=ON \
    -DBUILD_CARLA_UNREAL=ON \
    -DCARLA_UNREAL_ENGINE_PATH=$CARLA_UNREAL_ENGINE_PATH
echo "Building CARLA..."
retry --until=success --times=10 -- cmake --build Build
echo "Building + installing Python API..."
cmake --build Build --target carla-python-api-install
echo "Waiting for Content to finish downloading..."
wait #Waitting for content
echo "Installation and build succesful."
echo "Lauching Carla - Unreal Editor..."
cmake --build Build --target launch
