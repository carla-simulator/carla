from . import SyncSmokeTest

import carla
import math

# A wheel that jumps is off by far more than this; one that rolls, or holds for
# a frame, is not.
JUMP_TOLERANCE_DEG = 30.0


class VehicleWheelSmokeTest(SyncSmokeTest):
    """Shared setup for the smoke tests that read a vehicle's wheels. Runs on
    whatever map is loaded; each test destroys its own vehicle."""

    def tearDown(self):
        # Restores the settings without reloading the map, which no test here needs.
        self.world.apply_settings(self.settings)
        if self.settings.synchronous_mode:
            self.world.tick()
        self.settings = None
        self.world = None
        self.client = None

    def _spawn_vehicle(self, number_of_wheels, settle_ticks=40):
        # Motorbikes and bicycles are also "vehicle.*", and their Chaos setup
        # still carries four wheels, so select by blueprint instead.
        bp_lib = self.world.get_blueprint_library()
        vehicle_bps = [
            bp for bp in self.filter_vehicles_for_old_towns(bp_lib.filter("vehicle.*"))
            if int(bp.get_attribute("number_of_wheels")) == number_of_wheels]
        self.assertGreater(len(vehicle_bps), 0)

        vehicle = None
        for spawn_point in self.world.get_map().get_spawn_points():
            vehicle = self.world.try_spawn_actor(vehicle_bps[0], spawn_point)
            if vehicle is not None:
                break
        self.assertIsNotNone(vehicle, "no free spawn point")
        self._tick(vehicle, settle_ticks)
        return vehicle

    def _spawn_four_wheeler(self, settle_ticks=40):
        return self._spawn_vehicle(4, settle_ticks)

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

    def _assert_no_wheel_jump(self, vehicle, ticks, message):
        """Ticks, checking that no wheel bone turns by more than the vehicle's
        travel in any tick, give or take JUMP_TOLERANCE_DEG."""
        radius = vehicle.get_physics_control().wheels[0].wheel_radius / 100.0
        before = self._wheel_bones(vehicle)
        before_transform = vehicle.get_transform()
        for tick in range(ticks):
            self._tick(vehicle)
            after = self._wheel_bones(vehicle)
            after_transform = vehicle.get_transform()
            forward = before_transform.get_forward_vector()
            moved = after_transform.location - before_transform.location
            travel = moved.x * forward.x + moved.y * forward.y + moved.z * forward.z
            expected = math.degrees(travel / radius)
            for name in before:
                spin = self._spin_between(before[name], after[name])
                error = (spin - expected + 180.0) % 360.0 - 180.0
                self.assertLess(
                    abs(error), abs(expected) + JUMP_TOLERANCE_DEG,
                    "%s: %s turned %.1f degrees in tick %d, in which the vehicle "
                    "covered %.1f degrees worth of ground." % (
                        message, name, spin, tick, expected))
            before, before_transform = after, after_transform

    @staticmethod
    def _spin_between(before, after):
        """Degrees a wheel bone turned about its axle, positive when rolling
        forward."""
        before_axes = (before.get_forward_vector(), before.get_up_vector())
        after_axes = (after.get_forward_vector(), after.get_up_vector())
        # The wheel turns in the vehicle's XZ plane; follow the bone axis in it.
        axis = 0 if abs(before_axes[0].y) < abs(before_axes[1].y) else 1

        def angle(vector):
            return math.degrees(math.atan2(vector.z, vector.x))

        spin = angle(before_axes[axis]) - angle(after_axes[axis])
        return (spin + 180.0) % 360.0 - 180.0
