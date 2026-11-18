# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

from .vehicle_wheel_smoke_test import VehicleWheelSmokeTest

import carla
import math

# How far, in m, a wheel centre may stick out of the vehicle's bounding box:
# the box need not enclose the tyres exactly.
BOX_MARGIN = 0.3

# Ticks of throttle before the moving checks; enough for a few m/s.
DRIVE_TICKS = 40


def _local(transform, location):
    """location, in m and world space, in the frame of transform."""
    d = location - transform.location
    axes = (transform.get_forward_vector(), transform.get_right_vector(), transform.get_up_vector())
    return [d.x * a.x + d.y * a.y + d.z * a.z for a in axes]


class TestVehicleWheelLocation(VehicleWheelSmokeTest):
    """End-to-end smoke for `WheelPhysicsControl.location`, `old_location` and
    `velocity` (#8746).

    The UE5 simulator copied them from Chaos wheel fields that Chaos never
    updates, so every wheel was reported at (0, 0, 0), standing still. They are
    the world location of the wheel centre, in m, its velocity, in m/s, and
    where it was one step earlier.
    """

    def _wheels(self, vehicle):
        wheels = vehicle.get_physics_control().wheels
        # FL, FR, BL, BR below only exist on a four-wheeler.
        self.assertGreaterEqual(len(wheels), 4)
        return wheels[:4]

    def test_wheel_locations_are_on_the_vehicle(self):
        print("TestVehicleWheelLocation.test_wheel_locations_are_on_the_vehicle")
        vehicle = self._spawn_four_wheeler()
        try:
            wheels = self._wheels(vehicle)
            transform = vehicle.get_transform()
            box = vehicle.bounding_box
            local = [_local(transform, wheel.location) for wheel in wheels]

            for index, (x, y, z) in enumerate(local):
                for value, centre, extent, axis in (
                        (x, box.location.x, box.extent.x, "x"),
                        (y, box.location.y, box.extent.y, "y"),
                        (z, box.location.z, box.extent.z, "z")):
                    self.assertLessEqual(
                        abs(value - centre), extent + BOX_MARGIN,
                        "wheel %d is %.2f m from the vehicle along %s, outside its "
                        "bounding box (%s); world location %s." % (
                            index, value, axis, box, wheels[index].location))
                # Wheels hang in the lower half of the vehicle.
                self.assertLess(
                    z, box.location.z,
                    "wheel %d centre is %.2f m up, above the middle of the vehicle."
                    % (index, z))

            fl, fr, bl, br = local
            for front, rear in ((fl, bl), (fr, br)):
                self.assertGreater(
                    front[0] - rear[0], 1.0,
                    "front wheels are not ahead of the rear ones: %s" % local)
            for left, right in ((fl, fr), (bl, br)):
                self.assertGreater(
                    right[1] - left[1], 0.5,
                    "left wheels are not on the left of the right ones: %s" % local)

            # Settled: the wheels have not moved during the last step.
            for index, wheel in enumerate(wheels):
                self.assertLess(
                    wheel.location.distance(wheel.old_location), 0.02,
                    "parked wheel %d moved %.3f m in one step."
                    % (index, wheel.location.distance(wheel.old_location)))
                self.assertLess(
                    wheel.velocity.length(), 0.4,
                    "parked wheel %d reports %.2f m/s." % (index, wheel.velocity.length()))
        finally:
            vehicle.destroy()

    def test_wheel_locations_follow_the_vehicle(self):
        print("TestVehicleWheelLocation.test_wheel_locations_follow_the_vehicle")
        vehicle = self._spawn_four_wheeler()
        try:
            before = [wheel.location for wheel in self._wheels(vehicle)]
            vehicle.apply_control(carla.VehicleControl(throttle=0.6))
            self._tick(vehicle, DRIVE_TICKS)

            wheels = self._wheels(vehicle)
            speed = vehicle.get_velocity().length()
            self.assertGreater(speed, 1.0, "the vehicle did not drive off")
            step = self.world.get_settings().fixed_delta_seconds

            for index, wheel in enumerate(wheels):
                self.assertGreater(
                    wheel.location.distance(before[index]), 1.0,
                    "wheel %d stayed put while the vehicle drove off." % index)
                travelled = wheel.location.distance(wheel.old_location)
                self.assertAlmostEqual(
                    travelled, speed * step, delta=0.25 * speed * step + 0.02,
                    msg="wheel %d moved %.3f m in the last step; the vehicle, %.3f m."
                    % (index, travelled, speed * step))
                wheel_speed = wheel.velocity.length()
                self.assertAlmostEqual(
                    wheel_speed, speed, delta=0.25 * speed + 0.2,
                    msg="wheel %d reports %.2f m/s; the vehicle, %.2f m/s."
                    % (index, wheel_speed, speed))
                forward = vehicle.get_transform().get_forward_vector()
                self.assertGreater(
                    wheel.velocity.x * forward.x + wheel.velocity.y * forward.y +
                    wheel.velocity.z * forward.z, 0.0,
                    "wheel %d is not moving forward." % index)
        finally:
            vehicle.destroy()
