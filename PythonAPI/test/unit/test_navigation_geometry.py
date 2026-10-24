# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Offline navigation geometry regressions using the actual agents modules.

Only CARLA's vector/location constructors are stubbed; routing and geometry
run from the production modules with NumPy and networkx. No server or native
CARLA extension is required, and the import stubs do not escape this module.
"""

import importlib
import math
from pathlib import Path
import sys
import types
import unittest
from unittest import mock

import networkx as nx
import numpy as np


class _Vector:
    def __init__(self, x=0.0, y=0.0, z=0.0):
        self.x, self.y, self.z = x, y, z

    def __add__(self, other):
        return _Vector(self.x + other.x, self.y + other.y, self.z + other.z)


def _load_agents():
    agents_dir = Path(__file__).resolve().parents[2] / "carla" / "agents"
    carla_stub = types.ModuleType("carla")
    carla_stub.Vector3D = _Vector
    carla_stub.Location = _Vector
    # Restore both existing modules and any modules imported here, so discovery
    # alongside tests using the real CARLA extension remains independent.
    with mock.patch.dict(sys.modules):
        for name in list(sys.modules):
            if name == "agents" or name.startswith("agents."):
                del sys.modules[name]
        sys.modules["carla"] = carla_stub
        for name, directory in (
                ("agents", agents_dir),
                ("agents.navigation", agents_dir / "navigation"),
                ("agents.tools", agents_dir / "tools")):
            package = types.ModuleType(name)
            package.__path__ = [str(directory)]
            sys.modules[name] = package
        misc = importlib.import_module("agents.tools.misc")
        planner = importlib.import_module("agents.navigation.global_route_planner")
        local = importlib.import_module("agents.navigation.local_planner")
    return misc, planner.GlobalRoutePlanner, local.RoadOption


MISC, GlobalRoutePlanner, RoadOption = _load_agents()


class TestTurnDecisionGeometry(unittest.TestCase):
    def make_planner(self, turn_vector, alternatives=()):
        planner = GlobalRoutePlanner.__new__(GlobalRoutePlanner)
        planner._graph = nx.DiGraph()
        planner._graph.add_edge(
            0, 1, type=RoadOption.LANEFOLLOW, intersection=False,
            exit_vector=np.array([1.0, 0.0, 0.0]))
        planner._graph.add_edge(
            1, 2, type=RoadOption.LANEFOLLOW, intersection=True,
            exit_vector=np.array(turn_vector), net_vector=list(turn_vector))
        for node, vector in enumerate(alternatives, 3):
            planner._graph.add_edge(
                1, node, type=RoadOption.LANEFOLLOW, intersection=True,
                net_vector=vector)
        planner._previous_decision = RoadOption.VOID
        planner._intersection_end_node = -1
        return planner

    def add_loose_end(self, planner):
        tail = types.SimpleNamespace(
            road_id=99, section_id=0, lane_id=-1,
            transform=types.SimpleNamespace(location=_Vector(1.0, 1.0, 0.0)),
            next=lambda distance: [])
        exit_waypoint = types.SimpleNamespace(
            road_id=99, section_id=0, lane_id=-1, is_junction=True,
            next=lambda distance: [tail])
        planner._topology = [{"exit": exit_waypoint, "exitxyz": (1.0, 0.0, 0.0)}]
        planner._id_map = {(1.0, 0.0, 0.0): 1}
        planner._road_id_to_edge = {}
        planner._sampling_resolution = 2.0
        # Exercise the production creator of the None-valued side branch.
        planner._find_loose_ends()
        self.assertIsNone(planner._graph.edges[1, -1]["net_vector"])

    def test_turns_with_loose_end_alternative(self):
        for vector, expected in (
                ((1.0, 0.0, 0.0), RoadOption.STRAIGHT),
                ((0.0, -1.0, 0.0), RoadOption.LEFT),
                ((0.0, 1.0, 0.0), RoadOption.RIGHT)):
            with self.subTest(expected=expected):
                planner = self.make_planner(vector)
                self.add_loose_end(planner)
                self.assertEqual(planner._turn_decision(1, [0, 1, 2]), expected)
                self.assertEqual(planner._previous_decision, expected)

    def test_ordinary_junction_decisions(self):
        for vector, expected in (
                ((1.0, 0.0, 0.0), RoadOption.STRAIGHT),
                ((0.0, -1.0, 0.0), RoadOption.LEFT),
                ((0.0, 1.0, 0.0), RoadOption.RIGHT)):
            with self.subTest(expected=expected):
                planner = self.make_planner(vector, alternatives=[[1.0, 0.0, 0.0]])
                self.assertEqual(planner._turn_decision(1, [0, 1, 2]), expected)

    def test_valid_alternatives_still_influence_turn_decision(self):
        # The selected branch is to the left of the other branch even though
        # both branches have positive y. Ignoring all alternatives changes it.
        vector = (0.5, math.sqrt(3.0) / 2.0, 0.0)
        for include_loose_end in (False, True):
            with self.subTest(include_loose_end=include_loose_end):
                planner = self.make_planner(vector, alternatives=[[0.0, 1.0, 0.0]])
                if include_loose_end:
                    self.add_loose_end(planner)
                self.assertEqual(
                    planner._turn_decision(1, [0, 1, 2]), RoadOption.LEFT)


class TestPointRotation(unittest.TestCase):
    def assert_rotation(self, point, angle, expected):
        result = MISC._rotate_point(_Vector(*point), math.radians(angle))
        for actual, wanted in zip((result.x, result.y, result.z), expected):
            self.assertAlmostEqual(actual, wanted)

    def test_identity_preserves_nonzero_y(self):
        self.assert_rotation((2.0, 3.0, 4.0), 0.0, (2.0, 3.0, 4.0))

    def test_quarter_turn(self):
        self.assert_rotation((2.0, 3.0, 4.0), 90.0, (-3.0, 2.0, 4.0))

    def test_negative_quarter_turn(self):
        self.assert_rotation((2.0, 3.0, 4.0), -90.0, (3.0, -2.0, 4.0))

    def test_general_rotation(self):
        self.assert_rotation(
            (2.0, 3.0, -4.0), 30.0,
            (math.sqrt(3.0) - 1.5, 1.0 + 1.5 * math.sqrt(3.0), -4.0))

    def test_rotation_and_inverse_preserve_point(self):
        point = _Vector(-2.0, 3.0, 4.0)
        rotated = MISC._rotate_point(point, math.radians(37.0))
        restored = MISC._rotate_point(rotated, math.radians(-37.0))
        self.assertAlmostEqual(restored.x, point.x)
        self.assertAlmostEqual(restored.y, point.y)
        self.assertEqual(restored.z, point.z)

    def test_trigger_location_keeps_existing_vertical_offset_behavior(self):
        for yaw in (0.0, 90.0, -30.0):
            with self.subTest(yaw=yaw):
                transform = types.SimpleNamespace(
                    rotation=types.SimpleNamespace(yaw=yaw),
                    transform=lambda location: _Vector(10.0, 20.0, 30.0))
                light = types.SimpleNamespace(
                    get_transform=lambda: transform,
                    trigger_volume=types.SimpleNamespace(
                        location=_Vector(1.0, 2.0, 3.0), extent=_Vector(2.0, 3.0, 4.0)))
                result = MISC.get_trafficlight_trigger_location(light)
                self.assertEqual((result.x, result.y, result.z), (10.0, 20.0, 30.0))


if __name__ == "__main__":
    unittest.main()
