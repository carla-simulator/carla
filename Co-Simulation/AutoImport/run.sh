#! /bin/bash

python setup_for_carla.py
python setup_for_sumo.py
echo ""
echo "Run the following commands on your terminal."
echo ""
echo "----- commands in pipenv shell -----"
echo "exit "
echo ""
echo "----- commands in your shell -----"
echo "cd ~/carla/"
echo "make import"
echo "cd ~/carla/Co-Simulation/Sumo/auto_import"
echo "pipenv shell"
