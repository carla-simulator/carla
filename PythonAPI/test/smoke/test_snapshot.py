# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import carla
import random
import time

from . import SyncSmokeTest


class TestSnapshot(SyncSmokeTest):
    def test_spawn_points(self):
        print("TestSnapshot.test_spawn_points")
        self.world = self.reload_world()

        # Check why the world settings aren't applied after a reload
        self.settings = self.world.get_settings()
        settings = carla.WorldSettings(
            no_rendering_mode=False,
            synchronous_mode=True,
            fixed_delta_seconds=0.05)
        self.world.apply_settings(settings)

        # Spawn points can sit closer together than a vehicle is long, so
        # space them by the largest of the vehicles actually picked.
        vehicles = self.filter_vehicles_for_old_towns(
            self.world.get_blueprint_library().filter('vehicle.*'))
        chosen = [random.choice(vehicles) for _ in range(20)]
        radius = max(self.vehicle_spawn_radius(bp)
                     for bp in {bp.id: bp for bp in chosen}.values())
        spawn_points = self.spaced_spawn_points(
            self.world.get_map().get_spawn_points(), 2.0 * radius + 0.5)[:20]
        self.assertEqual(len(spawn_points), len(chosen))

        batch = [carla.command.SpawnActor(bp, t)
                 for bp, t in zip(chosen, spawn_points)]
        response = self.client.apply_batch_sync(batch, False)

        self.assertFalse(any(x.error for x in response))
        ids = [x.actor_id for x in response]
        self.assertEqual(len(ids), len(spawn_points))

        frame = self.world.tick()
        snapshot = self.world.get_snapshot()
        self.assertEqual(frame, snapshot.timestamp.frame)

        # Every actor is in the snapshot, or is a static actor, or is an AI
        # controller that has the transform of its walker; never two of them.
        actors = self.world.get_actors()
        by_id = {x.id: x for x in actors}
        static = {x.id for x in self.world.get_static_actors()}
        followers = {
            x.id for x in actors
            if x.type_id == "controller.ai.walker" and not snapshot.has_actor(x.id) and
            x.parent is not None and snapshot.has_actor(x.parent.id)}
        for x in actors:
            self.assertEqual(
                1, int(snapshot.has_actor(x.id)) + int(x.id in static) + int(x.id in followers), x)

        # The snapshot is exactly the other actors, and none of them is a
        # traffic light or sign: those do not move.
        self.assertEqual(len(snapshot), len(actors) - len(static) - len(followers))
        for actor_snapshot in snapshot:
            self.assertIn(actor_snapshot.id, by_id)
            self.assertFalse(by_id[actor_snapshot.id].type_id.startswith("traffic."))
        self.assertTrue(all(snapshot.has_actor(i) for i in ids))

        for actor_id, t0 in zip(ids, spawn_points):
            actor_snapshot = snapshot.find(actor_id)
            self.assertIsNotNone(actor_snapshot)
            t1 = actor_snapshot.get_transform()
            # The actor and its snapshot say the same.
            actor = by_id[actor_id]
            self.assertEqual(t1.location.distance(actor.get_transform().location), 0.0)
            self.assertEqual(t1.rotation.yaw, actor.get_transform().rotation.yaw)
            self.assertEqual(actor_snapshot.get_velocity(), actor.get_velocity())
            self.assertEqual(actor_snapshot.get_angular_velocity(), actor.get_angular_velocity())
            self.assertEqual(actor_snapshot.get_acceleration(), actor.get_acceleration())
            # Ignore Z cause vehicle is falling.
            self.assertAlmostEqual(t0.location.x, t1.location.x, places=2)
            self.assertAlmostEqual(t0.location.y, t1.location.y, places=2)
            self.assertAlmostEqual(t0.rotation.pitch, t1.rotation.pitch, places=2)
            self.assertAlmostEqual(t0.rotation.yaw, t1.rotation.yaw, places=2)
            self.assertAlmostEqual(t0.rotation.roll, t1.rotation.roll, places=2)
