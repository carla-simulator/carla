#!/bin/bash
$SUMO_HOME/tools/game/DRT/randomRides.py --seed 42 -n osm.net.xml -a drtstops.xml -o rides.xml      --poi-output drt.pois.xml --prefix a -b 0   -e 2000  -p 120  --poi-offset 32 --initial-duration 3
$SUMO_HOME/tools/game/DRT/randomRides.py --seed 42 -n osm.net.xml -a drtstops.xml -o rides2.xml     --poi-output drt.pois.xml --prefix a -b 0   -e 2000  -p 120  --poi-offset 32 --initial-duration 3
$SUMO_HOME/tools/game/DRT/randomRides.py --seed 42 -n osm.net.xml -a drtstops.xml -o rides3.xml     --poi-output drt.pois.xml --prefix b -b 600 -e 1600  -p 100  --poi-offset 32 --initial-duration 3
$SUMO_HOME/tools/game/DRT/randomRides.py --seed 42 -n osm.net.xml -a drtstops.xml -o rides_demo.xml --poi-output drt.pois.xml --prefix d -b 0   -e 20000 -p 200  --poi-offset 32 --initial-duration 3
