#!/usr/bin/env python

# Copyright (c) 2020 Intel Corporation
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
This module provides an example longitudinal control for vehicles
"""

import math

import carla

from srunner.scenariomanager.actorcontrols.basic_control import BasicControl


class VehicleLongitudinalControl(BasicControl):

    """
    Controller class for vehicles derived from BasicControl.

    The controller only controls the throttle of a vehicle, but not the steering.

    Args:
        actor (carla.Actor): Vehicle actor that should be controlled.
    """

    def __init__(self, actor, args=None):
        super(VehicleLongitudinalControl, self).__init__(actor)

    def reset(self):
        """
        Reset the controller
        """
        if self._actor and self._actor.is_alive:
            self._actor = None

    def run_step(self):
        """
        Execute on tick of the controller's control loop

        The control loop is very simplistic:
            If the actor speed is below the _target_speed, set throttle to 1.0,
            otherwise, set throttle to 0.0
        Note: This is a longitudinal controller only.

        If _init_speed is True, the control command is post-processed to ensure that
        the initial actor velocity is maintained independent of physics.
        """

        control = self._actor.get_control()

        velocity = self._actor.get_velocity()
        current_speed = math.sqrt(velocity.x**2 + velocity.y**2)
        if current_speed < self._target_speed and self._target_speed >= 0:
            control.reverse = False
            control.throttle = 1.0
        elif current_speed > self._target_speed and self._target_speed < 0:
            control.reverse = True
            control.throttle = 1.0
        else:
            control.throttle = 0.0

        self._actor.apply_control(control)

        if self._init_speed:
            if abs(self._target_speed - current_speed) > 3:
                yaw = self._actor.get_transform().rotation.yaw * (math.pi / 180)
                vx = math.cos(yaw) * self._target_speed
                vy = math.sin(yaw) * self._target_speed
                self._actor.set_target_velocity(carla.Vector3D(vx, vy, 0))
