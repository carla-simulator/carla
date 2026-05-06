# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Smoke test for the large-vehicle junction handling.

Spawns a known-large vehicle (firetruck or bus) in autopilot, ticks the
simulator for a bounded duration, and asserts that the Traffic Manager
neither crashes nor produces nan controls -- the wide-turn offset and
PID smoothing are exercised but, on the small Town10HD_Opt sandbox, are
not always observable as a clean lateral envelope, so this test focuses
on robustness rather than the exact offset shape.
"""

import math
import time

from . import SyncSmokeTest

import carla


# The TrafficManager classifies a vehicle as "large" via its base_type
# attribute (matching ALSM's large_vehicle_types: bus / truck). We discover
# the blueprint by attribute rather than hardcoded id so the test stays
# robust across packaged-build naming differences.
LARGE_VEHICLE_BASE_TYPES = ("truck", "bus")


class TestTrafficManagerLargeVehicle(SyncSmokeTest):

    def tearDown(self):
        try:
            tm = self.client.get_trafficmanager()
            tm.set_synchronous_mode(False)
        except Exception:
            pass
        self.world.apply_settings(self.settings)
        self.world.tick()
        self.settings = None
        self.client.load_world("Town10HD_Opt")
        time.sleep(5)
        self.world = None
        self.client = None

    def _find_large_vehicle_blueprint(self):
        bp_lib = self.world.get_blueprint_library()
        for bp in bp_lib.filter("vehicle.*"):
            if not bp.has_attribute("base_type"):
                continue
            if bp.get_attribute("base_type").as_str() in LARGE_VEHICLE_BASE_TYPES:
                return bp
        return None

    def test_large_vehicle_autopilot_runs_without_nans(self):
        tm = self.client.get_trafficmanager()
        tm.set_synchronous_mode(True)
        tm_port = tm.get_port()

        truck_bp = self._find_large_vehicle_blueprint()
        self.assertIsNotNone(
            truck_bp, "no large-vehicle blueprint found; test cannot exercise wide-turn path")

        spawn_points = self.world.get_map().get_spawn_points()
        truck = None
        for sp in spawn_points:
            truck = self.world.try_spawn_actor(truck_bp, sp)
            if truck is not None:
                break
        self.assertIsNotNone(truck, "could not spawn a large vehicle at any spawn point")

        truck.set_autopilot(True, tm_port)
        try:
            for _ in range(200):
                self.world.tick()
                control = truck.get_control()
                for name, value in (("steer", control.steer),
                                    ("throttle", control.throttle),
                                    ("brake", control.brake)):
                    self.assertFalse(
                        math.isnan(value),
                        "control.{} is nan on a large vehicle under autopilot".format(name))
                    if name == "steer":
                        self.assertGreaterEqual(value, -1.0)
                        self.assertLessEqual(value, 1.0)
        finally:
            truck.set_autopilot(False, tm_port)
            truck.destroy()
            tm.set_synchronous_mode(False)
