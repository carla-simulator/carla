from .vehicle_wheel_smoke_test import VehicleWheelSmokeTest

import carla
import math
import os
import tempfile

RECORD_DELTA_S = 0.05
REPLAY_DELTA_S = 1.0 / 60.0
ROLE_NAME = "wheel_replay"

# How far the replayed spin may stray from the body's travel in a tick.
SPIN_TOLERANCE = 0.3


class TestVehicleWheelReplay(VehicleWheelSmokeTest):
    """A replay shows the recorded wheel poses, interpolated between recorded
    frames like the positions."""

    def setUp(self):
        super(TestVehicleWheelReplay, self).setUp()
        self.log_path = os.path.join(tempfile.gettempdir(), "test_vehicle_wheel_replay.log")

    def tearDown(self):
        try:
            self.client.stop_replayer(False)
        except RuntimeError:
            pass
        try:
            os.remove(self.log_path)
        except OSError:
            pass
        super(TestVehicleWheelReplay, self).tearDown()

    def _set_delta(self, delta):
        settings = self.world.get_settings()
        settings.fixed_delta_seconds = delta
        self.world.apply_settings(settings)

    def _record(self):
        self._set_delta(RECORD_DELTA_S)
        bp = self.world.get_blueprint_library().find("vehicle.lincoln.mkz")
        bp.set_attribute("role_name", ROLE_NAME)
        vehicle = None
        for spawn_point in self.world.get_map().get_spawn_points():
            vehicle = self.world.try_spawn_actor(bp, spawn_point)
            if vehicle is not None:
                break
        self.assertIsNotNone(vehicle, "no free spawn point")
        try:
            self._tick(vehicle, 20)
            self.client.start_recorder(self.log_path)
            vehicle.apply_control(carla.VehicleControl(throttle=0.5))
            self._tick(vehicle, 60)
            self.client.stop_recorder()
            radius = vehicle.get_physics_control().wheels[0].wheel_radius / 100.0
        finally:
            vehicle.destroy()
        self.world.tick()
        return radius

    def test_replayed_wheels_are_interpolated(self):
        print("TestVehicleWheelReplay.test_replayed_wheels_are_interpolated")
        radius = self._record()

        # Three ticks for every recorded frame.
        self._set_delta(REPLAY_DELTA_S)
        self.client.replay_file(self.log_path, 0.0, 0.0, 0)
        self.world.tick()
        replayed = [
            actor for actor in self.world.get_actors().filter("vehicle.*")
            if actor.attributes.get("role_name") == ROLE_NAME]
        self.assertEqual(len(replayed), 1, "the replay did not spawn the vehicle")
        vehicle = replayed[0]
        # Into the recording, where the vehicle is moving.
        self._tick(vehicle, 90)

        wheel = carla.VehicleWheelLocation.FL_Wheel
        spin = vehicle.get_wheel_pitch_angle(wheel)
        transform = vehicle.get_transform()
        for tick in range(30):
            self._tick(vehicle)
            next_spin = vehicle.get_wheel_pitch_angle(wheel)
            next_transform = vehicle.get_transform()
            forward = transform.get_forward_vector()
            moved = next_transform.location - transform.location
            travel = moved.x * forward.x + moved.y * forward.y + moved.z * forward.z
            # Rolling forward turns the wheels to lower angles.
            expected = -math.degrees(travel / radius)
            turned = (next_spin - spin + 180.0) % 360.0 - 180.0
            self.assertGreater(
                abs(expected), 1.0, "the replayed vehicle is not moving in tick %d" % tick)
            self.assertLess(
                abs(turned - expected), SPIN_TOLERANCE * abs(expected),
                "the wheel turned %.2f degrees in tick %d, in which the body moved "
                "%.2f degrees worth of ground" % (turned, tick, expected))
            spin, transform = next_spin, next_transform
