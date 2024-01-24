#!/usr/bin/env python

# Copyright (c) 2020 Intel Corporation
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
This module provides an example control for pedestrians
"""

import math

import carla

from srunner.scenariomanager.actorcontrols.basic_control import BasicControl


class PedestrianControl(BasicControl):

    """
    Controller class for pedestrians derived from BasicControl.

    Args:
        actor (carla.Actor): Pedestrian actor that should be controlled.
    """

    def __init__(self, actor, args=None):
        if not isinstance(actor, carla.Walker):
            raise RuntimeError("PedestrianControl: The to be controlled actor is not a pedestrian")

        super(PedestrianControl, self).__init__(actor)

    def reset(self):
        """
        Reset the controller
        """
        if self._actor and self._actor.is_alive:
            self._actor = None

    def run_step(self):
        """
        Execute on tick of the controller's control loop

        Note: Walkers / pedestrians are not able to walk backwards.

        If _waypoints are provided, the pedestrian moves towards the next waypoint
        with the given _target_speed, until reaching the final waypoint. Upon reaching
        the final waypoint, _reached_goal is set to True.

        If _waypoints is empty, the pedestrians moves in its current direction with
        the given _target_speed.
        """
        if not self._actor or not self._actor.is_alive:
            return

        control = self._actor.get_control()
        control.speed = self._target_speed

        # If target speed is negavite, raise an exception
        if self._target_speed < 0:
            raise NotImplementedError("Negative target speeds are not yet supported")

        if self._waypoints:
            self._reached_goal = False
            location = self._waypoints[0].location
            direction = location - self._actor.get_location()
            direction_norm = math.sqrt(direction.x**2 + direction.y**2)
            control.direction = direction / direction_norm
            self._actor.apply_control(control)
            if direction_norm < 1.0:
                self._waypoints = self._waypoints[1:]
                if not self._waypoints:
                    self._reached_goal = True
        else:
            control.direction = self._actor.get_transform().rotation.get_forward_vector()
            self._actor.apply_control(control)
