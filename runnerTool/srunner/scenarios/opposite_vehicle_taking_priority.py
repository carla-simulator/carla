#!/usr/bin/env python

# Copyright (c) 2018-2020 Intel Corporation
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Scenarios in which another (opposite) vehicle 'illegally' takes
priority, e.g. by running a red traffic light.
"""

from __future__ import print_function

import py_trees
import carla

from srunner.scenariomanager.carla_data_provider import CarlaDataProvider
from srunner.scenariomanager.scenarioatomics.atomic_behaviors import (ActorTransformSetter,
                                                                      ActorDestroy,
                                                                      TrafficLightFreezer,
                                                                      BasicAgentBehavior)
from srunner.scenariomanager.scenarioatomics.atomic_criteria import CollisionTest
from srunner.scenariomanager.scenarioatomics.atomic_trigger_conditions import (InTriggerDistanceToLocation,
                                                                               InTimeToArrivalToLocation,
                                                                               WaitEndIntersection)
from srunner.scenarios.basic_scenario import BasicScenario
from srunner.tools.scenario_helper import (get_geometric_linear_intersection,
                                           generate_target_waypoint,
                                           get_junction_topology,
                                           filter_junction_wp_direction,
                                           get_closest_traffic_light)

from srunner.tools.background_manager import Scenario7Manager


class OppositeVehicleRunningRedLight(BasicScenario):
    """
    This class holds everything required for a scenario in which another vehicle runs a red light
    in front of the ego, forcing it to react. This vehicles are 'special' ones such as police cars,
    ambulances or firetrucks.
    """

    def __init__(self, world, ego_vehicles, config, randomize=False, debug_mode=False, criteria_enable=True,
                 timeout=180):
        """
        Setup all relevant parameters and create scenario
        and instantiate scenario manager
        """
        self._world = world
        self._map = CarlaDataProvider.get_map()
        self._source_dist = 30
        self._sink_dist = 20
        self._direction = None
        self._opposite_bp_wildcards = ['*firetruck*', '*ambulance*', '*police*']  # Wildcard patterns of the blueprints
        self.timeout = timeout

        self._adversary_speed = 70 / 3.6  # Speed of the adversary [m/s]
        self._sync_time = 2.2  # Time the agent has to react to avoid the collision [s]
        self._min_trigger_dist = 9.0  # Min distance to the collision location that triggers the adversary [m]
        self._speed_duration_ratio = 2.0
        self._speed_distance_ratio = 1.5

        # Get the CDP seed or at routes, all copies of the scenario will have the same configuration
        self._rng = CarlaDataProvider.get_random_seed()

        super(OppositeVehicleRunningRedLight, self).__init__("OppositeVehicleRunningRedLight",
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

        # Get the opposite entry lane wp
        possible_directions = ['right', 'left']
        self._rng.shuffle(possible_directions)
        for direction in possible_directions:
            entry_wps, _ = get_junction_topology(junction)
            source_entry_wps = filter_junction_wp_direction(starting_wp, entry_wps, direction)
            if source_entry_wps:
                self._direction = direction
                break
        if not self._direction:
            raise ValueError("Trying to find a lane to spawn the opposite actor but none was found")

        # Get the source transform
        spawn_wp = source_entry_wps[0]
        added_dist = 0
        while added_dist < self._source_dist:
            spawn_wps = spawn_wp.previous(1.0)
            if len(spawn_wps) == 0:
                raise ValueError("Failed to find a source location as a waypoint with no previous was detected")
            if spawn_wps[0].is_junction:
                break
            spawn_wp = spawn_wps[0]
            added_dist += 1
        self._spawn_wp = spawn_wp

        source_transform = spawn_wp.transform
        self._spawn_location = carla.Transform(
            source_transform.location + carla.Location(z=0.1),
            source_transform.rotation
        )

        # Spawn the actor and move it below ground
        opposite_bp_wildcard = self._rng.choice(self._opposite_bp_wildcards)
        opposite_actor = CarlaDataProvider.request_new_actor(opposite_bp_wildcard, self._spawn_location)
        if not opposite_actor:
            raise Exception("Couldn't spawn the actor")
        opposite_actor.set_light_state(carla.VehicleLightState(
            carla.VehicleLightState.Special1 | carla.VehicleLightState.Special2))
        self.other_actors.append(opposite_actor)

        opposite_transform = carla.Transform(
            source_transform.location - carla.Location(z=500),
            source_transform.rotation
        )
        opposite_actor.set_transform(opposite_transform)
        opposite_actor.set_simulate_physics(enabled=False)

        # Get the sink location
        sink_exit_wp = generate_target_waypoint(self._map.get_waypoint(source_transform.location), 0)
        sink_wps = sink_exit_wp.next(self._sink_dist)
        if len(sink_wps) == 0:
            raise ValueError("Failed to find a sink location as a waypoint with no next was detected")
        self._sink_wp = sink_wps[0]

        # get the collision location
        self._collision_location = get_geometric_linear_intersection(
            starting_wp.transform.location, source_entry_wps[0].transform.location)
        if not self._collision_location:
            raise ValueError("Couldn't find an intersection point")

        # Get the relevant traffic lights
        tls = self._world.get_traffic_lights_in_junction(junction.id)
        ego_tl = get_closest_traffic_light(ego_wp, tls)
        source_tl = get_closest_traffic_light(self._spawn_wp, tls,)
        self._tl_dict = {}
        for tl in tls:
            if tl in (ego_tl, source_tl):
                self._tl_dict[tl] = carla.TrafficLightState.Green
            else:
                self._tl_dict[tl] = carla.TrafficLightState.Red

    def _create_behavior(self):
        """
        Hero vehicle is entering a junction in an urban area, at a signalized intersection,
        while another actor runs a red lift, forcing the ego to break.
        """
        sequence = py_trees.composites.Sequence()

        # Wait until ego is close to the adversary
        trigger_adversary = py_trees.composites.Parallel(
            policy=py_trees.common.ParallelPolicy.SUCCESS_ON_ONE, name="TriggerAdversaryStart")
        trigger_adversary.add_child(InTimeToArrivalToLocation(
            self.ego_vehicles[0], self._sync_time, self._collision_location))
        trigger_adversary.add_child(InTriggerDistanceToLocation(
            self.ego_vehicles[0], self._collision_location, self._min_trigger_dist))

        sequence.add_child(trigger_adversary)
        sequence.add_child(BasicAgentBehavior(
            self.other_actors[0], target_location=self._sink_wp.transform.location,
            target_speed=self._adversary_speed, opt_dict={'ignore_vehicles': True}, name="AdversaryCrossing"))

        main_behavior = py_trees.composites.Parallel(policy=py_trees.common.ParallelPolicy.SUCCESS_ON_ONE)
        main_behavior.add_child(TrafficLightFreezer(self._tl_dict))
        main_behavior.add_child(sequence)

        root = py_trees.composites.Sequence()
        if CarlaDataProvider.get_ego_vehicle_route():
            root.add_child(Scenario7Manager(self._direction))
        root.add_child(ActorTransformSetter(self.other_actors[0], self._spawn_location))
        root.add_child(main_behavior)
        root.add_child(ActorDestroy(self.other_actors[0]))
        root.add_child(WaitEndIntersection(self.ego_vehicles[0]))

        return root

    def _create_test_criteria(self):
        """
        A list of all test criteria will be created that is later used
        in parallel behavior tree.
        """
        return [CollisionTest(self.ego_vehicles[0])]

    def __del__(self):
        """
        Remove all actors and traffic lights upon deletion
        """
        self.remove_all_actors()
