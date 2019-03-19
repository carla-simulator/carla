#!/usr/bin/env python

# Copyright (c) 2018 Intel Labs.
# authors: German Ros (german.ros@intel.com)
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

""" This module contains a local planner to perform low-level waypoint following based on PID controllers. """

from enum import Enum
from collections import deque
import random

import carla
from agents.navigation.controller import VehiclePIDController
from agents.tools.misc import distance_vehicle, draw_waypoints


class RoadOption(Enum):
    """
    RoadOption represents the possible topological configurations when moving from a segment of lane to other.
    """
    VOID = -1
    LEFT = 1
    RIGHT = 2
    STRAIGHT = 3
    LANEFOLLOW = 4


class LocalPlanner(object):
    """
    LocalPlanner implements the basic behavior of following a trajectory of waypoints that is generated on-the-fly.
    The low-level motion of the vehicle is computed by using two PID controllers, one is used for the lateral control
    and the other for the longitudinal control (cruise speed).

    When multiple paths are available (intersections) this local planner makes a random choice.
    """

    # minimum distance to target waypoint as a percentage (e.g. within 90% of
    # total distance)
    MIN_DISTANCE_PERCENTAGE = 0.9

    def __init__(self, vehicle, opt_dict=None):
        """
        :param vehicle: actor to apply to local planner logic onto
        :param opt_dict: dictionary of arguments with the following semantics:
            dt -- time difference between physics control in seconds. This is typically fixed from server side
                  using the arguments -benchmark -fps=F . In this case dt = 1/F

            target_speed -- desired cruise speed in Km/h

            sampling_radius -- search radius for next waypoints in seconds: e.g. 0.5 seconds ahead

            lateral_control_dict -- dictionary of arguments to setup the lateral PID controller
                                    {'K_P':, 'K_D':, 'K_I':, 'dt'}

            longitudinal_control_dict -- dictionary of arguments to setup the longitudinal PID controller
                                        {'K_P':, 'K_D':, 'K_I':, 'dt'}
        """
        self._vehicle = vehicle
        self._map = self._vehicle.get_world().get_map()

        self._buffer_size = 5
        self.dt = None
        self.target_speed = None
        self.sampling_radius = None
        self.min_distance = None
        self.current_waypoint = None
        self.target_road_option = None
        self.next_waypoints = None
        self.target_waypoint = None
        self.vehicle_controller = None
        self.global_plan = None
        # queue with tuples of (waypoint, RoadOption)
        self.waypoints_queue = deque(maxlen=600)
        self.waypoint_buffer = deque(maxlen=self._buffer_size)

        # initializing controller
        self.init_controller(opt_dict)

    def __del__(self):
        if self._vehicle:
            self._vehicle.destroy()
        print("Destroying ego-vehicle!")

    def reset_vehicle(self):
        self._vehicle = None
        print("Resetting ego-vehicle!")

    def init_controller(self, opt_dict):
        """
        Controller initialization.

        :param opt_dict: dictionary of arguments.
        :return:
        """
        # default params
        self.dt = 1.0 / 20.0
        self.target_speed = 20.0  # Km/h
        self.sampling_radius = self.target_speed * 0.5 / 3.6  # 0.5 seconds horizon
        self.min_distance = self.sampling_radius * self.MIN_DISTANCE_PERCENTAGE
        args_lateral_dict = {
            'K_P': 1.95,
            'K_D': 0.01,
            'K_I': 1.4,
            'dt': self.dt}
        args_longitudinal_dict = {
            'K_P': 1.0,
            'K_D': 0,
            'K_I': 1,
            'dt': self.dt}

        # parameters overload
        if opt_dict:
            if 'dt' in opt_dict:
                self.dt = opt_dict['dt']
            if 'target_speed' in opt_dict:
                self.target_speed = opt_dict['target_speed']
            if 'sampling_radius' in opt_dict:
                self.sampling_radius = self.target_speed * \
                    opt_dict['sampling_radius'] / 3.6
            if 'lateral_control_dict' in opt_dict:
                args_lateral_dict = opt_dict['lateral_control_dict']
            if 'longitudinal_control_dict' in opt_dict:
                args_longitudinal_dict = opt_dict['longitudinal_control_dict']

        self.current_waypoint = self._map.get_waypoint(self._vehicle.get_location())
        self.vehicle_controller = VehiclePIDController(self._vehicle,
                                                       args_lateral=args_lateral_dict,
                                                       args_longitudinal=args_longitudinal_dict)

        self.global_plan = False

        # compute initial waypoints
        self.waypoints_queue.append((self.current_waypoint.next(self.sampling_radius)[0], RoadOption.LANEFOLLOW))

        self.target_road_option = RoadOption.LANEFOLLOW
        # fill waypoint trajectory queue
        self._compute_next_waypoints(k=200)

    def set_speed(self, speed):
        """
        Request new target speed.

        :param speed: new target speed in Km/h
        :return:
        """
        self.target_speed = speed

    def _compute_next_waypoints(self, k=1):
        """
        Add new waypoints to the trajectory queue.

        :param k: how many waypoints to compute
        :return:
        """
        # check we do not overflow the queue
        available_entries = self.waypoints_queue.maxlen - len(self.waypoints_queue)
        k = min(available_entries, k)

        for _ in range(k):
            last_waypoint = self.waypoints_queue[-1][0]
            next_waypoints = list(last_waypoint.next(self.sampling_radius))

            if len(next_waypoints) == 1:
                # only one option available ==> lanefollowing
                next_waypoint = next_waypoints[0]
                road_option = RoadOption.LANEFOLLOW
            else:
                # random choice between the possible options
                road_options_list = retrieve_options(
                    next_waypoints, last_waypoint)
                road_option = random.choice(road_options_list)
                next_waypoint = next_waypoints[road_options_list.index(
                    road_option)]

            self.waypoints_queue.append((next_waypoint, road_option))

    def set_global_plan(self, current_plan):
        self.waypoints_queue.clear()
        for elem in current_plan:
            self.waypoints_queue.append(elem)
        self.target_road_option = RoadOption.LANEFOLLOW
        self.global_plan = True

    def run_step(self, debug=True):
        """
        Execute one step of local planning which involves running the longitudinal and lateral PID controllers to
        follow the waypoints trajectory.

        :param debug: boolean flag to activate waypoints debugging
        :return:
        """

        # not enough waypoints in the horizon? => add more!
        if len(self.waypoints_queue) < int(self.waypoints_queue.maxlen * 0.5):
            if not self.global_plan:
                self._compute_next_waypoints(k=100)

        if len(self.waypoints_queue) == 0:
            control = carla.VehicleControl()
            control.steer = 0.0
            control.throttle = 0.0
            control.brake = 0.0
            control.hand_brake = False
            control.manual_gear_shift = False

            return control

        #   Buffering the waypoints
        if not self.waypoint_buffer:
            for i in range(self._buffer_size):
                if self.waypoints_queue:
                    self.waypoint_buffer.append(
                        self.waypoints_queue.popleft())
                else:
                    break

        # current vehicle waypoint
        self.current_waypoint = self._map.get_waypoint(self._vehicle.get_location())
        # target waypoint
        self.target_waypoint, self.target_road_option = self.waypoint_buffer[0]
        # move using PID controllers
        control = self.vehicle_controller.run_step(self.target_speed, self.target_waypoint)

        # purge the queue of obsolete waypoints
        vehicle_transform = self._vehicle.get_transform()
        max_index = -1

        for i, (waypoint, _) in enumerate(self.waypoint_buffer):
            if distance_vehicle(
                    waypoint, vehicle_transform) < self.min_distance:
                max_index = i
        if max_index >= 0:
            for i in range(max_index + 1):
                self.waypoint_buffer.popleft()

        if debug:
            draw_waypoints(self._vehicle.get_world(), [self.target_waypoint], self._vehicle.get_location().z + 1.0)

        return control


def retrieve_options(list_waypoints, current_waypoint):
    """
    Compute the type of connection between the current active waypoint and the multiple waypoints present in
    list_waypoints. The result is encoded as a list of RoadOption enums.

    :param list_waypoints: list with the possible target waypoints in case of multiple options
    :param current_waypoint: current active waypoint
    :return: list of RoadOption enums representing the type of connection from the active waypoint to each
             candidate in list_waypoints
    """
    options = []
    for next_waypoint in list_waypoints:
        # this is needed because something we are linking to
        # the beggining of an intersection, therefore the
        # variation in angle is small
        next_next_waypoint = next_waypoint.next(3.0)[0]
        link = compute_connection(current_waypoint, next_next_waypoint)
        options.append(link)

    return options


def compute_connection(current_waypoint, next_waypoint):
    """
    Compute the type of topological connection between an active waypoint (current_waypoint) and a target waypoint
    (next_waypoint).

    :param current_waypoint: active waypoint
    :param next_waypoint: target waypoint
    :return: the type of topological connection encoded as a RoadOption enum:
             RoadOption.STRAIGHT
             RoadOption.LEFT
             RoadOption.RIGHT
    """
    n = next_waypoint.transform.rotation.yaw
    n = n % 360.0

    c = current_waypoint.transform.rotation.yaw
    c = c % 360.0

    diff_angle = (n - c) % 180.0
    if diff_angle < 1.0:
        return RoadOption.STRAIGHT
    elif diff_angle > 90.0:
        return RoadOption.LEFT
    else:
        return RoadOption.RIGHT
