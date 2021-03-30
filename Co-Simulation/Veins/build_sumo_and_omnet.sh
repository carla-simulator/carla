#! /bin/sh

# ----- for omnet++ -----
sudo apt-get install -y build-essential gcc g++ bison flex perl tcl-dev tk-dev blt libxml2-dev zlib1g-dev default-jre doxygen graphviz libwebkitgtk-1.0-0 openmpi-bin libopenmpi-dev libpcap-dev autoconf automake libtool libproj-dev libfox-1.6-dev libgdal-dev libxerces-c-dev qt4-dev-tools
sudo apt-get install -y qt5-qmake qt5-default
sudo add-apt-repository -y ppa:ubuntugis/ppa
sudo apt update
sudo apt install -y libosgearth-dev
sudo add-apt-repository -y -r ppa:ubuntugis/ppa

cd ~/Downloads/
wget https://github.com/omnetpp/omnetpp/releases/download/omnetpp-5.6.2/omnetpp-5.6.2-src-linux.tgz
tar -xzvf omnetpp-5.6.2-src-linux.tgz

cd omnetpp-5.6.2/
. setenv
./configure
make

# ----- for SUMO -----
sudo apt-get install -y cmake python libxerces-c-dev libfox-1.6-dev libgl1-mesa-dev libglu1-mesa-dev libgdal-dev libproj-dev

cd ~/Downloads/
wget https://sumo.dlr.de/releases/1.2.0/sumo-src-1.2.0.zip
unzip sumo-src-1.2.0.zip

cd sumo-1.2.0/
mkdir build/cmake-build

cd build/cmake-build
cmake ../..
make -j8
sudo make install
export SUMO_HOME=/usr/local/share/sumo

# ----- for veins -----
cd ~/Downloads/
wget https://veins.car2x.org/download/veins-5.0.zip
unzip veins-5.0.zip

# ----- for carla-veins syncher -----
sudo apt install -y git nlohmann-json-dev

# ----- set env to .bashrc -----
echo "export SUMO_HOME=/usr/local/share/sumo" >> ~/.bashrc
echo "cd ~/Downloads/omnetpp-5.6.2/" >> ~/.bashrc
echo "source setenv" >> ~/.bashrc
echo "cd ~/" >> ~/.bashrc

# ----- set env to environment -----
sudo cp /vagrant/sshd_config /etc/ssh/
echo "SUMO_HOME=/usr/local/share/sumo" >> ~/.ssh/environment
sudo /etc/init.d/ssh restart

# ----- valid cache for vagrant -----
sudo apt-get install -y cachefilesd
sudo echo "RUN=yes" > /etc/default/cachefilesd
