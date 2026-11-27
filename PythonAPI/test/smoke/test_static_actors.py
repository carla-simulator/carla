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
    """Actors that cannot move (static props, traffic signs and lights) are
    missing from the per-tick snapshot only: they are in get_static_actors()
    and get_actors(), found by id, with their transform, alive, tagged, and
    gone after destroy. Actors that can move stay in the snapshot."""

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
            # Its stream may connect after a tick was sent: tick until one reaches it.
            for _ in range(20):
                frame = self.world.tick()
                deadline = time.time() + 1.0
                while late_world.get_snapshot().frame < frame and time.time() < deadline:
                    time.sleep(0.001)
                if late_world.get_snapshot().frame == frame:
                    break
            self.assertEqual(late_world.get_snapshot().frame, frame)
            self._check_listed(late_world, ids, transforms, moving[0])
            ids = self._check_destroy_and_spawn(ids)
        finally:
            self.client.apply_batch_sync([carla.command.DestroyActor(i) for i in ids + moving])
            self.world.tick()

    def _is_static(self, actor_id):
        """True if only get_static_actors() has the actor, False if only the snapshot has it."""
        in_snapshot = self.world.get_snapshot().has_actor(actor_id)
        in_static = actor_id in {a.id for a in self.world.get_static_actors()}
        self.assertNotEqual(in_snapshot, in_static)
        # That call makes the server rebuild its static actors: tick it away, so the next change needs a mark of its own.
        self.world.tick()
        return in_static

    def test_snapshot_follows_physics_attach_and_parent_destroy(self):
        """A static prop leaves or rejoins the snapshot on the next tick when it
        starts or stops simulating physics; a prop spawned attached to it is in
        the snapshot, and is static on the next tick once that parent is destroyed."""
        print("TestStaticActors.test_snapshot_follows_physics_attach_and_parent_destroy")
        (parent,), _ = self._spawn(count=1)
        bp = self.world.get_blueprint_library().find("static.prop.mesh")
        bp.set_attribute("mesh_path", MESH)
        child = None
        try:
            self.world.tick()
            self.assertTrue(self._is_static(parent))
            parent_actor = self.world.get_actor(parent)
            parent_actor.set_simulate_physics(True)
            self.world.tick()
            self.assertFalse(self._is_static(parent))
            parent_actor.set_simulate_physics(False)
            self.world.tick()
            self.assertTrue(self._is_static(parent))
            child = self.world.spawn_actor(bp, carla.Transform(carla.Location(x=2.0)), attach_to=parent_actor).id
            self.world.tick()
            self.assertFalse(self._is_static(child))
            self.client.apply_batch_sync([carla.command.DestroyActor(parent)])
            parent = None
            self.world.tick()
            self.assertTrue(self._is_static(child))
        finally:
            self.client.apply_batch_sync([carla.command.DestroyActor(i) for i in (parent, child) if i is not None])
            self.world.tick()

    def test_swapped_light_times_reach_a_second_client(self):
        """Two traffic lights that swap their green times in one tick: a client
        that read the old times reads the swapped ones."""
        print("TestStaticActors.test_swapped_light_times_reach_a_second_client")
        all_lights = list(self.world.get_actors().filter("traffic.traffic_light"))
        self.assertGreater(len(all_lights), 1)
        # Lights of one group can share a controller, and so their green time.
        group = {light.id for light in all_lights[0].get_group_traffic_lights()}
        lights = [all_lights[0], next(light for light in all_lights if light.id not in group)]
        saved = [light.get_green_time() for light in lights]
        late = carla.Client(*self.testing_address)
        late.set_timeout(120.0)
        late_world = late.get_world()

        def read_on_late_client(frame):
            deadline = time.time() + 30.0
            while late_world.get_snapshot().frame < frame and time.time() < deadline:
                time.sleep(0.001)
            self.assertGreaterEqual(late_world.get_snapshot().frame, frame)
            return [late_world.get_actor(light.id).get_green_time() for light in lights]

        try:
            lights[0].set_green_time(11.0)
            lights[1].set_green_time(17.0)
            self.assertEqual(read_on_late_client(self.world.tick()), [11.0, 17.0])
            # That read made the server rebuild its static actors: tick it away, so the swap needs the per-tick hash.
            self.world.tick()
            lights[0].set_green_time(17.0)
            lights[1].set_green_time(11.0)
            self.assertEqual(read_on_late_client(self.world.tick()), [17.0, 11.0])
        finally:
            for light, green in zip(lights, saved):
                light.set_green_time(green)
            self.world.tick()


class TestStaticSigns(SmokeTest):
    """Traffic signs and traffic lights are static actors that still give
    their OpenDRIVE id and their transform, and a light its state; the
    spectator stays in the snapshot."""

    def test_signs_and_lights_are_static(self):
        print("TestStaticSigns.test_signs_and_lights_are_static")
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
        landmarks = {landmark.id: landmark for landmark in self.world.get_map().get_all_landmarks()}
        # A second client must read the same transforms: they come from the
        # server once, not from the snapshot.
        late = carla.Client(*self.testing_address)
        late.set_timeout(120.0)
        late_world = late.get_world()
        late_world.wait_for_tick()
        lights_from_opendrive = 0
        for light in lights:
            # Only the state of a light changes, and it does not need the
            # snapshot: the light is as static as a sign.
            self.assertIsNone(snapshot.find(light.id))
            self.assertIn(light.id, static_ids)
            self.assertTrue(light.is_alive)
            self.assertIn(light.get_state(), (carla.TrafficLightState.Red, carla.TrafficLightState.Yellow,
                                              carla.TrafficLightState.Green, carla.TrafficLightState.Off))
            transform = light.get_transform()
            self.assertGreater(abs(transform.location.x) + abs(transform.location.y), 1.0)
            seen_late = late_world.get_actor(light.id).get_transform()
            self.assertEqual(transform.location.distance(seen_late.location), 0.0)
            self.assertEqual(transform.rotation.yaw, seen_late.rotation.yaw)
            self.assertEqual(light.get_velocity().length(), 0.0)
            # The id of its OpenDRIVE signal, and the landmark leads back to it.
            sign_id = light.get_opendrive_id()
            self.assertTrue(sign_id)
            if sign_id in landmarks:
                self.assertEqual(self.world.get_traffic_light(landmarks[sign_id]).id, light.id)
                lights_from_opendrive += 1
            self.assertGreater(light.get_green_time(), 0.0)
            self.assertGreater(light.get_yellow_time(), 0.0)
            self.assertGreater(light.get_red_time(), 0.0)
            self.assertGreaterEqual(light.get_elapsed_time(), 0.0)
            # The lights of its group are found among the actors.
            self.assertTrue(all(member is not None and member.type_id == "traffic.traffic_light"
                                for member in light.get_group_traffic_lights()))
        self.assertGreater(lights_from_opendrive, 0)
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
        self.assertEqual({a.id for a in static.filter("traffic.*")}, {a.id for a in traffic})
