#!/usr/bin/env python

# Copyright (c) 2020 Intel Corporation
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
This module provides an example control for vehicles which
use CARLA's autopilot functionality

Limitations:
- No direct velocity control
- No lateral maneuvers can be enforced
"""


from srunner.scenariomanager.actorcontrols.basic_control import BasicControl


class CarlaAutoPilotControl(BasicControl):

    """
    Controller class for vehicles derived from BasicControl.

    The controller uses CARLA's autopilot functionality. As a result,
    the vehicle respects other traffic participants and traffic rules.
    However, no longitudinal or lateral maneuvers can be enforced.

    Args:
        actor (carla.Actor): Vehicle actor that should be controlled.
        args (dictionary): Dictonary of (key, value) arguments to be used by the controller.
    """

    def __init__(self, actor, args=None):
        super(CarlaAutoPilotControl, self).__init__(actor)
        self._actor.set_autopilot(enabled=True)

    def reset(self):
        """
        Reset the controller
        """
        self._actor.set_autopilot(enabled=False)

    def run_step(self):
        """
        Everything is controlled through CARLA's autopilot functionality.
        Nothing to do here
        """
        pass
