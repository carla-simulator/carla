#!/bin/bash
set -e

LAUNCH_COMMAND="retry --until=success --times=5 --delay=20 time"
sudo apt-get update
sudo apt-get install retry

#$LAUNCH_COMMAND sudo apt-get install wget software-properties-common
#$LAUNCH_COMMAND sudo add-apt-repository ppa:ubuntu-toolchain-r/test
#$LAUNCH_COMMAND wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key|sudo apt-key add

#$LAUNCH_COMMAND sudo apt-add-repository "deb http://archive.ubuntu.com/ubuntu focal main universe"
$LAUNCH_COMMAND "sudo apt-get update"
$LAUNCH_COMMAND "sudo apt-get install build-essential g++-7 ninja-build libvulkan1 python3 python3-dev python3-pip libpng-dev libtiff5-dev libjpeg-dev tzdata sed curl libtool rsync libxml2-dev git git-lfs"

$LAUNCH_COMMAND "curl -L -O https://github.com/Kitware/CMake/releases/download/v3.29.0-rc2/cmake-3.29.0-rc2.tar.gz"
sudo apt-get install build-essential libssl-dev tar
tar xvf cmake-3.29.0-rc2.tar.gz
cd cmake-3.29.0-rc2
./configure
make
sudo make install
cd ..
rm -rf cmake-3.29.0-rc2-linux-aarch64.tar.gz
