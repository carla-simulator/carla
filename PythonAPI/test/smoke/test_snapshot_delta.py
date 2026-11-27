# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""The server sends only what changed since the previous snapshot; a client must still read the whole world."""

import random
import time

import carla

from . import SyncSmokeTest


def wait_for_frame(world, frame):
    deadline = time.time() + 30.0
    while world.get_snapshot().frame < frame and time.time() < deadline:
        time.sleep(0.001)
    assert world.get_snapshot().frame >= frame


def late_client(address, world):
    """A new client and its world, holding the frame of the last tick of world."""
    client = carla.Client(*address)
    client.set_timeout(120.0)
    other = client.get_world()
    # Its stream may connect after a tick was sent: tick until one reaches it.
    for _ in range(20):
        frame = world.tick()
        deadline = time.time() + 1.0
        while other.get_snapshot().frame < frame and time.time() < deadline:
            time.sleep(0.001)
        if other.get_snapshot().frame == frame:
            return client, other
    raise AssertionError("no tick reached the new client")


class TestSnapshotDelta(SyncSmokeTest):

    def _spawn(self, count, physics=False):
        blueprints = self.filter_vehicles_for_old_towns(
            self.world.get_blueprint_library().filter("vehicle.*"))
        blueprint = sorted(blueprints, key=lambda bp: bp.id)[0]
        points = self.spaced_spawn_points(self.world.get_map().get_spawn_points(), 12.0)[:count]
        self.assertEqual(len(points), count)
        batch = [carla.command.SpawnActor(blueprint, point).then(
            carla.command.SetSimulatePhysics(carla.command.FutureActor, physics)) for point in points]
        responses = self.client.apply_batch_sync(batch, True)
        self.assertFalse(any(r.error for r in responses))
        self.world.tick()
        return [r.actor_id for r in responses]

    def _check(self, worlds, expected):
        """Every world reads every vehicle where it was put, within float precision."""
        for world in worlds:
            snapshot = world.get_snapshot()
            for actor_id, transform in expected.items():
                got = snapshot.find(actor_id).get_transform()
                self.assertLess(got.location.distance(transform.location), 2e-3, actor_id)
                for a, b in ((got.rotation.pitch, transform.rotation.pitch),
                             (got.rotation.yaw, transform.rotation.yaw),
                             (got.rotation.roll, transform.rotation.roll)):
                    self.assertAlmostEqual(a, b, delta=2e-2)
                self.assertEqual(world.get_actor(actor_id).get_transform().location, got.location)

    def test_teleports_reach_every_client(self):
        """Vehicles are moved or turned at random; an old client and one that joins half way read where each is."""
        print("TestSnapshotDelta.test_teleports_reach_every_client")
        ids = self._spawn(12)
        rng = random.Random(7)
        worlds = [self.world, late_client(self.testing_address, self.world)[1]]
        expected = {a.id: a.get_transform() for a in self.world.get_actors(ids)}
        self._check(worlds, expected)
        for step in range(300):
            batch = []
            for actor_id in rng.sample(ids, rng.randint(0, 3)):
                transform = expected[actor_id]
                kind = rng.choice(("location", "rotation", "both"))
                if kind != "rotation":
                    transform.location.x += rng.choice((-0.5, 0.25, 1.0))
                    transform.location.z += 0.125
                if kind != "location":
                    transform.rotation.yaw = rng.choice((-90.0, -30.0, 0.0, 45.0, 120.0))
                batch.append(carla.command.ApplyTransform(actor_id, transform))
            self.client.apply_batch_sync(batch, False)
            if step == 150:
                worlds.append(late_client(self.testing_address, self.world)[1])
            else:
                self.world.tick()
            for world in worlds[1:]:
                wait_for_frame(world, self.world.get_snapshot().frame)
            self._check(worlds, expected)

    def test_new_client_equals_old_client(self):
        """Driving vehicles change every tick: a client fed with deltas equals one that just got a whole snapshot."""
        print("TestSnapshotDelta.test_new_client_equals_old_client")
        ids = self._spawn(12, physics=True)
        self.client.apply_batch_sync(
            [carla.command.ApplyVehicleControl(i, carla.VehicleControl(throttle=0.6)) for i in ids], True)
        for _ in range(8):
            for _ in range(10):
                self.world.tick()
            other = late_client(self.testing_address, self.world)[1]
            old, new = self.world.get_snapshot(), other.get_snapshot()
            self.assertEqual(old.frame, new.frame)
            self.assertEqual(len(old), len(new))
            moving = 0
            for a in old:
                b = new.find(a.id)
                self.assertEqual(a.get_transform().location, b.get_transform().location)
                self.assertEqual(a.get_transform().rotation, b.get_transform().rotation)
                self.assertEqual(a.get_velocity(), b.get_velocity())
                self.assertEqual(a.get_angular_velocity(), b.get_angular_velocity())
                self.assertEqual(a.get_acceleration(), b.get_acceleration())
                moving += 1 if a.get_velocity().length() > 0.1 else 0
            self.assertGreater(moving, 0)
            for i in ids:
                a, b = self.world.get_actor(i), other.get_actor(i)
                self.assertEqual(a.get_control().throttle, b.get_control().throttle)
                self.assertEqual(a.get_speed_limit(), b.get_speed_limit())
                self.assertEqual(a.get_traffic_light_state(), b.get_traffic_light_state())
            for light in self.world.get_actors().filter("traffic.traffic_light*"):
                seen = other.get_actor(light.id)
                self.assertEqual(light.get_state(), seen.get_state())
                self.assertEqual(light.get_elapsed_time(), seen.get_elapsed_time())
                self.assertEqual(light.is_frozen(), seen.is_frozen())

    def test_spawn_and_destroy_between_deltas(self):
        """Actors come and go between ticks, also both in one tick; every client lists exactly the ones alive."""
        print("TestSnapshotDelta.test_spawn_and_destroy_between_deltas")
        blueprint = self.world.get_blueprint_library().find("sensor.other.imu")
        rng = random.Random(3)
        other = late_client(self.testing_address, self.world)[1]
        alive = []
        for step in range(60):
            gone = rng.sample(alive, rng.randint(0, min(2, len(alive))))
            batch = [carla.command.DestroyActor(i) for i in gone]
            batch += [carla.command.SpawnActor(blueprint, carla.Transform(carla.Location(x=float(step), z=200.0 + i)))
                      for i in range(rng.randint(0, 2))]
            responses = self.client.apply_batch_sync(batch, False)
            self.assertFalse(any(r.error for r in responses))
            alive = [i for i in alive if i not in gone] + [r.actor_id for r in responses[len(gone):]]
            frame = self.world.tick()
            wait_for_frame(other, frame)
            for world in (self.world, other):
                snapshot = world.get_snapshot()
                self.assertEqual(snapshot.frame, frame)
                self.assertTrue(all(snapshot.has_actor(i) for i in alive))
                self.assertFalse(any(snapshot.has_actor(i) for i in gone))
                self.assertEqual({a.id for a in world.get_actors().filter("sensor.other.imu")}, set(alive))

    def test_mode_switches_and_reload(self):
        """The deltas keep arriving right after a switch to asynchronous mode and back, and after a reload."""
        print("TestSnapshotDelta.test_mode_switches_and_reload")
        ids = self._spawn(4)
        other_client, other = late_client(self.testing_address, self.world)
        worlds = [self.world, other]
        expected = {a.id: a.get_transform() for a in self.world.get_actors(ids)}

        def move_and_check(synchronous):
            for step in range(20):
                actor_id = ids[step % len(ids)]
                expected[actor_id].location.z += 0.25
                expected[actor_id].rotation.yaw = float(10 * (step % 9))
                self.client.apply_batch_sync(
                    [carla.command.ApplyTransform(actor_id, expected[actor_id])], False)
                if synchronous:
                    frame = worlds[0].tick()
                else:
                    # The command may land after the next frame; two more are enough.
                    frame = max(worlds[0].wait_for_tick().frame, worlds[0].wait_for_tick().frame)
                for world in worlds:
                    wait_for_frame(world, frame)
                self._check(worlds, expected)

        move_and_check(True)
        settings = self.world.get_settings()
        settings.synchronous_mode = False
        settings.fixed_delta_seconds = None
        self.world.apply_settings(settings)
        move_and_check(False)
        settings.synchronous_mode = True
        settings.fixed_delta_seconds = 0.05
        self.world.apply_settings(settings)
        self.world.tick()
        move_and_check(True)

        self.world = self.reload_world(False)
        self.world.tick()
        worlds[:] = [self.world, other_client.get_world()]
        ids[:] = self._spawn(4)
        expected.clear()
        expected.update({a.id: a.get_transform() for a in self.world.get_actors(ids)})
        move_and_check(True)

    def test_slow_asynchronous_client_converges(self):
        """A client lagging behind an asynchronous server never reads a mix of two frames, and ends on the last one."""
        print("TestSnapshotDelta.test_slow_asynchronous_client_converges")
        ids = self._spawn(2)
        base = self.world.get_actor(ids[0]).get_transform()
        settings = self.world.get_settings()
        settings.synchronous_mode = False
        settings.fixed_delta_seconds = None
        self.world.apply_settings(settings)
        slow_client = carla.Client(*self.testing_address)
        slow_client.set_timeout(120.0)
        slow = slow_client.get_world()
        seen = []

        def on_tick(snapshot):
            found = snapshot.find(ids[0])
            if found is not None:
                transform = found.get_transform()
                seen.append((transform.location.z - base.location.z, transform.rotation.yaw))
            time.sleep(0.05)

        callback = slow.on_tick(on_tick)
        try:
            for step in range(1, 61):
                # Height and yaw move together: a mix of two frames would not match.
                transform = carla.Transform(
                    carla.Location(base.location.x, base.location.y, base.location.z + 0.5 * step),
                    carla.Rotation(base.rotation.pitch, 0.5 * step, base.rotation.roll))
                self.client.apply_batch_sync([carla.command.ApplyTransform(ids[0], transform)], False)
                self.world.wait_for_tick()
            deadline = time.time() + 30.0
            while time.time() < deadline and not (seen and abs(seen[-1][0] - 30.0) < 2e-3):
                time.sleep(0.05)
        finally:
            slow.remove_on_tick(callback)
            slow.wait_for_tick()   # returns after the callback in flight: a client that goes with one running hangs
        self.assertTrue(seen)
        for height, yaw in list(seen):
            if height > 0.25:   # before the first move the yaw is the spawn point's
                self.assertAlmostEqual(height, yaw, delta=2e-2)
        self.assertAlmostEqual(seen[-1][0], 30.0, delta=2e-3)
        self.assertLess(slow.get_actor(ids[0]).get_transform().location.distance(transform.location), 2e-3)
