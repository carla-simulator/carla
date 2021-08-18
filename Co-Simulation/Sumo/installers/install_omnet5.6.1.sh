#! /bin/bash
sudo apt-get install -y build-essential gcc g++ bison flex perl tcl-dev tk-dev blt libxml2-dev zlib1g-dev default-jre doxygen graphviz libwebkitgtk-1.0-0 openmpi-bin libopenmpi-dev libpcap-dev autoconf automake libtool libproj-dev libfox-1.6-dev libgdal-dev libxerces-c-dev qt4-dev-tools
sudo apt-get install -y qt5-qmake qt5-default
sudo add-apt-repository -y ppa:ubuntugis/ppa
sudo apt update
sudo apt install -y libosgearth-dev
sudo add-apt-repository -y -r ppa:ubuntugis/ppa
# ----- install omnet++ -----
cd ~/Downloads/
wget https://github.com/omnetpp/omnetpp/releases/download/omnetpp-5.6.1/omnetpp-5.6.1-src-linux.tgz
tar -xzvf omnetpp-5.6.1-src-linux.tgz 
cd omnetpp-5.6.1/
source setenv
./configure
make -j $(nproc)
# ----- set env to .bashrc -----
echo "CURRENT_PWD=\$(pwd)" >> ~/.bashrc
echo "cd ~/Downloads/omnetpp-5.6.1/" >> ~/.bashrc
echo "source setenv" >> ~/.bashrc
echo "cd CURRENT_PWD" >> ~/.bashrc


