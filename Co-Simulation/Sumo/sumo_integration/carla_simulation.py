#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# ==============================================================================
# -- imports -------------------------------------------------------------------
# ==============================================================================

import logging

import carla
SpawnActor = carla.command.SpawnActor
FutureActor = carla.command.FutureActor
SetSimulatePhysics = carla.command.SetSimulatePhysics

from .constants import *

# ==============================================================================
# -- carla simulation ----------------------------------------------------------
# ==============================================================================


class CarlaSimulation(object):
    def __init__(self, args):
        self.args = args
        self.host = args.carla_host
        self.port = args.carla_port
        self.step_length = args.step_length

        self.client = carla.Client(self.host, self.port)
        self.client.set_timeout(2.0)

        self.world = self.client.get_world()
        self.blueprint_library = self.world.get_blueprint_library()

        # Configuring carla simulation in sync mode.
        settings = self.world.get_settings()
        settings.synchronous_mode = True
        settings.fixed_delta_seconds = self.step_length
        self.world.apply_settings(settings)

        # The following sets contain updated information for the current frame.
        self._active_actors = set()
        self.spawned_actors = set()
        self.destroyed_actors = set()

    def get_actor(self, actor_id):
        return self.world.get_actor(actor_id)

    # This is a workaround to fix synchronization issues when other carla
    # clients remove an actor in carla without waiting for tick (e.g.,
    # running sumo co-simulation and manual control at the same time)
    def get_actor_light_state(self, actor_id):
        try:
            actor = self.get_actor(actor_id)
            return actor.get_light_state()
        except RuntimeError:
            return None

    def spawn_actor(self, blueprint, transform):
        """Spawns a new actor.
        """
        transform = carla.Transform(
            transform.location + carla.Location(0, 0, SPAWN_OFFSET_Z),
            transform.rotation)

        batch = [
            SpawnActor(blueprint,transform)
                .then(SetSimulatePhysics(FutureActor, False))
        ]
        response = self.client.apply_batch_sync(batch, True)[0]
        if response.error:
            logging.error('Spawn carla actor failed. {}'.format(response.error))
            return INVALID_ACTOR_ID
        else:
            return response.actor_id

    def destroy_actor(self, actor_id):
        actor = self.world.get_actor(actor_id)
        if actor is not None:
            return actor.destroy()
        return False

    def synchronize_vehicle(self, vehicle_id, transform, lights=None):
        vehicle = self.world.get_actor(vehicle_id)
        if vehicle is None:
            return False

        vehicle.set_transform(transform)
        if lights is not None and self.args.sync_vehicle_lights:
            vehicle.set_light_state(carla.VehicleLightState(lights))

    def synchronize_walker(self, walker_id, transform):
        pass

    def tick(self):
        self.world.tick()

        # Update data structures for the current frame.
        current_actors = set([
            vehicle.id for vehicle in self.world.get_actors().filter('vehicle.*')
        ])
        self.spawned_actors = current_actors.difference(self._active_actors)
        self.destroyed_actors = self._active_actors.difference(current_actors)
        self._active_actors = current_actors