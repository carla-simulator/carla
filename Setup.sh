#!/bin/bash



# ==================================================================================================
# -- FUNCTIONS -------------------------------------------------------------------------------------
# ==================================================================================================

satisfies_minimum_version() {
    CMAKE_VERSION="$(cmake --version | grep -Eo '[0-9]+\.[0-9]+\.[0-9]+')"
    CMAKE_MINIMUM_VERSION=$1
    MAJOR="${CMAKE_VERSION%%.*}"
    REMAINDER="${CMAKE_VERSION#*.}"
    MINOR="${REMAINDER%.*}"
    REVISION="${REMAINDER#*.}"
    MINIMUM_MAJOR="${CMAKE_MINIMUM_VERSION%%.*}"
    MINIMUM_REMAINDER="${CMAKE_MINIMUM_VERSION#*.}"
    MINIMUM_MINOR="${MINIMUM_REMAINDER%.*}"

    if [ $MAJOR -gt $MINIMUM_MAJOR ] ||
        ( [ $MAJOR -eq $MINIMUM_MAJOR ] &&
            ( [ $MINOR -gt $MINIMUM_MINOR ] || [ $MINOR -eq $MINIMUM_MINOR ])) ; then
        true
    else
        false
    fi
}



# ==================================================================================================
# -- MAIN ------------------------------------------------------------------------------------------
# ==================================================================================================

set -e
sudo echo "Got super powers..."

echo "Installing Ubuntu Pacakges..."
if ! command -v retry &> /dev/null
then
    sudo apt update
    sudo apt-get install retry
fi
retry --until=success --times=12 --delay=300 -- sudo apt-get update
retry --until=success --times=12 --delay=300 -- sudo apt-get install build-essential make ninja-build libvulkan1 python3 python3-dev python3-pip libpng-dev libtiff5-dev libjpeg-dev tzdata sed curl libtool rsync libxml2-dev git git-lfs
echo "Ubuntu Pacakges Installed..."

echo "Installing Python Pacakges..."
pip3 install --upgrade pip
pip3 install -r requirements.txt
echo "Python Pacakges Installed..."

CMAKE_MINIMUM_VERSION=3.28.0
if satisfies_minimum_version $CMAKE_MINIMUM_VERSION; then
    echo "Found CMake $CMAKE_VERSION - OK"
else
    echo "Found CMake $CMAKE_VERSION - FAIL"
    echo "Installing CMake 3.28.3..."
    curl -L -O https://github.com/Kitware/CMake/releases/download/v3.28.3/cmake-3.28.3-linux-x86_64.tar.gz
    sudo mkdir -p /opt
    sudo tar -xzf cmake-3.28.3-linux-x86_64.tar.gz -C /opt
    echo -e '\n#CARLA CMake 3.28.3\nPATH=/opt/cmake-3.28.3-linux-x86_64/bin:$PATH' >> ~/.bashrc
    export PATH=/opt/cmake-3.28.3-linux-x86_64/bin:$PATH
    rm -rf cmake-3.28.3-linux-x86_64.tar.gz
    echo "CMake Intalled 3.28.3..."
fi

echo "Configuring CARLA..."
retry --until=success --times=10 -- cmake -G Ninja -S . -B Build --toolchain=$PWD/CMake/LinuxToolchain.cmake -DLAUNCH_ARGS="-prefernvidia" -DCMAKE_BUILD_TYPE=Release -DENABLE_ROS2=ON -DBUILD_CARLA_UNREAL=ON -DCARLA_UNREAL_ENGINE_PATH=$CARLA_UNREAL_ENGINE_PATH
echo "Building CARLA..."
retry --until=success --times=10 -- cmake --build Build
echo "Installing PythonAPI..."
cmake --build Build --target carla-python-api-install
echo "Waitting for Content to be downloaded... (see the progres in ContentClone.log)"
wait #Waitting for content
echo "Instalation and build succesfull! :)"
echo "Lauching Carla with Unreal Editor..."
cmake --build Build --target launch
