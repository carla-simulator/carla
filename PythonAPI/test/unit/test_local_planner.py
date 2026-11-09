# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Local planner route exhaustion and resumption without a running server."""

from pathlib import Path
import sys
import unittest
from unittest import mock

import carla

# The agents are source modules, separate from the installed client wheel.
sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'carla'))
from agents.navigation.local_planner import LocalPlanner, RoadOption  # pylint: disable=wrong-import-position


class TestLocalPlanner(unittest.TestCase):
    """Exercise the production planner and PID with native CARLA value types."""

    def setUp(self):
        self.waypoint = self.make_waypoint(0.0)
        world = mock.Mock()
        world.get_map.return_value.get_waypoint.return_value = self.waypoint
        self.vehicle = mock.Mock()
        self.vehicle.get_world.return_value = world
        self.vehicle.get_control.return_value = carla.VehicleControl()
        self.vehicle.get_location.return_value = carla.Location()
        self.vehicle.get_velocity.return_value = carla.Vector3D()
        self.vehicle.get_transform.return_value = carla.Transform()
        self.planner = LocalPlanner(self.vehicle)

    @staticmethod
    def make_waypoint(x):
        """Supply a terminal waypoint with a native transform."""
        waypoint = mock.Mock(spec=['transform', 'next'])
        waypoint.transform = carla.Transform(carla.Location(x=x))
        waypoint.next.return_value = []
        return waypoint

    def assert_braking(self, control):
        """An exhausted route must retain the existing full brake command."""
        self.assertIsInstance(control, carla.VehicleControl)
        self.assertEqual((control.steer, control.throttle, control.brake), (0.0, 0.0, 1.0))
        self.assertFalse(control.hand_brake)
        self.assertFalse(control.manual_gear_shift)
        self.assertTrue(self.planner.done())

    def test_terminal_waypoint_keeps_braking_on_later_steps(self):
        """Consuming the final generated waypoint must remain safe next tick."""
        for _ in range(3):
            self.assert_braking(self.planner.run_step())
        self.waypoint.next.assert_called_once_with(2.0)

    def test_finite_plan_follows_then_stops_without_extending(self):
        """A supplied finite plan still uses PID and stops at its endpoint."""
        target = self.make_waypoint(10.0)
        self.planner.set_global_plan(
            [(target, RoadOption.LANEFOLLOW)], stop_waypoint_creation=True)
        control = self.planner.run_step()
        self.assertGreater(control.throttle, 0.0)
        self.assertEqual(control.brake, 0.0)
        self.assertIs(self.planner.target_waypoint, target)
        self.assertFalse(self.planner.done())
        target.next.assert_not_called()

        self.vehicle.get_location.return_value = target.transform.location
        self.assert_braking(self.planner.run_step())
        self.assert_braking(self.planner.run_step())
        target.next.assert_not_called()

    def test_new_plan_resumes_after_terminal_stop(self):
        """Stopping at a terminal waypoint must allow another route to start."""
        self.assert_braking(self.planner.run_step())
        target = self.make_waypoint(10.0)
        self.planner.set_global_plan(
            [(target, RoadOption.LANEFOLLOW)], stop_waypoint_creation=False)
        control = self.planner.run_step()
        self.assertGreater(control.throttle, 0.0)
        self.assertEqual(control.brake, 0.0)
        self.assertIs(self.planner.target_waypoint, target)
        self.assertFalse(self.planner.done())
        target.next.assert_called_once_with(2.0)


if __name__ == '__main__':
    unittest.main()
