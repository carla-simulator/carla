# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

from . import SyncSmokeTest

import carla
import math
import time


class TestVehicleTelemetry(SyncSmokeTest):
    """End-to-end smoke for `Vehicle.get_telemetry_data()`.

    Spawns a vehicle in Town10HD_Opt (the only town shipped with the
    packaged build), drives it under autopilot for a short window, and
    asserts the telemetry RPC returns a sane snapshot wired all the way
    from `ACarlaWheeledVehicle::GetVehicleTelemetryData()` (Chaos read)
    through `LibCarla` to the Python `VehicleTelemetryData` binding.
    """

    def tearDown(self):
        # Override SmokeTest.tearDown's `load_world("Town03")` -- the
        # packaged build only ships Town10HD_Opt and Mine_01.
        self.world.apply_settings(self.settings)
        self.world.tick()
        self.settings = None
        self.client.load_world("Town10HD_Opt")
        time.sleep(5)
        self.world = None
        self.client = None

    def test_get_telemetry_data_shape_and_values(self):
        print("TestVehicleTelemetry.test_get_telemetry_data_shape_and_values")
        self.world = self.client.load_world("Town10HD_Opt")
        settings = carla.WorldSettings(
            no_rendering_mode=False,
            synchronous_mode=True,
            fixed_delta_seconds=0.05)
        self.world.apply_settings(settings)
        self.world.tick()

        bp_lib = self.world.get_blueprint_library()
        vehicle_bps = self.filter_vehicles_for_old_towns(bp_lib.filter("vehicle.*"))
        self.assertGreater(len(vehicle_bps), 0)

        spawn_points = self.world.get_map().get_spawn_points()
        self.assertGreater(len(spawn_points), 0)

        vehicle = self.world.spawn_actor(vehicle_bps[0], spawn_points[0])
        try:
            vehicle.set_autopilot(True)
            for _ in range(30):
                self.world.tick()

            telemetry = vehicle.get_telemetry_data()
            self.assertIsNotNone(telemetry)
            self.assertIsInstance(telemetry, carla.VehicleTelemetryData)

            # Vehicle-level fields are floats and finite (sane sensors).
            self.assertIsInstance(telemetry.speed, float)
            self.assertTrue(math.isfinite(telemetry.speed))
            self.assertIsInstance(telemetry.throttle, float)
            self.assertTrue(math.isfinite(telemetry.throttle))
            self.assertIsInstance(telemetry.brake, float)
            self.assertTrue(math.isfinite(telemetry.brake))
            self.assertIsInstance(telemetry.steer, float)
            self.assertTrue(math.isfinite(telemetry.steer))
            self.assertIsInstance(telemetry.engine_rpm, float)
            self.assertTrue(math.isfinite(telemetry.engine_rpm))
            self.assertIsInstance(telemetry.gear, int)
            self.assertIsInstance(telemetry.drag, float)
            self.assertTrue(math.isfinite(telemetry.drag))

            # The wheel vector size matches the physics control's wheel array.
            physics_control = vehicle.get_physics_control()
            wheels = list(telemetry.wheels)
            self.assertEqual(len(wheels), len(physics_control.wheels))
            self.assertGreater(len(wheels), 0)

            # Per-wheel fields are finite floats.
            for wheel in wheels:
                self.assertIsInstance(wheel, carla.WheelTelemetryData)
                self.assertTrue(math.isfinite(wheel.tire_friction))
                self.assertTrue(math.isfinite(wheel.lat_slip))
                self.assertTrue(math.isfinite(wheel.long_slip))
                self.assertTrue(math.isfinite(wheel.omega))
                self.assertTrue(math.isfinite(wheel.tire_load))
                self.assertTrue(math.isfinite(wheel.normalized_tire_load))
                self.assertTrue(math.isfinite(wheel.torque))
                self.assertTrue(math.isfinite(wheel.long_force))
                self.assertTrue(math.isfinite(wheel.lat_force))
                self.assertTrue(math.isfinite(wheel.normalized_long_force))
                self.assertTrue(math.isfinite(wheel.normalized_lat_force))

            # repr() of the wheel vector exercises the operator<< chain.
            self.assertGreater(len(repr(telemetry.wheels)), 0)
        finally:
            vehicle.destroy()
