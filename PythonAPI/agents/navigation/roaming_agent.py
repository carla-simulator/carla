#!/usr/bin/env python

# Copyright (c) 2018 Intel Labs.
# authors: German Ros (german.ros@intel.com)
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

""" This module implements an agent that roams around a track following random waypoints and avoiding other vehicles.
The agent also responds to traffic lights. """

from enum import Enum

import carla
from agents.navigation.local_planner import LocalPlanner
from agents.tools.misc import is_within_distance_ahead, compute_magnitude_angle



class AgentState(Enum):
    """
    AGENT_STATE represents the possible states of a roaming agent
    """
    NAVIGATING = 1
    BLOCKED_BY_VEHICLE = 2
    BLOCKED_RED_LIGHT = 3


class RoamingAgent(object):
    """
    RoamingAgent implements a basic agent that navigates scenes making random choices when facing an intersection.
    This agent respects traffic lights and other vehicles.
    """

    def __init__(self, vehicle):
        """

        :param vehicle: actor to apply to local planner logic onto
        """
        self._vehicle = vehicle
        self._world = self._vehicle.get_world()
        self._map = self._vehicle.get_world().get_map()
        self._proximity_threshold = 10.0  # meters
        self._state = AgentState.NAVIGATING
        self._local_planner = LocalPlanner(self._vehicle)

    def run_step(self, debug=False):
        """
        Execute one step of navigation.
        :return:
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

    def _is_light_red(self, lights_list):
        """
        Method to check if there is a red light affecting us. This version of the method is compatible
        with both European and US style traffic lights.

        :param lights_list: list containing TrafficLight objects
        :return: a tuple given by (bool_flag, traffic_light), where
                 - bool_flag is True if there is a traffic light in RED affecting us and False otherwise
                 - traffic_light is the object itself or None if there is no red traffic light affecting us
        """
        if self._world.map_name == 'Town01' or self._world.map_name == 'Town02':
            return self._is_light_red_europe_style(lights_list)
        else:
            return self._is_light_red_us_style(lights_list)

    def _is_light_red_europe_style(self, lights_list):
        """
        This method is specialized to check European style traffic lights.

        :param lights_list: list containing TrafficLight objects
        :return: a tuple given by (bool_flag, traffic_light), where
                 - bool_flag is True if there is a traffic light in RED affecting us and False otherwise
                 - traffic_light is the object itself or None if there is no red traffic light affecting us
        """
        ego_vehicle_location = self._vehicle.get_location()
        ego_vehicle_waypoint = self._map.get_waypoint(ego_vehicle_location)

        for traffic_light in lights_list:
            object_waypoint = self._map.get_waypoint(traffic_light.get_location())
            if object_waypoint.road_id != ego_vehicle_waypoint.road_id or object_waypoint.lane_id != ego_vehicle_waypoint.lane_id:
                continue

            loc = traffic_light.get_location()
            if is_within_distance_ahead(loc, ego_vehicle_location, self._vehicle.get_transform().rotation.yaw,
                                        self._proximity_threshold):
                if traffic_light.state == carla.libcarla.TrafficLightState.Red:
                    return (True, traffic_light)

        return (False, None)

    def _is_light_red_us_style(self, lights_list, debug=False):
        """
        This method is specialized to check US style traffic lights.

        :param lights_list: list containing TrafficLight objects
        :return: a tuple given by (bool_flag, traffic_light), where
                 - bool_flag is True if there is a traffic light in RED affecting us and False otherwise
                 - traffic_light is the object itself or None if there is no red traffic light affecting us
        """
        ego_vehicle_location = self._vehicle.get_location()
        ego_vehicle_waypoint = self._map.get_waypoint(ego_vehicle_location)

        if ego_vehicle_waypoint.is_intersection:
            # It is too late. Do not block the intersection! Keep going!
            return (False, None)

        if self._local_planner._target_waypoint is not None:
            if self._local_planner._target_waypoint.is_intersection:
                potential_lights = []
                min_angle = 180.0
                sel_magnitude = 0.0
                sel_traffic_light = None
                for traffic_light in lights_list:
                    loc = traffic_light.get_location()
                    magnitude, angle = compute_magnitude_angle(loc, ego_vehicle_location, self._vehicle.get_transform().rotation.yaw)
                    if magnitude < 80.0 and angle < 25.0 and angle < min_angle:
                        sel_magnitude = magnitude
                        sel_traffic_light = traffic_light
                        min_angle = angle

                if sel_traffic_light is not None:
                    if debug:
                        print('=== Magnitude = {} | Angle = {} | ID = {}'.format(sel_magnitude, min_angle, sel_traffic_light.id))
                    if sel_traffic_light.state == carla.libcarla.TrafficLightState.Red:
                        return (True, sel_traffic_light)

        return (False, None)

    def _is_vehicle_hazard(self, vehicle_list):
        """
        Check if a given vehicle is an obstacle in our way. To this end we take into account the road and lane
        the target vehicle is on and run a geometry test to check if the target vehicle is under a certain distance
        in front of our ego vehicle.

        WARNING: This method is an approximation that could fail for very large vehicles, which center is actually
                 on a different lane but their extension falls within the ego vehicle lane.

        :param vehicle_list: list of potential obstacle to check
        :return: a tuple given by (bool_flag, vehicle), where
                 - bool_flag is True if there is a vehicle ahead blocking us and False otherwise
                 - vehicle is the blocker object itself
        """

        ego_vehicle_location = self._vehicle.get_location()
        ego_vehicle_waypoint = self._map.get_waypoint(ego_vehicle_location)

        for target_vehicle in vehicle_list:
            # do not account for the ego vehicle
            if target_vehicle.id == self._vehicle.id:
                continue

            # if the object is not in our lane it's not an obstacle
            target_vehicle_waypoint = self._map.get_waypoint(target_vehicle.get_location())
            if target_vehicle_waypoint.road_id != ego_vehicle_waypoint.road_id or \
                            target_vehicle_waypoint.lane_id != ego_vehicle_waypoint.lane_id:
                continue

            loc = target_vehicle.get_location()
            if is_within_distance_ahead(loc, ego_vehicle_location, self._vehicle.get_transform().rotation.yaw,
                                            self._proximity_threshold):
                return (True, target_vehicle)

        return (False, None)


    def emergency_stop(self):
        """
        Send an emergency stop command to the vehicle
        :return:
        """
        control = carla.VehicleControl()
        control.steer = 0.0
        control.throttle = 0.0
        control.brake = 1.0
        control.hand_brake = False

        return control