#!/bin/bash

# SUMO Activity-Based Mobility Generator - MoST Scenario example
#
# Author: Lara CODECA
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# http://www.eclipse.org/legal/epl-2.0.

# exit on error
set -e

MOBILITY_GENERATOR=$(pwd)
ACTIVITYGEN=$MOBILITY_GENERATOR/..

if [ ! -d "MoSTScenario" ]
then
  git clone https://github.com/lcodeca/MoSTScenario.git
else
  cd MoSTScenario
  git pull
  cd $MOBILITY_GENERATOR
fi

SCENARIO="$MOBILITY_GENERATOR/MoSTScenario/tools"

OUTPUT="most/rou"
mkdir -p $OUTPUT

## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ PUBLIC TRANSPORTS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##

INTERVAL="-b 0 -e 86400"

echo "[$(date)] --> Generate bus trips..."
python $SUMO_TOOLS/ptlines2flows.py -n $SCENARIO/out/most.net.xml $INTERVAL -p 900 \
    --random-begin --seed 42 --no-vtypes \
    --ptstops $SCENARIO/out/most.busstops.add.xml --ptlines $SCENARIO/out/most.buslines.add.xml \
    -o $OUTPUT/most.example.buses.flows.xml

sed -e s/:0//g -i'' $OUTPUT/most.example.buses.flows.xml

echo "[$(date)] --> Generate train trips..."
python $SUMO_TOOLS/ptlines2flows.py -n $SCENARIO/out/most.net.xml $INTERVAL -p 1200 \
    -d 300 --random-begin --seed 42 --no-vtypes \
    --ptstops $SCENARIO/out/most.trainstops.add.xml --ptlines $SCENARIO/out/most.trainlines.add.xml \
    -o $OUTPUT/most.example.trains.flows.xml

sed -e s/:0//g -i'' $OUTPUT/most.example.trains.flows.xml

## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ TRACI MOBILITY GENERATION ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##

echo "[$(date)] --> Generate mobility..."
python3 $ACTIVITYGEN/activitygen.py -c most.activitygen.json
echo "[$(date)] Routes in the most/rou directory..."

## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SUMO SIMULATION ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##

mkdir -p "most/sim"
echo "[$(date)] --> Running the SUMO simulation..."
sumo -c most.test.sumocfg
echo "[$(date)] Results in the most/sim directory..."
