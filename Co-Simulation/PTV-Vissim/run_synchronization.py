#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.
"""
Script to co-simulate CARLA and PTV-Vissim.
"""

# ==================================================================================================
# -- imports ---------------------------------------------------------------------------------------
# ==================================================================================================

import argparse
import json
import logging
import time

# ==================================================================================================
# -- find carla module -----------------------------------------------------------------------------
# ==================================================================================================

import glob
import os
import sys

try:
    sys.path.append(
        glob.glob('../../PythonAPI/carla/dist/carla-*%d.%d-%s.egg' %
                  (sys.version_info.major, sys.version_info.minor,
                   'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

# ==================================================================================================
# -- vissim integration imports --------------------------------------------------------------------
# ==================================================================================================

from vissim_integration.bridge_helper import BridgeHelper
from vissim_integration.carla_simulation import CarlaSimulation
from vissim_integration.vissim_simulation import PTVVissimSimulation
from vissim_integration.constants import INVALID_ACTOR_ID

# ==================================================================================================
# -- synchronization_loop --------------------------------------------------------------------------
# ==================================================================================================


class SimulationSynchronization(object):
    """
    SimulationSynchronization class is responsible for the synchronization of ptv-vissim and carla
    simulations.
    """
    def __init__(self, vissim_simulation, carla_simulation, args):
        self.args = args

        self.vissim = vissim_simulation
        self.carla = carla_simulation

        # Mapped actor ids.
        self.vissim2carla_ids = {}  # Contains only actors controlled by vissim.
        self.carla2vissim_ids = {}  # Contains only actors controlled by carla.

        BridgeHelper.blueprint_library = self.carla.world.get_blueprint_library()
        dir_path = os.path.dirname(os.path.realpath(__file__))
        with open(os.path.join(dir_path, 'data', 'vtypes.json')) as f:
            BridgeHelper.vtypes = json.load(f)

        # Configuring carla simulation in sync mode.
        settings = self.carla.world.get_settings()
        settings.synchronous_mode = True
        settings.fixed_delta_seconds = args.step_length
        self.carla.world.apply_settings(settings)

    def tick(self):
        """
        Tick to simulation synchronization
        """
        # -------------------
        # vissim-->carla sync
        # -------------------
        self.vissim.tick()

        # Spawning vissim controlled vehicles in carla.
        vissim_spawned_actors = self.vissim.spawned_vehicles - set(self.carla2vissim_ids.values())
        for vissim_actor_id in vissim_spawned_actors:
            vissim_actor = self.vissim.get_actor(vissim_actor_id)

            carla_blueprint = BridgeHelper.get_carla_blueprint(vissim_actor)
            if carla_blueprint is not None:
                carla_transform = BridgeHelper.get_carla_transform(vissim_actor.get_transform())
                carla_actor_id = self.carla.spawn_actor(carla_blueprint, carla_transform)

                if carla_actor_id != INVALID_ACTOR_ID:
                    self.vissim2carla_ids[vissim_actor_id] = carla_actor_id

        # Destroying vissim controlled vehicles in carla.
        for vissim_actor_id in self.vissim.destroyed_vehicles:
            if vissim_actor_id in self.vissim2carla_ids:
                self.vissim.destroy_actor(self.vissim2carla_ids.pop(vissim_actor_id))

        # Updating vissim controlled vehicles in carla.
        for vissim_actor_id in self.vissim2carla_ids:
            carla_actor_id = self.vissim2carla_ids[vissim_actor_id]

            vissim_actor = self.vissim.get_actor(vissim_actor_id)
            carla_actor = self.carla.get_actor(carla_actor_id)

            carla_transform = BridgeHelper.get_carla_transform(vissim_actor.get_transform(),
                                                               carla_actor.bounding_box.extent)
            carla_velocity = BridgeHelper.get_carla_velocity(vissim_actor.get_velocity())
            self.carla.synchronize_vehicle(carla_actor_id, carla_transform, carla_velocity)

        # -------------------
        # carla-->vissim sync
        # -------------------
        self.carla.tick()

        # Spawning carla controlled vehicles in vissim. This also takes into account carla vehicles
        # that could not be spawned in vissim in previous time steps.
        carla_spawned_actors = self.carla.spawned_actors - set(self.vissim2carla_ids.values())
        carla_spawned_actors.update(
            [c_id for c_id, v_id in self.carla2vissim_ids.items() if v_id == INVALID_ACTOR_ID])
        for carla_actor_id in carla_spawned_actors:
            carla_actor = self.carla.get_actor(carla_actor_id)

            vissim_transform = BridgeHelper.get_vissim_transform(carla_actor.get_transform())
            vissim_actor_id = self.vissim.spawn_actor(vissim_transform)

            # Add the vissim_actor_id even if it was not possible to spawn it (INVALID_ACTOR_ID) to
            # try to spawn it again in next time steps.
            self.carla2vissim_ids[carla_actor_id] = vissim_actor_id

        # Destroying carla controlled vehicles in vissim.
        for carla_actor_id in self.carla.destroyed_actors:
            if carla_actor_id in self.carla2vissim_ids:
                self.vissim.destroy_actor(self.carla2vissim_ids.pop(carla_actor_id))

        # Updating carla controlled vehicles in vissim.
        for carla_actor_id in self.carla2vissim_ids:
            vissim_actor_id = self.carla2vissim_ids[carla_actor_id]
            if vissim_actor_id != INVALID_ACTOR_ID:
                carla_actor = self.carla.get_actor(carla_actor_id)

                vissim_transform = BridgeHelper.get_vissim_transform(
                    carla_actor.get_transform(), carla_actor.bounding_box.extent)
                vissim_velocity = BridgeHelper.get_vissim_velocity(carla_actor.get_velocity())
                self.vissim.synchronize_vehicle(vissim_actor_id, vissim_transform, vissim_velocity)

    def close(self):
        """
        Cleans synchronization.
        """
        # Configuring carla simulation in async mode.
        settings = self.carla.world.get_settings()
        settings.synchronous_mode = False
        settings.fixed_delta_seconds = None
        self.carla.world.apply_settings(settings)

        # Destroying synchronized actors.
        for carla_actor_id in self.vissim2carla_ids.values():
            self.carla.destroy_actor(carla_actor_id)

        # Closing PTV-Vissim connection.
        self.vissim.close()


def synchronization_loop(args):
    """
    Entry point for vissim-carla co-simulation.
    """
    carla_simulation = CarlaSimulation(args)
    vissim_simulation = PTVVissimSimulation(args)

    try:
        synchronization = SimulationSynchronization(vissim_simulation, carla_simulation, args)

        while True:
            start = time.time()

            synchronization.tick()

            end = time.time()
            elapsed = end - start
            if elapsed < args.step_length:
                time.sleep(args.step_length - elapsed)

    except KeyboardInterrupt:
        logging.info('Cancelled by user.')

    finally:
        logging.info('Cleaning synchronization')
        synchronization.close()


# ==================================================================================================
# -- main ------------------------------------------------------------------------------------------
# ==================================================================================================

if __name__ == '__main__':
    argparser = argparse.ArgumentParser(description=__doc__)
    argparser.add_argument('vissim_network', type=str, help='vissim network file')
    argparser.add_argument('--carla-host',
                           metavar='H',
                           default='127.0.0.1',
                           help='IP of the carla host server (default: 127.0.0.1)')
    argparser.add_argument('--carla-port',
                           metavar='P',
                           default=2000,
                           type=int,
                           help='TCP port to listen to (default: 2000)')
    argparser.add_argument('--vissim-version',
                           default=2020,
                           type=int,
                           help='ptv-vissim version (default: 2020)')
    argparser.add_argument('--step-length',
                           default=0.05,
                           type=float,
                           help='set fixed delta seconds (default: 0.05s)')
    argparser.add_argument('--simulator-vehicles',
                           default=1,
                           type=int,
                           help='number of simulator vehicles to be passed to vissim (default: 1)')
    argparser.add_argument('--debug', action='store_true', help='enable debug messages')
    arguments = argparser.parse_args()

    if arguments.debug:
        logging.basicConfig(format='%(levelname)s: %(message)s', level=logging.DEBUG)
    else:
        logging.basicConfig(format='%(levelname)s: %(message)s', level=logging.INFO)

    synchronization_loop(arguments)
