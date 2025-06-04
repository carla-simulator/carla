
from __future__ import print_function

import unittest
import argparse
import glob
import math
import os
import sys
import time
try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass
try:
    sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))) + '/carla')
except IndexError:
    pass

import carla


#record the result as json to the same folder


class TestCollision(unittest.TestCase):
    def setUp(self):
        self.collisions = 0

    def _on_collision(self, event ):
        self.collisions += 1

    def test_collision_against_side_of_car(self):
        client = carla.Client()
        world = client.load_world('Town01')
        bp_lib = world.get_blueprint_library()
        spectator = world.get_spectator()

        # Spawn the actor
        bp = bp_lib.filter("*walker*")[0]
        # bp.set_attribute('is_invincible', 'false')
        walker = world.spawn_actor(bp, carla.Transform(carla.Location(200.7, 199.3, 0.2), carla.Rotation()))

        bp = bp_lib.filter("*mkz_2020*")[0]
        vehicle = world.spawn_actor(bp, carla.Transform(carla.Location(177.7, 198.8, 0.2), carla.Rotation()))
        spectator.set_transform(carla.Transform(carla.Location(205.9, 193.2, 3.9), carla.Rotation(pitch=-29, yaw=135)))

        collision_bp = bp_lib.find('sensor.other.collision')
        sensor_collision = world.spawn_actor(collision_bp, carla.Transform(), attach_to=vehicle)
        sensor_collision.listen(lambda event: self._on_collision(self. event))

        vehicle.apply_control(carla.VehicleControl(throttle=1))

        for _ in range(400):
            world.tick()

        walker.destroy()
        sensor_collision.destroy()
        vehicle.destroy()

        self.assertNotEqual(self.collisions, 0)

