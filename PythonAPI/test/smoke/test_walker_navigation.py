# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import time
from queue import Queue

import carla
import numpy as np

from . import SyncSmokeTest
from .test_lidar import wait_for_frame

SEMANTIC_POINT = np.dtype([('x', 'f4'), ('y', 'f4'), ('z', 'f4'), ('cos', 'f4'),
                           ('object_idx', 'u4'), ('object_tag', 'u4')])


WORLD = "Town10HD_Opt"


class TestWalkerNavigation(SyncSmokeTest):
    """Coverage for the WalkerManager traffic-light gather and AI walker routing.

    Driving a ``controller.ai.walker`` makes the server build a pedestrian
    route, which runs ``WalkerManager::GetAllTrafficLightWaypoints`` to collect
    every ``traffic.traffic_light`` actor affecting the path. That gather was
    reworked to iterate ``World::GetActors`` and ``dynamic_pointer_cast`` each
    candidate to ``TrafficLight`` behind a null guard, replacing an unchecked
    ``static_pointer_cast`` plus a dead per-actor snapshot query.

    With the maps shipped in the packaged build every ``traffic.traffic_light``
    actor resolves to a real ``TrafficLight``, so this test does not drive the
    null-guard branch itself; that branch is defensive and not reproducible from
    the client. What it does cover is that the reworked gather and the AI walker
    routing path still run end to end without taking the server down, and that a
    started controller actually moves the walker. No prior smoke test exercised
    the AI controller path at all.
    """

    def setUp(self):
        super(TestWalkerNavigation, self).setUp()
        if self.world.get_map().name.split("/")[-1] != WORLD:
            self.load_world(WORLD)
            self.world = self.client.get_world()
            settings = carla.WorldSettings(
                no_rendering_mode=False,
                synchronous_mode=True,
                fixed_delta_seconds=0.05)
            self.world.apply_settings(settings)
            self.world.tick()

    def tearDown(self):
        # SmokeTest.tearDown() loads Town03, which is not shipped in the
        # packaged build. Reload Town10HD_Opt instead so the next test starts
        # from a known good state.
        self.world.apply_settings(self.settings)
        if self.settings.synchronous_mode:
            self.world.tick()
        self.settings = None
        self.load_world(WORLD)
        self.world = None
        self.client = None

    def _random_navigation_location(self):
        location = self.world.get_random_location_from_navigation()
        self.assertIsNotNone(
            location,
            "Navigation mesh is unavailable; cannot exercise walker routing")
        return location

    def _destination_away_from(self, origin, min_distance=10.0):
        # Pick a navigation point a few metres away so the route is non-trivial
        # and the walker has somewhere to actually move towards.
        for _ in range(20):
            candidate = self._random_navigation_location()
            if candidate.distance(origin) >= min_distance:
                return candidate
        return candidate

    def test_walker_ai_controller_routes_without_crash(self):
        print("TestWalkerNavigation.test_walker_ai_controller_routes_without_crash")

        blueprint_library = self.world.get_blueprint_library()
        walker_bp = blueprint_library.filter("walker.pedestrian.*")[0]
        if walker_bp.has_attribute("is_invincible"):
            walker_bp.set_attribute("is_invincible", "false")
        controller_bp = blueprint_library.find("controller.ai.walker")

        spawn_location = self._random_navigation_location()
        spawn_location.z += 1.0
        walker = self.world.spawn_actor(walker_bp, carla.Transform(spawn_location))
        self.assertIsNotNone(walker, "Failed to spawn a walker actor")

        controller = None
        try:
            controller = self.world.spawn_actor(
                controller_bp, carla.Transform(), attach_to=walker)
            self.world.tick()

            controller.start()
            controller.go_to_location(self._destination_away_from(spawn_location))
            controller.set_max_speed(1.4)

            start = walker.get_location()
            for _ in range(120):
                self.world.tick()

            # Primary guarantee: the route computation (which runs the
            # traffic-light gather) did not crash the server, so it still
            # answers requests.
            snapshot = self.world.get_snapshot()
            self.assertIsNotNone(snapshot)
            self.assertTrue(walker.is_alive, "Walker died during navigation")

            # Secondary signal: a started AI controller should have moved the
            # walker, confirming a route was actually built and followed.
            travelled = walker.get_location().distance(start)
            self.assertGreater(
                travelled, 0.3,
                "Walker did not move under AI control; routing likely failed")
        finally:
            if controller is not None:
                controller.stop()
                controller.destroy()
            walker.destroy()
            self.world.tick()

    def _spawn_ai_walker(self):
        blueprint_library = self.world.get_blueprint_library()
        walker_bp = blueprint_library.filter("walker.pedestrian.*")[0]
        if walker_bp.has_attribute("is_invincible"):
            walker_bp.set_attribute("is_invincible", "false")
        spawn_location = self._random_navigation_location()
        spawn_location.z += 1.0
        walker = self.world.spawn_actor(walker_bp, carla.Transform(spawn_location))
        controller = self.world.spawn_actor(
            blueprint_library.find("controller.ai.walker"), carla.Transform(), attach_to=walker)
        self.world.tick()
        controller.start()
        controller.go_to_location(self._destination_away_from(spawn_location))
        controller.set_max_speed(1.4)
        return walker, controller

    def _cast_ray_hits_walker(self, walker):
        location = walker.get_location()
        hits = self.world.cast_ray(location + carla.Location(z=3.0), location - carla.Location(z=1.5))
        return any(hit.label == carla.CityObjectLabel.Pedestrians for hit in hits)

    def _semantic_lidar_points_on(self, walker):
        # Above the walker, looking down: nothing on a sidewalk can occlude it.
        bp = self.world.get_blueprint_library().find("sensor.lidar.ray_cast_semantic")
        for key, value in {"channels": "32", "range": "10", "points_per_second": "200000",
                           "rotation_frequency": "20", "upper_fov": "-30",
                           "lower_fov": "-90"}.items():
            bp.set_attribute(key, value)
        lidar = self.world.spawn_actor(
            bp, carla.Transform(walker.get_location() + carla.Location(z=3.0)))
        data_queue = Queue()
        lidar.listen(lambda data: data_queue.put((data.frame, bytes(data.raw_data))))
        try:
            frame = self.world.tick()
            points = np.frombuffer(wait_for_frame(self, data_queue, frame)[1], dtype=SEMANTIC_POINT)
        finally:
            lidar.stop()
            lidar.destroy()
        return int(np.count_nonzero(points["object_idx"] == walker.id))

    def test_ai_walker_visible_to_sensors(self):
        """A walker driven by the client-side crowd stays visible to ray-cast sensors (#10068)."""
        print("TestWalkerNavigation.test_ai_walker_visible_to_sensors")
        walker, controller = self._spawn_ai_walker()
        try:
            for _ in range(20):
                self.world.tick()
            self.assertTrue(self._cast_ray_hits_walker(walker),
                            "world.cast_ray does not hit the AI-driven walker")
            self.assertGreater(self._semantic_lidar_points_on(walker), 0,
                               "The semantic LiDAR does not hit the AI-driven walker")
        finally:
            controller.stop()
            controller.destroy()
            walker.destroy()
            self.world.tick()

    def test_stopped_ai_walker_obeys_control(self):
        """stop() gives the walker its movement back, and it stays visible."""
        print("TestWalkerNavigation.test_stopped_ai_walker_obeys_control")
        walker, controller = self._spawn_ai_walker()
        try:
            for _ in range(10):
                self.world.tick()
            controller.stop()
            self.world.tick()
            start = walker.get_location()
            walker.apply_control(carla.WalkerControl(
                walker.get_transform().get_forward_vector(), 1.4))
            for _ in range(40):
                self.world.tick()
            self.assertGreater(walker.get_location().distance(start), 1.0,
                               "A stopped AI walker does not obey apply_control()")
            self.assertTrue(self._cast_ray_hits_walker(walker),
                            "world.cast_ray does not hit the stopped walker")
        finally:
            controller.destroy()
            walker.destroy()
            self.world.tick()
