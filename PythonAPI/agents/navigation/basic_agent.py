#!/usr/bin/env python

# Copyright (c) 2018 Intel Labs.
# authors: German Ros (german.ros@intel.com)
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

""" This module implements an agent that roams around a track following random
waypoints and avoiding other vehicles.
The agent also responds to traffic lights. """

import math

import numpy as np
import carla

from agents.navigation.agent import *
from agents.navigation.local_planner import LocalPlanner
from agents.navigation.local_planner import compute_connection, RoadOption
from agents.navigation.global_route_planner import GlobalRoutePlanner
from agents.navigation.global_route_planner_dao import GlobalRoutePlannerDAO

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
        end_waypoint = self._map.get_waypoint(
            carla.Location(location[0], location[1], location[2]))

        solution = []

        dao = GlobalRoutePlannerDAO(self._vehicle.get_world().get_map())
        grp = GlobalRoutePlanner(dao)
        grp.setup()

        x1 = start_waypoint.transform.location.x
        y1 = start_waypoint.transform.location.y
        x2 = end_waypoint.transform.location.x
        y2 = end_waypoint.transform.location.y
        route = grp.plan_route((x1, y1), (x2, y2))

        route.pop(0)
        current_waypoint = start_waypoint

        for action in route:

            wp_choice = current_waypoint.next(5.0)
            while len(wp_choice) == 1:
                current_waypoint = wp_choice[0]
                solution.append((current_waypoint, RoadOption.LANEFOLLOW))
                wp_choice = current_waypoint.next(5.0)

            if action.value == "STOP":
                break

            if len(wp_choice) > 1:
                
                current_transform = current_waypoint.transform
                current_location = current_transform.location
                projected_location = current_location + \
                    carla.Location(
                        x=math.cos(math.radians(current_transform.rotation.yaw)),
                        y=math.sin(math.radians(current_transform.rotation.yaw)))
                v_current = self._vector(current_location, projected_location)

                direction = 0
                road_option = None
                if action.value == "LEFT":
                    direction = 1
                    road_option = RoadOption.LEFT
                elif action.value == "RIGHT":
                    direction = -1
                    road_option = RoadOption.RIGHT
                elif action.value == "GO_STRAIGHT":
                    direction = 0
                    road_option = RoadOption.STRAIGHT
                select_criteria = float('inf')

                for wp_select in wp_choice:
                    v_select = self._vector(
                        current_location, wp_select.transform.location)
                    cross = float('inf')
                    if direction == 0:
                        cross = abs(np.cross(v_current, v_select)[-1])
                    else:
                        cross = direction*np.cross(v_current, v_select)[-1]
                    if cross < select_criteria:
                        select_criteria = cross
                        current_waypoint = wp_select

                solution.append((current_waypoint, road_option))
                current_waypoint = current_waypoint.next(5.0)[0]
                while current_waypoint.is_intersection:
                    solution.append((current_waypoint, road_option))
                    current_waypoint = current_waypoint.next(5.0)[0]

        assert solution

        self._current_plan = solution
        self._local_planner.set_global_plan(self._current_plan)

    def _vector(self, l_1, l_2):
        """
        Returns the unit vector from l_1 to l_2
        l_1, l_2    :   carla.Location objects
        """
        x = l_2.x-l_1.x
        y = l_2.y-l_1.y
        norm = np.linalg.norm([x, y])
        return [x/norm, y/norm, 0]

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
