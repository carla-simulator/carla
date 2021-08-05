#! /bin/bash

# kill sumo-traci servers
ps awx | grep sumo | awk '{print $1}' | xargs kill -9

# kill carla-sumo synchronization
ps awx | grep my_synchronization.py | awk '{print $1}' | xargs kill -9

# kill veins-sumo synchronization
ps awx | grep my-sumo-launched.py | awk '{print $1}' | xargs kill -9

# kill tracis synchronization
ps awx | grep run_tracis_synchronization.py | awk '{print $1}' | xargs kill -9
