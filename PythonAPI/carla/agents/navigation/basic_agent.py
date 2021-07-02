# Copyright (c) # Copyright (c) 2018-2020 CVC.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
This module implements an agent that roams around a track following random
waypoints and avoiding other vehicles. The agent also responds to traffic lights.
"""

import carla
from enum import Enum

from agents.navigation.local_planner import LocalPlanner, RoadOption
from agents.navigation.global_route_planner import GlobalRoutePlanner
from agents.tools.misc import get_speed, is_within_distance, get_trafficlight_trigger_location


class AgentState(Enum):
    """
    AGENT_STATE represents the possible states of a roaming agent
    """
    NAVIGATING = 1
    BLOCKED_BY_VEHICLE = 2
    BLOCKED_RED_LIGHT = 3

class BasicAgent(object):
    """
    BasicAgent implements an agent that navigates the scene.
    This agent respects traffic lights and other vehicles, but ignores stop signs.
    It has several functions available to specify the route that the agent must follow
    """

    def __init__(self, vehicle, target_speed=20, debug=False):
        """
        :param vehicle: actor to apply to local planner logic onto
        :param target_speed: speed (in Km/h) at which the vehicle will move
        """
        self._vehicle = vehicle
        self._world = self._vehicle.get_world()
        self._map = self._world.get_map()
        self._debug = debug

        self._ignore_traffic_lights = False
        self._ignore_stop_signs = False
        self._ignore_vehicles = False
        self._last_traffic_light = None

        self._state = AgentState.NAVIGATING
        self._target_speed = target_speed
        self._sampling_resolution = 2.0
        self._grp = None

        self._base_tlight_threshold = 5.0  # meters
        self._base_vehicle_threshold = 5.0  # meters
        self._max_brake = 0.5
        self._max_steering = 0.3
        self._local_planner = LocalPlanner(
            self._vehicle,
            opt_dict={
                'target_speed' : target_speed,
                'max_brake': self._max_brake,
                'max_steering': self._max_steering
            }
        )

    def emergency_stop(self):
        """
        Send an emergency stop command to the vehicle
        """
        control = carla.VehicleControl()
        control.steer = 0.0
        control.throttle = 0.0
        control.brake = self._max_brake
        control.hand_brake = False

        return control

    def set_target_speed(self, speed):
        """
        Changes the target speed of the agent
        """
        self._local_planner.set_speed(speed)

    def follow_speed_limits(self, value=True):
        """
        (De)activates a flag to make the agent dynamically change the target speed according to the speed limits
        """
        self._local_planner.follow_speed_limits(value)

    def get_local_planner(self):
        """
        Get method for protected member local planner
        """
        return self._local_planner

    def set_destination(self, end_location, start_location=None):
        """
        This method creates a list of waypoints between a starting and ending location,
        based on the route returned by the global router, and adds it to the local planner.
        If no starting location is passed, the vehicle location is chosen.

            :param end_location: final location of the route
            :param start_location: starting location of the route
        """
        if not start_location:
            start_location = self._local_planner.target_waypoint.transform.location
            clean_queue = True
        else:
            start_location = self._vehicle.get_location()
            clean_queue = False

        start_waypoint = self._map.get_waypoint(start_location)
        end_waypoint = self._map.get_waypoint(end_location)

        route_trace = self.trace_route(start_waypoint, end_waypoint)
        self._local_planner.set_global_plan(route_trace, clean_queue=clean_queue)

        # for i, w in enumerate(route_trace):
        #     wp = w[0].transform.location + carla.Location(z=0.1)
        #     if w[1] == RoadOption.LEFT:  # Yellow
        #         color = carla.Color(255, 255, 0)
        #     elif w[1] == RoadOption.RIGHT:  # Cyan
        #         color = carla.Color(0, 255, 255)
        #     elif w[1] == RoadOption.CHANGELANELEFT:  # Orange
        #         color = carla.Color(255, 64, 0)
        #     elif w[1] == RoadOption.CHANGELANERIGHT:  # Dark Cyan
        #         color = carla.Color(0, 64, 255)
        #     elif w[1] == RoadOption.STRAIGHT:  # Gray
        #         color = carla.Color(128, 128, 128)
        #     else:  # LANEFOLLOW
        #         color = carla.Color(0, 255, 0) # Green
        #     self._world.debug.draw_point(wp, size=0.08, color=color, life_time=100)
        #     self._world.debug.draw_string(wp, str(i), color=color, life_time=100)

    def set_global_plan(self, plan, stop_waypoint_creation=True, clean_queue=True):
        """
        Adds a specific plan to the agent.

            :param plan: list of [carla.Waypoint, RoadOption] representing the route to be followed
            :param stop_waypoint_creation: stops the automatic creation of waypoints
            :param clean_queue: resets the current agent's plan
        """
        self._local_planner.set_global_plan(
            plan,
            stop_waypoint_creation=stop_waypoint_creation,
            clean_queue=clean_queue
        )

    def trace_route(self, start_waypoint, end_waypoint):
        """
        This method sets up a global router and returns the
        optimal route from start_waypoint to end_waypoint.

            :param start_waypoint: initial position
            :param end_waypoint: final position
        """
        if self._grp is None:
            self._grp = GlobalRoutePlanner(self._map, self._sampling_resolution)
        route = self._grp.trace_route(start_waypoint, end_waypoint)
        return route

    def run_step(self, debug=False):
        """
        Execute one step of navigation.
        :return: carla.VehicleControl
        """
        hazard_detected = False

        # Retrieve all relevant actors
        actor_list = self._world.get_actors()
        vehicle_list = actor_list.filter("*vehicle*")
        lights_list = actor_list.filter("*traffic_light*")

        vehicle_speed = get_speed(self._vehicle) / 3.6

        # Check for possible obstacles
        max_vehicle_distance = self._base_vehicle_threshold + vehicle_speed
        vehicle_state, vehicle = self._vehicle_obstacle_detected(vehicle_list, max_vehicle_distance)
        if vehicle_state:
            if debug:
                print('!!! VEHICLE BLOCKING AHEAD [{}])'.format(vehicle.id))

            self._state = AgentState.BLOCKED_BY_VEHICLE
            hazard_detected = True

        # Check if the vehicle is affected by a red traffic light
        max_tlight_distance = self._base_tlight_threshold + vehicle_speed
        light_state, traffic_light = self._affected_by_traffic_light(lights_list, max_tlight_distance)
        if light_state:
            if debug:
                print('=== RED LIGHT AHEAD [{}])'.format(traffic_light.id))

            self._state = AgentState.BLOCKED_RED_LIGHT
            hazard_detected = True

        if hazard_detected:
            control = self.emergency_stop()
        else:
            self._state = AgentState.NAVIGATING
            control = self._local_planner.run_step(debug=debug)

        return control

    def done(self):
        """
        Check whether the agent has reached its destination.
        """
        return self._local_planner.done()

    def ignore_traffic_lights(self, active=True):
        """
        (De)activates the checks for traffic lights
        """
        self._ignore_traffic_lights = active

    def ignore_stop_signs(self, active=True):
        """
        (De)activates the checks for stop signs
        """
        self._ignore_stop_signs = active

    def ignore_vehicles(self, active=True):
        """
        (De)activates the checks for stop signs
        """
        self._ignore_vehicles = active

    def _affected_by_traffic_light(self, lights_list, max_distance=None):
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
        if self._ignore_traffic_lights:
            return (False, None)

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

    def _vehicle_obstacle_detected(self, vehicle_list, max_distance=None):
        """
        :param vehicle_list: list of potential obstacle to check
        :param max_distance: max distance to check for obstacles
        :return: a tuple given by (bool_flag, vehicle), where
                 - bool_flag is True if there is a vehicle ahead blocking us
                   and False otherwise
                 - vehicle is the blocker object itself
        """
        if self._ignore_vehicles:
            return (False, None)

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
