#!/usr/bin/env python

# Copyright (c) 2019-2020 Intel Corporation
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Change lane scenario:

The scenario realizes a driving behavior, in which the user-controlled ego vehicle
follows a fast driving car on the highway. There's a slow car driving in great distance to the fast vehicle.
At one point the fast vehicle is changing the lane to overtake a slow car, which is driving on the same lane.

The ego vehicle doesn't "see" the slow car before the lane change of the fast car, therefore it hast to react
fast to avoid an collision. There are two options to avoid an accident:
The ego vehicle adjusts its velocity or changes the lane as well.
"""

import random
import py_trees
import carla

from srunner.scenariomanager.carla_data_provider import CarlaDataProvider
from srunner.scenariomanager.scenarioatomics.atomic_behaviors import (ActorTransformSetter,
                                                                      StopVehicle,
                                                                      LaneChange,
                                                                      WaypointFollower,
                                                                      Idle)
from srunner.scenariomanager.scenarioatomics.atomic_criteria import CollisionTest
from srunner.scenariomanager.scenarioatomics.atomic_trigger_conditions import InTriggerDistanceToVehicle, StandStill
from srunner.scenarios.basic_scenario import BasicScenario
from srunner.tools.scenario_helper import get_waypoint_in_distance


class ChangeLane(BasicScenario):

    """
    This class holds everything required for a "change lane" scenario involving three vehicles.
    There are two vehicles driving in the same direction on the highway: A fast car and a slow car in front.
    The fast car will change the lane, when it is close to the slow car.

    The ego vehicle is driving right behind the fast car.

    This is a single ego vehicle scenario
    """

    timeout = 1200

    def __init__(self, world, ego_vehicles, config, randomize=False, debug_mode=False, criteria_enable=True,
                 timeout=600):
        """
        Setup all relevant parameters and create scenario

        If randomize is True, the scenario parameters are randomized
        """
        self.timeout = timeout
        self._map = CarlaDataProvider.get_map()
        self._reference_waypoint = self._map.get_waypoint(config.trigger_points[0].location)

        self._fast_vehicle_velocity = 70
        self._slow_vehicle_velocity = 0
        self._change_lane_velocity = 15

        self._slow_vehicle_distance = 100
        self._fast_vehicle_distance = 20
        self._trigger_distance = 30
        self._max_brake = 1

        self.direction = 'left'  # direction of lane change
        self.lane_check = 'true'  # check whether a lane change is possible

        super(ChangeLane, self).__init__("ChangeLane",
                                         ego_vehicles,
                                         config,
                                         world,
                                         debug_mode,
                                         criteria_enable=criteria_enable)

        if randomize:
            self._fast_vehicle_distance = random.randint(10, 51)
            self._fast_vehicle_velocity = random.randint(100, 201)
            self._slow_vehicle_velocity = random.randint(1, 6)

    def _initialize_actors(self, config):

        # add actors from xml file
        for actor in config.other_actors:
            vehicle = CarlaDataProvider.request_new_actor(actor.model, actor.transform)
            self.other_actors.append(vehicle)
            vehicle.set_simulate_physics(enabled=False)

        # fast vehicle, tesla
        # transform visible
        fast_car_waypoint, _ = get_waypoint_in_distance(self._reference_waypoint, self._fast_vehicle_distance)
        self.fast_car_visible = carla.Transform(
            carla.Location(fast_car_waypoint.transform.location.x,
                           fast_car_waypoint.transform.location.y,
                           fast_car_waypoint.transform.location.z + 1),
            fast_car_waypoint.transform.rotation)

        # slow vehicle, vw
        # transform visible
        slow_car_waypoint, _ = get_waypoint_in_distance(self._reference_waypoint, self._slow_vehicle_distance)
        self.slow_car_visible = carla.Transform(
            carla.Location(slow_car_waypoint.transform.location.x,
                           slow_car_waypoint.transform.location.y,
                           slow_car_waypoint.transform.location.z),
            slow_car_waypoint.transform.rotation)

    def _create_behavior(self):

        # sequence vw
        # make visible
        sequence_vw = py_trees.composites.Sequence("VW T2")
        vw_visible = ActorTransformSetter(self.other_actors[1], self.slow_car_visible)
        sequence_vw.add_child(vw_visible)

        # brake, avoid rolling backwarts
        brake = StopVehicle(self.other_actors[1], self._max_brake)
        sequence_vw.add_child(brake)
        sequence_vw.add_child(Idle())

        # sequence tesla
        # make visible
        sequence_tesla = py_trees.composites.Sequence("Tesla")
        tesla_visible = ActorTransformSetter(self.other_actors[0], self.fast_car_visible)
        sequence_tesla.add_child(tesla_visible)

        # drive fast towards slow vehicle
        just_drive = py_trees.composites.Parallel("DrivingTowardsSlowVehicle",
                                                  policy=py_trees.common.ParallelPolicy.SUCCESS_ON_ONE)
        tesla_driving_fast = WaypointFollower(self.other_actors[0], self._fast_vehicle_velocity)
        just_drive.add_child(tesla_driving_fast)
        distance_to_vehicle = InTriggerDistanceToVehicle(
            self.other_actors[1], self.other_actors[0], self._trigger_distance)
        just_drive.add_child(distance_to_vehicle)
        sequence_tesla.add_child(just_drive)

        # change lane
        lane_change_atomic = LaneChange(self.other_actors[0], distance_other_lane=200)
        sequence_tesla.add_child(lane_change_atomic)
        sequence_tesla.add_child(Idle())

        # ego vehicle
        # end condition
        endcondition = py_trees.composites.Parallel("Waiting for end position of ego vehicle",
                                                    policy=py_trees.common.ParallelPolicy.SUCCESS_ON_ALL)
        endcondition_part1 = InTriggerDistanceToVehicle(self.other_actors[1],
                                                        self.ego_vehicles[0],
                                                        distance=20,
                                                        name="FinalDistance")
        endcondition_part2 = StandStill(self.ego_vehicles[0], name="FinalSpeed", duration=1)
        endcondition.add_child(endcondition_part1)
        endcondition.add_child(endcondition_part2)

        # build tree
        root = py_trees.composites.Parallel("Parallel Behavior", policy=py_trees.common.ParallelPolicy.SUCCESS_ON_ONE)
        root.add_child(sequence_vw)
        root.add_child(sequence_tesla)
        root.add_child(endcondition)
        return root

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
