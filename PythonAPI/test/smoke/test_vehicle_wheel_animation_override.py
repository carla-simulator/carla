from . import SyncSmokeTest

import carla
import math
import time

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


class TestVehicleWheelAnimationOverride(SyncSmokeTest):
    """While the wheel animation is overridden the wheel setters pose the
    wheels, with or without Chaos, the getters report what was set, and ending
    the override hands the wheels back."""

    def tearDown(self):
        self.world.apply_settings(self.settings)
        self.world.tick()
        self.settings = None
        self.client.load_world("Town10HD_Opt")
        time.sleep(5)
        self.world = None
        self.client = None

    def _spawn_four_wheeler(self):
        self.world = self.client.load_world("Town10HD_Opt")
        settings = carla.WorldSettings(
            no_rendering_mode=False,
            synchronous_mode=True,
            fixed_delta_seconds=0.05)
        self.world.apply_settings(settings)
        self.world.tick()

        bp_lib = self.world.get_blueprint_library()
        vehicle_bps = [
            bp for bp in self.filter_vehicles_for_old_towns(bp_lib.filter("vehicle.*"))
            if int(bp.get_attribute("number_of_wheels")) == 4]
        self.assertGreater(len(vehicle_bps), 0)

        spawn_points = self.world.get_map().get_spawn_points()
        self.assertGreater(len(spawn_points), 0)

        vehicle = self.world.spawn_actor(vehicle_bps[0], spawn_points[0])
        for _ in range(40):
            self._tick(vehicle)
        return vehicle

    def _tick(self, vehicle, count=1):
        # The bones are only refreshed while the vehicle is being rendered.
        for _ in range(count):
            transform = vehicle.get_transform()
            forward = transform.get_forward_vector()
            right = transform.get_right_vector()
            location = transform.location + carla.Location(
                forward.x - 6.0 * right.x, forward.y - 6.0 * right.y, 2.0)
            self.world.get_spectator().set_transform(carla.Transform(
                location, carla.Rotation(pitch=-10.0, yaw=transform.rotation.yaw + 90.0)))
            self.world.tick()

    def _wheel_bones(self, vehicle):
        """The relative transform of each wheel bone, by name."""
        bones = {
            name: transform
            for name, transform in zip(
                vehicle.get_bone_names(), vehicle.get_bone_relative_transforms())
            if name.lower().startswith("wheel")}
        self.assertEqual(
            len(bones), 4, "expected four wheel bones, found %s" % sorted(bones))
        return bones

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
