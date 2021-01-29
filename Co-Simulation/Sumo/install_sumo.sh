#! /bin/bash

cd ~/Download/
sudo apt-get install cmake python g++ libxerces-c-dev libfox-1.6-dev libgdal-dev libproj-dev libgl2ps-dev

# version 1.7.0: recommended
wget https://sumo.dlr.de/releases/1.7.0/sumo-src-1.7.0.tar.gz
tar xzf sumo-src-1.7.0.tar.gz
export SUMO_HOME="$PWD/sumo-1.7.0"
cd sumo-1.7.0

# version latest: unrecommended
# git clone --recursive https://github.com/eclipse/sumo
# export SUMO_HOME="$PWD/sumo"
# cd sumo

mkdir build/cmake-build && cd build/cmake-build
cmake ../..
make -j$(nproc)
sudo make install
echo "export SUMO_HOME=/usr/local/share/sumo" >> ~/.bashrc
