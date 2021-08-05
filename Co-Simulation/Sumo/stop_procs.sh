#! /bin/bash

# kill sumo-traci servers
ps awx | grep sumo | awk '{print $1}' | xargs kill -9
