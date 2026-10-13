from .vehicle_wheel_smoke_test import VehicleWheelSmokeTest

import carla
import math

ANGLE_TOLERANCE_DEG = 2.0
OFFSET_TOLERANCE_M = 0.01

STEER_DEG = 20.0
PITCH_DEG = 45.0
SUSPENSION_M = 0.05

WHEELS = (
    carla.VehicleWheelLocation.FL_Wheel,
    carla.VehicleWheelLocation.FR_Wheel,
    carla.VehicleWheelLocation.BL_Wheel,
    carla.VehicleWheelLocation.BR_Wheel)


class TestVehicleWheelAnimationOverride(VehicleWheelSmokeTest):
    """While the wheel animation is overridden the wheel setters pose the
    wheels, with or without Chaos, the getters report what was set, and ending
    the override hands the wheels back."""

    @staticmethod
    def _heading(transform):
        """Heading in the vehicle's XY plane of the bone axis closest to the axle."""
        axle = max(
            (transform.get_forward_vector(), transform.get_right_vector(),
             transform.get_up_vector()),
            key=lambda vector: abs(vector.y))
        if axle.y < 0.0:
            axle = carla.Vector3D(-axle.x, -axle.y, -axle.z)
        return math.degrees(math.atan2(axle.y, axle.x))

    @staticmethod
    def _wrap(angle):
        return (angle + 180.0) % 360.0 - 180.0

    def _steer_and_suspension(self, vehicle):
        return {
            wheel: (vehicle.get_wheel_steer_angle(wheel),
                    vehicle.get_wheel_suspension_offset(wheel))
            for wheel in WHEELS}

    def _assert_bones_moved(self, rest, posed, steer, rise, message):
        for name in rest:
            turned = self._wrap(self._heading(posed[name]) - self._heading(rest[name]))
            self.assertLess(
                abs(abs(turned) - abs(steer)), ANGLE_TOLERANCE_DEG,
                "%s %s turned %.2f degrees, not %.2f" % (message, name, turned, steer))
            risen = posed[name].location.z - rest[name].location.z
            self.assertLess(
                abs(risen - rise), OFFSET_TOLERANCE_M,
                "%s %s rose %.3f m, not %.3f" % (message, name, risen, rise))

    def _check_override(self, vehicle):
        """Poses the wheels through the override and ends it. Returns the steer and
        suspension before and after, and the bones at rest."""
        self._tick(vehicle, 2)
        rest = self._wheel_bones(vehicle)
        before = self._steer_and_suspension(vehicle)
        # Every wheel shares the steer and suspension of the front left one.
        steer_before, suspension_before = before[WHEELS[0]]

        vehicle.set_wheel_animation_override(True)
        # Spin every wheel from where it is, so the bones turn by the same angle.
        for wheel in WHEELS:
            vehicle.set_wheel_pitch_angle(wheel, vehicle.get_wheel_pitch_angle(wheel) + PITCH_DEG)
        self._tick(vehicle, 2)
        spun = self._wheel_bones(vehicle)
        for name in rest:
            spin = self._spin_between(rest[name], spun[name])
            self.assertLess(
                abs(abs(spin) - PITCH_DEG), ANGLE_TOLERANCE_DEG,
                "spun: %s turned %.2f degrees, not %.2f" % (name, spin, PITCH_DEG))

        for wheel in WHEELS:
            vehicle.set_wheel_steer_direction(wheel, STEER_DEG)
            vehicle.set_wheel_pitch_angle(wheel, PITCH_DEG)
            vehicle.set_wheel_suspension_offset(wheel, SUSPENSION_M)
        # The bones read back are one frame behind.
        self._tick(vehicle, 2)

        for wheel in WHEELS:
            self.assertAlmostEqual(vehicle.get_wheel_steer_angle(wheel), STEER_DEG, places=3)
            self.assertAlmostEqual(vehicle.get_wheel_pitch_angle(wheel), PITCH_DEG, places=3)
            self.assertAlmostEqual(
                vehicle.get_wheel_suspension_offset(wheel), SUSPENSION_M, places=3)
        self._assert_bones_moved(
            rest, self._wheel_bones(vehicle),
            STEER_DEG - steer_before, SUSPENSION_M - suspension_before, "posed:")

        vehicle.set_wheel_animation_override(False)
        self._tick(vehicle, 2)
        return before, self._steer_and_suspension(vehicle), rest

    def test_override_without_physics(self):
        print("TestVehicleWheelAnimationOverride.test_override_without_physics")
        vehicle = self._spawn_four_wheeler()
        try:
            vehicle.set_simulate_physics(False)
            before, after, rest = self._check_override(vehicle)
            # Nothing else steers or compresses the wheels without Chaos.
            for wheel in WHEELS:
                self.assertAlmostEqual(after[wheel][0], 0.0, places=3)
                self.assertAlmostEqual(after[wheel][1], 0.0, places=3)
            _, suspension_before = before[WHEELS[0]]
            self._assert_bones_moved(
                rest, self._wheel_bones(vehicle), 0.0, -suspension_before, "released:")
        finally:
            vehicle.destroy()

    def test_override_under_chaos(self):
        print("TestVehicleWheelAnimationOverride.test_override_under_chaos")
        vehicle = self._spawn_four_wheeler()
        try:
            before, after, _ = self._check_override(vehicle)
            # Chaos poses the wheels again, as it did before.
            for wheel in WHEELS:
                self.assertLess(abs(after[wheel][0] - before[wheel][0]), ANGLE_TOLERANCE_DEG)
                self.assertLess(abs(after[wheel][1] - before[wheel][1]), OFFSET_TOLERANCE_M)
        finally:
            vehicle.destroy()

    def test_setters_raise_without_override(self):
        print("TestVehicleWheelAnimationOverride.test_setters_raise_without_override")
        vehicle = self._spawn_four_wheeler()
        try:
            for wheel in WHEELS:
                with self.assertRaises(RuntimeError):
                    vehicle.set_wheel_steer_direction(wheel, STEER_DEG)
                with self.assertRaises(RuntimeError):
                    vehicle.set_wheel_pitch_angle(wheel, PITCH_DEG)
                with self.assertRaises(RuntimeError):
                    vehicle.set_wheel_suspension_offset(wheel, SUSPENSION_M)
        finally:
            vehicle.destroy()

    def test_missing_wheel_raises(self):
        print("TestVehicleWheelAnimationOverride.test_missing_wheel_raises")
        vehicle = self._spawn_vehicle(2)
        try:
            missing = WHEELS[len(vehicle.get_physics_control().wheels):]
            if not missing:
                self.skipTest("the two-wheeler's Chaos setup carries four wheels")
            for wheel in missing:
                for getter in (vehicle.get_wheel_steer_angle,
                               vehicle.get_wheel_pitch_angle,
                               vehicle.get_wheel_suspension_offset):
                    with self.assertRaises(RuntimeError):
                        getter(wheel)
                with self.assertRaises(RuntimeError):
                    vehicle.set_wheel_steer_direction(wheel, STEER_DEG)
                with self.assertRaises(RuntimeError):
                    vehicle.set_wheel_pitch_angle(wheel, PITCH_DEG)
                with self.assertRaises(RuntimeError):
                    vehicle.set_wheel_suspension_offset(wheel, SUSPENSION_M)
        finally:
            vehicle.destroy()
