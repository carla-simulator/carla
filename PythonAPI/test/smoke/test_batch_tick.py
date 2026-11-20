# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Smoke tests for apply_batch_sync(commands, True) and Actor.add_torque."""

import carla

from . import SyncSmokeTest


class TestBatchTick(SyncSmokeTest):
    """apply_batch_sync(commands, True) applies, ticks and returns in one call."""

    WALKERS = 12

    def spawn_crowd(self):
        """Walkers driven by the client-side crowd. Returns (walkers, controllers)."""
        library = self.world.get_blueprint_library()
        walker_bps = sorted(library.filter('walker.pedestrian.*'), key=lambda bp: bp.id)
        controller_bp = library.find('controller.ai.walker')
        batch = []
        for i in range(self.WALKERS):
            location = self.world.get_random_location_from_navigation()
            if location is None:
                continue
            # The navigation point is on the ground; spawn the capsule above it.
            location.z += 1.0
            blueprint = walker_bps[i % len(walker_bps)]
            if blueprint.has_attribute('is_invincible'):
                blueprint.set_attribute('is_invincible', 'false')
            batch.append(carla.command.SpawnActor(blueprint, carla.Transform(location)))
        walkers = [r.actor_id for r in self.client.apply_batch_sync(batch, False) if not r.error]
        self.world.tick()
        batch = [carla.command.SpawnActor(controller_bp, carla.Transform(), walker)
                 for walker in walkers]
        controllers = [r.actor_id for r in self.client.apply_batch_sync(batch, False)
                       if not r.error]
        self.world.tick()
        for controller in self.world.get_actors(controllers):
            controller.start()
            controller.go_to_location(self.world.get_random_location_from_navigation())
            controller.set_max_speed(1.4)
        for _ in range(5):
            self.world.tick()
        return walkers, controllers

    def test_batch_tick_with_walker_crowd(self):
        """The crowd's commands ride in the batch; only the caller's are answered."""
        print("TestBatchTick.test_batch_tick_with_walker_crowd")
        walkers, controllers = self.spawn_crowd()
        try:
            self.assertGreater(len(walkers), 0, "no walker could be spawned")
            start = {a.id: a.get_location() for a in self.world.get_actors(walkers)}
            spectator = self.world.get_spectator()
            spectator_transform = spectator.get_transform()
            for _ in range(40):
                frame = self.world.get_snapshot().frame
                responses = self.client.apply_batch_sync(
                    [carla.command.ApplyTransform(spectator.id, spectator_transform)], True)
                # Only the caller's commands are answered, never the walker
                # commands the crowd adds to the same batch.
                self.assertEqual(len(responses), 1)
                self.assertFalse(responses[0].error, responses[0].error)
                self.assertEqual(self.world.get_snapshot().frame, frame + 1)
            # The crowd's commands went in with the batch: the walkers moved.
            moved = [a.id for a in self.world.get_actors(walkers)
                     if a.get_location().distance(start[a.id]) > 0.5]
            self.assertGreater(len(moved), 0, "no walker moved while ticking with the batch")
        finally:
            for controller in self.world.get_actors(controllers):
                controller.stop()
            self.client.apply_batch_sync(
                [carla.command.DestroyActor(x) for x in controllers + walkers], True)

    def test_batch_tick_applies_before_the_tick(self):
        """A command in the batch is visible in the frame the batch ticks to."""
        print("TestBatchTick.test_batch_tick_applies_before_the_tick")
        blueprint = self.world.get_blueprint_library().find('static.prop.streetbarrier')
        start = self.world.get_map().get_spawn_points()[0]
        prop = self.world.spawn_actor(blueprint, start)
        try:
            self.world.tick()
            target = carla.Transform(start.location + carla.Location(x=3.0, z=0.5), start.rotation)
            frame = self.world.get_snapshot().frame
            self.client.apply_batch_sync([carla.command.ApplyTransform(prop.id, target)], True)
            snapshot = self.world.get_snapshot()
            self.assertEqual(snapshot.frame, frame + 1)
            # The command took effect before the tick, so the new frame shows it.
            moved = snapshot.find(prop.id).get_transform().location
            self.assertLess(moved.distance(target.location), 0.01)
        finally:
            prop.destroy()
            self.world.tick()


class TestAddTorque(SyncSmokeTest):
    """add_torque applies a torque, not an angular impulse."""

    # Large enough to overcome the tyres' grip in one step.
    VECTOR = carla.Vector3D(0.0, 0.0, 1.0e10)

    def yaw_rate_change(self, apply):
        """Yaw-rate change of a resting vehicle over one step after apply(vehicle)."""
        vehicle = self.world.spawn_actor(
            self.world.get_blueprint_library().find('vehicle.lincoln.mkz'),
            self.world.get_map().get_spawn_points()[0])
        try:
            for _ in range(30):
                self.world.tick()
            before = vehicle.get_angular_velocity().z
            apply(vehicle)
            self.world.tick()
            return abs(vehicle.get_angular_velocity().z - before)
        finally:
            vehicle.destroy()
            self.world.tick()

    def test_add_torque_is_not_an_impulse(self):
        """add_torque changes the yaw rate far less than an equal angular impulse."""
        print("TestAddTorque.test_add_torque_is_not_an_impulse")
        impulse = self.yaw_rate_change(lambda v: v.add_angular_impulse(self.VECTOR))
        torque = self.yaw_rate_change(lambda v: v.add_torque(self.VECTOR))
        self.assertGreater(impulse, 1.0, "the angular impulse barely moved the vehicle")
        # Over one 0.05 s step a torque changes the yaw rate about 20 times
        # less than an impulse of the same magnitude.
        self.assertLess(torque, 0.5 * impulse,
                        f"add_torque changed the yaw rate like an impulse: {torque} vs {impulse}")
