#!/usr/bin/env python

# Copyright (c) 2018-2020 Intel Corporation
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
All intersection related scenarios that are part of a route.
"""

from __future__ import print_function

import py_trees

from srunner.scenariomanager.scenarioatomics.atomic_behaviors import TrafficLightManipulator

from srunner.scenariomanager.scenarioatomics.atomic_criteria import CollisionTest, DrivenDistanceTest, MaxVelocityTest
from srunner.scenariomanager.scenarioatomics.atomic_trigger_conditions import DriveDistance, WaitEndIntersection
from srunner.scenarios.basic_scenario import BasicScenario


class SignalJunctionCrossingRoute(BasicScenario):

    """
    At routes, these scenarios are simplified, as they can be triggered making
    use of the background activity. To ensure interactions with this background
    activity, the traffic lights are modified, setting two of them to green
    """

    # ego vehicle parameters
    _ego_max_velocity_allowed = 20       # Maximum allowed velocity [m/s]
    _ego_expected_driven_distance = 50   # Expected driven distance [m]
    _ego_distance_to_drive = 20          # Allowed distance to drive

    _traffic_light = None

    # Depending on the route, decide which traffic lights can be modified

    def __init__(self, world, ego_vehicles, config, randomize=False, debug_mode=False, criteria_enable=True,
                 timeout=180):
        """
        Setup all relevant parameters and create scenario
        and instantiate scenario manager
        """
        # Timeout of scenario in seconds
        self.timeout = timeout
        self.subtype = config.subtype

        super(SignalJunctionCrossingRoute, self).__init__("SignalJunctionCrossingRoute",
                                                          ego_vehicles,
                                                          config,
                                                          world,
                                                          debug_mode,
                                                          criteria_enable=criteria_enable)

    def _initialize_actors(self, config):
        """
        Custom initialization
        """

    def _create_behavior(self):
        """
        Scenario behavior:
        When close to an intersection, the traffic lights will turn green for
        both the ego_vehicle and another lane, allowing the background activity
        to "run" their red light, creating scenarios 7, 8 and 9.

        If this does not happen within 120 seconds, a timeout stops the scenario
        """

        # Changes traffic lights
        traffic_hack = TrafficLightManipulator(self.ego_vehicles[0], self.subtype)

        # finally wait that ego vehicle drove a specific distance
        wait = DriveDistance(
            self.ego_vehicles[0],
            self._ego_distance_to_drive,
            name="DriveDistance")

        # Build behavior tree
        sequence = py_trees.composites.Sequence("SignalJunctionCrossingRoute")
        sequence.add_child(traffic_hack)
        sequence.add_child(wait)

        return sequence

    def _create_test_criteria(self):
        """
        A list of all test criteria will be created that is later used
        in parallel behavior tree.
        """
        criteria = []

        max_velocity_criterion = MaxVelocityTest(
            self.ego_vehicles[0],
            self._ego_max_velocity_allowed,
            optional=True)
        collision_criterion = CollisionTest(self.ego_vehicles[0])
        driven_distance_criterion = DrivenDistanceTest(
            self.ego_vehicles[0],
            self._ego_expected_driven_distance)

        criteria.append(max_velocity_criterion)
        criteria.append(collision_criterion)
        criteria.append(driven_distance_criterion)

        return criteria

    def __del__(self):
        """
        Remove all actors and traffic lights upon deletion
        """
        self._traffic_light = None
        self.remove_all_actors()


class NoSignalJunctionCrossingRoute(BasicScenario):

    """
    At routes, these scenarios are simplified, as they can be triggered making
    use of the background activity. For unsignalized intersections, just wait
    until the ego_vehicle has left the intersection.
    """

    # ego vehicle parameters
    _ego_max_velocity_allowed = 20       # Maximum allowed velocity [m/s]
    _ego_expected_driven_distance = 50   # Expected driven distance [m]
    _ego_distance_to_drive = 20          # Allowed distance to drive

    def __init__(self, world, ego_vehicles, config, randomize=False, debug_mode=False, criteria_enable=True,
                 timeout=180):
        """
        Setup all relevant parameters and create scenario
        and instantiate scenario manager
        """
        # Timeout of scenario in seconds
        self.timeout = timeout

        super(NoSignalJunctionCrossingRoute, self).__init__("NoSignalJunctionCrossingRoute",
                                                            ego_vehicles,
                                                            config,
                                                            world,
                                                            debug_mode,
                                                            criteria_enable=criteria_enable)

    def _initialize_actors(self, config):
        """
        Custom initialization
        """
        pass

    def _create_behavior(self):
        """
        Scenario behavior:
        When close to an intersection, the traffic lights will turn green for
        both the ego_vehicle and another lane, allowing the background activity
        to "run" their red light.

        If this does not happen within 120 seconds, a timeout stops the scenario
        """
        # finally wait that ego vehicle drove a specific distance
        wait = WaitEndIntersection(
            self.ego_vehicles[0],
            name="WaitEndIntersection")
        end_condition = DriveDistance(
            self.ego_vehicles[0],
            self._ego_distance_to_drive,
            name="DriveDistance")

        # Build behavior tree
        sequence = py_trees.composites.Sequence("NoSignalJunctionCrossingRoute")
        sequence.add_child(wait)
        sequence.add_child(end_condition)

        return sequence

    def _create_test_criteria(self):
        """
        A list of all test criteria will be created that is later used
        in parallel behavior tree.
        """
        criteria = []
        criteria.append(CollisionTest(self.ego_vehicles[0]))
        return criteria

    def __del__(self):
        """
        Remove all actors and traffic lights upon deletion
        """
        self.remove_all_actors()
