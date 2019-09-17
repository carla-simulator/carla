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
from agents.tools.misc import is_within_distance, compute_distance

class Agent():
    """
    Base class to define agents in CARLA
    """

    def __init__(self, vehicle):
        """
        :param vehicle: actor to apply to local planner logic onto
        """
        self._vehicle = vehicle
        self._local_planner = None
        self._world = self._vehicle.get_world()
        self._map = self._vehicle.get_world().get_map()
        self._last_traffic_light = None

    @staticmethod
    def run_step(debug=False):
        """
        Execute one step of navigation.
        :return: control
        """
        control = carla.VehicleControl()

        if debug:
            control.steer = 0.0
            control.throttle = 0.0
            control.brake = 0.0
            control.hand_brake = False
            control.manual_gear_shift = False

        return control

    def _is_vehicle_hazard(self, vehicle_list, proximity_threshold):
        """
        Check if a given vehicle is an obstacle in our way. To this end we take
        into account the road and lane the target vehicle is on and run a
        geometry test to check if the target vehicle is under a certain distance
        in front of our ego vehicle.

        WARNING: This method is an approximation that could fail for very large
        vehicles, which center is actually on a different lane but their
        extension falls within the ego vehicle lane.

        :param vehicle_list: list of potential obstacle to check
        :return: a tuple given by (bool_flag, vehicle), where
                 - bool_flag is True if there is a vehicle ahead blocking us
                   and False otherwise
                 - vehicle is the blocker object itself
        """

        ego_vehicle_location = self._vehicle.get_location()
        ego_vehicle_waypoint = self._map.get_waypoint(ego_vehicle_location)

        for target_vehicle in vehicle_list:
            # Do not account for the ego vehicle
            if target_vehicle.id == self._vehicle.id:
                continue

            # If the object is not in our lane it's not an obstacle
            target_vehicle_waypoint = self._map.get_waypoint(target_vehicle.get_location())
            if target_vehicle_waypoint.road_id != ego_vehicle_waypoint.road_id or \
                    target_vehicle_waypoint.lane_id != ego_vehicle_waypoint.lane_id:
                continue

            loc = target_vehicle.get_location()
            if is_within_distance(loc, ego_vehicle_location,
                                  self._vehicle.get_transform().rotation.yaw,
                                  proximity_threshold, d_angle_th=60.0):
                return (True, target_vehicle, compute_distance(loc, ego_vehicle_location))

        return (False, None, -1)

    def _is_vehicle_on_right_lane_hazard(self, vehicle_list, proximity_threshold):
        """
        Check if a given vehicle is an obstacle in our way to change lane to the right.
        """

        ego_vehicle_location = self._vehicle.get_location()
        ego_vehicle_waypoint = self._map.get_waypoint(ego_vehicle_location)

        for target_vehicle in vehicle_list:
            # Do not account for the ego vehicle
            if target_vehicle.id == self._vehicle.id:
                continue

            # If the object is not in our road it's not an obstacle,
            # However this time we are also checking for nearby lanes
            target_vehicle_waypoint = self._map.get_waypoint(target_vehicle.get_location())
            if target_vehicle_waypoint.road_id != ego_vehicle_waypoint.road_id or \
                    abs(target_vehicle_waypoint.lane_id) != abs(ego_vehicle_waypoint.lane_id)+1:
                continue

            loc = target_vehicle.get_location()
            if is_within_distance(loc, ego_vehicle_location,
                                  self._vehicle.get_transform().rotation.yaw,
                                  proximity_threshold, d_angle_th=180.0):
                return (True, target_vehicle, compute_distance(loc, ego_vehicle_location))

        return (False, None, -1)

    def _is_vehicle_on_left_lane_hazard(self, vehicle_list, proximity_threshold):
        """
        Check if a given vehicle is an obstacle in our way to change lane to the left.
        """

        ego_vehicle_location = self._vehicle.get_location()
        ego_vehicle_waypoint = self._map.get_waypoint(ego_vehicle_location)

        for target_vehicle in vehicle_list:
            # Do not account for the ego vehicle
            if target_vehicle.id == self._vehicle.id:
                continue

            # If the object is not in our road it's not an obstacle,
            # However this time we are also checking for nearby lanes
            target_vehicle_waypoint = self._map.get_waypoint(target_vehicle.get_location())
            if target_vehicle_waypoint.road_id != ego_vehicle_waypoint.road_id or \
                    abs(target_vehicle_waypoint.lane_id) != abs(ego_vehicle_waypoint.lane_id)-1:
                continue
            loc = target_vehicle.get_location()
            if is_within_distance(loc, ego_vehicle_location,
                                  self._vehicle.get_transform().rotation.yaw,
                                  proximity_threshold, d_angle_th=180.0):
                return (True, target_vehicle, compute_distance(loc, ego_vehicle_location))

        return (False, None, -1)


    def _is_vehicle_behind_hazard(self, vehicle_list, proximity_threshold):
        """
        Check if a given vehicle is an obstacle in
        our way to change lane based on vehicles behind it.
        """

        ego_vehicle_location = self._vehicle.get_location()
        ego_vehicle_waypoint = self._map.get_waypoint(ego_vehicle_location)

        for target_vehicle in vehicle_list:
            # Do not account for the ego vehicle
            if target_vehicle.id == self._vehicle.id:
                continue

            # If the object is not in our lane it's not an obstacle
            target_vehicle_waypoint = self._map.get_waypoint(target_vehicle.get_location())
            if target_vehicle_waypoint.road_id != ego_vehicle_waypoint.road_id or \
                    target_vehicle_waypoint.lane_id != ego_vehicle_waypoint.lane_id:
                continue

            loc = target_vehicle.get_location()
            if is_within_distance(loc, ego_vehicle_location,
                                  self._vehicle.get_transform().rotation.yaw,
                                  proximity_threshold, d_angle_th=180.0, d_angle_th_low=170.0):
                return (True, target_vehicle, compute_distance(loc, ego_vehicle_location))

        return (False, None, -1)

    @staticmethod
    def emergency_stop():
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
