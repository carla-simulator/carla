#! /bin/bash

netconvert --osm ../Sumo/osm_web_wizard/2021-02-01-12-42-25/osm_bbox.osm.xml --opendrive-output ../Sumo/osm_web_wizard/2021-02-01-12-42-25/Tmp1.xodr
cp ../Sumo/osm_web_wizard/2021-02-01-12-42-25/Tmp1.xodr /home/yoshida/carla/Unreal/CarlaUE4/Content/Carla/Maps/OpenDrive/
