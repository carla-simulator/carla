#!/usr/bin/env python

# Copyright (c) 2018 Intel Labs.
# authors: German Ros (german.ros@intel.com)
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

""" This module implements an agent that roams around a track following random
waypoints and avoiding other vehicles.
The agent also responds to traffic lights. """

import carla
from agents.navigation.agent import *
from agents.navigation.local_planner import LocalPlanner
from agents.navigation.local_planner import compute_connection, RoadOption

class BasicAgent(Agent):
    """
    BasicAgent implements a basic agent that navigates scenes to reach a given
    target destination. This agent respects traffic lights and other vehicles.
    """

    def __init__(self, vehicle, target_speed=20):
        """

        :param vehicle: actor to apply to local planner logic onto
        """
        super(BasicAgent, self).__init__(vehicle)

        self._proximity_threshold = 10.0  # meters
        self._state = AgentState.NAVIGATING
        self._local_planner = LocalPlanner(self._vehicle, opt_dict={'target_speed' : target_speed})

        # setting up global router
        self._current_plan = None

    def set_destination(self, location):
        start_waypoint = self._map.get_waypoint(self._vehicle.get_location())
        end_waypoint = self._map.get_waypoint(carla.Location(location[0],
                                                             location[1],
                                                             location[2]))

        current_waypoint = start_waypoint
        active_list = [ [(current_waypoint, RoadOption.LANEFOLLOW)] ]

        solution = []
        while not solution:
            for _ in range(len(active_list)):
                trajectory = active_list.pop()
                if len(trajectory) > 1000:
                    continue

                # expand this trajectory
                current_waypoint, _ = trajectory[-1]
                next_waypoints = current_waypoint.next(5.0)
                while len(next_waypoints) == 1:
                    next_option = compute_connection(current_waypoint, next_waypoints[0])
                    current_distance = next_waypoints[0].transform.location.distance(end_waypoint.transform.location)
                    if current_distance < 5.0:
                        solution = trajectory + [(end_waypoint, RoadOption.LANEFOLLOW)]
                        break

                    # keep adding nodes
                    trajectory.append((next_waypoints[0], next_option))
                    current_waypoint, _ = trajectory[-1]
                    next_waypoints = current_waypoint.next(5.0)

                if not solution:
                    # multiple choices
                    for waypoint in next_waypoints:
                        next_option = compute_connection(current_waypoint, waypoint)
                        active_list.append(trajectory + [(waypoint, next_option)])

        assert solution

        self._current_plan = solution
        self._local_planner.set_global_plan(self._current_plan)

    def run_step(self, debug=False):
        """
        Execute one step of navigation.
        :return: carla.VehicleControl
        """

        # is there an obstacle in front of us?
        hazard_detected = False

        # retrieve relevant elements for safe navigation, i.e.: traffic lights
        # and other vehicles
        actor_list = self._world.get_actors()
        vehicle_list = actor_list.filter("*vehicle*")
        lights_list = actor_list.filter("*traffic_light*")

        # check possible obstacles
        vehicle_state, vehicle = self._is_vehicle_hazard(vehicle_list)
        if vehicle_state:
            if debug:
                print('!!! VEHICLE BLOCKING AHEAD [{}])'.format(vehicle.id))

            self._state = AgentState.BLOCKED_BY_VEHICLE
            hazard_detected = True

        # check for the state of the traffic lights
        light_state, traffic_light = self._is_light_red(lights_list)
        if light_state:
            if debug:
                print('=== RED LIGHT AHEAD [{}])'.format(traffic_light.id))

            self._state = AgentState.BLOCKED_RED_LIGHT
            hazard_detected = True

        if hazard_detected:
            control = self.emergency_stop()
        else:
            self._state = AgentState.NAVIGATING
            # standard local planner behavior
            control = self._local_planner.run_step()

        return control
