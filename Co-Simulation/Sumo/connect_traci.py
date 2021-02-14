
# coding: utf-8

import argparse
import logging
import os
import sys

if 'SUMO_HOME' in os.environ:
    sys.path.append(os.path.join(os.environ['SUMO_HOME'], 'tools'))
    import traci
else:
    sys.exit("Please declare environment variable 'SUMO_HOME'")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='This script is a middleware for tracis synchronization.')

    parser.add_argument('--port', type=int)
    parser.add_argument('--host', default="localhost")

    args = parser.parse_args()

    traci_obj = traci.connect(port=args.port, host=args.host)

    while 0 < traci_obj.simulation.getMinExpectedNumber():
        traci_obj.simulationStep()

    traci_obj.close()
