# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Smoke test for the server-side speed limit.

The server resolves each vehicle's speed limit from the OpenDRIVE map, so
Vehicle.get_speed_limit() matches carla.Waypoint.speed_limit of the lane the
vehicle is on, and the Traffic Manager, which reads get_speed_limit(), drives
at that limit instead of the 30 km/h controller default.
"""

from . import SyncSmokeTest

MAP_NAME = "Town10HD_Opt"
VEHICLE_COUNT = 10
TICKS = 400
# The limit is resolved before physics and the location read after it, so a
# sample right at a lane boundary can see the previous lane's limit.
MIN_MATCH_RATIO = 0.95
TOLERANCE_KMH = 0.5
CONTROLLER_DEFAULT_KMH = 30.0


class TestVehicleSpeedLimit(SyncSmokeTest):

    def setUp(self):
        super(TestVehicleSpeedLimit, self).setUp()
        if self.world.get_map().name.split('/')[-1] != MAP_NAME:
            self.world = self.load_world(MAP_NAME, reset_settings=False)
        self.tm = self.client.get_trafficmanager()
        self.tm.set_synchronous_mode(True)
        self.vehicles = []

    def tearDown(self):
        for vehicle in self.vehicles:
            vehicle.set_autopilot(False, self.tm.get_port())
            vehicle.destroy()
        self.vehicles = []
        self.tm.set_synchronous_mode(False)
        super(TestVehicleSpeedLimit, self).tearDown()

    def _spawn_autopilot_vehicles(self):
        bp_lib = self.world.get_blueprint_library()
        blueprint = bp_lib.find("vehicle.lincoln.mkz")
        spawn_points = self.spaced_spawn_points(
            self.world.get_map().get_spawn_points(), 15.0)
        for spawn_point in spawn_points[:VEHICLE_COUNT]:
            vehicle = self.world.try_spawn_actor(blueprint, spawn_point)
            if vehicle is not None:
                vehicle.set_autopilot(True, self.tm.get_port())
                self.tm.ignore_lights_percentage(vehicle, 100.0)
                self.vehicles.append(vehicle)
        self.assertGreater(len(self.vehicles), 0)

    def test_speed_limit_follows_the_map(self):
        carla_map = self.world.get_map()
        self._spawn_autopilot_vehicles()

        samples = 0
        matches = 0
        max_speed_kmh = 0.0
        for _ in range(TICKS):
            self.world.tick()
            for vehicle in self.vehicles:
                waypoint = carla_map.get_waypoint(vehicle.get_location())
                if waypoint is None or waypoint.is_junction:
                    continue
                map_limit = waypoint.speed_limit
                if map_limit is None:
                    continue
                samples += 1
                if abs(vehicle.get_speed_limit() - map_limit) <= TOLERANCE_KMH:
                    matches += 1
                velocity = vehicle.get_velocity()
                max_speed_kmh = max(max_speed_kmh, 3.6 * velocity.length())

        self.assertGreater(samples, 0, "%s gives no speed limits" % MAP_NAME)
        self.assertGreaterEqual(
            matches / samples, MIN_MATCH_RATIO,
            "get_speed_limit() matched the lane's speed_limit in %d of %d "
            "samples" % (matches, samples))
        self.assertGreater(
            max_speed_kmh, CONTROLLER_DEFAULT_KMH + 5.0,
            "Traffic Manager vehicles stayed at the %.0f km/h controller "
            "default (max %.1f km/h)" % (CONTROLLER_DEFAULT_KMH, max_speed_kmh))
