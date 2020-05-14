# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import carla
import random

from . import SyncSmokeTest


class TestSnapshot(SyncSmokeTest):
    def test_spawn_points(self):
        self.world = self.client.reload_world()

        # Check why the world settings aren't applied after a reload
        self.settings = self.world.get_settings()
        settings = carla.WorldSettings(
            no_rendering_mode=False,
            synchronous_mode=True,
            fixed_delta_seconds=0.05)
        self.world.apply_settings(settings)

        spawn_points = self.world.get_map().get_spawn_points()[:20]
        vehicles = self.world.get_blueprint_library().filter('vehicle.*')
        batch = [(random.choice(vehicles), t) for t in spawn_points]
        batch = [carla.command.SpawnActor(*args) for args in batch]
        response = self.client.apply_batch_sync(batch, True)

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
