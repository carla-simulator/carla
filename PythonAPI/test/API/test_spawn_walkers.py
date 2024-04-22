# Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import unittest
import carla

class TestWalkersSpawn(unittest.TestCase):
    def test_walker_spawn(self):
        client = carla.Client()
        world = client.load_world('Town10HD_Opt')
        bp_lib = world.get_blueprint_library()
        spectator = world.get_spectator()
        spectator.set_transform(carla.Transform(carla.Location(-27.8, -63.28, 10.6), carla.Rotation(pitch=-29, yaw=135)))

        walkers_bp_to_spawn = bp_lib.filter("*walker*")
        # Spawn the actor
        walkers = []
        counter = 0
        for bp in walkers_bp_to_spawn:
            location = carla.Location(-27.8 + counter , -61.28, 10.6)
            walker = world.spawn_actor(bp, carla.Transform(location, carla.Rotation()))
            self.assertIsNotNone(walker)
            walkers.append(walker)
            counter += 1
        
        world.tick()
        actors = world.get_actors().filter('*walker*')
        gotactors = actors.__len__()

        for _ in range(300):
            world.tick()
        
        self.assertEqual(counter, gotactors)

        for walker in walkers:
            walker.destroy()

        world.tick()
        actors = world.get_actors().filter('*walker*')
        gotactors = actors.__len__()

        self.assertEqual(0, gotactors)
