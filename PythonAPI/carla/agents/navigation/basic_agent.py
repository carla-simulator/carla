# Copyright (c) # Copyright (c) 2018-2020 CVC.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

""" This module implements an agent that roams around a track following random
waypoints and avoiding other vehicles.
The agent also responds to traffic lights. """

import carla
from agents.navigation.agent import Agent
from agents.navigation.local_planner import LocalPlanner
from agents.navigation.global_route_planner import GlobalRoutePlanner
from agents.navigation.global_route_planner_dao import GlobalRoutePlannerDAO
from agents.tools.misc import get_speed

from enum import Enum

class AgentState(Enum):
    """
    AGENT_STATE represents the possible states of a roaming agent
    """
    NAVIGATING = 1
    BLOCKED_BY_VEHICLE = 2
    BLOCKED_RED_LIGHT = 3

class BasicAgent(Agent):
    """
    BasicAgent implements an agent that navigates the scene.
    This agent respects traffic lights and other vehicles, but ignores stop signs.
    It has several functions available to specify the route that the agent must follow
    """

    def __init__(self, vehicle, target_speed=20):
        """
        :param vehicle: actor to apply to local planner logic onto
        :param target_speed: speed (in Km/h) at which the vehicle will move
        """
        super(BasicAgent, self).__init__(vehicle)

        self._state = AgentState.NAVIGATING
        self._target_speed = target_speed
        self._sampling_resolution = 2.0
        self._grp = None

        self._base_tlight_threshold = 5.0  # meters
        self._base_vehicle_threshold = 5.0  # meters
        self._max_brake = 0.5
        self._local_planner = LocalPlanner(
            self._vehicle,
            opt_dict={
                'target_speed' : target_speed,
                'max_brake': self._max_brake
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

    def get_local_planner(self):
        """
        Get method for protected member local planner
        """
        return self._local_planner

    def set_destination(self, end_location, start_location=None):
        """
        This method creates a list of waypoints from agent's position to destination location
        based on the route returned by the global router
        """
        if not start_location:
            start_location = self._vehicle.get_location()

        start_waypoint = self._map.get_waypoint(self._vehicle.get_location())
        end_waypoint = self._map.get_waypoint(end_location)

        route_trace = self._trace_route(start_waypoint, end_waypoint)
        self._local_planner.set_global_plan(route_trace)

    def _trace_route(self, start_waypoint, end_waypoint):
        """
        This method sets up a global router and returns the optimal route
        from start_waypoint to end_waypoint

            :param start_waypoint: initial position
            :param end_waypoint: final position
        """

        # Setting up global router
        if self._grp is None:
            dao = GlobalRoutePlannerDAO(self._vehicle.get_world().get_map(), self._sampling_resolution)
            grp = GlobalRoutePlanner(dao)
            grp.setup()
            self._grp = grp

        # Obtain route plan
        route = self._grp.trace_route(
            start_waypoint.transform.location,
            end_waypoint.transform.location)

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
