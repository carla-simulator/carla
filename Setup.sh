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
retry --until=success --times=12 --delay=300 -- sudo apt-add-repository "deb http://archive.ubuntu.com/ubuntu focal main universe"
retry --until=success --times=12 --delay=300 -- sudo apt-get update
retry --until=success --times=12 --delay=300 -- sudo apt-get install build-essential make g++-7 ninja-build libvulkan1 python3 python3-dev python3-pip libpng-dev libtiff5-dev libjpeg-dev tzdata sed curl libtool rsync libxml2-dev git git-lfs
echo "Ubuntu Pacakges Installed..."

echo "Installing Python Pacakges..."
pip3 install --user numpy
pip3 install --user -Iv setuptools==47.3.1
pip3 install --user distro
pip3 install --user wheel auditwheel
echo "Python Pacakges Installed..."

echo "Clonning CARLA Content asynchronously... (see the progres in ContentClone.log)"
mkdir -p Unreal/CarlaUnreal/Content
git -C Unreal/CarlaUnreal/Content clone -b ue5-dev https://bitbucket.org/carla-simulator/carla-content.git Carla &> ContentClone.log&

CMAKE_MINIMUM_VERSION=3.28.0
if satisfies_minimum_version $CMAKE_MINIMUM_VERSION; then
    echo "Found CMake $CMAKE_VERSION - OK"
else
    echo "Found CMake $CMAKE_VERSION - FAIL"
    echo "Installing CMake 3.28.3..."
    curl -L -O https://github.com/Kitware/CMake/releases/download/v3.28.3/cmake-3.28.3-linux-x86_64.tar.gz
    sudo mkdir -p /opt
    sudo tar -xzf cmake-3.28.3-linux-x86_64.tar.gz -C /opt
    sudo echo -e '\n#CARLA CMake 3.28.3\nPATH=/opt/cmake-3.28.3-linux-x86_64/bin:$PATH' >> ~/.bashrc
    export PATH=/opt/cmake-3.28.3-linux-x86_64/bin:$PATH
    rm -rf cmake-3.28.3-linux-x86_64.tar.gz
    echo "CMake Intalled 3.28.3..."
fi

if [ ! -z $CARLA_UNREAL_ENGINE_PATH ] && [ -d $CARLA_UNREAL_ENGINE_PATH ]; then
    echo "Found UnrealEngine5 $CARLA_UNREAL_ENGINE_PATH - OK"
elif [ -d ../UnrealEngine5_carla ]; then
    echo "Found UnrealEngine5 ../UnrealEngine5_carla - OK"
    export CARLA_UNREAL_ENGINE_PATH=$PWD/../UnrealEngine5_carla
    sudo echo -e '\n#CARLA UnrealEngine5\nexport CARLA_UNREAL_ENGINE_PATH='$CARLA_UNREAL_ENGINE_PATH >> ~/.bashrc
    #TODO: Check if UnrealEngine binary file exists and if not build it
else
    echo "Found UnrealEngine5 $CARLA_UNREAL_ENGINE_PATH - FAIL"
    echo "Cloning CARLA UnrealEngine5..."
    pushd ..
    git clone -b ue5-dev-carla https://github.com/CarlaUnreal/UnrealEngine.git UnrealEngine5_carla
    pushd UnrealEngine5_carla
    echo "Setup CARLA UnrealEngine5..."
    ./Setup.sh
    echo "GenerateProjectFiles CARLA UnrealEngine5..."
    ./GenerateProjectFiles.sh
    echo "Build CARLA UnrealEngine5..."
    make
    sudo echo -e '\n#CARLA UnrealEngine5\nexport CARLA_UNREAL_ENGINE_PATH='$PWD >> ~/.bashrc
    export CARLA_UNREAL_ENGINE_PATH=$PWD
    popd
    popd
    echo "CARLA UnrealEngine5 Installed..."
fi

echo "Configuring CARLA..."
retry --until=success --times=10 -- cmake -G Ninja -S . -B Build --toolchain=CMake/LinuxToolchain.cmake -DLAUNCH_ARGS="-prefernvidia" -DCMAKE_BUILD_TYPE=Release -DENABLE_ROS2=ON -DBUILD_CARLA_UNREAL=ON -DCARLA_UNREAL_ENGINE_PATH=$CARLA_UNREAL_ENGINE_PATH
echo "Building CARLA..."
retry --until=success --times=10 -- cmake --build Build
echo "Waitting for Content to be downloaded... (see the progres in ContentClone.log)"
wait #Waitting for content
echo "Instalation and build succesfull! :)"
echo "Lauching Carla with Unreal Editor..."
cmake --build Build --target launch
