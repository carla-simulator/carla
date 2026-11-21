# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Static props with and without -carla-omit-static-actors."""

import carla

from . import SyncSmokeTest

MESH = "/Game/Carla/Static/Static/SM_PlantPot01.SM_PlantPot01"
COUNT = 40


class TestStaticActors(SyncSmokeTest):
    """Static props behave the same with and without -carla-omit-static-actors:
    in get_actors(), found by id, with their transform, alive, tagged, and gone
    after destroy. With the flag they are missing from the per-tick snapshot
    only; a prop with physics stays in it."""

    def _spawn(self, mass=None, count=COUNT):
        bp = self.world.get_blueprint_library().find("static.prop.mesh")
        bp.set_attribute("mesh_path", MESH)
        if mass is not None:
            bp.set_attribute("mass", str(mass))
        origin = self.world.get_map().get_spawn_points()[0].location
        transforms = [carla.Transform(carla.Location(x=origin.x + 3.0 * (i % 10), y=origin.y + 3.0 * (i // 10),
                                                     z=origin.z + 50.0), carla.Rotation(yaw=7.0 * i))
                      for i in range(count)]
        res = self.client.apply_batch_sync([carla.command.SpawnActor(bp, t) for t in transforms])
        for r in res:
            self.assertFalse(r.error, r.error)
        return [r.actor_id for r in res], transforms

    def _check_listed(self, ids, transforms, moving):
        listed = {a.id for a in self.world.get_actors()}
        self.assertTrue(set(ids) <= listed)
        self.assertIn(moving, listed)
        snapshot = self.world.get_snapshot()
        in_snapshot = [i for i in ids if snapshot.find(i) is not None]
        if not in_snapshot:
            print("  server omits static actors from the snapshot")
            self.assertIsNotNone(snapshot.find(moving))         # physics: dynamic
        else:
            self.assertEqual(len(in_snapshot), len(ids))         # stock: every actor
        found = self.world.get_actors(ids)
        self.assertEqual(sorted(a.id for a in found), sorted(ids))
        for i, t in zip(ids, transforms):
            actor = self.world.get_actor(i)
            self.assertIsNotNone(actor)
            self.assertEqual(actor.type_id, "static.prop.mesh")
            self.assertTrue(actor.is_alive)
            self.assertTrue(actor.semantic_tags)
            got = actor.get_transform()
            self.assertLess(got.location.distance(t.location), 0.05)
            yaw_error = (got.rotation.yaw - t.rotation.yaw + 180.0) % 360.0 - 180.0
            self.assertAlmostEqual(yaw_error, 0.0, delta=0.1)
            self.assertEqual(self.world.get_actors().find(i).id, i)

    def _check_destroy_and_spawn(self, ids):
        """Destroys half of ids, spawns one more; returns the ids still alive."""
        gone, kept = ids[: COUNT // 2], ids[COUNT // 2:]
        handles = [self.world.get_actor(i) for i in gone]
        self.client.apply_batch_sync([carla.command.DestroyActor(i) for i in gone])
        self.world.tick()
        listed = {a.id for a in self.world.get_actors()}
        self.assertFalse(set(gone) & listed)
        self.assertTrue(set(kept) <= listed)
        self.assertFalse(any(h.is_alive for h in handles))
        # one more spawned after the first fetch shows up after the next tick
        more, _ = self._spawn(count=1)
        self.world.tick()
        self.assertIn(more[0], {a.id for a in self.world.get_actors()})
        return kept + more

    def test_static_props_listed_found_and_destroyed(self):
        """Listed, found, transformed, tagged and destroyed as before."""
        print("TestStaticActors.test_static_props_listed_found_and_destroyed")
        ids, transforms = self._spawn()
        moving, _ = self._spawn(mass=10.0, count=1)
        try:
            self.world.tick()
            self.world.tick()
            self._check_listed(ids, transforms, moving[0])
            ids = self._check_destroy_and_spawn(ids)
        finally:
            self.client.apply_batch_sync([carla.command.DestroyActor(i) for i in ids + moving])
            self.world.tick()
