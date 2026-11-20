# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Smoke tests for the WorldSnapshot bulk getters."""

import numpy as np

import carla

from . import SyncSmokeTest

VEHICLES = 8


class TestSnapshotBulk(SyncSmokeTest):
    """WorldSnapshot.get_transforms and friends read many actors at once."""

    def spawn_vehicles(self, count=VEHICLES, physics=True):
        """Spawns vehicles at the first spawn points."""
        library = self.world.get_blueprint_library()
        blueprints = sorted(
            [bp for bp in library.filter('vehicle.*')
             if int(bp.get_attribute('number_of_wheels')) == 4],
            key=lambda bp: bp.id)
        points = self.world.get_map().get_spawn_points()
        batch = [carla.command.SpawnActor(blueprints[i % len(blueprints)], points[i])
                 .then(carla.command.SetSimulatePhysics(carla.command.FutureActor, physics))
                 for i in range(min(count, len(points)))]
        ids = [r.actor_id for r in self.client.apply_batch_sync(batch, True) if not r.error]
        self.assertGreater(len(ids), 0, "no vehicle could be spawned")
        return ids

    def destroy(self, ids):
        """Destroys the given actors."""
        self.client.apply_batch_sync([carla.command.DestroyActor(x) for x in ids], True)

    def test_bulk_getters_match_the_per_actor_getters(self):
        """Every row of the bulk getters is the actor's ActorSnapshot."""
        print("TestSnapshotBulk.test_bulk_getters_match_the_per_actor_getters")
        ids = self.spawn_vehicles()
        try:
            for _ in range(20):
                self.world.tick()
            snapshot = self.world.get_snapshot()

            actor_ids = snapshot.get_actor_ids()
            self.assertEqual(actor_ids.dtype, np.uint32)
            self.assertEqual(len(actor_ids), len(snapshot))
            self.assertEqual(list(actor_ids), sorted(actor_ids))
            self.assertEqual(list(actor_ids), [s.id for s in snapshot])
            self.assertTrue(set(ids) <= set(int(x) for x in actor_ids))

            transforms = snapshot.get_transforms()
            velocities = snapshot.get_velocities()
            angular_velocities = snapshot.get_angular_velocities()
            accelerations = snapshot.get_accelerations()
            self.assertEqual(transforms.shape, (len(actor_ids), 6))
            self.assertEqual(transforms.dtype, np.float64)
            for array in (velocities, angular_velocities, accelerations):
                self.assertEqual(array.shape, (len(actor_ids), 3))
                self.assertEqual(array.dtype, np.float64)

            for row, actor_id in enumerate(actor_ids):
                state = snapshot.find(int(actor_id))
                transform = state.get_transform()
                location = (transform.location.x, transform.location.y, transform.location.z)
                for column, value in enumerate(location):
                    # The float location is the double one rounded to float.
                    bulk = transforms[row, column]
                    tolerance = abs(float(np.spacing(np.float32(value))))
                    self.assertLessEqual(abs(bulk - value), tolerance)
                self.assertEqual(
                    list(transforms[row, 3:]),
                    [transform.rotation.pitch, transform.rotation.yaw, transform.rotation.roll])
                for array, vector in ((velocities, state.get_velocity()),
                                      (angular_velocities, state.get_angular_velocity()),
                                      (accelerations, state.get_acceleration())):
                    self.assertEqual(list(array[row]), [vector.x, vector.y, vector.z])
        finally:
            self.destroy(ids)

    def test_selected_ids_missing_ids_and_out(self):
        """actor_ids picks rows in its order, a missing actor is NaN, out is filled."""
        print("TestSnapshotBulk.test_selected_ids_missing_ids_and_out")
        ids = self.spawn_vehicles(count=3)
        try:
            self.world.tick()
            snapshot = self.world.get_snapshot()
            missing = max(int(x) for x in snapshot.get_actor_ids()) + 1000
            wanted = [ids[-1], missing, ids[0]]

            transforms, mask = snapshot.get_transforms(wanted, return_mask=True)
            self.assertEqual(transforms.shape, (3, 6))
            self.assertEqual(mask.dtype, np.bool_)
            self.assertEqual(list(mask), [True, False, True])
            self.assertTrue(np.isnan(transforms[1]).all())
            all_ids = list(snapshot.get_actor_ids())
            everything = snapshot.get_transforms()
            self.assertEqual(list(transforms[0]), list(everything[all_ids.index(ids[-1])]))
            self.assertEqual(list(transforms[2]), list(everything[all_ids.index(ids[0])]))

            # The ids may come in any integer buffer.
            from_numpy = snapshot.get_velocities(np.array(wanted, dtype=np.int64))
            self.assertTrue(np.isnan(from_numpy[1]).all())
            self.assertFalse(np.isnan(from_numpy[0]).any())

            out = np.zeros((3, 3))
            result = snapshot.get_velocities(wanted, out=out)
            self.assertIs(result, out)
            np.testing.assert_array_equal(out, from_numpy)

            with self.assertRaises((ValueError, RuntimeError)):
                snapshot.get_velocities(wanted, out=np.zeros((3, 3), dtype=np.float32))
            with self.assertRaises((ValueError, RuntimeError)):
                snapshot.get_velocities(wanted, out=np.zeros((2, 3)))

            self.assertEqual(snapshot.get_transforms([]).shape, (0, 6))
        finally:
            self.destroy(ids)

    def test_location_far_from_the_origin_keeps_double(self):
        """A location sent in double 7 km from the origin is read back in double.

        Town13 reaches about 7 km from the origin, where a float in meters
        rounds a location to about 0.5 mm. The server holds it in centimeters,
        so the value read back is the one sent, converted to centimeters and
        back, both in double.
        """
        print("TestSnapshotBulk.test_location_far_from_the_origin_keeps_double")
        ids = self.spawn_vehicles(count=1, physics=False)
        try:
            locations = np.array([[7000.123456789012, -6500.987654321098, 12.345678901234]])
            rotations = np.array([[0.0, 30.0, 0.0]], dtype=np.float32)
            batch = carla.ActorUpdateBatch()
            batch.add_transforms(np.array(ids, dtype=np.uint32), locations, rotations)
            self.assertEqual(self.client.apply_actor_updates(batch, True), bytes(1))

            snapshot = self.world.get_snapshot()
            transforms = snapshot.get_transforms(ids)
            expected = [(value * 100.0) / 100.0 for value in locations[0]]
            self.assertEqual(list(transforms[0, :3]), expected)
            # Within one double step of what was sent: about 1e-12 m here.
            for got, sent in zip(transforms[0, :3], locations[0]):
                self.assertLessEqual(abs(got - sent), abs(float(np.spacing(sent))))

            # The per-actor getter still returns float meters, which lose it.
            location = snapshot.find(ids[0]).get_transform().location
            self.assertNotEqual(location.x, locations[0][0])
            self.assertLessEqual(abs(location.x - locations[0][0]), 1e-3)
        finally:
            self.destroy(ids)
