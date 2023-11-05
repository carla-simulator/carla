#!/usr/bin/env python

# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Vehicle Maneuvering In Opposite Direction:

Vehicle is passing another vehicle in a rural area, in daylight, under clear
weather conditions, at a non-junction and encroaches into another
vehicle traveling in the opposite direction.
"""

from six.moves.queue import Queue   # pylint: disable=relative-import,bad-option-value

import math  # pylint: disable=wrong-import-order
import py_trees
import carla

from srunner.scenariomanager.carla_data_provider import CarlaDataProvider
from srunner.scenariomanager.scenarioatomics.atomic_behaviors import (ActorTransformSetter,
                                                                      ActorDestroy,
                                                                      ActorSource,
                                                                      ActorSink,
                                                                      WaypointFollower)
from srunner.scenariomanager.scenarioatomics.atomic_criteria import CollisionTest
from srunner.scenariomanager.scenarioatomics.atomic_trigger_conditions import DriveDistance
from srunner.scenarios.basic_scenario import BasicScenario
from srunner.tools.scenario_helper import get_waypoint_in_distance


class ManeuverOppositeDirection(BasicScenario):

    """
    "Vehicle Maneuvering In Opposite Direction" (Traffic Scenario 06)

    This is a single ego vehicle scenario
    """

    def __init__(self, world, ego_vehicles, config, randomize=False, debug_mode=False, criteria_enable=True,
                 obstacle_type='barrier', timeout=120):
        """
        Setup all relevant parameters and create scenario
        obstacle_type -> flag to select type of leading obstacle. Values: vehicle, barrier
        """
        self._world = world
        self._map = CarlaDataProvider.get_map()
        self._first_vehicle_location = 50
        self._second_vehicle_location = self._first_vehicle_location + 60
        self._ego_vehicle_drive_distance = self._second_vehicle_location * 2
        self._start_distance = self._first_vehicle_location * 0.9
        self._opposite_speed = 5.56   # m/s
        self._source_gap = 40   # m
        self._reference_waypoint = self._map.get_waypoint(config.trigger_points[0].location)
        self._source_transform = None
        self._sink_location = None
        self._blackboard_queue_name = 'ManeuverOppositeDirection/actor_flow_queue'
        self._queue = py_trees.blackboard.Blackboard().set(self._blackboard_queue_name, Queue())
        self._obstacle_type = obstacle_type
        self._first_actor_transform = None
        self._second_actor_transform = None
        self._third_actor_transform = None
        # Timeout of scenario in seconds
        self.timeout = timeout

        super(ManeuverOppositeDirection, self).__init__(
            "ManeuverOppositeDirection",
            ego_vehicles,
            config,
            world,
            debug_mode,
            criteria_enable=criteria_enable)

    def _initialize_actors(self, config):
        """
        Custom initialization
        """
        first_actor_waypoint, _ = get_waypoint_in_distance(self._reference_waypoint, self._first_vehicle_location)
        second_actor_waypoint, _ = get_waypoint_in_distance(self._reference_waypoint, self._second_vehicle_location)
        second_actor_waypoint = second_actor_waypoint.get_left_lane()

        first_actor_transform = carla.Transform(
            first_actor_waypoint.transform.location,
            first_actor_waypoint.transform.rotation)
        if self._obstacle_type == 'vehicle':
            first_actor_model = 'vehicle.nissan.micra'
        else:
            first_actor_transform.rotation.yaw += 90
            first_actor_model = 'static.prop.streetbarrier'
            second_prop_waypoint = first_actor_waypoint.next(2.0)[0]
            position_yaw = second_prop_waypoint.transform.rotation.yaw + 90
            offset_location = carla.Location(
                0.50 * second_prop_waypoint.lane_width * math.cos(math.radians(position_yaw)),
                0.50 * second_prop_waypoint.lane_width * math.sin(math.radians(position_yaw)))
            second_prop_transform = carla.Transform(
                second_prop_waypoint.transform.location + offset_location, first_actor_transform.rotation)
            second_prop_actor = CarlaDataProvider.request_new_actor(first_actor_model, second_prop_transform)
            second_prop_actor.set_simulate_physics(True)
        first_actor = CarlaDataProvider.request_new_actor(first_actor_model, first_actor_transform)
        first_actor.set_simulate_physics(True)
        second_actor = CarlaDataProvider.request_new_actor('vehicle.audi.tt', second_actor_waypoint.transform)

        self.other_actors.append(first_actor)
        self.other_actors.append(second_actor)
        if self._obstacle_type != 'vehicle':
            self.other_actors.append(second_prop_actor)

        self._source_transform = second_actor_waypoint.transform
        sink_waypoint = second_actor_waypoint.next(1)[0]
        while not sink_waypoint.is_intersection:
            sink_waypoint = sink_waypoint.next(1)[0]
        self._sink_location = sink_waypoint.transform.location

        self._first_actor_transform = first_actor_transform
        self._second_actor_transform = second_actor_waypoint.transform
        self._third_actor_transform = second_prop_transform

    def _create_behavior(self):
        """
        The behavior tree returned by this method is as follows:
        The ego vehicle is trying to pass a leading vehicle in the same lane
        by moving onto the oncoming lane while another vehicle is moving in the
        opposite direction in the oncoming lane.
        """

        # Leaf nodes
        actor_source = ActorSource(
            ['vehicle.audi.tt', 'vehicle.tesla.model3', 'vehicle.nissan.micra'],
            self._source_transform, self._source_gap, self._blackboard_queue_name)
        actor_sink = ActorSink(self._sink_location, 10)
        ego_drive_distance = DriveDistance(self.ego_vehicles[0], self._ego_vehicle_drive_distance)
        waypoint_follower = WaypointFollower(
            self.other_actors[1], self._opposite_speed,
            blackboard_queue_name=self._blackboard_queue_name, avoid_collision=True)

        # Non-leaf nodes
        parallel_root = py_trees.composites.Parallel(policy=py_trees.common.ParallelPolicy.SUCCESS_ON_ONE)

        # Building tree
        parallel_root.add_child(ego_drive_distance)
        parallel_root.add_child(actor_source)
        parallel_root.add_child(actor_sink)
        parallel_root.add_child(waypoint_follower)

        scenario_sequence = py_trees.composites.Sequence()
        scenario_sequence.add_child(ActorTransformSetter(self.other_actors[0], self._first_actor_transform))
        scenario_sequence.add_child(ActorTransformSetter(self.other_actors[1], self._second_actor_transform))
        scenario_sequence.add_child(ActorTransformSetter(self.other_actors[2], self._third_actor_transform))
        scenario_sequence.add_child(parallel_root)
        scenario_sequence.add_child(ActorDestroy(self.other_actors[0]))
        scenario_sequence.add_child(ActorDestroy(self.other_actors[1]))
        scenario_sequence.add_child(ActorDestroy(self.other_actors[2]))

        return scenario_sequence

    def _create_test_criteria(self):
        """
        A list of all test criteria will be created that is later used
        in parallel behavior tree.
        """
        criteria = []

        collision_criterion = CollisionTest(self.ego_vehicles[0])
        criteria.append(collision_criterion)

        return criteria

    def __del__(self):
        """
        Remove all actors upon deletion
        """
        self.remove_all_actors()
