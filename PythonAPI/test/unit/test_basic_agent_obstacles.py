# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Offline obstacle detection checks using real CARLA geometry and inert actors."""

import math
import os
import sys
from types import SimpleNamespace
import unittest

import carla

sys.path.insert(0, os.path.abspath(os.path.join(
    os.path.dirname(__file__), "..", "..", "carla")))

from agents.navigation.basic_agent import BasicAgent
from agents.navigation.local_planner import RoadOption


class _Vehicle:
    def __init__(self, actor_id, x, y=0.0, yaw=0.0, extent_x=2.0):
        self.id = actor_id
        self._location = carla.Location(x, y, 0.0)
        self._rotation = carla.Rotation(yaw=yaw)
        self.bounding_box = carla.BoundingBox(
            carla.Location(), carla.Vector3D(extent_x, 1.0, 1.0))
        self.returned_transforms = []

    def get_transform(self):
        transform = carla.Transform(self._location, self._rotation)
        self.returned_transforms.append(transform)
        return transform

    def get_location(self):
        return carla.Location(self._location)


class TestBasicAgentObstacles(unittest.TestCase):
    @staticmethod
    def _agent(ego, junction=False, plan=()):
        # Map/actor/planner RPC boundaries are inert. The production detection
        # method, CARLA transforms and bounding boxes, and Shapely stay real.
        waypoint = SimpleNamespace(
            lane_id=-1, road_id=1, lane_width=4.0, is_junction=junction)
        agent = BasicAgent.__new__(BasicAgent)
        agent._vehicle = ego
        agent._map = SimpleNamespace(get_waypoint=lambda *args, **kwargs: waypoint)
        agent._local_planner = SimpleNamespace(get_plan=lambda: plan)
        agent._ignore_vehicles = False
        agent._base_vehicle_threshold = 50.0
        agent._offset = 0.0
        agent._use_bbs_detection = False
        return agent

    def test_same_lane_distance_is_between_actor_centers(self):
        for ego_extent, target_extent in [(2.0, 2.0), (3.0, 1.0)]:
            with self.subTest(ego_extent=ego_extent, target_extent=target_extent):
                ego = _Vehicle(1, 0.0, extent_x=ego_extent)
                target = _Vehicle(2, 20.0, extent_x=target_extent)
                result = self._agent(ego)._vehicle_obstacle_detected(
                    [target], 50.0, low_angle_th=-1.0)
                self.assertTrue(result.obstacle_was_found)
                self.assertIs(result.obstacle, target)
                # BehaviorAgent subtracts the bounding box extents itself.
                self.assertAlmostEqual(result.distance, 20.0, places=5)

    def test_rotated_vehicles_retain_their_center_distance(self):
        yaw = 35.0
        radians = math.radians(yaw)
        ego = _Vehicle(1, 10.0, 5.0, yaw=yaw)
        target = _Vehicle(2, 10.0 + 20.0 * math.cos(radians),
                          5.0 + 20.0 * math.sin(radians), yaw=yaw)
        result = self._agent(ego)._vehicle_obstacle_detected(
            [target], 50.0, low_angle_th=-1.0)
        self.assertTrue(result.obstacle_was_found)
        self.assertAlmostEqual(result.distance, 20.0, places=4)

    def test_target_rotation_does_not_move_its_center_for_distance(self):
        ego = _Vehicle(1, 0.0)
        target = _Vehicle(2, 20.0, yaw=90.0)
        result = self._agent(ego)._vehicle_obstacle_detected([target], 50.0)
        self.assertTrue(result.obstacle_was_found)
        self.assertAlmostEqual(result.distance, 20.0, places=5)

    def test_angle_filter_still_uses_front_and_rear_bumpers(self):
        # Centers have a 5.7 degree bearing; the bumper bearing is 9.5 degrees.
        # This brackets the latter and detects removal of either bumper offset.
        for upper_angle, expected in [(8.0, False), (12.0, True)]:
            with self.subTest(upper_angle=upper_angle):
                ego = _Vehicle(1, 0.0)
                target = _Vehicle(2, 10.0, y=1.0)
                result = self._agent(ego)._vehicle_obstacle_detected(
                    [target], 50.0, up_angle_th=upper_angle)
                self.assertEqual(result.obstacle_was_found, expected)

    def test_bumper_offsets_do_not_modify_center_transforms(self):
        # Actor.get_transform() already returns a local snapshot. Keep that
        # snapshot centered for later calculations; no server actor is changed.
        ego = _Vehicle(1, 0.0)
        target = _Vehicle(2, 20.0)
        self._agent(ego)._vehicle_obstacle_detected(
            [target], 50.0, low_angle_th=-1.0)
        self.assertEqual(ego.returned_transforms[0].location, ego.get_location())
        self.assertEqual(target.returned_transforms[0].location, target.get_location())

    def test_distance_prefilter_uses_actor_centers(self):
        ego = _Vehicle(1, 0.0)
        target = _Vehicle(2, 21.0)
        result = self._agent(ego)._vehicle_obstacle_detected(
            [target], 20.0, low_angle_th=-1.0)
        self.assertFalse(result.obstacle_was_found)

    def test_vehicle_behind_ego_is_not_detected_as_ahead(self):
        ego = _Vehicle(1, 0.0)
        target = _Vehicle(2, -20.0)
        result = self._agent(ego)._vehicle_obstacle_detected(
            [target], 50.0, low_angle_th=-1.0)
        self.assertFalse(result.obstacle_was_found)

    def test_junction_vehicle_outside_route_polygon_is_not_detected(self):
        ego = _Vehicle(1, 0.0)
        target = _Vehicle(2, 20.0, y=10.0)
        plan = [(SimpleNamespace(transform=carla.Transform(carla.Location(30, 0, 0))),
                 RoadOption.LANEFOLLOW)]
        result = self._agent(ego, junction=True, plan=plan)._vehicle_obstacle_detected(
            [target], 50.0)
        self.assertFalse(result.obstacle_was_found)

    def test_junction_distance_is_between_actor_centers(self):
        ego = _Vehicle(1, 0.0)
        target = _Vehicle(2, 20.0)
        plan = [(SimpleNamespace(transform=carla.Transform(carla.Location(30, 0, 0))),
                 RoadOption.LANEFOLLOW)]
        result = self._agent(ego, junction=True, plan=plan)._vehicle_obstacle_detected(
            [target], 50.0)
        self.assertTrue(result.obstacle_was_found)
        self.assertAlmostEqual(result.distance, 20.0, places=5)

    def test_junction_route_polygon_starts_at_actor_center(self):
        ego = _Vehicle(1, 0.0)
        target = _Vehicle(2, 1.0, extent_x=0.25)
        plan = [(SimpleNamespace(transform=carla.Transform(carla.Location(30, 0, 0))),
                 RoadOption.LANEFOLLOW)]
        result = self._agent(ego, junction=True, plan=plan)._vehicle_obstacle_detected(
            [target], 50.0)
        self.assertTrue(result.obstacle_was_found)
        self.assertIs(result.obstacle, target)
        self.assertAlmostEqual(result.distance, 1.0, places=5)


if __name__ == "__main__":
    unittest.main()
