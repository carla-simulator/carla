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
    packaged build), applies full throttle for a short window, and asserts
    the telemetry RPC returns live values from the Chaos solver. The
    assertions check magnitude (non-zero engine RPM, at least one rolling
    wheel) so a regression that drops the simulator side back to default
    zeros cannot pass silently.
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
            # Apply full throttle so the powertrain is loaded; this guarantees
            # engine RPM and at least one rolling wheel are non-zero on a
            # working build.
            vehicle.apply_control(carla.VehicleControl(throttle=1.0))
            for _ in range(30):
                self.world.tick()

            telemetry = vehicle.get_telemetry_data()
            self.assertIsNotNone(telemetry)
            self.assertIsInstance(telemetry, carla.VehicleTelemetryData)

            # Vehicle-level fields are floats and finite.
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

            # Magnitude check: under full throttle the engine must be turning
            # well above zero. A regression that wires this to a never-written
            # debug field would read 0.0 and trip this assertion.
            self.assertGreater(
                telemetry.engine_rpm, 100.0,
                "engine_rpm is suspiciously low; the simulator may be "
                "reporting an unwired field as zero.")
            self.assertEqual(telemetry.throttle, 1.0)

            # The wheel vector size matches the physics control's wheel array.
            physics_control = vehicle.get_physics_control()
            wheels = list(telemetry.wheels)
            self.assertEqual(len(wheels), len(physics_control.wheels))
            self.assertGreater(len(wheels), 0)

            # Per-wheel fields are finite floats; at least one wheel must be
            # rolling under throttle.
            for wheel in wheels:
                self.assertIsInstance(wheel, carla.WheelTelemetryData)
                self.assertTrue(math.isfinite(wheel.lat_slip))
                self.assertTrue(math.isfinite(wheel.long_slip))
                self.assertTrue(math.isfinite(wheel.omega))

            self.assertGreater(
                max(abs(wheel.omega) for wheel in wheels), 0.0,
                "no wheel is rolling under full throttle; the omega field "
                "may have been disconnected from the Chaos solver.")

            # repr() of the wheel vector exercises the operator<< chain.
            self.assertGreater(len(repr(telemetry.wheels)), 0)
        finally:
            vehicle.destroy()