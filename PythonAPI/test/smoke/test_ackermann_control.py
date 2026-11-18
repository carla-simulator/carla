# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Smoke test: Ackermann control starts a resting vehicle at small physics steps (#10291)."""

import carla

from . import SyncSmokeTest

SETTLE_SECONDS = 3.0  # long enough for the vehicle to come fully to rest
DRIVE_SECONDS = 4.0
TARGET_SPEED = 5.0    # [m/s]
MIN_DISTANCE = 5.0    # [m]; about 9 m in practice, 0.02 m with the bug


class TestAckermannControl(SyncSmokeTest):
    def straight_spawn_point(self, length=40.0):
        """A spawn point with length meters of straight road ahead."""
        carla_map = self.world.get_map()
        for spawn_point in carla_map.get_spawn_points():
            waypoint = carla_map.get_waypoint(spawn_point.location)
            yaw = waypoint.transform.rotation.yaw
            distance = 0.0
            while distance < length:
                following = waypoint.next(2.0)
                if not following or following[0].is_junction:
                    break
                waypoint = following[0]
                turn = (waypoint.transform.rotation.yaw - yaw + 180.0) % 360.0 - 180.0
                if abs(turn) > 4.0:
                    break
                distance += 2.0
            if distance >= length:
                return spawn_point
        self.skipTest(f'no spawn point with {length:.0f} m of straight road')
        return None

    def distance_driven(self, delta_seconds):
        """Meters covered in DRIVE_SECONDS after one Ackermann command from rest."""
        settings = self.world.get_settings()
        settings.fixed_delta_seconds = delta_seconds
        settings.max_substep_delta_time = min(0.01, delta_seconds)
        self.world.apply_settings(settings)

        blueprint = self.world.get_blueprint_library().find('vehicle.lincoln.mkz')
        vehicle = self.world.spawn_actor(blueprint, self.straight_spawn_point())
        try:
            for _ in range(int(round(SETTLE_SECONDS / delta_seconds))):
                self.world.tick()
            start = vehicle.get_location()
            vehicle.apply_ackermann_control(
                carla.VehicleAckermannControl(speed=TARGET_SPEED))
            for _ in range(int(round(DRIVE_SECONDS / delta_seconds))):
                self.world.tick()
            return vehicle.get_location().distance(start)
        finally:
            vehicle.destroy()
            self.world.tick()

    def test_starts_from_rest(self):
        for delta_seconds in (0.05, 0.002):
            with self.subTest(fixed_delta_seconds=delta_seconds):
                distance = self.distance_driven(delta_seconds)
                self.assertGreater(
                    distance, MIN_DISTANCE,
                    f'with fixed_delta_seconds={delta_seconds} the vehicle moved '
                    f'{distance:.2f} m in {DRIVE_SECONDS:.0f} s towards '
                    f'{TARGET_SPEED:.0f} m/s')
