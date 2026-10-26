# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Offline agent regressions with real control/geometry objects and inert RPCs."""

from collections import deque
import contextlib
import io
import os
from pathlib import Path
import sys
from types import SimpleNamespace
import unittest
from unittest import mock

import carla

SOURCE_ROOT = Path(os.environ.get(
    "CARLA_REGRESSION_SOURCE_ROOT", str(Path(__file__).resolve().parents[3])))
sys.path.insert(0, str(SOURCE_ROOT / "PythonAPI" / "carla"))

from agents.navigation.basic_agent import BasicAgent
from agents.navigation.constant_velocity_agent import ConstantVelocityAgent
from agents.navigation.local_planner import LocalPlanner, RoadOption


def _initialize_base(agent, vehicle, *_args, **_kwargs):
    agent._vehicle = vehicle


class TestPythonAgentRegressions(unittest.TestCase):
    def test_constant_velocity_initial_speed_uses_meters_per_second(self):
        vehicle = mock.Mock()
        vehicle.get_velocity.return_value = carla.Vector3D()
        with mock.patch.object(BasicAgent, "__init__", _initialize_base), \
                mock.patch.object(ConstantVelocityAgent, "_set_collision_sensor"):
            ConstantVelocityAgent(vehicle, target_speed=36.0)
        velocity = vehicle.enable_constant_velocity.call_args.args[0]
        self.assertAlmostEqual(velocity.x, 10.0)
        self.assertEqual((velocity.y, velocity.z), (0.0, 0.0))

    def test_constant_velocity_restart_retains_speed_units(self):
        vehicle = mock.Mock()
        vehicle.get_velocity.return_value = carla.Vector3D()
        with mock.patch.object(BasicAgent, "__init__", _initialize_base), \
                mock.patch.object(ConstantVelocityAgent, "_set_collision_sensor"):
            agent = ConstantVelocityAgent(vehicle, target_speed=36.0)
        vehicle.enable_constant_velocity.reset_mock()
        agent.restart_constant_velocity()
        self.assertAlmostEqual(vehicle.enable_constant_velocity.call_args.args[0].x, 10.0)

    @staticmethod
    def _planner(options):
        waypoint = SimpleNamespace(transform=carla.Transform())
        world = SimpleNamespace(get_map=lambda: SimpleNamespace(get_waypoint=lambda _: waypoint))
        vehicle = SimpleNamespace(
            get_world=lambda: world, get_control=carla.VehicleControl,
            get_location=carla.Location)
        return LocalPlanner(vehicle, opt_dict=options)

    def test_planner_dt_reaches_both_default_pid_controllers(self):
        planner = self._planner({"dt": 0.1})
        self.assertEqual(planner._vehicle_controller._lat_controller._dt, 0.1)
        self.assertEqual(planner._vehicle_controller._lon_controller._dt, 0.1)

    def test_planner_explicit_pid_dt_is_preserved(self):
        planner = self._planner({
            "dt": 0.1,
            "lateral_control_dict": {"K_P": 1, "K_D": 0, "K_I": 0, "dt": 0.2},
            "longitudinal_control_dict": {"K_P": 1, "K_D": 0, "K_I": 0, "dt": 0.3},
        })
        self.assertEqual(planner._vehicle_controller._lat_controller._dt, 0.2)
        self.assertEqual(planner._vehicle_controller._lon_controller._dt, 0.3)

    def test_planner_dt_reaches_custom_gains_without_mutating_options(self):
        lateral = {"K_P": 1.0}
        longitudinal = {"K_P": 2.0}
        planner = self._planner({
            "dt": 0.1, "lateral_control_dict": lateral,
            "longitudinal_control_dict": longitudinal,
        })
        self.assertEqual(planner._vehicle_controller._lat_controller._dt, 0.1)
        self.assertEqual(planner._vehicle_controller._lon_controller._dt, 0.1)
        self.assertEqual(lateral, {"K_P": 1.0})
        self.assertEqual(longitudinal, {"K_P": 2.0})

    def test_planner_default_dt_is_unchanged(self):
        planner = self._planner({})
        self.assertEqual(planner._vehicle_controller._lat_controller._dt, 0.05)
        self.assertEqual(planner._vehicle_controller._lon_controller._dt, 0.05)

    @staticmethod
    def _lane_change_agent(path):
        agent = BasicAgent.__new__(BasicAgent)
        original = (object(), RoadOption.LANEFOLLOW)
        queue = deque([original])
        def set_plan(plan, **kwargs):
            if kwargs["clean_queue"]:
                queue.clear()
            queue.extend(plan)
        agent._vehicle = SimpleNamespace(
            get_velocity=lambda: carla.Vector3D(5, 0, 0), get_location=carla.Location)
        agent._map = SimpleNamespace(get_waypoint=lambda _: object())
        agent._local_planner = SimpleNamespace(set_global_plan=set_plan)
        agent._sampling_resolution = 2.0
        agent._generate_lane_change_path = mock.Mock(return_value=path)
        return agent, queue, original

    def test_failed_lane_change_keeps_existing_route(self):
        agent, queue, original = self._lane_change_agent([])
        with contextlib.redirect_stdout(io.StringIO()):
            agent.lane_change("left")
        self.assertEqual(list(queue), [original])

    def test_valid_lane_change_replaces_route(self):
        path = [(object(), RoadOption.CHANGELANELEFT)]
        agent, queue, _ = self._lane_change_agent(path)
        agent.lane_change("left")
        self.assertEqual(list(queue), path)


if __name__ == "__main__":
    unittest.main()
