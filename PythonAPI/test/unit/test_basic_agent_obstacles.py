# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Offline obstacle detection checks using real CARLA geometry and inert actors."""

# These tests exercise the existing detector entry point and its internal inputs.
# pylint: disable=protected-access

import math
import os
import sys
import unittest
from types import SimpleNamespace

import carla

sys.path.insert(0, os.path.abspath(os.path.join(
    os.path.dirname(__file__), "..", "..", "carla")))

from agents.navigation.basic_agent import BasicAgent  # pylint: disable=wrong-import-position
from agents.navigation.local_planner import RoadOption  # pylint: disable=wrong-import-position


class _Vehicle:
    """Inert actor RPC endpoint that returns native CARLA geometry."""

    def __init__(self, actor_id, x, y=0.0, yaw=0.0, extent_x=2.0):
        self.id = actor_id
        self._location = carla.Location(x, y, 0.0)
        self._rotation = carla.Rotation(yaw=yaw)
        self.bounding_box = carla.BoundingBox(
            carla.Location(), carla.Vector3D(extent_x, 1.0, 1.0))
        self.returned_transforms = []

    def get_transform(self):
        """Return an independent native transform, as the real API does."""
        transform = carla.Transform(self._location, self._rotation)
        self.returned_transforms.append(transform)
        return transform

    def get_location(self):
        """Return the actor center without a bumper adjustment."""
        return carla.Location(self._location)


class TestBasicAgentObstacles(unittest.TestCase):
    """Check footprint intersection and center distances in the real detector."""

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
        """Different bumper lengths must not alter the reported center distance."""
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
        """A rotated bumper offset must preserve the center distance."""
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
        """Target yaw must only affect its bumper bearing."""
        ego = _Vehicle(1, 0.0)
        target = _Vehicle(2, 20.0, yaw=90.0)
        result = self._agent(ego)._vehicle_obstacle_detected([target], 50.0)
        self.assertTrue(result.obstacle_was_found)
        self.assertAlmostEqual(result.distance, 20.0, places=5)

    def test_angle_filter_still_uses_front_and_rear_bumpers(self):
        """The bearing filter continues to compare the adjusted bumpers."""
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
        """The snapshots retained for geometry stay centered on their actors."""
        # Actor.get_transform() already returns a local snapshot. Keep that
        # snapshot centered for later calculations; no server actor is changed.
        ego = _Vehicle(1, 0.0)
        target = _Vehicle(2, 20.0)
        self._agent(ego)._vehicle_obstacle_detected(
            [target], 50.0, low_angle_th=-1.0)
        self.assertEqual(ego.returned_transforms[0].location, ego.get_location())
        self.assertEqual(target.returned_transforms[0].location, target.get_location())

    def test_distance_prefilter_uses_actor_centers(self):
        """An actor beyond the radius is excluded before footprint checks."""
        ego = _Vehicle(1, 0.0)
        target = _Vehicle(2, 21.0)
        result = self._agent(ego)._vehicle_obstacle_detected(
            [target], 20.0, low_angle_th=-1.0)
        self.assertFalse(result.obstacle_was_found)

    def test_vehicle_behind_ego_is_not_detected_as_ahead(self):
        """A following actor must not pass the forward bearing filter."""
        ego = _Vehicle(1, 0.0)
        target = _Vehicle(2, -20.0)
        result = self._agent(ego)._vehicle_obstacle_detected(
            [target], 50.0, low_angle_th=-1.0)
        self.assertFalse(result.obstacle_was_found)

    def test_junction_vehicle_outside_route_polygon_is_not_detected(self):
        """A side actor outside the route must not trigger a junction hazard."""
        ego = _Vehicle(1, 0.0)
        target = _Vehicle(2, 20.0, y=10.0)
        plan = [(SimpleNamespace(transform=carla.Transform(carla.Location(30, 0, 0))),
                 RoadOption.LANEFOLLOW)]
        result = self._agent(ego, junction=True, plan=plan)._vehicle_obstacle_detected(
            [target], 50.0)
        self.assertFalse(result.obstacle_was_found)

    def test_junction_distance_is_between_actor_centers(self):
        """Footprint detection reports the same center distance as bearing checks."""
        ego = _Vehicle(1, 0.0)
        target = _Vehicle(2, 20.0)
        plan = [(SimpleNamespace(transform=carla.Transform(carla.Location(30, 0, 0))),
                 RoadOption.LANEFOLLOW)]
        result = self._agent(ego, junction=True, plan=plan)._vehicle_obstacle_detected(
            [target], 50.0)
        self.assertTrue(result.obstacle_was_found)
        self.assertAlmostEqual(result.distance, 20.0, places=5)

    def test_junction_route_polygon_starts_at_actor_center(self):
        """The route contains hazards between the actor center and its bumper."""
        ego = _Vehicle(1, 0.0)
        target = _Vehicle(2, 1.0, extent_x=0.25)
        plan = [(SimpleNamespace(transform=carla.Transform(carla.Location(30, 0, 0))),
                 RoadOption.LANEFOLLOW)]
        result = self._agent(ego, junction=True, plan=plan)._vehicle_obstacle_detected(
            [target], 50.0)
        self.assertTrue(result.obstacle_was_found)
        self.assertIs(result.obstacle, target)
        self.assertAlmostEqual(result.distance, 1.0, places=5)

    def test_route_corridor_contains_small_obstacles_between_samples(self):
        """Both sides of the route remain covered between waypoint samples."""
        for stations in [(30,), (5, 10, 15, 20, 25, 30)]:
            for lateral in (-0.8, 0.8):
                with self.subTest(stations=stations, lateral=lateral):
                    ego = _Vehicle(1, 0.0)
                    target = _Vehicle(2, 7.5, y=lateral, extent_x=0.5)
                    target.bounding_box.extent.y = 0.15
                    plan = [(SimpleNamespace(transform=carla.Transform(
                        carla.Location(station, 0, 0))), RoadOption.LANEFOLLOW)
                        for station in stations]
                    agent = self._agent(ego, junction=True, plan=plan)
                    self.assertTrue(agent._vehicle_obstacle_detected(
                        [target], 50.0).obstacle_was_found)

    def test_route_corridor_excludes_side_obstacles(self):
        """Correcting the perimeter does not widen the detection corridor."""
        ego = _Vehicle(1, 0.0)
        target = _Vehicle(2, 7.5, y=2.0, extent_x=0.5)
        target.bounding_box.extent.y = 0.15
        plan = [(SimpleNamespace(transform=carla.Transform(
            carla.Location(30, 0, 0))), RoadOption.LANEFOLLOW)]
        self.assertFalse(self._agent(ego, junction=True, plan=plan)
                         ._vehicle_obstacle_detected([target], 50.0)
                         .obstacle_was_found)

    def test_projected_box_contains_its_interior(self):
        """Projection must preserve the interior of a target bounding box."""
        ego = _Vehicle(1, 0.0)
        ego.bounding_box.extent.y = 0.1
        target = _Vehicle(2, 7.5, y=0.5, extent_x=3.0)
        plan = [(SimpleNamespace(transform=carla.Transform(
            carla.Location(7.5, 0, 0))), RoadOption.LANEFOLLOW)]
        self.assertTrue(self._agent(ego, junction=True, plan=plan)
                        ._vehicle_obstacle_detected([target], 50.0)
                        .obstacle_was_found)


if __name__ == "__main__":
    unittest.main()
