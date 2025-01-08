#!/bin/bash



# ==================================================================================================
# -- FUNCTIONS -------------------------------------------------------------------------------------
# ==================================================================================================

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

echo "Parsing GIT_LOCAL_CREDENTIALS local variable "
arrIN=(${GIT_LOCAL_CREDENTIALS//@/ })
GIT_LOCAL_USER=${arrIN[0]}
GIT_LOCAL_TOKEN=${arrIN[1]}
if [ -z "$GIT_LOCAL_CREDENTIALS" ]
then
    echo "Git credentials are not set, they will be requested later on during the download of Unreal Engine Carla fork"
fi

echo "Installing Ubuntu Packages..."
if ! command -v retry &> /dev/null
then
    sudo apt update
    sudo apt-get install retry
fi
retry --until=success --times=12 --delay=300 -- sudo apt-get update
retry --until=success --times=12 --delay=300 -- sudo apt-get -y install \
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

echo "Clonning CARLA Content asynchronously... (see the progres in ContentClone.log)"
mkdir -p Unreal/CarlaUnreal/Content
git -C Unreal/CarlaUnreal/Content clone -b ue5-dev https://bitbucket.org/carla-simulator/carla-content.git Carla &> ContentClone.log&

CMAKE_MINIMUM_VERSION=3.28.0
if (satisfies_minimum_version $CMAKE_MINIMUM_VERSION cmake) || (satisfies_minimum_version $CMAKE_MINIMUM_VERSION /opt/cmake-3.28.3-linux-x86_64/bin/cmake); then
    echo "Found CMake $CMAKE_VERSION - OK"
else
    echo "Found CMake $CMAKE_VERSION - FAIL"
    echo "Installing CMake 3.28.3..."
    curl -L -O https://github.com/Kitware/CMake/releases/download/v3.28.3/cmake-3.28.3-linux-x86_64.tar.gz
    sudo mkdir -p /opt
    sudo tar -xzf cmake-3.28.3-linux-x86_64.tar.gz -C /opt
    if [[ ":$PATH:" != *":/opt/cmake-3.28.3-linux-x86_64/bin:"* ]]; then
        echo -e '\n#CARLA CMake 3.28.3\nPATH=/opt/cmake-3.28.3-linux-x86_64/bin:$PATH' >> ~/.bashrc
        export PATH=/opt/cmake-3.28.3-linux-x86_64/bin:$PATH
    fi
    rm -rf cmake-3.28.3-linux-x86_64.tar.gz
    echo "CMake Intalled 3.28.3..."
fi

if [ ! -z $CARLA_UNREAL_ENGINE_PATH ] && [ -d $CARLA_UNREAL_ENGINE_PATH ]; then
    echo "Found UnrealEngine5 $CARLA_UNREAL_ENGINE_PATH - OK"
elif [ -d ../UnrealEngine5_carla ]; then
    pushd ..
    pushd UnrealEngine5_carla
    echo "Found UnrealEngine5 ../UnrealEngine5_carla - OK"
    export CARLA_UNREAL_ENGINE_PATH=$PWD
    echo -e '\n#CARLA UnrealEngine5\nexport CARLA_UNREAL_ENGINE_PATH='$CARLA_UNREAL_ENGINE_PATH >> ~/.bashrc
    popd
    popd
else
    echo "Found UnrealEngine5 $CARLA_UNREAL_ENGINE_PATH - FAIL"
    echo "Cloning CARLA UnrealEngine5..."
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
    echo "CARLA UnrealEngine5 Installed..."
fi
pushd ..
pushd $CARLA_UNREAL_ENGINE_PATH
echo Checking if UnreaEngine5 is in the last commit...
git fetch
if [[ $(git status) =~ "up to date" ]]; then
    echo UnreaEngine5 is already in the last commit - OK
else
    echo UnreaEngine5 is NOT in the last commit - FAIL
    echo Cleaning UnrealEngine5 build...
    git clean -fdx
    echo Pulling last UnrealEngine5 changes...
    git pull
fi
echo "Setup CARLA UnrealEngine5..."
./Setup.sh --force
echo "GenerateProjectFiles CARLA UnrealEngine5..."
./GenerateProjectFiles.sh
echo "Build CARLA UnrealEngine5..."
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
echo "Installing PythonAPI..."
cmake --build Build --target carla-python-api-install
echo "Waitting for Content to be downloaded... (see the progres in ContentClone.log)"
wait #Waitting for content
echo "Instalation and build succesfull! :)"
echo "Lauching Carla with Unreal Editor..."
cmake --build Build --target launch
