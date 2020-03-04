#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Script to randomly spawn vehicles in sumo.
"""

# ==============================================================================
# -- find traci module ---------------------------------------------------------
# ==============================================================================

import glob
import os
import sys

if 'SUMO_HOME' in os.environ:
    tools = os.path.join(os.environ['SUMO_HOME'], 'tools')
    sys.path.append(tools)
else:
    sys.exit("please declare environment variable 'SUMO_HOME'")

# ==============================================================================
# -- imports -------------------------------------------------------------------
# ==============================================================================

import argparse
import logging
import random

import sumolib
import traci

# ==============================================================================
# -- main ----------------------------------------------------------------------
# ==============================================================================

def get_random_color():
    r = random.randrange(256)
    g = random.randrange(256)
    b = random.randrange(256)
    a = 255
    return (r, g, b, a)

def main(args):
    try:
        if args.sumo_gui is True:
            sumo_binary = sumolib.checkBinary('sumo-gui')
        else:
            sumo_binary = sumolib.checkBinary('sumo')

        if args.sumo_host is None or args.sumo_port is None:
            logging.info('Starting new sumo server...')
            if args.sumo_gui:
                logging.info('Remember to press the play button in sumo-gui to start the simulation')

            traci.start([
                sumo_binary,
                "-c", args.sumo_cfg_file,
                '--step-length', str(args.step_length),
            ])
        else:
            logging.info('Connection to sumo server. Host: {} Port: {}'.format(args.sumo_host, args.sumo_port))
            traci.init(host=args.sumo_host, port=args.sumo_port)
            traci.setOrder(1)

        # Creating a random route to be able to spawn vehicles.
        traci.route.add("spawn_route", [traci.edge.getIDList()[0]])

        sequential_id = 0

        while True:
            input('Press a key to spawn a new actor')

            # Remove previously spawned vehicle.
            if sequential_id > 0:
                traci.vehicle.remove('spawn_' + str(sequential_id - 1))

            type_id = random.choice(traci.vehicletype.getIDList())

            actor_id = 'spawn_' + str(sequential_id)
            traci.vehicle.add(actor_id, 'spawn_route', typeID=type_id)
            traci.vehicle.setColor(actor_id, get_random_color())  # Randomly select color.

            traci.simulationStep()
            traci.simulationStep()
            sequential_id += 1

            logging.info('''Spawned new sumo vehicle:
                \tvtype: {vtype:}
                \tcolor: {color:}'''.format(
                    vtype=type_id,
                    color=traci.vehicle.getColor(actor_id)
                )
            )

    except KeyboardInterrupt:
        logging.info('Cancelled by user.')

    finally:
        traci.close()

if __name__ == '__main__':
    argparser = argparse.ArgumentParser(
        description=__doc__
    )
    argparser.add_argument(
        '--sumo-host',
        metavar='H',
        default=None,
        help='IP of the sumo host server (default: 127.0.0.1)'
    )
    argparser.add_argument(
        '--sumo-port',
        metavar='P',
        default=None,
        type=int,
        help='TCP port to liston to (default: 8813)'
    )
    argparser.add_argument(
        '-c', '--sumo-cfg-file',
        default=None,
        type=str,
        help='sumo configuration file'
    )
    argparser.add_argument(
        '--sumo-gui',
        default=True,
        help='run the gui version of sumo (default: True)'
    )
    argparser.add_argument(
        '--step-length',
        default=0.05,
        type=float,
        help='set fixed delta seconds (default: 0.05s)'
    )
    args = argparser.parse_args()

    logging.basicConfig(format='%(levelname)s: %(message)s', level=logging.INFO)

    main(args)