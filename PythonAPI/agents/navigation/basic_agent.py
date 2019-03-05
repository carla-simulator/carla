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
from agents.navigation.agent import Agent, AgentState
from agents.navigation.local_planner import LocalPlanner
from agents.navigation.local_planner import RoadOption
from agents.navigation.global_route_planner import GlobalRoutePlanner
from agents.navigation.global_route_planner_dao import GlobalRoutePlannerDAO
from agents.tools.misc import vector


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
        args_lateral_dict = {
            'K_P': 0.8,
            'K_D': 0.01,
            'K_I': 0,
            'dt': 1.0/20.0}
        self._local_planner = LocalPlanner(
            self._vehicle, opt_dict={'target_speed' : target_speed,
            'lateral_control_dict':args_lateral_dict})
        self._hop_resolution = 2.0
        self._path_seperation_hop = 2
        self._path_seperation_threshold = 0.5
        self._current_waypoint = None
        self._solution = []
        self._grp = None

        # setting up global router
        self._current_plan = None

    def set_destination(self, location):
        """
        This method creates a list of waypoints from agent's position to destination location
        based on the route returned by the global router
        """

        self._start_waypoint = self._map.get_waypoint(self._vehicle.get_location())
        self._end_waypoint = self._map.get_waypoint(
            carla.Location(location[0], location[1], location[2]))
        self._current_waypoint = self._start_waypoint

        print "Setting up GRP and retrieving route plan !"
        route = self._get_route(self._start_waypoint, self._end_waypoint)
        print "Gathered route plan : ", route
        
        print "Begining route assimilation !"
        for i, action in enumerate(route):
            print "Doing action : ", action
            if action == RoadOption.CHANGELANELEFT or action == RoadOption.CHANGELANERIGHT:
                self._update_lane_change(action, 20)
            else:

                if route[i+1] == RoadOption.VOID:
                    1+1
                    pass
                next_lane_change = False
                if i < len(route)-1 and route[i+1] != RoadOption.LANEFOLLOW and route[i+1] != RoadOption.VOID:
                    next_lane_change = True
                wp_choice = self._generate_waypoint_to_junction(next_lane_change)
                if action == RoadOption.VOID:
                    break                

                if len(wp_choice) > 1:
                    wp_choice, trajectories = self._seperate_trajectories(wp_choice)
                    
                    current_transform = self._current_waypoint.transform
                    current_location = current_transform.location
                    projected_location = current_location + carla.Location(
                        x=math.cos(math.radians(current_transform.rotation.yaw)),
                        y=math.sin(math.radians(current_transform.rotation.yaw)))
                    current_vector = vector(current_location, projected_location)

                    selected_waypoint, trajectory_index = self._choose_at_junction(
                        wp_choice, action, current_location, current_vector)
                    self._append_trjectory(trajectories, trajectory_index, action)
                else:
                    selected_waypoint = wp_choice[0]
                self._generate_waypoints_in_current_intersection(selected_waypoint, action)

        assert self._solution

        self._current_plan = self._solution
        self._local_planner.set_global_plan(self._current_plan)

    def _generate_waypoints_in_current_intersection(self, selected_waypoint, action):
        """
        This method updates self._solution with all the waypoints in the current
        intersection segment
        """

        self._current_waypoint = selected_waypoint
        self._solution.append((self._current_waypoint, action))
        self._current_waypoint = self._current_waypoint.next(self._hop_resolution)[0]
        while self._current_waypoint.is_intersection:
            self._solution.append((self._current_waypoint, action))
            self._current_waypoint = self._current_waypoint.next(self._hop_resolution)[0]

    def _append_trjectory(self, trajectories, trajectory_index, action):
        """
        This method updates self._solution with waypoints of a list acording to the
        tragejctory_index
        """

        for wp in trajectories[trajectory_index]:
            self._solution.append((wp, action))

    def _choose_at_junction(self, wp_choice, action, current_location, current_vector):
        """
        This method chooses the appropriate waypoint in a given list according to the
        action directive
        """

        direction = 0
        if action == RoadOption.LEFT: direction = 1
        elif action == RoadOption.RIGHT: direction = -1
        elif action == RoadOption.STRAIGHT: direction = 0
        select_criteria = float('inf')
        choosen_waypoint = None

        #   Choose correct path
        tragectory_index = 0
        for i, wp_select in enumerate(wp_choice):
            select_vector = vector(
                current_location, wp_select.transform.location)
            cross = float('inf')
            if direction == 0:
                cross = abs(np.cross(current_vector, select_vector)[-1])
            else:
                cross = direction*np.cross(current_vector, select_vector)[-1]
            if cross < select_criteria:
                tragectory_index = i
                select_criteria = cross
                choosen_waypoint = wp_select
        
        return choosen_waypoint, tragectory_index

    def _seperate_trajectories(self, wp_choice):
        """
        This method Seperates overlapping waypoint choices at intersections
        """

        split = False
        tragectories = []
        #   initializing paths
        for wp_select in wp_choice:
            tragectories.append([wp_select])
        #   finding points of seperation
        while not split and len(tragectories) > 1:
            #   take step in each path
            for tragectory in tragectories:
                tragectory.append(tragectory[-1].next(self._path_seperation_hop)[0])
            #   measure seperation
            for i in range(len(tragectories)-1):
                if tragectories[i][-1].transform.location.distance(
                    tragectories[i+1][-1].transform.location) > self._path_seperation_threshold:
                    split = True
        #   update waypoints for path choice
        for i, tragectory in enumerate(tragectories):
            wp_choice[i] = tragectory[-1]

        return wp_choice, tragectories

    def _generate_waypoint_to_junction(self, next_lane_change):
        """
        This method generates waypoint to a junction
        """

        wp_choice = self._current_waypoint.next(self._hop_resolution)
        not_junction = True

        while not_junction:
            if next_lane_change:
                1+1
                pass
            self._current_waypoint = wp_choice[0]
            self._solution.append((self._current_waypoint, RoadOption.LANEFOLLOW))
            wp_choice = self._current_waypoint.next(self._hop_resolution)
            #   Stop at destination
            if self._current_waypoint.transform.location.distance(
                self._end_waypoint.transform.location) < 2 * self._hop_resolution: break
            not_junction = len(wp_choice) < 2 and \
                not self._grp.verify_intersection(wp_choice[0]) and \
                not next_lane_change

        return wp_choice

    def _update_lane_change(self, action, change_over_distance=10):
        """
        This method updates self._solution with waypoints for lane change
        """

        if action == RoadOption.CHANGELANELEFT:
            self._current_waypoint = self._current_waypoint.get_left_lane().next(change_over_distance)[0]
        else:
            self._current_waypoint = self._current_waypoint.get_right_lane().next(change_over_distance)[0]
        self._solution.append((self._current_waypoint, action))

    def _get_route(self, start_waypoint, end_waypoint):
        """
        This method sets up a global router and returns the optimal route
        from start_waypoint to end_waypoint
        """

        # Setting up global router
        dao = GlobalRoutePlannerDAO(self._vehicle.get_world().get_map())
        grp = GlobalRoutePlanner(dao)
        grp.setup()
        self._grp = grp

        # Obtain route plan
        x1 = start_waypoint.transform.location.x
        y1 = start_waypoint.transform.location.y
        x2 = end_waypoint.transform.location.x
        y2 = end_waypoint.transform.location.y
        route = self._grp.plan_route(carla.Location(x1, y1), carla.Location(x2, y2))
        route.append(RoadOption.VOID)
        
        return route

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
