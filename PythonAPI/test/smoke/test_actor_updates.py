# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Smoke tests for carla.ActorUpdateBatch and the ordering of fire-and-forget calls."""

import array

import carla

from . import SyncSmokeTest

VEHICLES = 8


class TestActorUpdates(SyncSmokeTest):
    """Client.apply_actor_updates applies typed updates for many actors."""

    def spawn_vehicles(self, count=VEHICLES):
        """Spawns vehicles with physics off at the first spawn points."""
        library = self.world.get_blueprint_library()
        blueprints = sorted(
            [bp for bp in library.filter('vehicle.*')
             if int(bp.get_attribute('number_of_wheels')) == 4],
            key=lambda bp: bp.id)
        points = self.world.get_map().get_spawn_points()
        batch = [carla.command.SpawnActor(blueprints[i % len(blueprints)], points[i])
                 .then(carla.command.SetSimulatePhysics(carla.command.FutureActor, False))
                 for i in range(min(count, len(points)))]
        ids = [r.actor_id for r in self.client.apply_batch_sync(batch, True) if not r.error]
        self.assertGreater(len(ids), 0, "no vehicle could be spawned")
        return ids

    def destroy(self, ids):
        """Destroys the given actors."""
        self.client.apply_batch_sync([carla.command.DestroyActor(x) for x in ids], True)

    @staticmethod
    def target(i, step):
        """A pose near the map center for vehicle i at step."""
        return carla.Transform(
            carla.Location(x=10.0 * i, y=-20.0 + 0.5 * step, z=30.0),
            carla.Rotation(yaw=15.0 * i + step))

    def assert_at(self, snapshot, actor_id, transform):
        """The actor's pose in the snapshot is the one sent."""
        state = snapshot.find(actor_id)
        self.assertIsNotNone(state)
        got = state.get_transform()
        self.assertAlmostEqual(got.location.x, transform.location.x, places=2)
        self.assertAlmostEqual(got.location.y, transform.location.y, places=2)
        self.assertAlmostEqual(got.location.z, transform.location.z, places=2)
        self.assertAlmostEqual(
            (got.rotation.yaw - transform.rotation.yaw + 180.0) % 360.0 - 180.0, 0.0, places=2)

    def test_updates_and_tick_in_one_call(self):
        """Teleports sent with do_tick=True are in the frame that call returns."""
        print("TestActorUpdates.test_updates_and_tick_in_one_call")
        ids = self.spawn_vehicles()
        try:
            batch = carla.ActorUpdateBatch()
            for step in range(10):
                for i, actor_id in enumerate(ids):
                    batch.add_transform(actor_id, self.target(i, step))
                self.assertEqual(len(batch), len(ids))
                frame = self.world.get_snapshot().frame
                status = self.client.apply_actor_updates(batch, True)
                self.assertEqual(len(batch), 0, "sending must empty the batch")
                self.assertEqual(status, bytes(len(ids)))
                snapshot = self.world.get_snapshot()
                self.assertEqual(snapshot.frame, frame + 1)
                for i, actor_id in enumerate(ids):
                    self.assert_at(snapshot, actor_id, self.target(i, step))
        finally:
            self.destroy(ids)

    def test_bulk_arrays_match_scalar_adders(self):
        """add_transforms from flat buffers lands where add_transform does."""
        print("TestActorUpdates.test_bulk_arrays_match_scalar_adders")
        ids = self.spawn_vehicles()
        try:
            targets = [self.target(i, 3) for i in range(len(ids))]
            locations = array.array('d', [v for t in targets
                                          for v in (t.location.x, t.location.y, t.location.z)])
            rotations = array.array('f', [v for t in targets
                                          for v in (t.rotation.pitch, t.rotation.yaw,
                                                    t.rotation.roll)])
            batch = carla.ActorUpdateBatch()
            batch.add_transforms(array.array('I', ids), locations, rotations)
            self.assertEqual(len(batch), len(ids))
            status = self.client.apply_actor_updates(batch, True)
            self.assertEqual(status, bytes(len(ids)))
            snapshot = self.world.get_snapshot()
            for actor_id, transform in zip(ids, targets):
                self.assert_at(snapshot, actor_id, transform)

            # A plain list of rows works too.
            batch.add_transforms(
                ids,
                [[t.location.x, t.location.y + 5.0, t.location.z] for t in targets],
                [[t.rotation.pitch, t.rotation.yaw, t.rotation.roll] for t in targets])
            self.client.apply_actor_updates(batch, True)
            snapshot = self.world.get_snapshot()
            for actor_id, transform in zip(ids, targets):
                transform.location.y += 5.0
                self.assert_at(snapshot, actor_id, transform)

            with self.assertRaises(Exception):
                batch.add_transforms(ids, locations[:-3], rotations)
        finally:
            self.destroy(ids)

    def test_statuses_report_each_update(self):
        """A missing actor or a wrong kind of update fails alone."""
        print("TestActorUpdates.test_statuses_report_each_update")
        ids = self.spawn_vehicles(2)
        try:
            missing = max(a.id for a in self.world.get_actors()) + 1000
            spectator = self.world.get_spectator().id
            batch = carla.ActorUpdateBatch()
            batch.add_transform(ids[0], self.target(0, 0))
            batch.add_transform(missing, self.target(1, 0))
            batch.add_vehicle_control(ids[0], carla.VehicleControl(throttle=0.5))
            batch.add_vehicle_control(spectator, carla.VehicleControl(throttle=0.5))
            status = self.client.apply_actor_updates(batch, True)
            # Grouped by kind: the two transforms, then the two controls.
            self.assertEqual(list(status), [
                int(carla.ActorUpdateStatus.Success),
                int(carla.ActorUpdateStatus.ActorNotFound),
                int(carla.ActorUpdateStatus.Success),
                int(carla.ActorUpdateStatus.Failed)])
            self.assert_at(self.world.get_snapshot(), ids[0], self.target(0, 0))
            self.world.tick()
            control = self.world.get_actor(ids[0]).get_control()
            self.assertAlmostEqual(control.throttle, 0.5, places=3)
        finally:
            self.destroy(ids)

    def test_async_updates_land_before_the_next_tick(self):
        """apply_actor_updates_async is always in the frame of the next tick."""
        print("TestActorUpdates.test_async_updates_land_before_the_next_tick")
        ids = self.spawn_vehicles()
        try:
            batch = carla.ActorUpdateBatch()
            for step in range(30):
                for i, actor_id in enumerate(ids):
                    batch.add_transform(actor_id, self.target(i, step))
                self.client.apply_actor_updates_async(batch)
                self.world.tick()
                snapshot = self.world.get_snapshot()
                for i, actor_id in enumerate(ids):
                    self.assert_at(snapshot, actor_id, self.target(i, step))
        finally:
            self.destroy(ids)


class TestFireAndForgetOrder(SyncSmokeTest):
    """A fire-and-forget call sent before tick() is applied in that frame."""

    def test_set_transform_lands_in_the_ticked_frame(self):
        """Actor.set_transform right before tick() is in the frame tick returns."""
        print("TestFireAndForgetOrder.test_set_transform_lands_in_the_ticked_frame")
        library = self.world.get_blueprint_library()
        blueprint = sorted(library.filter('vehicle.*'), key=lambda bp: bp.id)[0]
        point = self.world.get_map().get_spawn_points()[0]
        vehicle = self.world.spawn_actor(blueprint, point)
        try:
            vehicle.set_simulate_physics(False)
            self.world.tick()
            for step in range(200):
                location = carla.Location(x=point.location.x + 0.1 * (step % 50),
                                          y=point.location.y, z=point.location.z + 20.0)
                vehicle.set_transform(carla.Transform(location, point.rotation))
                self.world.tick()
                got = self.world.get_snapshot().find(vehicle.id).get_transform().location
                self.assertAlmostEqual(got.x, location.x, places=2,
                                       msg=f"step {step}: the teleport landed a frame late")
        finally:
            vehicle.destroy()
