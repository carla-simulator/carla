#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Script to integrate CARLA and SUMO simulations
"""

# ==============================================================================
# -- find carla module ---------------------------------------------------------
# ==============================================================================

import glob
import os
import sys

try:
    sys.path.append(glob.glob('../../PythonAPI/carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

# ==============================================================================
# -- find traci module ---------------------------------------------------------
# ==============================================================================

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
import time

from sumo_integration.bridge_helper import BridgeHelper
from sumo_integration.carla_simulation import CarlaSimulation
from sumo_integration.constants import *
from sumo_integration.sumo_simulation import SumoSimulation

# ==============================================================================
# -- simulation synchro --------------------------------------------------------
# ==============================================================================

def main(args):
    sumo = SumoSimulation(args)
    carla = CarlaSimulation(args)

    # Mapped actor ids.
    sumo2carla_ids = {}  # Contains only actors controlled by sumo.
    carla2sumo_ids = {}  # Contains only actors controlled by carla.

    BridgeHelper._blueprint_library = carla.world.get_blueprint_library()
    BridgeHelper._offset = sumo.get_net_offset()

    try:
        while True:
            start = time.time()

            # -----------------
            # sumo-->carla sync
            # -----------------
            sumo.tick()

            # Spawning new sumo actors in carla (i.e, not controlled by carla).
            sumo_spawned_actors = sumo.spawned_actors - set(carla2sumo_ids.values())
            for sumo_actor_id in sumo_spawned_actors:
                SumoSimulation.subscribe(sumo_actor_id)
                sumo_actor = sumo.get_actor(sumo_actor_id)

                carla_blueprint = BridgeHelper.get_carla_blueprint(sumo_actor)
                if carla_blueprint is not None:
                    carla_transform = BridgeHelper.get_carla_transform(sumo_actor.transform, sumo_actor.extent)

                    carla_actor_id = carla.spawn_actor(carla_blueprint, carla_transform)
                    if carla_actor_id != INVALID_ACTOR_ID:
                        sumo2carla_ids[sumo_actor_id] = carla_actor_id
                else:
                    SumoSimulation.unsubscribe(sumo_actor_id)

            # Destroying sumo arrived actors in carla.
            for sumo_actor_id in sumo.destroyed_actors:
                if sumo_actor_id in sumo2carla_ids:
                    carla.destroy_actor(sumo2carla_ids.pop(sumo_actor_id))

            # Updating sumo actors in carla.
            for sumo_actor_id in sumo2carla_ids:
                carla_actor_id = sumo2carla_ids[sumo_actor_id]

                sumo_actor = sumo.get_actor(sumo_actor_id)
                carla_actor = carla.get_actor(carla_actor_id)

                carla_transform = BridgeHelper.get_carla_transform(sumo_actor.transform, sumo_actor.extent)
                if args.sync_vehicle_lights:
                    carla_lights = BridgeHelper.get_carla_lights_state(carla_actor.get_light_state(), sumo_actor.signals)
                else:
                    carla_lights = None

                carla.synchronize_vehicle(carla_actor_id, carla_transform, carla_lights)

            # -----------------
            # carla-->sumo sync
            # -----------------
            carla.tick()

            # Spawning new carla actors (not controlled by sumo)
            carla_spawned_actors = carla.spawned_actors - set(sumo2carla_ids.values())
            for carla_actor_id in carla_spawned_actors:
                carla_actor = carla.get_actor(carla_actor_id)

                type_id = BridgeHelper.get_sumo_vtype(carla_actor)
                if type_id is not None:
                    sumo_actor_id = sumo.spawn_actor(type_id, carla_actor.attributes)
                    if sumo_actor_id != INVALID_ACTOR_ID:
                        carla2sumo_ids[carla_actor_id] = sumo_actor_id
                        sumo.subscribe(sumo_actor_id)

            # Destroying required carla actors in sumo.
            for carla_actor_id in carla.destroyed_actors:
                if carla_actor_id in carla2sumo_ids:
                    sumo.destroy_actor(carla2sumo_ids.pop(carla_actor_id))

            # Updating carla actors in sumo.
            for carla_actor_id in carla2sumo_ids:
                sumo_actor_id = carla2sumo_ids[carla_actor_id]

                carla_actor = carla.get_actor(carla_actor_id)
                sumo_actor = sumo.get_actor(sumo_actor_id)

                sumo_transform = BridgeHelper.get_sumo_transform(carla_actor.get_transform(), carla_actor.bounding_box.extent)
                if args.sync_vehicle_lights:
                    carla_lights = carla_actor.get_light_state()
                    sumo_lights = BridgeHelper.get_sumo_lights_state(sumo_actor.signals, carla_lights)
                else:
                    sumo_lights = None

                sumo.synchronize_vehicle(sumo_actor_id, sumo_transform, sumo_lights)

            end = time.time()
            elapsed = end - start
            if elapsed < args.step_length:
                time.sleep(args.step_length - elapsed)

    except KeyboardInterrupt:
        logging.info('Cancelled by user.')

    except Exception as error:
        logging.error('Synchronization failed. {}'.format(error))

    finally:
        logging.info('Cleaning up synchronization')

        # Configuring carla simulation in async mode.
        settings = carla.world.get_settings()
        settings.synchronous_mode = False
        settings.fixed_delta_seconds = None
        carla.world.apply_settings(settings)

        # Destroying synchronized actors.
        for carla_actor_id in sumo2carla_ids.values():
            carla.destroy_actor(carla_actor_id)

        for sumo_actor_id in carla2sumo_ids.values():
            sumo.destroy_actor(sumo_actor_id)

        # Closing sumo client.
        sumo.close()

if __name__ == '__main__':
    argparser = argparse.ArgumentParser(
        description=__doc__
    )
    argparser.add_argument(
        '--carla-host',
        metavar='H',
        default='127.0.0.1',
        help='IP of the carla host server (default: 127.0.0.1)'
    )
    argparser.add_argument(
        '--carla-port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)'
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
    argparser.add_argument(
        '--sync-vehicle-lights',
        action='store_true',
        help='synchronize vehicle lights state between simulations (default: False)'
    )
    argparser.add_argument(
        '--debug',
        action='store_true',
        help='enable debug messages'
    )
    args = argparser.parse_args()

    if args.debug:
        logging.basicConfig(format='%(levelname)s: %(message)s', level=logging.DEBUG)
    else:
        logging.basicConfig(format='%(levelname)s: %(message)s', level=logging.INFO)

    main(args)