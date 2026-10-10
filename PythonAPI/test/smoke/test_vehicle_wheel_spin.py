from .vehicle_wheel_smoke_test import VehicleWheelSmokeTest

import carla
import math

ANGLE_TOLERANCE_DEG = 2.0

# Small steps, so that none of them looks like a teleport.
MOVE_TICKS = 10

# Far enough in one frame for the simulator to treat it as a teleport.
TELEPORT_DISTANCE_M = 20.0


class TestVehicleWheelSpin(VehicleWheelSmokeTest):
    """The wheels of a vehicle roll with the distance it travels, whether or
    not Chaos is simulating it. Read from the rotation of the wheel bones."""

    def _assert_spin(self, spin, expected, radii, message):
        # Nothing tells which bone carries which wheel, so accept any radius.
        candidates = [expected * radii[0] / radius for radius in radii]
        self.assertLess(
            min(abs(spin - candidate) for candidate in candidates),
            ANGLE_TOLERANCE_DEG,
            "%s (turned %.2f degrees, expected one of %s)" % (
                message, spin, ["%.2f" % candidate for candidate in candidates]))

    def _move(self, vehicle, distance):
        transform = vehicle.get_transform()
        forward = transform.get_forward_vector()
        step = distance / MOVE_TICKS
        for _ in range(MOVE_TICKS):
            transform.location += carla.Location(
                forward.x * step, forward.y * step, forward.z * step)
            vehicle.set_transform(transform)
            self._tick(vehicle)
        # The bones read back are one frame behind.
        self._tick(vehicle)

    def test_wheels_roll_with_physics_disabled(self):
        print("TestVehicleWheelSpin.test_wheels_roll_with_physics_disabled")
        vehicle = self._spawn_four_wheeler()
        try:
            radii = [wheel.wheel_radius / 100.0
                     for wheel in vehicle.get_physics_control().wheels]
            self.assertGreater(min(radii), 0.0)
            eighth_turn = 2.0 * math.pi * radii[0] / 8.0

            vehicle.set_simulate_physics(False)
            self._tick(vehicle)
            self._tick(vehicle)
            start = self._wheel_bones(vehicle)

            self._move(vehicle, eighth_turn)
            rolled = self._wheel_bones(vehicle)
            for name in start:
                self._assert_spin(
                    self._spin_between(start[name], rolled[name]), 45.0, radii,
                    "%s did not turn with the vehicle as it moved forward" % name)

            self._move(vehicle, -eighth_turn)
            returned = self._wheel_bones(vehicle)
            for name in start:
                self._assert_spin(
                    self._spin_between(start[name], returned[name]), 0.0, radii,
                    "%s did not turn back as the vehicle reversed to where it "
                    "started" % name)

            # Two and an eighth turns.
            self._move(vehicle, 17.0 * eighth_turn)
            wrapped = self._wheel_bones(vehicle)
            for name in start:
                self._assert_spin(
                    self._spin_between(start[name], wrapped[name]), 45.0, radii,
                    "%s is not an eighth of a turn on after two and an eighth "
                    "turns" % name)

            transform = vehicle.get_transform()
            forward = transform.get_forward_vector()
            transform.location += carla.Location(
                forward.x * TELEPORT_DISTANCE_M, forward.y * TELEPORT_DISTANCE_M, 0.0)
            vehicle.set_transform(transform)
            self._tick(vehicle)
            self._tick(vehicle)
            teleported = self._wheel_bones(vehicle)
            for name in start:
                self._assert_spin(
                    self._spin_between(wrapped[name], teleported[name]), 0.0, radii,
                    "%s turned when the vehicle was teleported" % name)
        finally:
            vehicle.destroy()

    def test_wheels_roll_under_chaos(self):
        print("TestVehicleWheelSpin.test_wheels_roll_under_chaos")
        vehicle = self._spawn_four_wheeler()
        try:
            radius = vehicle.get_physics_control().wheels[0].wheel_radius / 100.0
            vehicle.apply_control(carla.VehicleControl(throttle=0.35))
            for _ in range(25):
                self._tick(vehicle)

            before_location = vehicle.get_transform().location
            before = self._wheel_bones(vehicle)
            self._tick(vehicle)
            distance = vehicle.get_transform().location.distance(before_location)
            after = self._wheel_bones(vehicle)

            self.assertGreater(distance, 0.01, "the vehicle is not moving")
            expected = math.degrees(distance / radius)
            for name in before:
                spin = self._spin_between(before[name], after[name])
                self.assertAlmostEqual(
                    spin, expected, delta=0.5 * expected,
                    msg="%s turned %.1f degrees in a tick in which the vehicle "
                    "covered %.1f degrees worth of ground." % (name, spin, expected))
        finally:
            vehicle.destroy()
