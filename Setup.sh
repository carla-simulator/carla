#!/bin/bash
set -e

y | sudo apt-add-repository "deb http://archive.ubuntu.com/ubuntu focal main universe"
sudo apt-get update
sudo apt-get install build-essential retry g++-7 ninja-build libvulkan1 python3 python3-dev python3-pip libpng-dev libtiff5-dev libjpeg-dev tzdata sed curl libtool rsync libxml2-dev git git-lfs

pip3 install --user numpy
pip3 install --user -Iv setuptools==47.3.1
pip3 install --user distro
pip3 install --user wheel auditwheel

git clone -b ue5-dev https://github.com/carla-simulator/carla.git carlaUE5
cd carlaUE5

curl -L -O https://github.com/Kitware/CMake/releases/download/v3.28.3/cmake-3.28.3-linux-x86_64.tar.gz
tar -xvf cmake-3.28.3-linux-x86_64.tar.gz
cp cmake-3.28.3-linux-x86_64/bin/cmake .
rm -rf cmake-3.28.3-linux-x86_64.sh
rm -rf cmake-3.28.3-linux-x86_64

./cmake -G Ninja -S . -B Build --toolchain=CMake/LinuxToolchain.cmake -DLAUNCH_ARGS="-prefernvidia" -DCMAKE_BUILD_TYPE=Release -DENABLE_ROS2=ON -DBUILD_CARLA_UNREAL=ON -DCARLA_UNREAL_ENGINE_PATH=$CARLA_UNREAL_ENGINE_PATH
./cmake --build Build --target launch
