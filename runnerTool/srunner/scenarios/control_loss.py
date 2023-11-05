#!/usr/bin/env python

#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Control Loss Vehicle scenario:

The scenario realizes that the vehicle looses control due to
bad road conditions, etc. and checks to see if the vehicle
regains control and corrects it's course.
"""

from numpy import random
import py_trees
import carla

from srunner.scenariomanager.carla_data_provider import CarlaDataProvider
from srunner.scenariomanager.scenarioatomics.atomic_criteria import CollisionTest
from srunner.scenariomanager.scenarioatomics.atomic_trigger_conditions import DriveDistance
from srunner.scenarios.basic_scenario import BasicScenario
from srunner.tools.scenario_helper import get_waypoint_in_distance


class ControlLoss(BasicScenario):

    """
    Implementation of "Control Loss Vehicle" (Traffic Scenario 01)

    This is a single ego vehicle scenario
    """

    def __init__(self, world, ego_vehicles, config, randomize=False, debug_mode=False, criteria_enable=True,
                 timeout=60):
        """
        Setup all relevant parameters and create scenario
        """
        self.timeout = timeout
        self._randomize = randomize

        self._map = CarlaDataProvider.get_map()
        self._end_distance = 110

        super(ControlLoss, self).__init__("ControlLoss",
                                          ego_vehicles,
                                          config,
                                          world,
                                          debug_mode,
                                          criteria_enable=criteria_enable)

    def _initialize_actors(self, config):
        """
        Custom initialization
        """
        if self._randomize:
            self._distance = random.randint(low=10, high=80, size=3)
            self._distance = sorted(self._distance)
        else:
            self._distance = [14, 48, 74]

        self._reference_waypoint = self._map.get_waypoint(config.trigger_points[0].location)

        # Get the debris locations
        first_wp, _ = get_waypoint_in_distance(self._reference_waypoint, self._distance[0])
        first_ground_loc = self.world.ground_projection(first_wp.transform.location, 2)
        self.first_loc_prev = first_ground_loc.location if first_ground_loc else first_wp.transform.location

        second_wp, _ = get_waypoint_in_distance(self._reference_waypoint, self._distance[1])
        second_ground_loc = self.world.ground_projection(second_wp.transform.location, 2)
        self.second_loc_prev = second_ground_loc.location if second_ground_loc else second_wp.transform.location

        third_wp, _ = get_waypoint_in_distance(self._reference_waypoint, self._distance[2])
        third_ground_loc = self.world.ground_projection(third_wp.transform.location, 2)
        self.third_loc_prev = third_ground_loc.location if third_ground_loc else third_wp.transform.location

        # Get the debris transforms
        self.first_transform = carla.Transform(self.first_loc_prev, first_wp.transform.rotation)
        self.second_transform = carla.Transform(self.second_loc_prev, second_wp.transform.rotation)
        self.third_transform = carla.Transform(self.third_loc_prev, third_wp.transform.rotation)

        # Spawn the debris
        first_debris = CarlaDataProvider.request_new_actor(
            'static.prop.dirtdebris01', self.first_transform, rolename='prop')
        second_debris = CarlaDataProvider.request_new_actor(
            'static.prop.dirtdebris01', self.second_transform, rolename='prop')
        third_debris = CarlaDataProvider.request_new_actor(
            'static.prop.dirtdebris01', self.third_transform, rolename='prop')

        first_debris.set_transform(self.first_transform)
        second_debris.set_transform(self.second_transform)
        third_debris.set_transform(self.third_transform)

        # Remove their physics
        first_debris.set_simulate_physics(False)
        second_debris.set_simulate_physics(False)
        third_debris.set_simulate_physics(False)

        self.other_actors.append(first_debris)
        self.other_actors.append(second_debris)
        self.other_actors.append(third_debris)

    def _create_behavior(self):
        """
        The scenario defined after is a "control loss vehicle" scenario.
        """
        sequence = py_trees.composites.Sequence("ControlLoss")
        sequence.add_child(DriveDistance(self.ego_vehicles[0], self._end_distance))
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
        Remove all actors upon deletion
        """
        self.remove_all_actors()
