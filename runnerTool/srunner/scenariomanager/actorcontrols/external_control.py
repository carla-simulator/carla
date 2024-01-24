#!/usr/bin/env python

# Copyright (c) 2020 Intel Corporation
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
This module provides an example controller for actors, that use an external
software for longitudinal and lateral control command calculation.
Examples for external controls are: Autoware, CARLA manual_control, etc.

This module is not intended for modification.
"""

from srunner.scenariomanager.actorcontrols.basic_control import BasicControl


class ExternalControl(BasicControl):

    """
    Actor control class for actors, with externally implemented longitudinal and
    lateral controlers (e.g. Autoware).

    Args:
        actor (carla.Actor): Actor that should be controlled by the agent.
    """

    def __init__(self, actor, args=None):
        super(ExternalControl, self).__init__(actor)

    def reset(self):
        """
        Reset the controller
        """
        if self._actor and self._actor.is_alive:
            self._actor = None

    def run_step(self):
        """
        The control loop and setting the actor controls is implemented externally.
        """
        pass
