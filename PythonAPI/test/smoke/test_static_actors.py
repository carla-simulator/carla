# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Static actors are not in the world snapshot, and nothing else changes."""

import time

import carla

from . import SmokeTest, SyncSmokeTest

MESH = "/Game/Carla/Static/Static/SM_PlantPot01.SM_PlantPot01"
COUNT = 40


class TestStaticActors(SyncSmokeTest):
    """Actors that cannot move (static props, traffic signs) are missing from
    the per-tick snapshot only: they are in get_static_actors() and
    get_actors(), found by id, with their transform, alive, tagged, and gone
    after destroy. Actors that can change stay in the snapshot."""

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

    def _check_listed(self, world, ids, transforms, moving):
        snapshot = world.get_snapshot()
        static = {a.id for a in world.get_static_actors()}
        listed = [a.id for a in world.get_actors()]
        self.assertEqual(len(listed), len(set(listed)))          # no actor twice
        self.assertTrue(set(ids) <= static)
        self.assertTrue(static <= set(listed))
        self.assertFalse(any(snapshot.has_actor(i) for i in static))
        self.assertIn(moving, listed)                             # physics: dynamic
        self.assertIsNotNone(snapshot.find(moving))
        self.assertNotIn(moving, static)
        found = world.get_actors(ids)
        self.assertEqual(sorted(a.id for a in found), sorted(ids))
        for i, t in zip(ids, transforms):
            actor = world.get_actor(i)
            self.assertIsNotNone(actor)
            self.assertEqual(actor.type_id, "static.prop.mesh")
            self.assertTrue(actor.is_alive)
            self.assertTrue(actor.semantic_tags)
            got = actor.get_transform()
            self.assertLess(got.location.distance(t.location), 0.05)
            yaw_error = (got.rotation.yaw - t.rotation.yaw + 180.0) % 360.0 - 180.0
            self.assertAlmostEqual(yaw_error, 0.0, delta=0.1)
            self.assertEqual(world.get_actors().find(i).id, i)

    def _check_destroy_and_spawn(self, ids):
        """Destroys half of ids, spawns one more; returns the ids still alive."""
        gone, kept = ids[: COUNT // 2], ids[COUNT // 2:]
        handles = [self.world.get_actor(i) for i in gone]
        self.client.apply_batch_sync([carla.command.DestroyActor(i) for i in gone])
        self.world.tick()
        static = {a.id for a in self.world.get_static_actors()}
        listed = {a.id for a in self.world.get_actors()}
        self.assertFalse(set(gone) & (static | listed))
        self.assertTrue(set(kept) <= static)
        self.assertFalse(any(h.is_alive for h in handles))
        # one more spawned after the first fetch shows up after the next tick
        more, _ = self._spawn(count=1)
        self.world.tick()
        self.assertIn(more[0], {a.id for a in self.world.get_static_actors()})
        self.assertTrue(self.world.get_actor(more[0]).is_alive)
        return kept + more

    def test_static_props_listed_found_and_destroyed(self):
        """Static props leave the snapshot and are listed, found, transformed,
        tagged and destroyed as before, also for a client that connects late."""
        print("TestStaticActors.test_static_props_listed_found_and_destroyed")
        ids, transforms = self._spawn()
        moving, _ = self._spawn(mass=10.0, count=1)
        try:
            self.world.tick()
            self.world.tick()
            self._check_listed(self.world, ids, transforms, moving[0])
            late = carla.Client(*self.testing_address)
            late.set_timeout(120.0)
            late_world = late.get_world()
            frame = self.world.tick()
            deadline = time.time() + 30.0
            while late_world.get_snapshot().frame < frame and time.time() < deadline:
                time.sleep(0.001)
            self.assertEqual(late_world.get_snapshot().frame, frame)
            self._check_listed(late_world, ids, transforms, moving[0])
            ids = self._check_destroy_and_spawn(ids)
        finally:
            self.client.apply_batch_sync([carla.command.DestroyActor(i) for i in ids + moving])
            self.world.tick()


class TestStaticSigns(SmokeTest):
    """Traffic signs are static actors that still give their OpenDRIVE id and
    their transform; traffic lights and the spectator stay in the snapshot."""

    def test_signs_are_static_and_lights_are_not(self):
        print("TestStaticSigns.test_signs_are_static_and_lights_are_not")
        # Town10HD_Opt ships with every package. It has traffic lights, and
        # stop and yield signs that come from OpenDRIVE signals.
        if self.initial_map_name != "Town10HD_Opt":
            self.world = self.load_world("Town10HD_Opt")
        self.world.wait_for_tick()
        snapshot = self.world.get_snapshot()
        static = self.world.get_static_actors()
        static_ids = {a.id for a in static}
        self.assertIsNotNone(snapshot.find(self.world.get_spectator().id))
        traffic = self.world.get_actors().filter("traffic.*")
        lights = [a for a in traffic if a.type_id == "traffic.traffic_light"]
        signs = [a for a in traffic if a.type_id != "traffic.traffic_light"]
        self.assertTrue(lights)
        self.assertTrue(signs)
        for light in lights:
            self.assertIsNotNone(snapshot.find(light.id))
            self.assertNotIn(light.id, static_ids)
            self.assertTrue(light.is_alive)
            self.assertTrue(light.get_opendrive_id())
            self.assertIn(light.get_state(), (carla.TrafficLightState.Red, carla.TrafficLightState.Yellow,
                                              carla.TrafficLightState.Green, carla.TrafficLightState.Off))
        landmarks = {landmark.id: landmark for landmark in self.world.get_map().get_all_landmarks()}
        from_opendrive = 0
        for sign in signs:
            self.assertIsNone(snapshot.find(sign.id))
            self.assertIn(sign.id, static_ids)
            self.assertTrue(sign.is_alive)
            location = sign.get_transform().location
            self.assertGreater(abs(location.x) + abs(location.y), 1.0)
            # A sign made from an OpenDRIVE signal gives the id of the signal,
            # and the landmark with that id leads back to the sign.
            sign_id = sign.get_opendrive_id()
            if sign_id:
                self.assertIn(sign_id, landmarks)
                self.assertEqual(self.world.get_traffic_sign(landmarks[sign_id]).id, sign.id)
                from_opendrive += 1
        self.assertGreater(from_opendrive, 0)
        self.assertEqual({a.id for a in static.filter("traffic.*")}, {s.id for s in signs})
