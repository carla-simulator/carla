#!/usr/bin/env python

# Copyright (c) 2020 Intel Corporation
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
This module provides the base class for user-defined actor
controllers. All user-defined controls must be derived from
this class.

A user must not modify the module.
"""


class BasicControl(object):

    """
    This class is the base class for user-defined actor controllers
    All user-defined agents must be derived from this class.

    Args:
        actor (carla.Actor): Actor that should be controlled by the controller.

    Attributes:
        _actor (carla.Actor): Controlled actor.
            Defaults to None.
        _target_speed (float): Logitudinal target speed of the controller.
            Defaults to 0.
        _init_speed (float): Initial longitudinal speed of the controller.
            Defaults to 0.
        _waypoints (list of carla.Transform): List of target waypoints the actor
            should travel along. A waypoint here is of type carla.Transform!
            Defaults to [].
        _waypoints_updated (boolean):
            Defaults to False.
        _reached_goal (boolean):
            Defaults to False.
    """

    _actor = None
    _waypoints = []
    _waypoints_updated = False
    _offset = 0
    _offset_updated = False
    _target_speed = 0
    _reached_goal = False
    _init_speed = False

    def __init__(self, actor):
        """
        Initialize the actor
        """
        self._actor = actor

    def update_target_speed(self, speed):
        """
        Update the actor's target speed and set _init_speed to False.

        Args:
            speed (float): New target speed [m/s].
        """
        self._target_speed = speed
        self._init_speed = False

    def update_waypoints(self, waypoints, start_time=None):
        """
        Update the actor's waypoints

        Args:
            waypoints (List of carla.Transform): List of new waypoints.
        """
        self._waypoints = waypoints
        self._waypoints_updated = True

    def update_offset(self, offset, start_time=None):
        """
        Update the actor's waypoints

        Args:
            waypoints (List of carla.Transform): List of new waypoints.
        """
        self._offset = offset
        self._offset_updated = True

    def set_init_speed(self):
        """
        Set _init_speed to True
        """
        self._init_speed = True

    def check_reached_waypoint_goal(self):
        """
        Check if the actor reached the end of the waypoint list

        returns:
            True if the end was reached, False otherwise.
        """
        return self._reached_goal

    def reset(self):
        """
        Pure virtual function to reset the controller. This should be implemented
        in the user-defined agent implementation.
        """
        raise NotImplementedError(
            "This function must be re-implemented by the user-defined actor control."
            "If this error becomes visible the class hierarchy is somehow broken")

    def run_step(self):
        """
        Pure virtual function to run one step of the controllers's control loop.
        This should be implemented in the user-defined agent implementation.
        """
        raise NotImplementedError(
            "This function must be re-implemented by the user-defined actor control."
            "If this error becomes visible the class hierarchy is somehow broken")
