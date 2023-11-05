#!/usr/bin/env python

# Copyright (c) 2020 Intel Corporation
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
This module provides a wrapper to access/use user-defined actor
controls for example to realize OpenSCENARIO controllers.

At the moment only controls implemented in Python are supported.

A user must not modify this module.
"""

import importlib
import os
import sys

import carla

from srunner.scenariomanager.actorcontrols.external_control import ExternalControl
from srunner.scenariomanager.actorcontrols.npc_vehicle_control import NpcVehicleControl
from srunner.scenariomanager.actorcontrols.pedestrian_control import PedestrianControl


class ActorControl(object):

    """
    This class provides a wrapper (access mechanism) for user-defined actor controls.
    The controllers are loaded via importlib. Therefore, the module name of the controller
    has to match the control class name (e.g. my_own_control.py and MyOwnControl()).

    At the moment only controllers implemented in Python are supported, or controllers that
    are completely implemented outside of ScenarioRunner (see ExternalControl).

    This wrapper is for example used to realize the OpenSCENARIO controllers.

    Note:
       If no controllers are provided in OpenSCENARIO a default controller for vehicles and
       pedestrians is instantiated. For vehicles the NpcVehicleControl is used, for pedestrians
       it is the PedestrianControl.

    Args:
        actor (carla.Actor): Actor that should be controlled by the controller.
        control_py_module (string): Fully qualified path to the controller python module.
        args (dict): A dictionary containing all parameters of the controller as (key, value) pairs.
        scenario_file_path (string): Path to search for the controller implementation.

    Attributes:
        control_instance: Instance of the user-defined controller.
        _last_longitudinal_command: Timestamp of the last issued longitudinal control command (e.g. target speed).
            Defaults to None. Used to avoid that 2 longitudinal control commands are issued at the same time.
        _last_waypoint_command: Timestamp of the last issued waypoint control command.
            Defaults to None. Used to avoid that 2 waypoint control commands are issued at the same time.
    """

    control_instance = None

    _last_longitudinal_command = None
    _last_waypoint_command = None
    _last_lane_offset_command = None

    def __init__(self, actor, control_py_module, args, scenario_file_path):

        # use importlib to import the control module
        if not control_py_module:
            if isinstance(actor, carla.Walker):
                self.control_instance = PedestrianControl(actor)
            elif isinstance(actor, carla.Vehicle):
                self.control_instance = NpcVehicleControl(actor)
            else:
                # use ExternalControl for all misc objects to handle all actors the same way
                self.control_instance = ExternalControl(actor)
        else:
            if scenario_file_path:
                sys.path.append(scenario_file_path)
            if ".py" in control_py_module:
                module_name = os.path.basename(control_py_module).split('.')[0]
                sys.path.append(os.path.dirname(control_py_module))
                module_control = importlib.import_module(module_name)
                control_class_name = module_control.__name__.title().replace('_', '')
            else:
                sys.path.append(os.path.dirname(__file__))
                module_control = importlib.import_module(control_py_module)
                control_class_name = control_py_module.split('.')[-1].title().replace('_', '')

            self.control_instance = getattr(module_control, control_class_name)(actor, args)

    def reset(self):
        """
        Reset the controller
        """
        self.control_instance.reset()

    def update_target_speed(self, target_speed, start_time=None):
        """
        Update the actor's target speed.

        Args:
            target_speed (float): New target speed [m/s].
            start_time (float): Start time of the new "maneuver" [s].
        """
        self.control_instance.update_target_speed(target_speed)
        if start_time:
            self._last_longitudinal_command = start_time

    def update_waypoints(self, waypoints, start_time=None):
        """
        Update the actor's waypoints

        Args:
            waypoints (List of carla.Transform): List of new waypoints.
            start_time (float): Start time of the new "maneuver" [s].
        """
        self.control_instance.update_waypoints(waypoints)
        if start_time:
            self._last_waypoint_command = start_time

    def update_offset(self, offset, start_time=None):
        """
        Update the actor's offset

        Args:
            offset (float): Value of the new offset.
            start_time (float): Start time of the new "maneuver" [s].
        """
        self.control_instance.update_offset(offset)
        if start_time:
            self._last_waypoint_command = start_time
            self._last_lane_offset_command = start_time

    def check_reached_waypoint_goal(self):
        """
        Check if the actor reached the end of the waypoint list

        returns:
            True if the end was reached, False otherwise.
        """
        return self.control_instance.check_reached_waypoint_goal()

    def get_last_longitudinal_command(self):
        """
        Get timestamp of the last issued longitudinal control command (target_speed)

        returns:
            Timestamp of last longitudinal control command
        """
        return self._last_longitudinal_command

    def get_last_waypoint_command(self):
        """
        Get timestamp of the last issued waypoint control command

        returns:
            Timestamp of last waypoint control command
        """
        return self._last_waypoint_command

    def get_last_lane_offset_command(self):
        """
        Get timestamp of the last issued lane offset control command

        returns:
            Timestamp of last lane offset control command
        """
        return self._last_lane_offset_command

    def set_init_speed(self):
        """
        Update the actor's initial speed setting
        """
        self.control_instance.set_init_speed()

    def run_step(self):
        """
        Execute on tick of the controller's control loop
        """
        self.control_instance.run_step()
