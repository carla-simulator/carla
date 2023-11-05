#!/usr/bin/env python

#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Collection of traffic scenarios where the ego vehicle (hero)
is making a right turn
"""

from __future__ import print_function

import py_trees

import carla
from agents.navigation.global_route_planner import GlobalRoutePlanner

from srunner.scenariomanager.carla_data_provider import CarlaDataProvider
from srunner.scenariomanager.scenarioatomics.atomic_behaviors import ActorFlow, TrafficLightFreezer
from srunner.scenariomanager.scenarioatomics.atomic_criteria import CollisionTest
from srunner.scenariomanager.scenarioatomics.atomic_trigger_conditions import WaitEndIntersection
from srunner.scenarios.basic_scenario import BasicScenario
from srunner.tools.scenario_helper import (generate_target_waypoint,
                                           get_junction_topology,
                                           filter_junction_wp_direction,
                                           get_closest_traffic_light)

from srunner.tools.background_manager import Scenario9Manager


class SignalizedJunctionRightTurn(BasicScenario):

    """
    Scenario where the vehicle is turning right at an intersection an has to avoid
    colliding with a vehicle coming from its left
    """

    def __init__(self, world, ego_vehicles, config, randomize=False, debug_mode=False, criteria_enable=True,
                 timeout=80):
        """
        Setup all relevant parameters and create scenario
        """
        self._world = world
        self._map = CarlaDataProvider.get_map()
        self._source_dist = 40
        self._sink_dist = 10
        self._source_dist_interval = [25, 50]
        self._opposite_speed = 35 / 3.6
        self._green_light_delay = 5  # Wait before the ego's lane traffic light turns green
        self._direction = 'left'
        self._route_planner = GlobalRoutePlanner(self._map, 2.0)
        self.timeout = timeout
        super(SignalizedJunctionRightTurn, self).__init__("SignalizedJunctionRightTurn",
                                                          ego_vehicles,
                                                          config,
                                                          world,
                                                          debug_mode,
                                                          criteria_enable=criteria_enable)

    def _initialize_actors(self, config):
        """
        Custom initialization
        """
        ego_location = config.trigger_points[0].location
        ego_wp = CarlaDataProvider.get_map().get_waypoint(ego_location)

        # Get the junction
        starting_wp = ego_wp
        while not starting_wp.is_junction:
            starting_wps = starting_wp.next(1.0)
            if len(starting_wps) == 0:
                raise ValueError("Failed to find junction as a waypoint with no next was detected")
            starting_wp = starting_wps[0]
        junction = starting_wp.get_junction()

        # Get the source entry lane wp
        entry_wps, _ = get_junction_topology(junction)
        source_entry_wps = filter_junction_wp_direction(starting_wp, entry_wps, self._direction)
        if not source_entry_wps:
            raise ValueError("Trying to find a lane in the {} direction but none was found".format(self._direction))

        # Get the rightmost lane
        source_entry_wp = source_entry_wps[0]
        while True:
            right_wp = source_entry_wp.get_right_lane()
            if not right_wp or right_wp.lane_type != carla.LaneType.Driving:
                break
            source_entry_wp = right_wp

        # Get the source transform
        source_wp = source_entry_wp
        accum_dist = 0
        while accum_dist < self._source_dist:
            source_wps = source_wp.previous(5)
            if len(source_wps) == 0:
                raise ValueError("Failed to find a source location as a waypoint with no previous was detected")
            if source_wps[0].is_junction:
                break
            source_wp = source_wps[0]
            accum_dist += 5

        self._source_wp = source_wp
        source_transform = self._source_wp.transform

        # Get the sink location
        sink_exit_wp = generate_target_waypoint(self._map.get_waypoint(source_transform.location), 0)
        sink_wps = sink_exit_wp.next(self._sink_dist)
        if len(sink_wps) == 0:
            raise ValueError("Failed to find a sink location as a waypoint with no next was detected")
        self._sink_wp = sink_wps[0]

        # Get the relevant traffic lights
        tls = self._world.get_traffic_lights_in_junction(junction.id)
        ego_tl = get_closest_traffic_light(ego_wp, tls)
        source_tl = get_closest_traffic_light(source_wps[0], tls)
        self._flow_tl_dict = {}
        self._init_tl_dict = {}
        for tl in tls:
            if tl == ego_tl:
                self._flow_tl_dict[tl] = carla.TrafficLightState.Green
                self._init_tl_dict[tl] = carla.TrafficLightState.Red
            elif tl == source_tl:
                self._flow_tl_dict[tl] = carla.TrafficLightState.Green
                self._init_tl_dict[tl] = carla.TrafficLightState.Green
            else:
                self._flow_tl_dict[tl] = carla.TrafficLightState.Red
                self._init_tl_dict[tl] = carla.TrafficLightState.Red

    def _create_behavior(self):
        """
        Hero vehicle is turning right in an urban area, at a signalized intersection,
        while other actor coming straight from the left. The ego has to avoid colliding with it
        """
        root = py_trees.composites.Parallel(policy=py_trees.common.ParallelPolicy.SUCCESS_ON_ONE)
        root.add_child(WaitEndIntersection(self.ego_vehicles[0]))
        root.add_child(ActorFlow(
            self._source_wp, self._sink_wp, self._source_dist_interval, 2, self._opposite_speed))

        tl_freezer_sequence = py_trees.composites.Sequence("Traffic Light Behavior")
        tl_freezer_sequence.add_child(TrafficLightFreezer(self._init_tl_dict, duration=self._green_light_delay))
        tl_freezer_sequence.add_child(TrafficLightFreezer(self._flow_tl_dict))
        root.add_child(tl_freezer_sequence)

        sequence = py_trees.composites.Sequence("Sequence")
        if CarlaDataProvider.get_ego_vehicle_route():
            sequence.add_child(Scenario9Manager(self._direction))
        sequence.add_child(root)

        return sequence

    def _create_test_criteria(self):
        """
        A list of all test criteria will be created that is later used
        in parallel behavior tree.
        """
        return [CollisionTest(self.ego_vehicles[0])]

    def __del__(self):
        """
        Remove all actors upon deletion
        """
        self.remove_all_actors()
