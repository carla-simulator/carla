from . import SyncSmokeTest

import carla
import math
import time

# A steered front wheel passes well beyond this while the vehicle is turning,
# and settles well inside it once the steering command returns to centre. The
# exact angle depends on the asset's steering curve, so nothing here asserts a
# particular value.
ANGLE_EPSILON_DEG = 1.0

# Ticks to let the Chaos solver move the wheels to the commanded angle. The
# steering rate is per-asset, so this is generous rather than tight.
SETTLE_TICKS = 25


class TestVehicleWheelSteerAngle(SyncSmokeTest):
    """End-to-end smoke for `Vehicle.get_wheel_steer_angle()`.

    The UE5 simulator used to answer this RPC with a hard-coded 0.0 for every
    wheel, which no client could tell apart from a vehicle driving straight.
    The test steers one way, then the other, then back to centre, and asserts
    the reported front-wheel angles track that -- so the stub cannot come back
    unnoticed.
    """

    def tearDown(self):
        self.world.apply_settings(self.settings)
        self.world.tick()
        self.settings = None
        self.client.load_world("Town10HD_Opt")
        time.sleep(5)
        self.world = None
        self.client = None

    def _steer_and_read(self, vehicle, steer):
        vehicle.apply_control(carla.VehicleControl(throttle=0.2, steer=steer))
        for _ in range(SETTLE_TICKS):
            self.world.tick()
        return {
            location: vehicle.get_wheel_steer_angle(location)
            for location in (
                carla.VehicleWheelLocation.FL_Wheel,
                carla.VehicleWheelLocation.FR_Wheel,
                carla.VehicleWheelLocation.BL_Wheel,
                carla.VehicleWheelLocation.BR_Wheel)
        }

    def test_get_wheel_steer_angle_tracks_steering(self):
        print("TestVehicleWheelSteerAngle.test_get_wheel_steer_angle_tracks_steering")
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
            # The four wheel locations below only exist on a four-wheeler.
            self.assertGreaterEqual(len(vehicle.get_physics_control().wheels), 4)

            front = (carla.VehicleWheelLocation.FL_Wheel,
                     carla.VehicleWheelLocation.FR_Wheel)
            rear = (carla.VehicleWheelLocation.BL_Wheel,
                    carla.VehicleWheelLocation.BR_Wheel)

            right = self._steer_and_read(vehicle, 0.5)
            for location in front + rear:
                self.assertTrue(
                    math.isfinite(right[location]),
                    "wheel %s reported a non-finite angle" % location)
            for location in front:
                self.assertGreater(
                    abs(right[location]), ANGLE_EPSILON_DEG,
                    "front wheel %s reads ~0 deg while the vehicle is steering; "
                    "the simulator may have stopped reporting the physical "
                    "wheel angle." % location)

            left = self._steer_and_read(vehicle, -0.5)
            for location in front:
                self.assertGreater(
                    abs(left[location]), ANGLE_EPSILON_DEG,
                    "front wheel %s reads ~0 deg while the vehicle is steering "
                    "the other way." % location)
                self.assertLess(
                    right[location] * left[location], 0.0,
                    "front wheel %s did not change sign between opposite "
                    "steering commands (right=%f, left=%f)."
                    % (location, right[location], left[location]))

            # This asset steers with its front wheels only, so its rear wheels
            # stay put while the front ones swing. Whatever Chaos reports for
            # them is passed through, hence a comparison rather than a zero.
            for location in rear:
                self.assertLess(
                    abs(right[location]),
                    max(abs(right[other]) for other in front),
                    "rear wheel %s turned as far as the steered front wheels."
                    % location)

            centre = self._steer_and_read(vehicle, 0.0)
            for location in front:
                self.assertLess(
                    abs(centre[location]), ANGLE_EPSILON_DEG,
                    "front wheel %s did not return to centre after the "
                    "steering command did (%f deg)."
                    % (location, centre[location]))
        finally:
            vehicle.destroy()
