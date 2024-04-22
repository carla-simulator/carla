# Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import unittest
import carla


class TestVehiclesSpawnTest(unittest.TestCase):
    def test_vehicle_spawn(self):
        client = carla.Client()
        world = client.load_world('Town10HD_Opt')
        bp_lib = world.get_blueprint_library()
        spectator = world.get_spectator()
        spectator.set_transform(carla.Transform(carla.Location(-27.8, -63.28, 10.6), carla.Rotation(pitch=-29, yaw=135)))

        vehicles_bp_to_spawn = bp_lib.filter("*vehicle*")
        # Spawn the actor
        vehicles = []
        counter = 0
        for bp in vehicles_bp_to_spawn:
            location = carla.Location(-70.8 + (counter * 4) , -61.28, 6.0)
            vehicle = world.spawn_actor(bp, carla.Transform(location, carla.Rotation(0,-90,0)))
            self.assertIsNotNone(vehicle)
            vehicles.append(vehicle)
            counter += 1
        
        # Tick for Engine do it's stuff
        world.tick()
        actors = world.get_actors().filter('*vehicle*')
        gotactors = actors.__len__()

        for _ in range(300):
            world.tick()
        
        self.assertEqual(counter, gotactors)

        for vehicle in vehicles:
            vehicle.destroy()

        world.tick()
        actors = world.get_actors().filter('*vehicle*')
        gotactors = actors.__len__()

        self.assertEqual(0, gotactors)
