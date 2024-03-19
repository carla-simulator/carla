#!/bin/bash
set -e

#y | sudo apt-add-repository "deb http://archive.ubuntu.com/ubuntu focal main universe"
#sudo apt-get update
#sudo apt-get install build-essential retry g++-7 ninja-build libvulkan1 python3 python3-dev python3-pip libpng-dev libtiff5-dev libjpeg-dev tzdata sed curl libtool rsync libxml2-dev git git-lfs

pip3 install --user numpy
pip3 install --user -Iv setuptools==47.3.1
pip3 install --user distro
pip3 install --user wheel auditwheel

CMAKE_FOUND_VERSION=$(cmake --version | head -1 | cut -f3 -d" ")
CMAKE_MINIMUM_VERSION=3.13.4
if version_comp "$CMAKE_MINIMUM_VERSION" "$CMAKE_FOUND_VERSION"; then
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
fi

cmake -G Ninja -S . -B Build --toolchain=CMake/LinuxToolchain.cmake -DLAUNCH_ARGS="-prefernvidia" -DCMAKE_BUILD_TYPE=Release -DENABLE_ROS2=ON -DBUILD_CARLA_UNREAL=ON -DCARLA_UNREAL_ENGINE_PATH=$CARLA_UNREAL_ENGINE_PATH
cmake --build Build --target launch


version_comp() {
    local min_version
    local have_version

    [[ "$1" == "$2" ]] && return 0

    mapfile -t -d. min_version <<<"$1"
    mapfile -t -d. have_version <<<"$2"

    for ((i=0; i<${#min_version[*]}; i++)); do
        if (( have_version[i] > min_version[i] )); then
            return 0
        elif (( have_version[i] < min_version[i] )); then
            return 1
        fi
    done

    return 0
}
