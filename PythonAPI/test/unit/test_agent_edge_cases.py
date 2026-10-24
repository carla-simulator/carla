# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Offline regressions for navigation at coincident targets and exhausted routes.

Import the actual navigation and miscellaneous modules with small CARLA value
objects in place of the native extension. No simulator is needed.
"""

from collections import deque
import importlib
import math
from pathlib import Path
import sys
import types
import unittest
from unittest import mock


class Location:
    def __init__(self, x=0.0, y=0.0, z=0.0):
        self.x, self.y, self.z = x, y, z

    def __add__(self, other):
        return Location(self.x + other.x, self.y + other.y, self.z + other.z)

    def distance(self, other):
        return math.sqrt((self.x - other.x) ** 2 + (self.y - other.y) ** 2
                         + (self.z - other.z) ** 2)


class Transform:
    def __init__(self, x=0.0, y=0.0, yaw=0.0):
        self.location = Location(x, y)
        self.rotation = types.SimpleNamespace(yaw=yaw)

    def get_forward_vector(self):
        yaw = math.radians(self.rotation.yaw)
        return Location(math.cos(yaw), math.sin(yaw))

    def get_right_vector(self):
        yaw = math.radians(self.rotation.yaw)
        return Location(-math.sin(yaw), math.cos(yaw))


class VehicleControl:
    def __init__(self):
        self.steer = self.throttle = self.brake = 0.0
        self.hand_brake = self.manual_gear_shift = False


def load_navigation_modules():
    carla = types.ModuleType('carla')
    carla.Location = Location
    carla.VehicleControl = VehicleControl
    api_path = str(Path(__file__).resolve().parents[2] / 'carla')
    # Restore any pre-existing agents package and native CARLA module after import
    # so these offline tests do not affect tests that use the real extension.
    def is_replaced(name):
        return name == 'carla' or name == 'agents' or name.startswith('agents.')

    saved = {name: value for name, value in sys.modules.items() if is_replaced(name)}
    try:
        for name in saved:
            del sys.modules[name]
        sys.modules['carla'] = carla
        with mock.patch.object(sys, 'path', [api_path] + sys.path):
            controller = importlib.import_module('agents.navigation.controller')
            planner = importlib.import_module('agents.navigation.local_planner')
    finally:
        for name in list(sys.modules):
            if is_replaced(name):
                del sys.modules[name]
        sys.modules.update(saved)
    return controller, planner


controller, local_planner = load_navigation_modules()


class Waypoint:
    def __init__(self, x=0.0, y=0.0, yaw=0.0):
        self.transform = Transform(x, y, yaw)
        self.successors = []

    def next(self, _distance):
        return self.successors


class Vehicle:
    def __init__(self, transform=None, waypoint=None):
        self.transform = transform or Transform()
        self.map = types.SimpleNamespace(get_waypoint=mock.Mock(return_value=waypoint or Waypoint()))
        self.world = types.SimpleNamespace(get_map=lambda: self.map)

    def get_transform(self):
        return self.transform

    def get_location(self):
        return self.transform.location

    def get_velocity(self):
        return Location()

    def get_world(self):
        return self.world

    def get_control(self):
        return VehicleControl()


class TestLateralControllerTargets(unittest.TestCase):
    def test_coincident_target_has_no_steering_error(self):
        vehicle = Vehicle(Transform(4.0, 2.0))
        lateral = controller.PIDLateralController(vehicle)

        self.assertEqual(lateral.run_step(Waypoint(4.0, 2.0)), 0.0)

    def test_ahead_left_and_right_targets_keep_their_steering(self):
        for x, y, expected in [(10.0, 0.0, 0.0), (10.0, 10.0, math.pi / 8),
                               (10.0, -10.0, -math.pi / 8)]:
            with self.subTest(x=x, y=y):
                lateral = controller.PIDLateralController(Vehicle(), K_P=0.5)

                self.assertAlmostEqual(lateral.run_step(Waypoint(x, y)), expected)

    def test_coincident_target_is_checked_after_applying_offset(self):
        lateral = controller.PIDLateralController(Vehicle(Transform(0.0, 2.0)), offset=2.0)

        self.assertEqual(lateral.run_step(Waypoint()), 0.0)

    def test_offset_from_coincident_waypoint_still_requires_steering(self):
        lateral = controller.PIDLateralController(Vehicle(), offset=2.0, K_P=0.5)

        self.assertAlmostEqual(lateral.run_step(Waypoint()), math.pi / 4)

    def test_coincident_target_preserves_pid_history_with_zero_new_error(self):
        lateral = controller.PIDLateralController(
            Vehicle(), K_P=0.5, K_I=0.2, K_D=0.01, dt=0.1)
        lateral.run_step(Waypoint(10.0, 10.0))

        # A previous pi/4 error contributes an integral of pi/40 and a
        # derivative of -pi/0.4 when the new target error becomes zero.
        self.assertAlmostEqual(lateral.run_step(Waypoint()), -math.pi / 50)


class TestLocalPlannerExhaustedRoutes(unittest.TestCase):
    def make_planner(self, waypoint=None):
        vehicle = Vehicle(waypoint=waypoint)
        planner = local_planner.LocalPlanner(vehicle)
        planner._vehicle_controller = mock.Mock()
        return planner

    def assert_stopped(self, control):
        self.assertEqual((control.steer, control.throttle, control.brake), (0.0, 0.0, 1.0))
        self.assertFalse(control.hand_brake)
        self.assertFalse(control.manual_gear_shift)

    def test_empty_queue_cannot_be_extended_without_a_seed_waypoint(self):
        planner = self.make_planner()
        planner._waypoints_queue.clear()

        planner._compute_next_waypoints(k=100)

        self.assertTrue(planner.done())

    def test_empty_queue_returns_safe_braking_control(self):
        planner = self.make_planner()
        planner._waypoints_queue.clear()

        self.assert_stopped(planner.run_step())
        planner._vehicle_controller.run_step.assert_not_called()

    def test_repeated_steps_after_reaching_dead_end_stay_stopped(self):
        planner = self.make_planner(Waypoint())

        for _ in range(3):
            self.assert_stopped(planner.run_step())
        self.assertTrue(planner.done())
        # An exhausted route should not be silently restarted from the map.
        planner._vehicle.map.get_waypoint.assert_called_once()

    def test_nonempty_queue_extends_along_successors(self):
        start, middle, end = Waypoint(5.0), Waypoint(7.0), Waypoint(9.0)
        start.successors, middle.successors = [middle], [end]
        planner = self.make_planner(start)

        planner._compute_next_waypoints(k=2)

        self.assertEqual(list(planner.get_plan()), [
            (start, local_planner.RoadOption.LANEFOLLOW),
            (middle, local_planner.RoadOption.LANEFOLLOW),
            (end, local_planner.RoadOption.LANEFOLLOW)])

    def test_waypoint_extension_respects_queue_capacity(self):
        start, middle, end = Waypoint(5.0), Waypoint(7.0), Waypoint(9.0)
        start.successors, middle.successors = [middle], [end]
        planner = self.make_planner(start)
        planner._waypoints_queue = deque(planner.get_plan(), maxlen=2)

        planner._compute_next_waypoints(k=100)

        self.assertEqual([wp for wp, _ in planner.get_plan()], [start, middle])

    def test_new_plan_after_exhaustion_resumes_vehicle_controller(self):
        planner = self.make_planner()
        planner._waypoints_queue.clear()
        target = Waypoint(10.0)
        planner.set_global_plan([(target, local_planner.RoadOption.LANEFOLLOW)])

        result = planner.run_step()

        planner._vehicle_controller.run_step.assert_called_once_with(20.0, target)
        self.assertIs(result, planner._vehicle_controller.run_step.return_value)


if __name__ == '__main__':
    unittest.main()
