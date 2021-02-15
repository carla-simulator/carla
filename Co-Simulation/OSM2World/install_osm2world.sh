#! /bin/bash

# ----- download osm2world -----
wget http://osm2world.org/download/files/0.2.0/OSM2World-0.2.0-bin.zip
unzip OSM2World-0.2.0-bin.zip

wget http://osm2world.org/download/files/misc/OSM2World%20standard%20style%202019-07-09.zip
unzip 'OSM2World standard style 2019-07-09.zip' 

# ----- install jre -----
sudo apt install -y default-jre

# ----- obj viewer -----
sudo apt -y install meshlab

# ----- add OSM2WORLD_HOME to .bashrc -----
echo "OSM2WORLD_HOME=$PWD" >> ~/.bashrc
