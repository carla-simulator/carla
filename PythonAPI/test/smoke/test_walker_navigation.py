# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import time

import carla

from . import SyncSmokeTest


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
            self.client.load_world(WORLD)
            time.sleep(5)
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
        self.client.load_world(WORLD)
        time.sleep(5)
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
            controller.set_max_speed(1.4)

            # A destination at the current position can produce an empty or
            # single-point path. WalkerManager previously requested its
            # replacement recursively, so consecutive short routes could
            # overflow the client stack.
            controller.go_to_location(walker.get_location())
            self.world.tick()
            self.assertIsNotNone(
                self.world.get_snapshot(),
                "Server stopped responding after a zero-length walker route")

            controller.go_to_location(self._destination_away_from(spawn_location))

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
