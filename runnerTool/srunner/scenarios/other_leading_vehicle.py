#!/usr/bin/env python

#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Other Leading Vehicle scenario:

The scenario realizes a common driving behavior, in which the
user-controlled ego vehicle follows a leading car driving down
a given road. At some point the leading car has to decelerate.
The ego vehicle has to react accordingly by changing lane to avoid a
collision and follow the leading car in other lane. The scenario ends
either via a timeout, or if the ego vehicle drives some distance.
"""

import py_trees

import carla

from srunner.scenariomanager.carla_data_provider import CarlaDataProvider
from srunner.scenariomanager.scenarioatomics.atomic_behaviors import (ActorTransformSetter,
                                                                      WaypointFollower,
                                                                      ActorDestroy)
from srunner.scenariomanager.scenarioatomics.atomic_criteria import CollisionTest
from srunner.scenariomanager.scenarioatomics.atomic_trigger_conditions import (InTriggerDistanceToVehicle,
                                                                               DriveDistance)
from srunner.scenarios.basic_scenario import BasicScenario
from srunner.tools.scenario_helper import get_waypoint_in_distance


class OtherLeadingVehicle(BasicScenario):

    """
    This class holds everything required for a simple "Other Leading Vehicle"
    scenario involving a user controlled vehicle and two other actors.
    Traffic Scenario 05

    This is a single ego vehicle scenario
    """

    def __init__(self, world, ego_vehicles, config, randomize=False, debug_mode=False, criteria_enable=True,
                 timeout=80):
        """
        Setup all relevant parameters and create scenario
        """
        self._world = world
        self._map = CarlaDataProvider.get_map()
        self._first_vehicle_location = 35
        self._second_vehicle_location = self._first_vehicle_location + 1
        self._ego_vehicle_drive_distance = self._first_vehicle_location * 4
        self._first_vehicle_speed = 55 / 3.6
        self._second_vehicle_speed = 45 / 3.6
        self._reference_waypoint = self._map.get_waypoint(config.trigger_points[0].location)
        self._other_actor_max_brake = 1.0
        self._first_actor_transform = None
        self._second_actor_transform = None
        # Timeout of scenario in seconds
        self.timeout = timeout

        super(OtherLeadingVehicle, self).__init__("VehicleDeceleratingInMultiLaneSetUp",
                                                  ego_vehicles,
                                                  config,
                                                  world,
                                                  debug_mode,
                                                  criteria_enable=criteria_enable)

    def _initialize_actors(self, config):
        """
        Custom initialization
        """
        first_vehicle_waypoint, _ = get_waypoint_in_distance(self._reference_waypoint, self._first_vehicle_location)
        second_vehicle_waypoint, _ = get_waypoint_in_distance(self._reference_waypoint, self._second_vehicle_location)
        second_vehicle_waypoint = second_vehicle_waypoint.get_left_lane()

        first_vehicle_transform = carla.Transform(first_vehicle_waypoint.transform.location,
                                                  first_vehicle_waypoint.transform.rotation)
        second_vehicle_transform = carla.Transform(second_vehicle_waypoint.transform.location,
                                                   second_vehicle_waypoint.transform.rotation)

        first_vehicle = CarlaDataProvider.request_new_actor('vehicle.nissan.patrol', first_vehicle_transform)
        second_vehicle = CarlaDataProvider.request_new_actor('vehicle.audi.tt', second_vehicle_transform)

        self.other_actors.append(first_vehicle)
        self.other_actors.append(second_vehicle)

        self._first_actor_transform = first_vehicle_transform
        self._second_actor_transform = second_vehicle_transform

    def _create_behavior(self):
        """
        The scenario defined after is a "other leading vehicle" scenario. After
        invoking this scenario, the user controlled vehicle has to drive towards the
        moving other actors, then make the leading actor to decelerate when user controlled
        vehicle is at some close distance. Finally, the user-controlled vehicle has to change
        lane to avoid collision and follow other leading actor in other lane to end the scenario.
        If this does not happen within 90 seconds, a timeout stops the scenario or the ego vehicle
        drives certain distance and stops the scenario.
        """
        # start condition
        driving_in_same_direction = py_trees.composites.Parallel("All actors driving in same direction",
                                                                 policy=py_trees.common.ParallelPolicy.SUCCESS_ON_ONE)
        leading_actor_sequence_behavior = py_trees.composites.Sequence("Decelerating actor sequence behavior")

        # both actors moving in same direction
        keep_velocity = py_trees.composites.Parallel("Trigger condition for deceleration",
                                                     policy=py_trees.common.ParallelPolicy.SUCCESS_ON_ONE)
        keep_velocity.add_child(WaypointFollower(self.other_actors[0], self._first_vehicle_speed, avoid_collision=True))
        keep_velocity.add_child(InTriggerDistanceToVehicle(self.other_actors[0], self.ego_vehicles[0], 55))

        # Decelerating actor sequence behavior
        decelerate = self._first_vehicle_speed / 3.2
        leading_actor_sequence_behavior.add_child(keep_velocity)
        leading_actor_sequence_behavior.add_child(WaypointFollower(self.other_actors[0], decelerate,
                                                                   avoid_collision=True))
        # end condition
        ego_drive_distance = DriveDistance(self.ego_vehicles[0], self._ego_vehicle_drive_distance)

        # Build behavior tree
        sequence = py_trees.composites.Sequence("Scenario behavior")
        parallel_root = py_trees.composites.Parallel(policy=py_trees.common.ParallelPolicy.SUCCESS_ON_ONE)

        parallel_root.add_child(ego_drive_distance)
        parallel_root.add_child(driving_in_same_direction)
        driving_in_same_direction.add_child(leading_actor_sequence_behavior)
        driving_in_same_direction.add_child(WaypointFollower(self.other_actors[1], self._second_vehicle_speed,
                                                             avoid_collision=True))

        sequence.add_child(ActorTransformSetter(self.other_actors[0], self._first_actor_transform))
        sequence.add_child(ActorTransformSetter(self.other_actors[1], self._second_actor_transform))
        sequence.add_child(parallel_root)
        sequence.add_child(ActorDestroy(self.other_actors[0]))
        sequence.add_child(ActorDestroy(self.other_actors[1]))

        return sequence

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
        self.remove_all_actors()
