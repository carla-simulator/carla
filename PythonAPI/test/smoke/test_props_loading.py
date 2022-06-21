# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import carla
import random

from . import SmokeTest


class TestPropsLoading(SmokeTest):
    def test_spawn_loaded_props(self):
        print("TestPropsLoading.test_spawn_loaded_props")
        client = self.client
        world = client.get_world()

        SpawnActor = carla.command.SpawnActor

        props = world.get_blueprint_library().filter("static.prop.*")
        spawned_props = []

        spawn_points = world.get_map().get_spawn_points()

        z = 0
        batch = []
        for prop in props:
            spawn_point = random.choice(spawn_points)
            spawn_point.location.z += z

            batch.append(SpawnActor(prop, spawn_point))
            z += 100

        for response in client.apply_batch_sync(batch):
            self.assertFalse(response.error)
            spawned_props.append(response.actor_id)

        self.assertEqual(len(spawned_props), len(props))
