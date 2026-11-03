# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import glob
import os
import sys
import unittest

try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla
import math
import time

TESTING_ADDRESS = ('localhost', 3654)
# A map is ready when the server reports it as the current episode. Town13 is
# 12478 spawn points and an 89 MB OpenDRIVE, so the ceiling has to be generous.
MAP_READY_TIMEOUT = 300.0
MAP_READY_POLL = 1.0
VEHICLE_VEHICLES_EXCLUDE_FROM_OLD_TOWNS = ['vehicle.fuso.mitsubishi', 'vehicle.carlacola.actors', 'vehicle.firetruck.actors']

class SmokeTest(unittest.TestCase):
    def setUp(self):
        self.testing_address = TESTING_ADDRESS
        self.client = carla.Client(*TESTING_ADDRESS)
        self.vehicle_vehicles_exclude_from_old_towns = VEHICLE_VEHICLES_EXCLUDE_FROM_OLD_TOWNS
        self.client.set_timeout(120.0)
        self.world = self.client.get_world()
        self.initial_map_name = self.world.get_map().name.split('/')[-1]

    def tearDown(self):
        try:
            self.load_world(self.initial_map_name)
        except RuntimeError:
            self.reload_world()
        self.world = None
        self.client = None

    def wait_for_map(self, map_name, previous_id=None):
        """Block until the server's current episode is map_name, and return it."""
        wanted = map_name.split('/')[-1]
        deadline = time.time() + MAP_READY_TIMEOUT
        while time.time() < deadline:
            world = self.client.get_world()
            if previous_id is None or world.id != previous_id:
                if world.get_map().name.split('/')[-1] == wanted:
                    return world
            time.sleep(MAP_READY_POLL)
        self.fail("%s was not the current map after %.0f s"
                  % (map_name, MAP_READY_TIMEOUT))

    def load_world(self, map_name, *args, **kwargs):
        """load_world that waits for the map instead of sleeping a fixed time.

        carla.Client.load_world returns before the episode is current, so a
        caller that queries straight away reads the previous map."""
        previous_id = self.client.get_world().id
        self.client.load_world(map_name, *args, **kwargs)
        return self.wait_for_map(map_name, previous_id)

    def reload_world(self, *args, **kwargs):
        """reload_world that waits for the map, as load_world does."""
        current = self.client.get_world().get_map().name
        previous_id = self.client.get_world().id
        self.client.reload_world(*args, **kwargs)
        return self.wait_for_map(current, previous_id)
    
    def vehicle_spawn_radius(self, blueprint):
        """Half-diagonal of a vehicle's bounding box, measured in the air."""
        probe = self.world.spawn_actor(
            blueprint, carla.Transform(carla.Location(z=500)))
        self.advance_frame()
        extent = probe.bounding_box.extent
        probe.destroy()
        return math.hypot(extent.x, extent.y)

    def advance_frame(self):
        """One frame, whichever mode the world is in."""
        settings = self.world.get_settings()
        if settings.synchronous_mode:
            return self.world.tick()
        return self.world.wait_for_tick().frame

    def spaced_spawn_points(self, spawn_points, min_distance):
        """Spawn points at least min_distance apart, greedily from the first.

        A map may place spawn points closer together than a vehicle is long,
        so filling all of them at once collides whatever the vehicle."""
        kept = []
        for point in spawn_points:
            if all(point.location.distance(k.location) >= min_distance
                   for k in kept):
                kept.append(point)
        return kept

    def filter_vehicles_for_old_towns(self, blueprint_list):
        new_list = []
        for blueprint in blueprint_list:
            if blueprint.id not in self.vehicle_vehicles_exclude_from_old_towns:
                new_list.append(blueprint)
        return new_list


class SyncSmokeTest(SmokeTest):
    def setUp(self):
        super(SyncSmokeTest, self).setUp()
        self.settings = self.world.get_settings()
        settings = carla.WorldSettings(
            no_rendering_mode=False,
            synchronous_mode=True,
            fixed_delta_seconds=0.05)
        self.world.apply_settings(settings)
        self.world.tick()

    def tearDown(self):
        self.world.apply_settings(self.settings)
        if self.settings.synchronous_mode:
            # only tick when the restored settings keep synchronous mode active
            self.world.tick()
        self.settings = None
        super(SyncSmokeTest, self).tearDown()
