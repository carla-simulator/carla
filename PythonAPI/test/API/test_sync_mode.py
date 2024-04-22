#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

from __future__ import print_function
import random
import unittest
import glob
import os
import sys
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

class TestSyncMode(unittest.TestCase):
    def test_sync_mode(self):
        client = carla.Client()
        actor = None

        world = client.get_world()
        client.set_timeout(100.0)
        spectator = world.get_spectator()

        settings = world.get_settings()
        settings.synchronous_mode = True
        settings.fixed_delta_seconds = 0.05
        world.apply_settings(settings)

        # Spawn the actor
        bp = world.get_blueprint_library().filter("*vendingmachine*")[0]
        transform = world.get_map().get_spawn_points()[0]
        actor = world.spawn_actor(bp, transform)
        world.tick()

        spectator.set_transform(carla.Transform(carla.Location(5,10,20), carla.Rotation(pitch=-90)))

        for i in range(400):
            new_tran = carla.Transform(
                carla.Location(
                    x = random.randint(0, 300),
                    y = random.randint(0, 300),
                    z = random.randint(0, 300),
                ),
                carla.Rotation(
                    roll = 0,
                    pitch = 0,
                    yaw = 0
                )
            )

            actor.set_transform(new_tran)
            world.tick()
            transfor = actor.get_transform()
            spectator.set_transform(carla.Transform(new_tran.location - carla.Location(
                    x = 10,
                    y = 0,
                    z = 0,
                )))

            self.assertEqual(transfor.location.x, new_tran.location.x)
            self.assertEqual(transfor.location.y, new_tran.location.y)
            self.assertEqual(transfor.location.z, new_tran.location.z)

        actor.destroy()

