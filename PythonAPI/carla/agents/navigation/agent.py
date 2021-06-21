# Copyright (c) # Copyright (c) 2018-2020 CVC.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

""" This module implements an agent that roams around a track following random
waypoints and avoiding other vehicles.
The agent also responds to traffic lights. """

import carla
from agents.tools.misc import is_within_distance, get_trafficlight_trigger_location


class Agent(object):
    """Base class to define agents in CARLA"""

    def __init__(self, vehicle, debug=False):
        """
        Constructor method.

            :param vehicle: actor to apply to local planner logic onto
        """
        self._vehicle = vehicle
        self._base_tlight_threshold = 5.0  # meters
        self._base_vehicle_threshold = 5.0  # meters

        self._world = self._vehicle.get_world()
        self._map = self._world.get_map()
        self._debug = debug

        self._last_traffic_light = None

    def run_step(self):
        """
        Execute one step of navigation.

            :param debug: boolean flag for debugging
            :return: control
        """
        control = carla.VehicleControl()

        return control

    def emergency_stop(self):
        """
        Send an emergency stop command to the vehicle

            :return: control for braking
        """
        control = carla.VehicleControl()
        control.steer = 0.0
        control.throttle = 0.0
        control.brake = 1.0
        control.hand_brake = False

        return control

    def _is_light_red(self, lights_list, max_distance=None):
        """
        Method to check if there is a red light affecting us. This version of
        the method is compatible with both European and US style traffic lights.

        :param lights_list: list containing TrafficLight objects
        :return: a tuple given by (bool_flag, traffic_light), where
                 - bool_flag is True if there is a traffic light in RED
                   affecting us and False otherwise
                 - traffic_light is the object itself or None if there is no
                   red traffic light affecting us
        """
        if not max_distance:
            max_distance = self._base_tlight_threshold

        if self._last_traffic_light:
            if self._last_traffic_light.state != carla.TrafficLightState.Red:
                self._last_traffic_light = None
            else:
                return (True, self._last_traffic_light)

        ego_vehicle_location = self._vehicle.get_location()
        ego_vehicle_waypoint = self._map.get_waypoint(ego_vehicle_location)

        for traffic_light in lights_list:
            object_location = get_trafficlight_trigger_location(traffic_light)
            object_waypoint = self._map.get_waypoint(object_location)

            if object_waypoint.road_id != ego_vehicle_waypoint.road_id:
                continue

            ve_dir = ego_vehicle_waypoint.transform.get_forward_vector()
            wp_dir = object_waypoint.transform.get_forward_vector()
            dot_ve_wp = ve_dir.x * wp_dir.x + ve_dir.y * wp_dir.y + ve_dir.z * wp_dir.z

            if dot_ve_wp < 0:
                continue

            if traffic_light.state != carla.TrafficLightState.Red:
                continue

            if is_within_distance(object_waypoint.transform,
                                  self._vehicle.get_transform(),
                                  max_distance,
                                  [0, 90]):

                self._last_traffic_light = traffic_light
                return (True, traffic_light)

        return (False, None)

    def _is_vehicle_hazard(self, vehicle_list, max_distance=None):
        """
        :param vehicle_list: list of potential obstacle to check
        :return: a tuple given by (bool_flag, vehicle), where
                 - bool_flag is True if there is a vehicle ahead blocking us
                   and False otherwise
                 - vehicle is the blocker object itself
        """
        if not max_distance:
            max_distance = self._base_vehicle_threshold

        ego_vehicle_transform = self._vehicle.get_transform()
        ego_vehicle_forward_vector = ego_vehicle_transform.get_forward_vector()
        ego_vehicle_extent = self._vehicle.bounding_box.extent.x
        ego_vehicle_waypoint = self._map.get_waypoint(self._vehicle.get_location())

        # Get the transform of the front of the ego
        ego_vehicle_front_transform = ego_vehicle_transform
        ego_vehicle_front_transform.location += carla.Location(
            x=ego_vehicle_extent * ego_vehicle_forward_vector.x,
            y=ego_vehicle_extent * ego_vehicle_forward_vector.y,
        )
        for target_vehicle in vehicle_list:
            # Do not account for the ego vehicle
            if target_vehicle.id == self._vehicle.id:
                continue

            # If the object is not in our lane it's not an obstacle
            target_vehicle_transform = target_vehicle.get_transform()
            target_vehicle_forward_vector = target_vehicle_transform.get_forward_vector()
            target_vehicle_extent = target_vehicle.bounding_box.extent.x
            target_vehicle_waypoint = self._map.get_waypoint(target_vehicle.get_location())
            if target_vehicle_waypoint.road_id != ego_vehicle_waypoint.road_id or \
                    target_vehicle_waypoint.lane_id != ego_vehicle_waypoint.lane_id:
                continue

            # Get the transform of the back of the vehicle
            target_vehicle_rear_transform = target_vehicle_transform
            target_vehicle_rear_transform.location -= carla.Location(
                x=target_vehicle_extent * target_vehicle_forward_vector.x,
                y=target_vehicle_extent * target_vehicle_forward_vector.y,
            )

            if is_within_distance(target_vehicle_rear_transform,
                                  ego_vehicle_front_transform,
                                  max_distance,
                                  [0, 90]):

                return (True, target_vehicle)

        return (False, None)
