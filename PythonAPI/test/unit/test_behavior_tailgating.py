# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Offline actual-module tests for the destination used by tailgating reroutes."""

from collections import deque
from enum import IntEnum
import importlib
from pathlib import Path
import sys
import types
import unittest
from unittest import mock


class Location:
    def __init__(self, x=0.0, y=0.0, z=0.0):
        self.x, self.y, self.z = x, y, z


class LaneChange(IntEnum):
    NONE = 0
    Left = 1
    Right = 2
    Both = 3


def load_navigation_modules():
    carla = types.ModuleType("carla")
    carla.Actor = type("Actor", (), {})
    carla.TrafficLight = type("TrafficLight", (carla.Actor,), {})
    carla.Location = Location
    carla.LaneChange = LaneChange
    carla.LaneType = types.SimpleNamespace(Driving=1)
    api_path = str(Path(__file__).resolve().parents[2] / "carla")

    def is_replaced(name):
        return name == "carla" or name == "agents" or name.startswith("agents.")

    saved = {name: value for name, value in sys.modules.items() if is_replaced(name)}
    try:
        for name in saved:
            del sys.modules[name]
        sys.modules["carla"] = carla
        with mock.patch.object(sys, "path", [api_path] + sys.path):
            behavior = importlib.import_module("agents.navigation.behavior_agent")
            local = importlib.import_module("agents.navigation.local_planner")
    finally:
        for name in list(sys.modules):
            if is_replaced(name):
                del sys.modules[name]
        sys.modules.update(saved)
    return behavior.BehaviorAgent, local.LocalPlanner, local.RoadOption


BehaviorAgent, LocalPlanner, RoadOption = load_navigation_modules()


class Waypoint:
    def __init__(self, x, lane_id=-1):
        self.lane_id = lane_id
        self.lane_type = 1
        self.transform = types.SimpleNamespace(location=Location(x, lane_id))


class TestBehaviorTailgatingDestination(unittest.TestCase):
    def setUp(self):
        self.near = Waypoint(5.0)
        self.middle = Waypoint(40.0)
        self.destination = Waypoint(100.0)
        self.right = Waypoint(0.0, lane_id=-2)
        self.left = Waypoint(0.0, lane_id=-3)
        self.current = Waypoint(0.0)
        self.current.left_lane_marking = types.SimpleNamespace(lane_change=LaneChange.NONE)
        self.current.right_lane_marking = types.SimpleNamespace(lane_change=LaneChange.NONE)
        self.current.get_left_lane = lambda: self.left
        self.current.get_right_lane = lambda: self.right

        self.planner = LocalPlanner.__new__(LocalPlanner)
        self.original_plan = [(self.near, RoadOption.LANEFOLLOW),
                              (self.middle, RoadOption.RIGHT),
                              (self.destination, RoadOption.LANEFOLLOW)]
        self.planner._waypoints_queue = deque(self.original_plan, maxlen=10000)
        self.planner.target_waypoint = self.near
        self.planner._stop_waypoint_creation = False

        self.agent = BehaviorAgent.__new__(BehaviorAgent)
        self.agent._local_planner = self.planner
        self.agent._speed = 10.0
        self.agent._speed_limit = 30.0
        self.agent._behavior = types.SimpleNamespace(
            min_proximity_threshold=10.0, tailgate_counter=7)
        self.behind = types.SimpleNamespace(
            get_velocity=lambda: Location(x=20.0 / 3.6))
        lookup = {id(wp.transform.location): wp
                  for wp in (self.near, self.middle, self.destination, self.right, self.left)}
        self.agent._map = types.SimpleNamespace(
            get_waypoint=lambda location: lookup[id(location)])
        self.agent.trace_route = mock.Mock(side_effect=lambda start, end: [
            (start, RoadOption.LANEFOLLOW), (end, RoadOption.LANEFOLLOW)])

    def choose_lane(self, side, blocked=False, tailgater=True):
        getattr(self.current, side + "_lane_marking").lane_change = getattr(LaneChange, side.title())
        self.agent._vehicle_obstacle_detected = mock.Mock(side_effect=[
            (tailgater, self.behind if tailgater else None, 1.0),
            (blocked, None, 0.0)])

    def assert_unchanged(self, expected_plan=None):
        self.agent.trace_route.assert_not_called()
        self.assertEqual(self.agent._behavior.tailgate_counter, 7)
        self.assertEqual(list(self.planner.get_plan()),
                         self.original_plan if expected_plan is None else expected_plan)
        self.assertFalse(self.planner._stop_waypoint_creation)

    def assert_reroutes_to_final_destination(self, side):
        self.choose_lane(side)
        self.agent._tailgating(self.current, [self.behind])
        start = getattr(self, side)
        # BasicAgent.set_destination and LocalPlanner.set_global_plan execute
        # unchanged: the recomputed route replaces the previous queue.
        self.agent.trace_route.assert_called_once_with(start, self.destination)
        self.assertEqual(list(self.planner.get_plan()), [
            (start, RoadOption.LANEFOLLOW), (self.destination, RoadOption.LANEFOLLOW)])
        self.assertEqual(self.agent._behavior.tailgate_counter, 200)
        self.assertTrue(self.planner._stop_waypoint_creation)
        detection = self.agent._vehicle_obstacle_detected.call_args_list[1]
        self.assertEqual(detection.kwargs["lane_offset"], 1 if side == "right" else -1)

    def test_right_lane_reroute_retains_far_destination(self):
        self.assert_reroutes_to_final_destination("right")

    def test_left_lane_reroute_retains_far_destination(self):
        self.assert_reroutes_to_final_destination("left")

    def test_empty_plan_does_not_reroute_or_reset_counter(self):
        self.planner._waypoints_queue.clear()
        self.choose_lane("right")
        self.agent._tailgating(self.current, [self.behind])
        self.assert_unchanged(expected_plan=[])

    def test_blocked_right_lane_leaves_route_and_counter_unchanged(self):
        self.choose_lane("right", blocked=True)
        self.agent._tailgating(self.current, [self.behind])
        self.assert_unchanged()

    def test_blocked_left_lane_leaves_route_and_counter_unchanged(self):
        self.choose_lane("left", blocked=True)
        self.agent._tailgating(self.current, [self.behind])
        self.assert_unchanged()

    def test_no_tailgater_leaves_route_and_counter_unchanged(self):
        self.choose_lane("right", tailgater=False)
        self.agent._tailgating(self.current, [])
        self.assert_unchanged()

    def test_slower_vehicle_behind_does_not_trigger_reroute(self):
        self.choose_lane("right")
        self.behind.get_velocity = lambda: Location(x=5.0 / 3.6)
        self.agent._tailgating(self.current, [self.behind])
        self.assert_unchanged()


if __name__ == "__main__":
    unittest.main()
