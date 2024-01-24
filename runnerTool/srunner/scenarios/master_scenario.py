#!/usr/bin/env python

#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Basic CARLA Autonomous Driving training scenario
"""

import py_trees

from srunner.scenarioconfigs.route_scenario_configuration import RouteConfiguration
from srunner.scenariomanager.scenarioatomics.atomic_behaviors import Idle
from srunner.scenariomanager.scenarioatomics.atomic_criteria import (CollisionTest,
                                                                     InRouteTest,
                                                                     RouteCompletionTest,
                                                                     OutsideRouteLanesTest,
                                                                     RunningRedLightTest,
                                                                     RunningStopTest,
                                                                     ActorSpeedAboveThresholdTest)
from srunner.scenarios.basic_scenario import BasicScenario


class MasterScenario(BasicScenario):

    """
    Implementation of a  Master scenario that controls the route.

    This is a single ego vehicle scenario
    """

    radius = 10.0           # meters

    def __init__(self, world, ego_vehicles, config, randomize=False, debug_mode=False, criteria_enable=True,
                 timeout=300):
        """
        Setup all relevant parameters and create scenario
        """
        self.config = config
        self.route = None
        # Timeout of scenario in seconds
        self.timeout = timeout

        if hasattr(self.config, 'route'):
            self.route = self.config.route
        else:
            raise ValueError("Master scenario must have a route")

        super(MasterScenario, self).__init__("MasterScenario", ego_vehicles=ego_vehicles, config=config,
                                             world=world, debug_mode=debug_mode,
                                             terminate_on_failure=True, criteria_enable=criteria_enable)

    def _create_behavior(self):
        """
        Basic behavior do nothing, i.e. Idle
        """

        # Build behavior tree
        sequence = py_trees.composites.Sequence("MasterScenario")
        idle_behavior = Idle()
        sequence.add_child(idle_behavior)

        return sequence

    def _create_test_criteria(self):
        """
        A list of all test criteria will be created that is later used
        in parallel behavior tree.
        """

        if isinstance(self.route, RouteConfiguration):
            route = self.route.data
        else:
            route = self.route

        collision_criterion = CollisionTest(self.ego_vehicles[0], terminate_on_failure=False)

        route_criterion = InRouteTest(self.ego_vehicles[0],
                                      route=route,
                                      offroad_max=30,
                                      terminate_on_failure=True)

        completion_criterion = RouteCompletionTest(self.ego_vehicles[0], route=route)

        outsidelane_criterion = OutsideRouteLanesTest(self.ego_vehicles[0], route=route)

        red_light_criterion = RunningRedLightTest(self.ego_vehicles[0])

        stop_criterion = RunningStopTest(self.ego_vehicles[0])

        blocked_criterion = ActorSpeedAboveThresholdTest(self.ego_vehicles[0],
                                                         speed_threshold=0.1,
                                                         below_threshold_max_time=90.0,
                                                         terminate_on_failure=True)

        parallel_criteria = py_trees.composites.Parallel("group_criteria",
                                                         policy=py_trees.common.ParallelPolicy.SUCCESS_ON_ONE)

        parallel_criteria.add_child(completion_criterion)
        parallel_criteria.add_child(collision_criterion)
        parallel_criteria.add_child(route_criterion)
        parallel_criteria.add_child(outsidelane_criterion)
        parallel_criteria.add_child(red_light_criterion)
        parallel_criteria.add_child(stop_criterion)
        parallel_criteria.add_child(blocked_criterion)

        return parallel_criteria

    def __del__(self):
        """
        Remove all actors upon deletion
        """
        self.remove_all_actors()
