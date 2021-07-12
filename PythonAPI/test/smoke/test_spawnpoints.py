# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import carla
import time

from . import SyncSmokeTest


class TestSpawnpoints(SyncSmokeTest):
    def test_spawn_points(self):
        print("TestSpawnpoints.test_spawn_points")
        self.world = self.client.get_world()
        blueprints = self.world.get_blueprint_library().filter("vehicle.*")

        # get all available maps
        maps = self.client.get_available_maps()
        for m in maps:

            # load the map
            self.client.load_world(m)
            # workaround: give time to UE4 to clean memory after loading (old assets)
            time.sleep(5)
            
            self.world = self.client.get_world()

            # get all spawn points
            spawn_points = self.world.get_map().get_spawn_points()

            # Check why the world settings aren't applied after a reload
            self.settings = self.world.get_settings()
            settings = carla.WorldSettings(
                no_rendering_mode=False,
                synchronous_mode=True,
                fixed_delta_seconds=0.05)
            self.world.apply_settings(settings)

            # spawn all kind of vehicle
            for vehicle in blueprints:
                batch = [(vehicle, t) for t in spawn_points]
                batch = [carla.command.SpawnActor(*args) for args in batch]
                response = self.client.apply_batch_sync(batch, False)

                self.assertFalse(any(x.error for x in response))
                ids = [x.actor_id for x in response]
                self.assertEqual(len(ids), len(spawn_points))

                frame = self.world.tick()
                snapshot = self.world.get_snapshot()
                self.assertEqual(frame, snapshot.timestamp.frame)

                actors = self.world.get_actors()
                self.assertTrue(all(snapshot.has_actor(x.id) for x in actors))

                for actor_id, t0 in zip(ids, spawn_points):
                    actor_snapshot = snapshot.find(actor_id)
                    self.assertIsNotNone(actor_snapshot)
                    t1 = actor_snapshot.get_transform()
                    # Ignore Z cause vehicle is falling.
                    self.assertAlmostEqual(t0.location.x, t1.location.x, places=2)
                    self.assertAlmostEqual(t0.location.y, t1.location.y, places=2)
                    self.assertAlmostEqual(t0.rotation.pitch, t1.rotation.pitch, places=2)
                    self.assertAlmostEqual(t0.rotation.yaw, t1.rotation.yaw, places=2)
                    self.assertAlmostEqual(t0.rotation.roll, t1.rotation.roll, places=2)

                self.client.apply_batch_sync([carla.command.DestroyActor(x) for x in ids], True)
                frame = self.world.tick()
