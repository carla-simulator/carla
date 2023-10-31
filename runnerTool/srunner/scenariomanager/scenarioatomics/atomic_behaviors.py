#!/usr/bin/env python

# Copyright (c) 2018-2020 Intel Corporation
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
This module provides all atomic scenario behaviors required to realize
complex, realistic scenarios such as "follow a leading vehicle", "lane change",
etc.

The atomic behaviors are implemented with py_trees.
"""

from __future__ import print_function

import copy
import math
import operator
import os
import time
import subprocess
from bisect import bisect_right

import numpy as np
from numpy import random
import py_trees
from py_trees.blackboard import Blackboard
import networkx

import carla
from agents.navigation.basic_agent import BasicAgent
from agents.navigation.local_planner import RoadOption, LocalPlanner
from agents.navigation.global_route_planner import GlobalRoutePlanner
from agents.tools.misc import is_within_distance

from srunner.scenariomanager.carla_data_provider import CarlaDataProvider
from srunner.scenariomanager.actorcontrols.actor_control import ActorControl
from srunner.scenariomanager.timer import GameTime
from srunner.tools.scenario_helper import detect_lane_obstacle
from srunner.tools.scenario_helper import generate_target_waypoint_list_multilane


import srunner.tools as sr_tools

EPSILON = 0.001


def calculate_distance(location, other_location, global_planner=None):
    """
    Method to calculate the distance between to locations

    Note: It uses the direct distance between the current location and the
          target location to estimate the time to arrival.
          To be accurate, it would have to use the distance along the
          (shortest) route between the two locations.
    """
    if global_planner:
        distance = 0

        # Get the route
        route = global_planner.trace_route(location, other_location)

        # Get the distance of the route
        for i in range(1, len(route)):
            curr_loc = route[i][0].transform.location
            prev_loc = route[i - 1][0].transform.location

            distance += curr_loc.distance(prev_loc)

        return distance

    return location.distance(other_location)


def get_actor_control(actor):
    """
    Method to return the type of control to the actor.
    """
    control = actor.get_control()
    actor_type = actor.type_id.split('.')[0]
    if not isinstance(actor, carla.Walker):
        control.steering = 0
    else:
        control.speed = 0

    return control, actor_type


class AtomicBehavior(py_trees.behaviour.Behaviour):

    """
    Base class for all atomic behaviors used to setup a scenario

    *All behaviors should use this class as parent*

    Important parameters:
    - name: Name of the atomic behavior
    """

    def __init__(self, name, actor=None):
        """
        Default init. Has to be called via super from derived class
        """
        super(AtomicBehavior, self).__init__(name)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))
        self.name = name
        self._actor = actor

    def setup(self, unused_timeout=15):
        """
        Default setup
        """
        self.logger.debug("%s.setup()" % (self.__class__.__name__))
        return True

    def initialise(self):
        """
        Initialise setup terminates WaypointFollowers
        Check whether WF for this actor is running and terminate all active WFs
        """
        if self._actor is not None:
            try:
                check_attr = operator.attrgetter("running_WF_actor_{}".format(self._actor.id))
                terminate_wf = copy.copy(check_attr(py_trees.blackboard.Blackboard()))
                py_trees.blackboard.Blackboard().set(
                    "terminate_WF_actor_{}".format(self._actor.id), terminate_wf, overwrite=True)
            except AttributeError:
                # It is ok to continue, if the Blackboard variable does not exist
                pass
        self.logger.debug("%s.initialise()" % (self.__class__.__name__))

    def terminate(self, new_status):
        """
        Default terminate. Can be extended in derived class
        """
        self.logger.debug("%s.terminate()[%s->%s]" % (self.__class__.__name__, self.status, new_status))


class RunScript(AtomicBehavior):

    """
    This is an atomic behavior to start execution of an additional script.

    Args:
        script (str): String containing the interpreter, scriptpath and arguments
            Example: "python /path/to/script.py --arg1"
        base_path (str): String containing the base path of for the script

    Attributes:
        _script (str): String containing the interpreter, scriptpath and arguments
            Example: "python /path/to/script.py --arg1"
        _base_path (str): String containing the base path of for the script
            Example: "/path/to/"

    Note:
        This is intended for the use with OpenSCENARIO. Be aware of security side effects.
    """

    def __init__(self, script, base_path=None, name="RunScript"):
        """
        Setup parameters
        """
        super(RunScript, self).__init__(name)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))
        self._script = script
        self._base_path = base_path

    def update(self):
        """
        Start script
        """
        path = None
        script_components = self._script.split(' ')
        if len(script_components) > 1:
            path = script_components[1]

        if not os.path.isfile(path):
            path = os.path.join(self._base_path, path)
        if not os.path.isfile(path):
            new_status = py_trees.common.Status.FAILURE
            print("Script file does not exists {}".format(path))
        else:
            subprocess.Popen(self._script, shell=True, cwd=self._base_path)  # pylint: disable=consider-using-with
            new_status = py_trees.common.Status.SUCCESS

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))
        return new_status


class ChangeParameter(AtomicBehavior):
    """
    This is an atomic behavior to change the osc parameter value.

    Args:
        parameter_ref (str): parameter name
        value (any): ParameterRef or number
    """

    def __init__(self, parameter_ref, value, rule=None, name="ChangeParameter"):
        super(ChangeParameter, self).__init__(name)
        self.logger.debug("%s.__init__()" % self.__class__.__name__)
        self._parameter_ref = parameter_ref
        self._rule = rule
        self._value = value

    def update(self):
        """
        update value of global osc parameter.
        """
        old_value = CarlaDataProvider.get_osc_global_param_value(self._parameter_ref)

        if self._rule == '+':
            new_value = self._value + float(old_value)
        elif self._rule == '*':
            new_value = self._value * float(old_value)
        else:
            new_value = self._value

        CarlaDataProvider.update_osc_global_params({self._parameter_ref: new_value})
        new_status = py_trees.common.Status.SUCCESS

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))
        return new_status


class ChangeWeather(AtomicBehavior):

    """
    Atomic to write a new weather configuration into the blackboard.
    Used in combination with WeatherBehavior() to have a continuous weather simulation.

    The behavior immediately terminates with SUCCESS after updating the blackboard.

    Args:
        weather (srunner.scenariomanager.weather_sim.Weather): New weather settings.
        name (string): Name of the behavior.
            Defaults to 'UpdateWeather'.

    Attributes:
        _weather (srunner.scenariomanager.weather_sim.Weather): Weather settings.
    """

    def __init__(self, weather, name="ChangeWeather"):
        """
        Setup parameters
        """
        super(ChangeWeather, self).__init__(name)
        self._weather = weather

    def update(self):
        """
        Write weather into blackboard and exit with success

        returns:
            py_trees.common.Status.SUCCESS
        """
        py_trees.blackboard.Blackboard().set("CarlaWeather", self._weather, overwrite=True)
        return py_trees.common.Status.SUCCESS


class ChangeRoadFriction(AtomicBehavior):

    """
    Atomic to update the road friction in CARLA.

    The behavior immediately terminates with SUCCESS after updating the friction.

    Args:
        friction (float): New friction coefficient.
        name (string): Name of the behavior.
            Defaults to 'UpdateRoadFriction'.

    Attributes:
        _friction (float): Friction coefficient.
    """

    def __init__(self, friction, name="ChangeRoadFriction"):
        """
        Setup parameters
        """
        super(ChangeRoadFriction, self).__init__(name)
        self._friction = friction

    def update(self):
        """
        Update road friction. Spawns new friction blueprint and removes the old one, if existing.

        returns:
            py_trees.common.Status.SUCCESS
        """

        for actor in CarlaDataProvider.get_world().get_actors().filter('static.trigger.friction'):
            actor.destroy()

        friction_bp = CarlaDataProvider.get_world().get_blueprint_library().find('static.trigger.friction')
        extent = carla.Location(1000000.0, 1000000.0, 1000000.0)
        friction_bp.set_attribute('friction', str(self._friction))
        friction_bp.set_attribute('extent_x', str(extent.x))
        friction_bp.set_attribute('extent_y', str(extent.y))
        friction_bp.set_attribute('extent_z', str(extent.z))

        # Spawn Trigger Friction
        transform = carla.Transform()
        transform.location = carla.Location(-10000.0, -10000.0, 0.0)
        CarlaDataProvider.get_world().spawn_actor(friction_bp, transform)

        return py_trees.common.Status.SUCCESS


class ChangeActorControl(AtomicBehavior):

    """
    Atomic to change the longitudinal/lateral control logic for an actor.
    The (actor, controller) pair is stored inside the Blackboard.

    The behavior immediately terminates with SUCCESS after the controller.

    Args:
        actor (carla.Actor): Actor that should be controlled by the controller.
        control_py_module (string): Name of the python module containing the implementation
            of the controller.
        args (dictionary): Additional arguments for the controller.
        scenario_file_path (string): Additional path to controller implementation.
        name (string): Name of the behavior.
            Defaults to 'ChangeActorControl'.

    Attributes:
        _actor_control (ActorControl): Instance of the actor control.
    """

    def __init__(self, actor, control_py_module, args, scenario_file_path=None, name="ChangeActorControl"):
        """
        Setup actor controller.
        """
        super(ChangeActorControl, self).__init__(name, actor)

        self._actor_control = ActorControl(actor, control_py_module=control_py_module,
                                           args=args, scenario_file_path=scenario_file_path)

    def update(self):
        """
        Write (actor, controler) pair to Blackboard, or update the controller
        if actor already exists as a key.

        returns:
            py_trees.common.Status.SUCCESS
        """

        actor_dict = {}

        try:
            check_actors = operator.attrgetter("ActorsWithController")
            actor_dict = check_actors(py_trees.blackboard.Blackboard())
        except AttributeError:
            pass

        if actor_dict:
            if self._actor.id in actor_dict:
                actor_dict[self._actor.id].reset()

        actor_dict[self._actor.id] = self._actor_control
        py_trees.blackboard.Blackboard().set("ActorsWithController", actor_dict, overwrite=True)

        return py_trees.common.Status.SUCCESS


class UpdateAllActorControls(AtomicBehavior):

    """
    Atomic to update (run one control loop step) all actor controls.

    The behavior is always in RUNNING state.

    Args:
        name (string): Name of the behavior.
            Defaults to 'UpdateAllActorControls'.
    """

    def __init__(self, name="UpdateAllActorControls"):
        """
        Constructor
        """
        super(UpdateAllActorControls, self).__init__(name)

    def update(self):
        """
        Execute one control loop step for all actor controls.

        returns:
            py_trees.common.Status.RUNNING
        """

        actor_dict = {}

        try:
            check_actors = operator.attrgetter("ActorsWithController")
            actor_dict = check_actors(py_trees.blackboard.Blackboard())
        except AttributeError:
            pass

        for actor_id in actor_dict:
            actor_dict[actor_id].run_step()

        return py_trees.common.Status.RUNNING


class ChangeActorTargetSpeed(AtomicBehavior):

    """
    Atomic to change the target speed for an actor controller.

    The behavior is in RUNNING state until the distance/duration
    conditions are satisfied, or if a second ChangeActorTargetSpeed atomic
    for the same actor is triggered.

    Args:
        actor (carla.Actor): Controlled actor.
        target_speed (float): New target speed [m/s].
        init_speed (boolean): Flag to indicate if the speed is the initial actor speed.
            Defaults to False.
        duration (float): Duration of the maneuver [s].
            Defaults to None.
        distance (float): Distance of the maneuver [m].
            Defaults to None.
        relative_actor (carla.Actor): If the target speed setting should be relative to another actor.
            Defaults to None.
        value (float): Offset, if the target speed setting should be relative to another actor.
            Defaults to None.
        value_type (string): Either 'Delta' or 'Factor' influencing how the offset to the reference actors
            velocity is applied. Defaults to None.
        continuous (boolean): If True, the atomic remains in RUNNING, independent of duration or distance.
            Defaults to False.
        name (string): Name of the behavior.
            Defaults to 'ChangeActorTargetSpeed'.

    Attributes:
        _target_speed (float): New target speed [m/s].
        _init_speed (boolean): Flag to indicate if the speed is the initial actor speed.
            Defaults to False.
        _start_time (float): Start time of the atomic [s].
            Defaults to None.
        _start_location (carla.Location): Start location of the atomic.
            Defaults to None.
        _duration (float): Duration of the maneuver [s].
            Defaults to None.
        _distance (float): Distance of the maneuver [m].
            Defaults to None.
        _relative_actor (carla.Actor): If the target speed setting should be relative to another actor.
            Defaults to None.
        _value (float): Offset, if the target speed setting should be relative to another actor.
            Defaults to None.
        _value_type (string): Either 'Delta' or 'Factor' influencing how the offset to the reference actors
            velocity is applied. Defaults to None.
        _continuous (boolean): If True, the atomic remains in RUNNING, independent of duration or distance.
            Defaults to False.
    """

    def __init__(self, actor, target_speed, init_speed=False,
                 duration=None, distance=None, relative_actor=None,
                 value=None, value_type=None, continuous=False, name="ChangeActorTargetSpeed"):
        """
        Setup parameters
        """
        super(ChangeActorTargetSpeed, self).__init__(name, actor)

        self._target_speed = target_speed
        self._init_speed = init_speed

        self._start_time = None
        self._start_location = None

        self._relative_actor = relative_actor
        self._value = value
        self._value_type = value_type
        self._continuous = continuous
        self._duration = duration
        self._distance = distance

    def initialise(self):
        """
        Set initial parameters such as _start_time and _start_location,
        and get (actor, controller) pair from Blackboard.

        May throw if actor is not available as key for the ActorsWithController
        dictionary from Blackboard.
        """
        actor_dict = {}

        try:
            check_actors = operator.attrgetter("ActorsWithController")
            actor_dict = check_actors(py_trees.blackboard.Blackboard())
        except AttributeError:
            pass

        if not actor_dict or not self._actor.id in actor_dict:
            raise RuntimeError("Actor not found in ActorsWithController BlackBoard")

        self._start_time = GameTime.get_time()
        self._start_location = CarlaDataProvider.get_location(self._actor)

        if self._relative_actor:
            relative_velocity = CarlaDataProvider.get_velocity(self._relative_actor)

            # get target velocity
            if self._value_type == 'delta':
                self._target_speed = relative_velocity + self._value
            elif self._value_type == 'factor':
                self._target_speed = relative_velocity * self._value
            else:
                print('self._value_type must be delta or factor')

        actor_dict[self._actor.id].update_target_speed(self._target_speed, start_time=self._start_time)

        if self._init_speed:
            actor_dict[self._actor.id].set_init_speed()

        super(ChangeActorTargetSpeed, self).initialise()

    def update(self):
        """
        Check the actor's current state and update target speed, if it is relative to another actor.

        returns:
            py_trees.common.Status.SUCCESS, if the duration or distance driven exceeded limits
                                            if another ChangeActorTargetSpeed atomic for the same actor was triggered.
            py_trees.common.Status.FAILURE, if the actor is not found in ActorsWithController Blackboard dictionary.
            py_trees.common.Status.FAILURE, else.
        """
        try:
            check_actors = operator.attrgetter("ActorsWithController")
            actor_dict = check_actors(py_trees.blackboard.Blackboard())
        except AttributeError:
            pass

        if not actor_dict or not self._actor.id in actor_dict:
            return py_trees.common.Status.FAILURE

        if actor_dict[self._actor.id].get_last_longitudinal_command() != self._start_time:
            return py_trees.common.Status.SUCCESS

        new_status = py_trees.common.Status.RUNNING

        if self._relative_actor:
            relative_velocity = CarlaDataProvider.get_velocity(self._relative_actor)

            # get target velocity
            if self._value_type == 'delta':
                actor_dict[self._actor.id].update_target_speed(relative_velocity + self._value)
            elif self._value_type == 'factor':
                actor_dict[self._actor.id].update_target_speed(relative_velocity * self._value)
            else:
                print('self._value_type must be delta or factor')

        # check duration and driven_distance
        if not self._continuous:
            if (self._duration is not None) and (GameTime.get_time() - self._start_time > self._duration):
                new_status = py_trees.common.Status.SUCCESS

            driven_distance = CarlaDataProvider.get_location(self._actor).distance(self._start_location)
            if (self._distance is not None) and (driven_distance > self._distance):
                new_status = py_trees.common.Status.SUCCESS

        if self._distance is None and self._duration is None:
            new_status = py_trees.common.Status.SUCCESS

        return new_status


class SyncArrivalOSC(AtomicBehavior):

    """
    Atomic to make two actors arrive at their corresponding places at the same time

    The behavior is in RUNNING state until the "main" actor has rezached its destination

    Args:
        actor (carla.Actor): Controlled actor.
        master_actor (carla.Actor): Reference actor to sync up to.
        actor_target (carla.Transform): Endpoint of the actor after the behavior finishes.
        master_target (carla.Transform): Endpoint of the master_actor after the behavior finishes.
        final_speed (float): Speed of the actor after the behavior ends.
        relative_to_master (boolean): Whether or not the final speed is relative to master_actor.
            Defaults to False.
        relative_type (string): Type of relative speed. Either 'delta' or 'factor'.
            Defaults to ''.
        name (string): Name of the behavior.
            Defaults to 'SyncArrivalOSC'.
    """

    DISTANCE_THRESHOLD = 1

    def __init__(self, actor, master_actor, actor_target, master_target, final_speed,
                 relative_to_master=False, relative_type='', name="SyncArrivalOSC"):
        """
        Setup required parameters
        """
        super(SyncArrivalOSC, self).__init__(name, actor)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))

        self._actor = actor
        self._actor_target = actor_target
        self._master_actor = master_actor
        self._master_target = master_target

        self._final_speed = final_speed
        self._final_speed_set = False
        self._relative_to_master = relative_to_master
        self._relative_type = relative_type

        self._start_time = None

    def initialise(self):
        """
        Set initial parameters and get (actor, controller) pair from Blackboard.

        May throw if actor is not available as key for the ActorsWithController
        dictionary from Blackboard.
        """
        actor_dict = {}

        try:
            check_actors = operator.attrgetter("ActorsWithController")
            actor_dict = check_actors(py_trees.blackboard.Blackboard())
        except AttributeError:
            pass

        if not actor_dict or not self._actor.id in actor_dict:
            raise RuntimeError("Actor not found in ActorsWithController BlackBoard")

        self._start_time = GameTime.get_time()

        # Get the distance of the actor to its endpoint
        distance = calculate_distance(
            CarlaDataProvider.get_location(self._actor), self._actor_target.location)

        # Get the time to arrival of the reference to its endpoint
        distance_reference = calculate_distance(
            CarlaDataProvider.get_location(self._master_actor), self._master_target.location)

        velocity_reference = CarlaDataProvider.get_velocity(self._master_actor)
        if velocity_reference > 0:
            time_reference = distance_reference / velocity_reference
        else:
            time_reference = float('inf')

        # Get the required velocity of the actor
        desired_velocity = distance / time_reference
        actor_dict[self._actor.id].update_target_speed(desired_velocity, start_time=self._start_time)

    def update(self):
        """
        Dynamic control update for actor velocity to ensure that both actors reach their target
        positions at the same time.
        """

        try:
            check_actors = operator.attrgetter("ActorsWithController")
            actor_dict = check_actors(py_trees.blackboard.Blackboard())
        except AttributeError:
            pass

        if not actor_dict or not self._actor.id in actor_dict:
            return py_trees.common.Status.FAILURE

        if actor_dict[self._actor.id].get_last_longitudinal_command() != self._start_time:
            return py_trees.common.Status.SUCCESS

        new_status = py_trees.common.Status.RUNNING

        # Get the distance of the actor to its endpoint
        distance = calculate_distance(
            CarlaDataProvider.get_location(self._actor), self._actor_target.location)

        if distance < self.DISTANCE_THRESHOLD:
            return py_trees.common.Status.SUCCESS  # Behaviour ends when the actor reaches its endpoint

        # Get the time to arrival of the reference to its endpoint
        distance_reference = calculate_distance(
            CarlaDataProvider.get_location(self._master_actor), self._master_target.location)

        velocity_reference = CarlaDataProvider.get_velocity(self._master_actor)
        if velocity_reference > 0:
            time_reference = distance_reference / velocity_reference
        else:
            time_reference = float('inf')

        # Get the required velocity of the actor
        desired_velocity = distance / time_reference
        actor_dict[self._actor.id].update_target_speed(desired_velocity)

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))
        return new_status

    def terminate(self, new_status):
        """
        On termination of this behavior, set the target speed to its desired one.
        This function is called several times, so the use of self._final_speed_set
        is needed to avoid interfering with other running behaviors
        """
        if not self._final_speed_set:
            try:
                check_actors = operator.attrgetter("ActorsWithController")
                actor_dict = check_actors(py_trees.blackboard.Blackboard())
            except AttributeError:
                pass

            if actor_dict and self._actor.id in actor_dict:

                if self._relative_to_master:
                    master_speed = CarlaDataProvider.get_velocity(self._master_actor)
                    if self._relative_type == 'delta':
                        final_speed = master_speed + self._final_speed
                    elif self._relative_type == 'factor':
                        final_speed = master_speed * self._final_speed
                    else:
                        print("'relative_type' must be delta or factor")
                else:
                    final_speed = self._final_speed

                actor_dict[self._actor.id].update_target_speed(final_speed)

            self._final_speed_set = True

        super(SyncArrivalOSC, self).terminate(new_status)


class ChangeActorWaypoints(AtomicBehavior):

    """
    Atomic to change the waypoints for an actor controller.

    The behavior is in RUNNING state until the last waypoint is reached, or if a
    second waypoint related atomic for the same actor is triggered. These are:
    - ChangeActorWaypoints
    - ChangeActorLateralMotion
    - ChangeActorLaneOffset

    Args:
        actor (carla.Actor): Controlled actor.
        waypoints (List of (OSC position, OSC route option)): List of (Position, Route Option) as OpenScenario elements.
            position will be converted to Carla transforms, considering the corresponding
            route option (e.g. shortest, fastest)
        name (string): Name of the behavior.
            Defaults to 'ChangeActorWaypoints'.

    Attributes:
        _waypoints (List of (OSC position, OSC route option)): List of (Position, Route Option) as OpenScenario elements
        _start_time (float): Start time of the atomic [s].
            Defaults to None.

    '''Note: When using routing options such as fastest or shortest, it is advisable to run
             in synchronous mode
    """

    def __init__(self, actor, waypoints, times=None, name="ChangeActorWaypoints"):
        """
        Setup parameters
        """
        super(ChangeActorWaypoints, self).__init__(name, actor)

        self._waypoints = waypoints
        self._start_time = None
        self._times = times

    def initialise(self):
        """
        Set _start_time and get (actor, controller) pair from Blackboard.

        Set waypoint list for actor controller.

        May throw if actor is not available as key for the ActorsWithController
        dictionary from Blackboard.
        """
        actor_dict = {}

        try:
            check_actors = operator.attrgetter("ActorsWithController")
            actor_dict = check_actors(py_trees.blackboard.Blackboard())
        except AttributeError:
            pass

        if not actor_dict or not self._actor.id in actor_dict:
            raise RuntimeError("Actor not found in ActorsWithController BlackBoard")

        self._start_time = GameTime.get_time()

        # Transforming OSC waypoints to Carla waypoints
        carla_route_elements = []
        for (osc_point, routing_option) in self._waypoints:
            carla_transforms = sr_tools.openscenario_parser.OpenScenarioParser.convert_position_to_transform(osc_point)
            carla_route_elements.append((carla_transforms, routing_option))

        # Obtain final route, considering the routing option
        # At the moment everything besides "shortest" will use the CARLA GlobalPlanner
        grp = GlobalRoutePlanner(CarlaDataProvider.get_map(), 2.0)
        route = []
        for i, _ in enumerate(carla_route_elements):
            if carla_route_elements[i][1] == "shortest":
                route.append(carla_route_elements[i][0])
            else:
                if i == 0:
                    mmap = CarlaDataProvider.get_map()
                    ego_location = CarlaDataProvider.get_location(self._actor)
                    ego_waypoint = mmap.get_waypoint(ego_location)
                    try:
                        ego_next_wp = ego_waypoint.next(1)[0]
                    except IndexError:
                        ego_next_wp = ego_waypoint
                    waypoint = ego_next_wp.transform.location
                else:
                    waypoint = carla_route_elements[i - 1][0].location
                waypoint_next = carla_route_elements[i][0].location
                try:
                    interpolated_trace = grp.trace_route(waypoint, waypoint_next)
                except networkx.NetworkXNoPath:
                    print("WARNING: No route from {} to {} - Using direct path instead".format(waypoint, waypoint_next))
                    route.append(carla_route_elements[i][0])
                    continue
                for wp_tuple in interpolated_trace:
                    # The router sometimes produces points that go backward, or are almost identical
                    # We have to filter these, to avoid problems
                    if route and wp_tuple[0].transform.location.distance(route[-1].location) > 1.0:
                        new_heading_vec = wp_tuple[0].transform.location - route[-1].location
                        new_heading = np.arctan2(new_heading_vec.y, new_heading_vec.x)
                        if len(route) > 1:
                            last_heading_vec = route[-1].location - route[-2].location
                        else:
                            last_heading_vec = route[-1].location - ego_next_wp.transform.location
                        last_heading = np.arctan2(last_heading_vec.y, last_heading_vec.x)

                        heading_delta = math.fabs(new_heading - last_heading)
                        if math.fabs(heading_delta) < 0.5 or math.fabs(heading_delta) > 5.5:
                            route.append(wp_tuple[0].transform)
                    elif not route:
                        route.append(wp_tuple[0].transform)

        actor_dict[self._actor.id].update_waypoints(route, start_time=self._start_time)

        super(ChangeActorWaypoints, self).initialise()

    def update(self):
        """
        Check the actor's state along the waypoint route.

        returns:
            py_trees.common.Status.SUCCESS, if the final waypoint was reached, or
                                            if another ChangeActorWaypoints atomic for the same actor was triggered.
            py_trees.common.Status.FAILURE, if the actor is not found in ActorsWithController Blackboard dictionary.
            py_trees.common.Status.FAILURE, else.
        """
        try:
            check_actors = operator.attrgetter("ActorsWithController")
            actor_dict = check_actors(py_trees.blackboard.Blackboard())
        except AttributeError:
            pass

        if not actor_dict or not self._actor.id in actor_dict:
            return py_trees.common.Status.FAILURE

        actor = actor_dict[self._actor.id]

        if actor.get_last_waypoint_command() != self._start_time:
            return py_trees.common.Status.SUCCESS

        if actor.check_reached_waypoint_goal():
            return py_trees.common.Status.SUCCESS

        if self._times is not None:
            current_relative_time = GameTime.get_time() - self._start_time
            current_waypoint_idx = bisect_right(self._times, current_relative_time)
            if current_waypoint_idx >= len(self._times):
                return py_trees.common.Status.SUCCESS
            remaining_time = self._times[current_waypoint_idx] - current_relative_time
            self._update_speed(actor, self._waypoints[current_waypoint_idx], remaining_time)

        return py_trees.common.Status.RUNNING

    def _update_speed(self, actor, target_waypoint, remaining_time):
        target_location = sr_tools.openscenario_parser.OpenScenarioParser.convert_position_to_transform(
            target_waypoint[0]).location
        remaining_dist = calculate_distance(CarlaDataProvider.get_location(self._actor), target_location)
        target_speed = remaining_dist / max(remaining_time, 0.001)
        actor.update_target_speed(target_speed)


class ChangeActorLateralMotion(AtomicBehavior):

    """
    Atomic to change the waypoints for an actor controller.

    The behavior is in RUNNING state until the last waypoint is reached, or if a
    second waypoint related atomic for the same actor is triggered. These are:
    - ChangeActorWaypoints
    - ChangeActorLateralMotion
    - ChangeActorLaneOffset

    If an impossible lane change is asked for (due to the lack of lateral lanes,
    next waypoints, continuous line, etc) the atomic will return a plan with the
    waypoints until such impossibility is found.

    Args:
        actor (carla.Actor): Controlled actor.
        direction (string): Lane change direction ('left' or 'right').
            Defaults to 'left'.
        distance_lane_change (float): Distance of the lance change [meters].
            Defaults to 25.
        distance_other_lane (float): Driven distance after the lange change [meters].
            Defaults to 100.
        name (string): Name of the behavior.
            Defaults to 'ChangeActorLateralMotion'.

    Attributes:
        _waypoints (List of carla.Transform): List of waypoints representing the lane change (CARLA transforms).
        _direction (string): Lane change direction ('left' or 'right').
        _distance_same_lane (float): Distance on the same lane before the lane change starts [meters]
            Constant to 5.
        _distance_other_lane (float): Max. distance on the target lane after the lance change [meters]
            Constant to 100.
        _distance_lane_change (float): Max. total distance of the lane change [meters].
        _pos_before_lane_change: carla.Location of the actor before the lane change.
            Defaults to None.
        _target_lane_id (int): Id of the target lane
            Defaults to None.
        _start_time (float): Start time of the atomic [s].
            Defaults to None.
    """

    def __init__(self, actor, direction='left', distance_lane_change=25, distance_other_lane=100,
                 lane_changes=1, name="ChangeActorLateralMotion"):
        """
        Setup parameters
        """
        super(ChangeActorLateralMotion, self).__init__(name, actor)

        self._waypoints = []
        self._direction = direction
        self._distance_same_lane = 5
        self._distance_other_lane = distance_other_lane
        self._distance_lane_change = distance_lane_change
        self._lane_changes = lane_changes
        self._pos_before_lane_change = None
        self._target_lane_id = None
        self._plan = None

        self._start_time = None

    def initialise(self):
        """
        Set _start_time and get (actor, controller) pair from Blackboard.

        Generate a waypoint list (route) which representes the lane change. Set
        this waypoint list for the actor controller.

        May throw if actor is not available as key for the ActorsWithController
        dictionary from Blackboard.
        """
        actor_dict = {}

        try:
            check_actors = operator.attrgetter("ActorsWithController")
            actor_dict = check_actors(py_trees.blackboard.Blackboard())
        except AttributeError:
            pass

        if not actor_dict or not self._actor.id in actor_dict:
            raise RuntimeError("Actor not found in ActorsWithController BlackBoard")

        self._start_time = GameTime.get_time()

        # get start position
        position_actor = CarlaDataProvider.get_map().get_waypoint(CarlaDataProvider.get_location(self._actor))

        # calculate plan with scenario_helper function
        self._plan, self._target_lane_id = generate_target_waypoint_list_multilane(
            position_actor, self._direction, self._distance_same_lane,
            self._distance_other_lane, self._distance_lane_change, check=False, lane_changes=self._lane_changes)

        if self._plan:
            for elem in self._plan:
                self._waypoints.append(elem[0].transform)

        actor_dict[self._actor.id].update_waypoints(self._waypoints, start_time=self._start_time)

        super(ChangeActorLateralMotion, self).initialise()

    def update(self):
        """
        Check the actor's current state and if the lane change was completed

        returns:
            py_trees.common.Status.SUCCESS, if lane change was successful, or
                                            if another ChangeActorLateralMotion atomic for the same actor was triggered.
            py_trees.common.Status.FAILURE, if the actor is not found in ActorsWithController Blackboard dictionary.
            py_trees.common.Status.FAILURE, else.
        """

        try:
            check_actors = operator.attrgetter("ActorsWithController")
            actor_dict = check_actors(py_trees.blackboard.Blackboard())
        except AttributeError:
            pass

        if not actor_dict or not self._actor.id in actor_dict:
            return py_trees.common.Status.FAILURE

        if not self._plan:
            print("{} couldn't perform the expected lane change".format(self._actor))
            return py_trees.common.Status.FAILURE

        if actor_dict[self._actor.id].get_last_waypoint_command() != self._start_time:
            return py_trees.common.Status.SUCCESS

        new_status = py_trees.common.Status.RUNNING

        current_position_actor = CarlaDataProvider.get_map().get_waypoint(self._actor.get_location())
        current_lane_id = current_position_actor.lane_id

        if current_lane_id == self._target_lane_id:
            # driving on new lane
            distance = current_position_actor.transform.location.distance(self._pos_before_lane_change)

            if distance > self._distance_other_lane:
                # long enough distance on new lane --> SUCCESS
                new_status = py_trees.common.Status.SUCCESS

                new_waypoints = []
                map_wp = current_position_actor
                while len(new_waypoints) < 200:
                    map_wps = map_wp.next(2.0)
                    if map_wps:
                        new_waypoints.append(map_wps[0].transform)
                        map_wp = map_wps[0]
                    else:
                        break

                actor_dict[self._actor.id].update_waypoints(new_waypoints, start_time=self._start_time)

        else:
            self._pos_before_lane_change = current_position_actor.transform.location

        return new_status


class ChangeActorLaneOffset(AtomicBehavior):

    """
    OpenSCENARIO atomic.
    Atomic to change the offset of the controller.

    The behavior is in RUNNING state until the offset os reached (if 'continuous' is set to False)
    or forever (if 'continuous' is True). This behavior will automatically stop if a second waypoint
    related atomic for the same actor is triggered. These are:
    - ChangeActorWaypoints
    - ChangeActorLateralMotion
    - ChangeActorLaneOffset

    Args:
        actor (carla.Actor): Controlled actor.
        offset (float): Float determined the distance to the center of the lane. Positive distance imply a
            displacement to the right, while negative displacements are to the left.
        relative_actor (carla.Actor): The actor from which the offset is taken from. Defaults to None
        continuous (bool): If True, the behaviour never ends. If False, the behaviour ends when the lane
            offset is reached. Defaults to True.

    Attributes:
        _offset (float): lane offset.
        _relative_actor (carla.Actor): relative actor.
        _continuous (bool): stored the value of the 'continuous' argument.
        _start_time (float): Start time of the atomic [s].
            Defaults to None.
        _overwritten (bool): flag to check whether or not this behavior was overwritten by another. Helps
            to avoid the missinteraction between two ChangeActorLaneOffsets.
        _current_target_offset (float): stores the value of the offset when dealing with relative distances
        _map (carla.Map): instance of the CARLA map.
    """

    OFFSET_THRESHOLD = 0.1

    def __init__(self, actor, offset, relative_actor=None, continuous=True, name="ChangeActorWaypoints"):
        """
        Setup parameters
        """
        super(ChangeActorLaneOffset, self).__init__(name, actor)

        self._offset = offset
        self._relative_actor = relative_actor
        self._continuous = continuous
        self._start_time = None
        self._current_target_offset = 0

        self._overwritten = False
        self._map = CarlaDataProvider.get_map()

    def initialise(self):
        """
        Set _start_time and get (actor, controller) pair from Blackboard.

        Set offset for actor controller.

        May throw if actor is not available as key for the ActorsWithController
        dictionary from Blackboard.
        """
        actor_dict = {}

        try:
            check_actors = operator.attrgetter("ActorsWithController")
            actor_dict = check_actors(py_trees.blackboard.Blackboard())
        except AttributeError:
            pass

        if not actor_dict or not self._actor.id in actor_dict:
            raise RuntimeError("Actor not found in ActorsWithController BlackBoard")

        self._start_time = GameTime.get_time()

        actor_dict[self._actor.id].update_offset(self._offset, start_time=self._start_time)

        super(ChangeActorLaneOffset, self).initialise()

    def update(self):
        """
        Check the actor's state along the waypoint route.

        returns:
            py_trees.common.Status.SUCCESS, if the lane offset was reached (and 'continuous' was False), or
                                            if another waypoint atomic for the same actor was triggered
            py_trees.common.Status.FAILURE, if the actor is not found in ActorsWithController Blackboard dictionary.
            py_trees.common.Status.RUNNING, else.
        """
        try:
            check_actors = operator.attrgetter("ActorsWithController")
            actor_dict = check_actors(py_trees.blackboard.Blackboard())
        except AttributeError:
            pass

        if not actor_dict or not self._actor.id in actor_dict:
            return py_trees.common.Status.FAILURE

        if actor_dict[self._actor.id].get_last_lane_offset_command() != self._start_time:
            # Differentiate between lane offset and other lateral commands
            self._overwritten = True
            return py_trees.common.Status.SUCCESS

        if actor_dict[self._actor.id].get_last_waypoint_command() != self._start_time:
            return py_trees.common.Status.SUCCESS

        if self._relative_actor:
            # Calculate new offset
            relative_actor_loc = CarlaDataProvider.get_location(self._relative_actor)
            relative_center_wp = self._map.get_waypoint(relative_actor_loc)

            # Value
            relative_center_loc = relative_center_wp.transform.location
            relative_actor_offset = relative_actor_loc.distance(relative_center_loc)

            # Sign
            f_vec = relative_center_wp.transform.get_forward_vector()
            d_vec = relative_actor_loc - relative_center_loc
            cross = f_vec.x * d_vec.y - f_vec.y * d_vec.x

            if cross < 0:
                relative_actor_offset *= -1.0

            self._current_target_offset = relative_actor_offset + self._offset
            # Set the new offset
            actor_dict[self._actor.id].update_offset(self._current_target_offset)

        if not self._continuous:
            # Calculate new offset
            actor_loc = CarlaDataProvider.get_location(self._actor)
            center_wp = self._map.get_waypoint(actor_loc)

            # Value
            center_loc = center_wp.transform.location
            actor_offset = actor_loc.distance(center_loc)

            # Sign
            f_vec = center_wp.transform.get_forward_vector()
            d_vec = actor_loc - center_loc
            cross = f_vec.x * d_vec.y - f_vec.y * d_vec.x

            if cross < 0:
                actor_offset *= -1.0

            # Check if the offset has been reached
            if abs(actor_offset - self._current_target_offset) < self.OFFSET_THRESHOLD:
                return py_trees.common.Status.SUCCESS

        # TODO: As their is no way to check the distance to a specific lane, both checks will fail if the
        # actors are outside its 'route lane' or at an intersection

        new_status = py_trees.common.Status.RUNNING

        return new_status

    def terminate(self, new_status):
        """
        On termination of this behavior, the offset is set back to zero
        """

        if not self._overwritten:
            try:
                check_actors = operator.attrgetter("ActorsWithController")
                actor_dict = check_actors(py_trees.blackboard.Blackboard())
            except AttributeError:
                pass

            if actor_dict and self._actor.id in actor_dict:
                actor_dict[self._actor.id].update_offset(0)

            self._overwritten = True

        super(ChangeActorLaneOffset, self).terminate(new_status)


class ActorTransformSetterToOSCPosition(AtomicBehavior):

    """
    OpenSCENARIO atomic
    This class contains an atomic behavior to set the transform of an OpenSCENARIO actor.

    Important parameters:
    - actor: CARLA actor to execute the behavior
    - osc_position: OpenSCENARIO position
    - physics [optional]: If physics is true, the actor physics will be reactivated upon success

    The behavior terminates when actor is set to the new actor transform (closer than 1 meter)

    NOTE:
    It is very important to ensure that the actor location is spawned to the new transform because of the
    appearence of a rare runtime processing error. WaypointFollower with LocalPlanner,
    might fail if new_status is set to success before the actor is really positioned at the new transform.
    Therefore: calculate_distance(actor, transform) < 1 meter
    """

    def __init__(self, actor, osc_position, physics=True, name="ActorTransformSetterToOSCPosition"):
        """
        Setup parameters
        """
        super(ActorTransformSetterToOSCPosition, self).__init__(name, actor)
        self._osc_position = osc_position
        self._physics = physics
        self._osc_transform = None

    def initialise(self):

        super(ActorTransformSetterToOSCPosition, self).initialise()

        if self._actor.is_alive:
            self._actor.set_target_velocity(carla.Vector3D(0, 0, 0))
            self._actor.set_target_angular_velocity(carla.Vector3D(0, 0, 0))

    def update(self):
        """
        Transform actor
        """
        new_status = py_trees.common.Status.RUNNING

        # calculate transform with method in openscenario_parser.py
        self._osc_transform = sr_tools.openscenario_parser.OpenScenarioParser.convert_position_to_transform(
            self._osc_position)
        self._actor.set_transform(self._osc_transform)

        if not self._actor.is_alive:
            new_status = py_trees.common.Status.FAILURE

        if calculate_distance(self._actor.get_location(), self._osc_transform.location) < 1.0:
            if self._physics:
                self._actor.set_simulate_physics(enabled=True)
            new_status = py_trees.common.Status.SUCCESS

        return new_status


class AccelerateToVelocity(AtomicBehavior):

    """
    This class contains an atomic acceleration behavior. The controlled
    traffic participant will accelerate with _throttle_value_ until reaching
    a given _target_velocity_

    Important parameters:
    - actor: CARLA actor to execute the behavior
    - throttle_value: The amount of throttle used to accelerate in [0,1]
    - target_velocity: The target velocity the actor should reach in m/s

    The behavior will terminate, if the actor's velocity is at least target_velocity
    """

    def __init__(self, actor, throttle_value, target_velocity, name="Acceleration"):
        """
        Setup parameters including acceleration value (via throttle_value)
        and target velocity
        """
        super(AccelerateToVelocity, self).__init__(name, actor)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))
        self._control, self._type = get_actor_control(actor)
        self._throttle_value = throttle_value
        self._target_velocity = target_velocity

    def initialise(self):
        # In case of walkers, we have to extract the current heading
        if self._type == 'walker':
            self._control.speed = self._target_velocity
            self._control.direction = CarlaDataProvider.get_transform(self._actor).get_forward_vector()

        super(AccelerateToVelocity, self).initialise()

    def update(self):
        """
        Set throttle to throttle_value, as long as velocity is < target_velocity
        """
        new_status = py_trees.common.Status.RUNNING

        if self._type == 'vehicle':
            if CarlaDataProvider.get_velocity(self._actor) < self._target_velocity:
                self._control.throttle = self._throttle_value
            else:
                new_status = py_trees.common.Status.SUCCESS
                self._control.throttle = 0

        self._actor.apply_control(self._control)
        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))

        return new_status


class AccelerateToCatchUp(AtomicBehavior):

    """
    This class contains an atomic acceleration behavior.
    The car will accelerate until it is faster than another car, in order to catch up distance.
    This behaviour is especially useful before a lane change (e.g. LaneChange atom).

    Important parameters:
    - actor: CARLA actor to execute the behaviour
    - other_actor: Reference CARLA actor, actor you want to catch up to
    - throttle_value: acceleration value between 0.0 and 1.0
    - delta_velocity: speed up to the velocity of other actor plus delta_velocity
    - trigger_distance: distance between the actors
    - max_distance: driven distance to catch up has to be smaller than max_distance

    The behaviour will terminate succesful, when the two actors are in trigger_distance.
    If max_distance is driven by the actor before actors are in trigger_distance,
    then the behaviour ends with a failure.
    """

    def __init__(self, actor, other_actor, throttle_value=1, delta_velocity=10, trigger_distance=5,
                 max_distance=500, name="AccelerateToCatchUp"):
        """
        Setup parameters
        The target_speet is calculated on the fly.
        """
        super(AccelerateToCatchUp, self).__init__(name, actor)

        self._other_actor = other_actor
        self._throttle_value = throttle_value
        self._delta_velocity = delta_velocity  # 1m/s=3.6km/h
        self._trigger_distance = trigger_distance
        self._max_distance = max_distance

        self._control, self._type = get_actor_control(actor)

        self._initial_actor_pos = None

    def initialise(self):

        # get initial actor position
        self._initial_actor_pos = CarlaDataProvider.get_location(self._actor)
        super(AccelerateToCatchUp, self).initialise()

    def update(self):

        # get actor speed
        actor_speed = CarlaDataProvider.get_velocity(self._actor)
        target_speed = CarlaDataProvider.get_velocity(self._other_actor) + self._delta_velocity

        # distance between actors
        distance = CarlaDataProvider.get_location(self._actor).distance(
            CarlaDataProvider.get_location(self._other_actor))

        # driven distance of actor
        driven_distance = CarlaDataProvider.get_location(self._actor).distance(self._initial_actor_pos)

        if actor_speed < target_speed:
            # set throttle to throttle_value to accelerate
            self._control.throttle = self._throttle_value

        if actor_speed >= target_speed:
            # keep velocity until the actors are in trigger distance
            self._control.throttle = 0

        self._actor.apply_control(self._control)

        # new status:
        if distance <= self._trigger_distance:
            new_status = py_trees.common.Status.SUCCESS

        elif driven_distance > self._max_distance:
            new_status = py_trees.common.Status.FAILURE
        else:
            new_status = py_trees.common.Status.RUNNING

        return new_status


class KeepVelocity(AtomicBehavior):

    """
    This class contains an atomic behavior to keep the provided velocity.
    The controlled traffic participant will accelerate as fast as possible
    until reaching a given _target_velocity_, which is then maintained for
    as long as this behavior is active.

    Important parameters:
    - actor: CARLA actor to execute the behavior
    - target_velocity: The target velocity the actor should reach
    - forced_speed: Whether or not to forcefully set the actors speed
    - duration[optional]: Duration in seconds of this behavior
    - distance[optional]: Maximum distance in meters covered by the actor during this behavior

    A termination can be enforced by providing distance or duration values.
    Alternatively, a parallel termination behavior has to be used.
    """

    def __init__(self, actor, target_velocity, force_speed=False,
                 duration=float("inf"), distance=float("inf"), name="KeepVelocity"):
        """
        Setup parameters including acceleration value (via throttle_value)
        and target velocity
        """
        super(KeepVelocity, self).__init__(name, actor)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))
        self._target_velocity = target_velocity

        self._control, self._type = get_actor_control(actor)
        self._map = self._actor.get_world().get_map()
        self._waypoint = self._map.get_waypoint(self._actor.get_location())

        self._forced_speed = force_speed
        self._duration = duration
        self._target_distance = distance
        self._distance = 0
        self._start_time = 0
        self._location = None

    def initialise(self):
        self._location = CarlaDataProvider.get_location(self._actor)
        self._start_time = GameTime.get_time()

        # In case of walkers, we have to extract the current heading
        if self._type == 'walker':
            self._control.speed = self._target_velocity
            self._control.direction = CarlaDataProvider.get_transform(self._actor).get_forward_vector()
        elif self._type == 'vehicle':
            self._control.hand_brake = False
        self._actor.apply_control(self._control)

        super(KeepVelocity, self).initialise()

    def update(self):
        """
        As long as the stop condition (duration or distance) is not violated, set a new vehicle control
        For vehicles: set throttle to throttle_value, as long as velocity is < target_velocity
        For walkers: simply apply the given self._control
        """
        new_status = py_trees.common.Status.RUNNING

        if self._type == 'vehicle':
            if not self._forced_speed:
                if CarlaDataProvider.get_velocity(self._actor) < self._target_velocity:
                    self._control.throttle = 1.0
                else:
                    self._control.throttle = 0.0
                self._actor.apply_control(self._control)
            else:
                yaw = CarlaDataProvider.get_transform(self._actor).rotation.yaw * (math.pi / 180)
                self._actor.set_target_velocity(carla.Vector3D(
                    math.cos(yaw) * self._target_velocity, math.sin(yaw) * self._target_velocity, 0))

        new_location = CarlaDataProvider.get_location(self._actor)
        self._distance += calculate_distance(self._location, new_location)
        self._location = new_location

        if self._distance > self._target_distance:
            new_status = py_trees.common.Status.SUCCESS

        if GameTime.get_time() - self._start_time > self._duration:
            new_status = py_trees.common.Status.SUCCESS

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))

        return new_status

    def terminate(self, new_status):
        """
        On termination of this behavior, the throttle should be set back to 0.,
        to avoid further acceleration.
        """

        if self._type == 'vehicle':
            self._control.throttle = 0.0
        elif self._type == 'walker':
            self._control.speed = 0.0
        if self._actor is not None and self._actor.is_alive:
            self._actor.apply_control(self._control)
        super(KeepVelocity, self).terminate(new_status)


class ChangeAutoPilot(AtomicBehavior):

    """
    This class contains an atomic behavior to disable/enable the use of the autopilot.

    Important parameters:
    - actor: CARLA actor to execute the behavior
    - activate: True (=enable autopilot) or False (=disable autopilot)
    - lane_change: Traffic Manager parameter. True (=enable lane changes) or False (=disable lane changes)
    - distance_between_vehicles: Traffic Manager parameter
    - max_speed: Traffic Manager parameter. Max speed of the actor. This will only work for road segments
                 with the same speed limit as the first one

    The behavior terminates after changing the autopilot state
    """

    def __init__(self, actor, activate, parameters=None, name="ChangeAutoPilot"):
        """
        Setup parameters
        """
        super(ChangeAutoPilot, self).__init__(name, actor)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))
        self._activate = activate
        self._tm = CarlaDataProvider.get_client().get_trafficmanager(
            CarlaDataProvider.get_traffic_manager_port())
        self._parameters = parameters

    def update(self):
        """
        De/activate autopilot
        """
        self._actor.set_autopilot(self._activate, CarlaDataProvider.get_traffic_manager_port())

        if self._parameters is not None:
            if "auto_lane_change" in self._parameters:
                lane_change = self._parameters["auto_lane_change"]
                self._tm.auto_lane_change(self._actor, lane_change)

            if "max_speed" in self._parameters:
                max_speed = self._parameters["max_speed"]
                max_road_speed = self._actor.get_speed_limit()
                if max_road_speed is not None:
                    percentage = (max_road_speed - max_speed) / max_road_speed * 100.0
                    self._tm.vehicle_percentage_speed_difference(self._actor, percentage)
                else:
                    print("ChangeAutopilot: Unable to find the vehicle's speed limit")

            if "distance_between_vehicles" in self._parameters:
                dist_vehicles = self._parameters["distance_between_vehicles"]
                self._tm.distance_to_leading_vehicle(self._actor, dist_vehicles)

            if "force_lane_change" in self._parameters:
                force_lane_change = self._parameters["force_lane_change"]
                self._tm.force_lane_change(self._actor, force_lane_change)

            if "ignore_vehicles_percentage" in self._parameters:
                ignore_vehicles = self._parameters["ignore_vehicles_percentage"]
                self._tm.ignore_vehicles_percentage(self._actor, ignore_vehicles)

        new_status = py_trees.common.Status.SUCCESS

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))
        return new_status


class StopVehicle(AtomicBehavior):

    """
    This class contains an atomic stopping behavior. The controlled traffic
    participant will decelerate with _bake_value_ until reaching a full stop.

    Important parameters:
    - actor: CARLA actor to execute the behavior
    - brake_value: Brake value in [0,1] applied

    The behavior terminates when the actor stopped moving
    """

    def __init__(self, actor, brake_value, name="Stopping"):
        """
        Setup _actor and maximum braking value
        """
        super(StopVehicle, self).__init__(name, actor)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))
        self._control, self._type = get_actor_control(actor)
        if self._type == 'walker':
            self._control.speed = 0
        self._brake_value = brake_value

    def update(self):
        """
        Set brake to brake_value until reaching full stop
        """
        new_status = py_trees.common.Status.RUNNING

        if self._type == 'vehicle':
            if CarlaDataProvider.get_velocity(self._actor) > EPSILON:
                self._control.brake = self._brake_value
            else:
                new_status = py_trees.common.Status.SUCCESS
                self._control.brake = 0
        else:
            new_status = py_trees.common.Status.SUCCESS

        self._actor.apply_control(self._control)

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))

        return new_status


class SyncArrival(AtomicBehavior):

    """
    This class contains an atomic behavior to
    set velocity of actor so that it reaches location at the same time as
    actor_reference. The behavior assumes that the two actors are moving
    towards location in a straight line.
    Important parameters:
    - actor: CARLA actor to execute the behavior
    - actor_reference: Reference actor with which arrival is synchronized
    - target_location: CARLA location where the actors should "meet"
    - gain[optional]: Coefficient for actor's throttle and break controls
    Note: In parallel to this behavior a termination behavior has to be used
          to keep continue synchronization for a certain duration, or for a
          certain distance, etc.
    """

    def __init__(self, actor, actor_reference, target_location, gain=1, name="SyncArrival"):
        """
        Setup required parameters
        """
        super(SyncArrival, self).__init__(name, actor)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))
        self._control = carla.VehicleControl()
        self._actor_reference = actor_reference
        self._target_location = target_location
        self._gain = gain

        self._control.steering = 0

    def update(self):
        """
        Dynamic control update for actor velocity
        """
        new_status = py_trees.common.Status.RUNNING

        distance_reference = calculate_distance(CarlaDataProvider.get_location(self._actor_reference),
                                                self._target_location)
        distance = calculate_distance(CarlaDataProvider.get_location(self._actor),
                                      self._target_location)

        velocity_reference = CarlaDataProvider.get_velocity(self._actor_reference)
        time_reference = float('inf')
        if velocity_reference > 0:
            time_reference = distance_reference / velocity_reference

        velocity_current = CarlaDataProvider.get_velocity(self._actor)
        time_current = float('inf')
        if velocity_current > 0:
            time_current = distance / velocity_current

        control_value = (self._gain) * (time_current - time_reference)

        if control_value > 0:
            self._control.throttle = min([control_value, 1])
            self._control.brake = 0
        else:
            self._control.throttle = 0
            self._control.brake = min([abs(control_value), 1])

        self._actor.apply_control(self._control)
        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))
        return new_status

    def terminate(self, new_status):
        """
        On termination of this behavior, the throttle should be set back to 0.,
        to avoid further acceleration.
        """
        if self._actor is not None and self._actor.is_alive:
            self._control.throttle = 0.0
            self._control.brake = 0.0
            self._actor.apply_control(self._control)
        super(SyncArrival, self).terminate(new_status)


class AddNoiseToVehicle(AtomicBehavior):

    """
    This class contains an atomic jitter behavior.
    To add noise to steer as well as throttle of the vehicle.

    Important parameters:
    - actor: CARLA actor to execute the behavior
    - steer_value: Applied steering noise in [0,1]
    - throttle_value: Applied throttle noise in [0,1]

    The behavior terminates after setting the new actor controls
    """

    def __init__(self, actor, steer_value, throttle_value, name="Jittering"):
        """
        Setup actor , maximum steer value and throttle value
        """
        super(AddNoiseToVehicle, self).__init__(name, actor)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))
        self._control = carla.VehicleControl()
        self._steer_value = steer_value
        self._throttle_value = throttle_value

    def update(self):
        """
        Set steer to steer_value and throttle to throttle_value until reaching full stop
        """
        self._control = self._actor.get_control()
        self._control.steer = self._steer_value
        self._control.throttle = self._throttle_value
        new_status = py_trees.common.Status.SUCCESS

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))
        self._actor.apply_control(self._control)

        return new_status


class ChangeNoiseParameters(AtomicBehavior):

    """
    This class contains an atomic jitter behavior.
    To add noise to steer as well as throttle of the vehicle.

    This behavior should be used in conjuction with AddNoiseToVehicle

    The behavior terminates after one iteration
    """

    def __init__(self, new_steer_noise, new_throttle_noise,
                 noise_mean, noise_std, dynamic_mean_for_steer, dynamic_mean_for_throttle, name="ChangeJittering"):
        """
        Setup actor , maximum steer value and throttle value
        """
        super(ChangeNoiseParameters, self).__init__(name)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))
        self._new_steer_noise = new_steer_noise
        self._new_throttle_noise = new_throttle_noise
        self._noise_mean = noise_mean
        self._noise_std = noise_std
        self._dynamic_mean_for_steer = dynamic_mean_for_steer
        self._dynamic_mean_for_throttle = dynamic_mean_for_throttle

        self._noise_to_apply = abs(random.normal(self._noise_mean, self._noise_std))

    def update(self):
        """
        Change the noise parameters from the structure copy that it receives.
        """

        self._new_steer_noise[0] = min(0, -(self._noise_to_apply - self._dynamic_mean_for_steer))
        self._new_throttle_noise[0] = min(self._noise_to_apply + self._dynamic_mean_for_throttle, 1)

        new_status = py_trees.common.Status.SUCCESS
        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))
        return new_status


class BasicAgentBehavior(AtomicBehavior):

    """
    This class contains an atomic behavior, which uses the
    basic_agent from CARLA to control the actor until
    reaching a target location.
    Important parameters:
    - actor: CARLA actor to execute the behavior
    - target_location: Is the desired target location (carla.location),
                       the actor should move to
    The behavior terminates after reaching the target_location (within 2 meters)
    """

    def __init__(self, actor, target_location, target_speed=None,
                 opt_dict=None, name="BasicAgentBehavior"):
        """
        Setup actor and maximum steer value
        """
        super(BasicAgentBehavior, self).__init__(name, actor)
        self._map = CarlaDataProvider.get_map()
        self._target_speed = target_speed
        self._target_location = target_location
        self._opt_dict = opt_dict if opt_dict else {}
        self._control = carla.VehicleControl()
        self._agent = None
        self._plan = None

    def initialise(self):
        """Initialises the agent"""
        self._agent = BasicAgent(self._actor, self._target_speed * 3.6, opt_dict=self._opt_dict)
        self._plan = self._agent.trace_route(
            self._map.get_waypoint(CarlaDataProvider.get_location(self._actor)),
            self._map.get_waypoint(self._target_location))
        self._agent.set_global_plan(self._plan)

    def update(self):
        new_status = py_trees.common.Status.RUNNING

        if self._agent.done():
            new_status = py_trees.common.Status.SUCCESS
        self._control = self._agent.run_step()
        self._actor.apply_control(self._control)

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))
        return new_status

    def terminate(self, new_status):
        """Resets the control"""
        self._control.throttle = 0.0
        self._control.brake = 0.0
        self._actor.apply_control(self._control)
        super(BasicAgentBehavior, self).terminate(new_status)


class Idle(AtomicBehavior):

    """
    This class contains an idle behavior scenario

    Important parameters:
    - duration[optional]: Duration in seconds of this behavior

    A termination can be enforced by providing a duration value.
    Alternatively, a parallel termination behavior has to be used.
    """

    def __init__(self, duration=float("inf"), name="Idle"):
        """
        Setup actor
        """
        super(Idle, self).__init__(name)
        self._duration = duration
        self._start_time = 0
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))

    def initialise(self):
        """
        Set start time
        """
        self._start_time = GameTime.get_time()
        super(Idle, self).initialise()

    def update(self):
        """
        Keep running until termination condition is satisfied
        """
        new_status = py_trees.common.Status.RUNNING

        if GameTime.get_time() - self._start_time > self._duration:
            new_status = py_trees.common.Status.SUCCESS

        return new_status


class WaypointFollower(AtomicBehavior):

    """
    This is an atomic behavior to follow waypoints while maintaining a given speed.
    If no plan is provided, the actor will follow its foward waypoints indefinetely.
    Otherwise, the behavior will end with SUCCESS upon reaching the end of the plan.
    If no target velocity is provided, the actor continues with its current velocity.

    Args:
        actor (carla.Actor):  CARLA actor to execute the behavior.
        target_speed (float, optional): Desired speed of the actor in m/s. Defaults to None.
        plan ([carla.Location] or [(carla.Waypoint, carla.agent.navigation.local_planner)], optional):
            Waypoint plan the actor should follow. Defaults to None.
        blackboard_queue_name (str, optional):
            Blackboard variable name, if additional actors should be created on-the-fly. Defaults to None.
        avoid_collision (bool, optional):
            Enable/Disable(=default) collision avoidance for vehicles/bikes. Defaults to False.
        name (str, optional): Name of the behavior. Defaults to "FollowWaypoints".

    Attributes:
        actor (carla.Actor):  CARLA actor to execute the behavior.
        name (str, optional): Name of the behavior.
        _target_speed (float, optional): Desired speed of the actor in m/s. Defaults to None.
        _plan ([carla.Location] or [(carla.Waypoint, carla.agent.navigation.local_planner)]):
            Waypoint plan the actor should follow. Defaults to None.
        _blackboard_queue_name (str):
            Blackboard variable name, if additional actors should be created on-the-fly. Defaults to None.
        _avoid_collision (bool): Enable/Disable(=default) collision avoidance for vehicles/bikes. Defaults to False.
        _actor_dict: Dictonary of all actors, and their corresponding plans (e.g. {actor: plan}).
        _local_planner_dict: Dictonary of all actors, and their corresponding local planners.
            Either "Walker" for pedestrians, or a carla.agent.navigation.LocalPlanner for other actors.
        _args_lateral_dict: Parameters for the PID of the used carla.agent.navigation.LocalPlanner.
        _unique_id: Unique ID of the behavior based on timestamp in nanoseconds.

    Note:
        OpenScenario:
        The WaypointFollower atomic must be called with an individual name if multiple consecutive WFs.
        Blackboard variables with lists are used for consecutive WaypointFollower behaviors.
        Termination of active WaypointFollowers in initialise of AtomicBehavior because any
        following behavior must terminate the WaypointFollower.
    """

    def __init__(self, actor, target_speed=None, plan=None, blackboard_queue_name=None,
                 avoid_collision=False, name="FollowWaypoints"):
        """
        Set up actor and local planner
        """
        super(WaypointFollower, self).__init__(name, actor)
        self._actor_dict = {}
        self._actor_dict[actor] = None
        self._target_speed = target_speed
        self._local_planner_dict = {}
        self._local_planner_dict[actor] = None
        self._plan = plan
        self._blackboard_queue_name = blackboard_queue_name
        if blackboard_queue_name is not None:
            self._queue = Blackboard().get(blackboard_queue_name)
        self._args_lateral_dict = {'K_P': 1.0, 'K_D': 0.01, 'K_I': 0.0, 'dt': 0.05}
        self._avoid_collision = avoid_collision
        self._unique_id = 0

    def initialise(self):
        """
        Delayed one-time initialization

        Checks if another WaypointFollower behavior is already running for this actor.
        If this is the case, a termination signal is sent to the running behavior.
        """
        super(WaypointFollower, self).initialise()
        self._unique_id = int(round(time.time() * 1e9))
        try:
            # check whether WF for this actor is already running and add new WF to running_WF list
            check_attr = operator.attrgetter("running_WF_actor_{}".format(self._actor.id))
            running = check_attr(py_trees.blackboard.Blackboard())
            active_wf = copy.copy(running)
            active_wf.append(self._unique_id)
            py_trees.blackboard.Blackboard().set(
                "running_WF_actor_{}".format(self._actor.id), active_wf, overwrite=True)
        except AttributeError:
            # no WF is active for this actor
            py_trees.blackboard.Blackboard().set("terminate_WF_actor_{}".format(self._actor.id), [], overwrite=True)
            py_trees.blackboard.Blackboard().set(
                "running_WF_actor_{}".format(self._actor.id), [self._unique_id], overwrite=True)

        for actor in self._actor_dict:
            self._apply_local_planner(actor)
        return True

    def _apply_local_planner(self, actor):
        """
        Convert the plan into locations for walkers (pedestrians), or to a waypoint list for other actors.
        For non-walkers, activate the carla.agent.navigation.LocalPlanner module.
        """
        if self._target_speed is None:
            self._target_speed = CarlaDataProvider.get_velocity(actor)
        else:
            self._target_speed = self._target_speed

        if isinstance(actor, carla.Walker):
            self._local_planner_dict[actor] = "Walker"
            if self._plan is not None:
                if isinstance(self._plan[0], carla.Location):
                    self._actor_dict[actor] = self._plan
                else:
                    self._actor_dict[actor] = [element[0].transform.location for element in self._plan]
        else:
            local_planner = LocalPlanner(  # pylint: disable=undefined-variable
                actor, opt_dict={
                    'target_speed': self._target_speed * 3.6,
                    'lateral_control_dict': self._args_lateral_dict})

            if self._plan is not None:
                if isinstance(self._plan[0], carla.Location):
                    plan = []
                    for location in self._plan:
                        waypoint = CarlaDataProvider.get_map().get_waypoint(location,
                                                                            project_to_road=True,
                                                                            lane_type=carla.LaneType.Any)
                        plan.append((waypoint, RoadOption.LANEFOLLOW))
                    local_planner.set_global_plan(plan)
                else:
                    local_planner.set_global_plan(self._plan)

            self._local_planner_dict[actor] = local_planner
            self._actor_dict[actor] = self._plan

    def update(self):
        """
        Compute next control step for the given waypoint plan, obtain and apply control to actor
        """
        new_status = py_trees.common.Status.RUNNING

        check_term = operator.attrgetter("terminate_WF_actor_{}".format(self._actor.id))
        terminate_wf = check_term(py_trees.blackboard.Blackboard())

        check_run = operator.attrgetter("running_WF_actor_{}".format(self._actor.id))
        active_wf = check_run(py_trees.blackboard.Blackboard())

        # Termination of WF if the WFs unique_id is listed in terminate_wf
        # only one WF should be active, therefore all previous WF have to be terminated
        if self._unique_id in terminate_wf:
            terminate_wf.remove(self._unique_id)
            if self._unique_id in active_wf:
                active_wf.remove(self._unique_id)

            py_trees.blackboard.Blackboard().set(
                "terminate_WF_actor_{}".format(self._actor.id), terminate_wf, overwrite=True)
            py_trees.blackboard.Blackboard().set(
                "running_WF_actor_{}".format(self._actor.id), active_wf, overwrite=True)
            new_status = py_trees.common.Status.SUCCESS
            return new_status

        if self._blackboard_queue_name is not None:
            while not self._queue.empty():
                actor = self._queue.get()
                if actor is not None and actor not in self._actor_dict:
                    self._apply_local_planner(actor)

        success = True
        for actor in self._local_planner_dict:
            local_planner = self._local_planner_dict[actor] if actor else None
            if actor is not None and actor.is_alive and local_planner is not None:
                # Check if the actor is a vehicle/bike
                if not isinstance(actor, carla.Walker):
                    control = local_planner.run_step(debug=False)
                    if self._avoid_collision and detect_lane_obstacle(actor):
                        control.throttle = 0.0
                        control.brake = 1.0
                    actor.apply_control(control)
                    # Check if the actor reached the end of the plan
                    # @TODO replace access to private _waypoints_queue with public getter
                    if local_planner._waypoints_queue:  # pylint: disable=protected-access
                        success = False
                # If the actor is a pedestrian, we have to use the WalkerAIController
                # The walker is sent to the next waypoint in its plan
                else:
                    actor_location = CarlaDataProvider.get_location(actor)
                    success = False
                    if self._actor_dict[actor]:
                        location = self._actor_dict[actor][0]
                        direction = location - actor_location
                        direction_norm = math.sqrt(direction.x**2 + direction.y**2)
                        control = actor.get_control()
                        control.speed = self._target_speed
                        control.direction = direction / direction_norm
                        actor.apply_control(control)
                        if direction_norm < 1.0:
                            self._actor_dict[actor] = self._actor_dict[actor][1:]
                            if self._actor_dict[actor] is None:
                                success = True
                    else:
                        control = actor.get_control()
                        control.speed = self._target_speed
                        control.direction = CarlaDataProvider.get_transform(actor).rotation.get_forward_vector()
                        actor.apply_control(control)

        if success:
            new_status = py_trees.common.Status.SUCCESS

        return new_status

    def terminate(self, new_status):
        """
        On termination of this behavior,
        the controls should be set back to 0.
        """
        for actor in self._local_planner_dict:
            if actor is not None and actor.is_alive:
                control, _ = get_actor_control(actor)
                actor.apply_control(control)
                local_planner = self._local_planner_dict[actor]
                if local_planner is not None and local_planner != "Walker":
                    local_planner.reset_vehicle()
                    local_planner = None

        self._local_planner_dict = {}
        self._actor_dict = {}
        super(WaypointFollower, self).terminate(new_status)


class LaneChange(WaypointFollower):

    """
    This class inherits from the class WaypointFollower.

    This class contains an atomic lane change behavior to a parallel lane.
    The vehicle follows a waypoint plan to the other lane, which is calculated in the initialise method.
    This waypoint plan is calculated with a scenario helper function.

    If an impossible lane change is asked for (due to the lack of lateral lanes,
    next waypoints, continuous line, etc) the atomic will return a plan with the
    waypoints until such impossibility is found.

    Important parameters:
    - actor: CARLA actor to execute the behavior
    - speed: speed of the actor for the lane change, in m/s
    - direction: 'right' or 'left', depending on which lane to change
    - distance_same_lane: straight distance before lane change, in m
    - distance_other_lane: straight distance after lane change, in m
    - distance_lane_change: straight distance for the lane change itself, in m

    The total distance driven is greater than the sum of distance_same_lane and distance_other_lane.
    It results from the lane change distance plus the distance_same_lane plus distance_other_lane.
    The lane change distance is set to 25m (straight), the driven distance is slightly greater.

    A parallel termination behavior has to be used.
    """

    def __init__(self, actor, speed=10, direction='left', distance_same_lane=5, distance_other_lane=100,
                 distance_lane_change=25, lane_changes=1, name='LaneChange'):

        self._direction = direction
        self._distance_same_lane = distance_same_lane
        self._distance_other_lane = distance_other_lane
        self._distance_lane_change = distance_lane_change
        self._lane_changes = lane_changes

        self._target_lane_id = None
        self._distance_new_lane = 0
        self._pos_before_lane_change = None
        self._plan = None

        super(LaneChange, self).__init__(actor, target_speed=speed, name=name)

    def initialise(self):

        # get start position
        position_actor = CarlaDataProvider.get_map().get_waypoint(self._actor.get_location())

        # calculate plan with scenario_helper function
        self._plan, self._target_lane_id = generate_target_waypoint_list_multilane(
            position_actor, self._direction, self._distance_same_lane,
            self._distance_other_lane, self._distance_lane_change, check=True, lane_changes=self._lane_changes)
        super(LaneChange, self).initialise()

    def update(self):

        if not self._plan:
            print("{} couldn't perform the expected lane change".format(self._actor))
            return py_trees.common.Status.FAILURE

        status = super(LaneChange, self).update()

        current_position_actor = CarlaDataProvider.get_map().get_waypoint(self._actor.get_location())
        current_lane_id = current_position_actor.lane_id

        if current_lane_id == self._target_lane_id:
            # driving on new lane
            distance = current_position_actor.transform.location.distance(self._pos_before_lane_change)

            if distance > self._distance_other_lane:
                # long enough distance on new lane --> SUCCESS
                status = py_trees.common.Status.SUCCESS
        else:
            self._pos_before_lane_change = current_position_actor.transform.location

        return status


class SetInitSpeed(AtomicBehavior):

    """
    This class contains an atomic behavior to set the init_speed of an actor,
    succeding immeditely after initializing
    """

    def __init__(self, actor, init_speed=10, name='SetInitSpeed'):

        self._init_speed = init_speed
        self._terminate = None
        self._actor = actor

        super(SetInitSpeed, self).__init__(name, actor)

    def initialise(self):
        """
        Initialize it's speed
        """

        transform = self._actor.get_transform()
        yaw = transform.rotation.yaw * (math.pi / 180)

        vx = math.cos(yaw) * self._init_speed
        vy = math.sin(yaw) * self._init_speed
        self._actor.set_target_velocity(carla.Vector3D(vx, vy, 0))

    def update(self):
        """
        Nothing to update, end the behavior
        """

        return py_trees.common.Status.SUCCESS


class HandBrakeVehicle(AtomicBehavior):

    """
    This class contains an atomic hand brake behavior.
    To set the hand brake value of the vehicle.

    Important parameters:
    - vehicle: CARLA actor to execute the behavior
    - hand_brake_value to be applied in [0,1]

    The behavior terminates after setting the hand brake value
    """

    def __init__(self, vehicle, hand_brake_value, name="Braking"):
        """
        Setup vehicle control and brake value
        """
        super(HandBrakeVehicle, self).__init__(name)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))
        self._vehicle = vehicle
        self._control, self._type = get_actor_control(vehicle)
        self._hand_brake_value = hand_brake_value

    def update(self):
        """
        Set handbrake
        """
        new_status = py_trees.common.Status.SUCCESS
        if self._type == 'vehicle':
            self._control.hand_brake = self._hand_brake_value
            self._vehicle.apply_control(self._control)
        else:
            self._hand_brake_value = None
            self.logger.debug("%s.update()[%s->%s]" %
                              (self.__class__.__name__, self.status, new_status))
            self._vehicle.apply_control(self._control)

        return new_status


class ActorDestroy(AtomicBehavior):

    """
    This class contains an actor destroy behavior.
    Given an actor this behavior will delete it.

    Important parameters:
    - actor: CARLA actor to be deleted

    The behavior terminates after removing the actor
    """

    def __init__(self, actor, name="ActorDestroy"):
        """
        Setup actor
        """
        super(ActorDestroy, self).__init__(name, actor)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))

    def update(self):
        new_status = py_trees.common.Status.RUNNING
        if self._actor:
            CarlaDataProvider.remove_actor_by_id(self._actor.id)
            self._actor = None
            new_status = py_trees.common.Status.SUCCESS

        return new_status


class ActorTransformSetter(AtomicBehavior):

    """
    This class contains an atomic behavior to set the transform
    of an actor.

    Important parameters:
    - actor: CARLA actor to execute the behavior
    - transform: New target transform (position + orientation) of the actor
    - physics [optional]: If physics is true, the actor physics will be reactivated upon success

    The behavior terminates when actor is set to the new actor transform (closer than 1 meter)

    NOTE:
    It is very important to ensure that the actor location is spawned to the new transform because of the
    appearence of a rare runtime processing error. WaypointFollower with LocalPlanner,
    might fail if new_status is set to success before the actor is really positioned at the new transform.
    Therefore: calculate_distance(actor, transform) < 1 meter
    """

    def __init__(self, actor, transform, physics=True, name="ActorTransformSetter"):
        """
        Init
        """
        super(ActorTransformSetter, self).__init__(name, actor)
        self._transform = transform
        self._physics = physics
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))

    def initialise(self):
        if self._actor.is_alive:
            self._actor.set_target_velocity(carla.Vector3D(0, 0, 0))
            self._actor.set_target_angular_velocity(carla.Vector3D(0, 0, 0))
            self._actor.set_transform(self._transform)
        super(ActorTransformSetter, self).initialise()

    def update(self):
        """
        Transform actor
        """
        new_status = py_trees.common.Status.RUNNING

        if not self._actor.is_alive:
            new_status = py_trees.common.Status.FAILURE

        if calculate_distance(self._actor.get_location(), self._transform.location) < 1.0:
            if self._physics:
                self._actor.set_simulate_physics(enabled=True)
            new_status = py_trees.common.Status.SUCCESS

        return new_status


class TrafficLightStateSetter(AtomicBehavior):

    """
    This class contains an atomic behavior to set the state of a given traffic light

    Args:
        actor (carla.TrafficLight): ID of the traffic light that shall be changed
        state (carla.TrafficLightState): New target state

    The behavior terminates after trying to set the new state
    """

    def __init__(self, actor, state, name="TrafficLightStateSetter"):
        """
        Init
        """
        super(TrafficLightStateSetter, self).__init__(name)

        self._actor = actor if "traffic_light" in actor.type_id else None
        self._state = state
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))

    def update(self):
        """
        Change the state of the traffic light
        """
        if self._actor is None:
            return py_trees.common.Status.FAILURE

        new_status = py_trees.common.Status.RUNNING
        if self._actor.is_alive:
            self._actor.set_state(self._state)
            new_status = py_trees.common.Status.SUCCESS
        else:
            # For some reason the actor is gone...
            new_status = py_trees.common.Status.FAILURE

        return new_status


class ActorSource(AtomicBehavior):

    """
    Implementation for a behavior that will indefinitely create actors
    at a given transform if no other actor exists in a given radius
    from the transform.

    Important parameters:
    - actor_type_list: Type of CARLA actors to be spawned
    - transform: Spawn location
    - threshold: Min available free distance between other actors and the spawn location
    - blackboard_queue_name: Name of the blackboard used to control this behavior
    - actor_limit [optional]: Maximum number of actors to be spawned (default=7)

    A parallel termination behavior has to be used.
    """

    def __init__(self, actor_type_list, transform, threshold, blackboard_queue_name,
                 actor_limit=7, name="ActorSource"):
        """
        Setup class members
        """
        super(ActorSource, self).__init__(name)
        self._world = CarlaDataProvider.get_world()
        self._actor_types = actor_type_list
        self._spawn_point = transform
        self._threshold = threshold
        self._queue = Blackboard().get(blackboard_queue_name)
        self._actor_limit = actor_limit
        self._last_blocking_actor = None

    def update(self):
        new_status = py_trees.common.Status.RUNNING
        if self._actor_limit > 0:
            world_actors = self._world.get_actors()
            spawn_point_blocked = False
            if (self._last_blocking_actor and
                    self._spawn_point.location.distance(self._last_blocking_actor.get_location()) < self._threshold):
                spawn_point_blocked = True

            if not spawn_point_blocked:
                for actor in world_actors:
                    if self._spawn_point.location.distance(actor.get_location()) < self._threshold:
                        spawn_point_blocked = True
                        self._last_blocking_actor = actor
                        break

            if not spawn_point_blocked:
                try:
                    new_actor = CarlaDataProvider.request_new_actor(
                        random.choice(self._actor_types), self._spawn_point)
                    self._actor_limit -= 1
                    self._queue.put(new_actor)
                except:                             # pylint: disable=bare-except
                    print("ActorSource unable to spawn actor")
        return new_status


class ActorSink(AtomicBehavior):

    """
    Implementation for a behavior that will indefinitely destroy actors
    that wander near a given location within a specified threshold.

    Important parameters:
    - actor_type_list: Type of CARLA actors to be spawned
    - sink_location: Location (carla.location) at which actors will be deleted
    - threshold: Distance around sink_location in which actors will be deleted

    A parallel termination behavior has to be used.
    """

    def __init__(self, sink_location, threshold, name="ActorSink"):
        """
        Setup class members
        """
        super(ActorSink, self).__init__(name)
        self._sink_location = sink_location
        self._threshold = threshold

    def update(self):
        new_status = py_trees.common.Status.RUNNING
        CarlaDataProvider.remove_actors_in_surrounding(self._sink_location, self._threshold)
        return new_status


class ActorFlow(AtomicBehavior):
    """
    Behavior that indefinitely creates actors at a location,
    controls them until another location, and then destroys them.
    Therefore, a parallel termination behavior has to be used.

    Important parameters:
    - source_transform (carla.Transform): Transform at which actors will be spawned
    - sink_location (carla.Location): Location at which actors will be deleted
    - spawn_distance: Distance between spawned actors
    - sink_distance: Actors closer to the sink than this distance will be deleted
    """

    def __init__(self, source_wp, sink_wp, spawn_dist_interval, sink_dist=2, actors_speed=30/3.6, name="ActorFlow"):
        """
        Setup class members
        """
        super(ActorFlow, self).__init__(name)
        self._rng = random.RandomState(2000)
        self._world = CarlaDataProvider.get_world()

        self._source_wp = source_wp
        self._sink_wp = sink_wp

        self._sink_location = self._sink_wp.transform.location
        self._source_transform = self._source_wp.transform

        self._sink_dist = sink_dist
        self._speed = actors_speed

        self._min_spawn_dist = spawn_dist_interval[0]
        self._max_spawn_dist = spawn_dist_interval[1]
        self._spawn_dist = self._rng.uniform(self._min_spawn_dist, self._max_spawn_dist)

        self._actor_agent_list = []
        self._route = []
        self._grp = GlobalRoutePlanner(CarlaDataProvider.get_map(), 2.0)
        self._route = self._grp.trace_route(self._source_wp.transform.location, self._sink_wp.transform.location)

    def update(self):
        """Controls the created actors and creaes / removes other when needed"""
        # Control the vehicles, removing them when needed
        for actor_info in list(self._actor_agent_list):
            actor, agent = actor_info
            sink_distance = self._sink_location.distance(CarlaDataProvider.get_location(actor))
            if sink_distance < self._sink_dist:
                actor.destroy()
                self._actor_agent_list.remove(actor_info)
            else:
                control = agent.run_step()
                actor.apply_control(control)

        # Spawn new actors if needed
        if len(self._actor_agent_list) == 0:
            distance = self._spawn_dist + 1
        else:
            actor_location = CarlaDataProvider.get_location(self._actor_agent_list[-1][0])
            distance = self._source_transform.location.distance(actor_location)

        if distance > self._spawn_dist:
            actor = CarlaDataProvider.request_new_actor(
                'vehicle.*', self._source_transform, rolename='scenario', safe_blueprint=True, tick=False
            )
            if actor is not None:
                actor_agent = BasicAgent(actor, 3.6 * self._speed)
                actor_agent.set_global_plan(self._route, False)
                self._actor_agent_list.append([actor, actor_agent])
                self._spawn_dist = self._rng.uniform(self._min_spawn_dist, self._max_spawn_dist)

                ground_loc = self._world.ground_projection(self._source_transform.location, 2)
                if ground_loc.location:
                    initial_location = ground_loc.location
                    initial_location.z += 0.06
                    actor.set_location(initial_location)

        return py_trees.common.Status.RUNNING

    def terminate(self, new_status):
        """
        Default terminate. Can be extended in derived class
        """
        try:
            for actor, _ in self._actor_agent_list:
                actor.destroy()
        except RuntimeError:
            pass  # Actor was already destroyed


class TrafficLightFreezer(AtomicBehavior):
    """
    Behavior that freezes a group of traffic lights for a specific amount of time,
    returning them to their original state after ending it

    Important parameters:
    - traffic_lights_dict dict{carla.TrafficLight: carla.TrafficLightState}
    - timeout: Amount of time the traffic lights are frozen
    """

    def __init__(self, traffic_lights_dict, duration=10000, name="TrafficLightFreezer"):
        """Setup class members"""
        super(TrafficLightFreezer, self).__init__(name)
        self._traffic_lights_dict = traffic_lights_dict
        self._duration = duration
        self._previous_traffic_light_info = {}
        self._start_time = None

    def initialise(self):
        """
        Sets the traffic lights to the desired states and remembers their previous one.
        These should technically be frozen, but that freezes all tls in the town
        """
        self._start_time = GameTime.get_time()
        for tl in self._traffic_lights_dict:
            elapsed_time = tl.get_elapsed_time()
            self._previous_traffic_light_info[tl] = {
                'state': tl.get_state(),
                'green_time': tl.get_green_time(),
                'red_time': tl.get_red_time(),
                'yellow_time': tl.get_yellow_time()
            }
            tl.set_state(self._traffic_lights_dict[tl])
            tl.set_green_time(self._duration + elapsed_time)
            tl.set_red_time(self._duration + elapsed_time)
            tl.set_yellow_time(self._duration + elapsed_time)

    def update(self):
        """Waits until the adequate time has passed"""
        if GameTime.get_time() - self._start_time > self._duration:
            return py_trees.common.Status.SUCCESS
        else:
            return py_trees.common.Status.RUNNING

    def terminate(self, new_status):
        """Reset all traffic lights back to their previous states"""
        if self._previous_traffic_light_info:
            for tl in self._traffic_lights_dict:
                tl.set_state(self._previous_traffic_light_info[tl]['state'])
                tl.set_green_time(self._previous_traffic_light_info[tl]['green_time'])
                tl.set_red_time(self._previous_traffic_light_info[tl]['red_time'])
                tl.set_yellow_time(self._previous_traffic_light_info[tl]['yellow_time'])


class StartRecorder(AtomicBehavior):

    """
    Atomic that starts the CARLA recorder. Only one can be active
    at a time, and if this isn't the case, the recorder will
    automatically stop the previous one.

    Args:
        recorder_name (str): name of the file to write the recorded data.
            Remember that a simple name will save the recording in
            'CarlaUE4/Saved/'. Otherwise, if some folder appears in the name,
            it will be considered an absolute path.
        name (str): name of the behavior
    """

    def __init__(self, recorder_name, name="StartRecorder"):
        """
        Setup class members
        """
        super(StartRecorder, self).__init__(name)
        self._client = CarlaDataProvider.get_client()
        self._recorder_name = recorder_name

    def update(self):
        self._client.start_recorder(self._recorder_name)
        return py_trees.common.Status.SUCCESS


class StopRecorder(AtomicBehavior):

    """
    Atomic that stops the CARLA recorder.

    Args:
        name (str): name of the behavior
    """

    def __init__(self, name="StopRecorder"):
        """
        Setup class members
        """
        super(StopRecorder, self).__init__(name)
        self._client = CarlaDataProvider.get_client()

    def update(self):
        self._client.stop_recorder()
        return py_trees.common.Status.SUCCESS


class TrafficLightManipulator(AtomicBehavior):

    """
    Atomic behavior that manipulates traffic lights around the ego_vehicle to trigger scenarios 7 to 10.
    This is done by setting 2 of the traffic light at the intersection to green (with some complex precomputation
    to set everything up).

    Important parameters:
    - ego_vehicle: CARLA actor that controls this behavior
    - subtype: string that gathers information of the route and scenario number
      (check SUBTYPE_CONFIG_TRANSLATION below)
    """

    RED = carla.TrafficLightState.Red
    YELLOW = carla.TrafficLightState.Yellow
    GREEN = carla.TrafficLightState.Green

    # Time constants
    RED_TIME = 1.5  # Minimum time the ego vehicle waits in red (seconds)
    YELLOW_TIME = 2  # Time spent at yellow state (seconds)
    RESET_TIME = 6  # Time waited before resetting all the junction (seconds)

    # Experimental values
    TRIGGER_DISTANCE = 10  # Distance that makes all vehicles in the lane enter the junction (meters)
    DIST_TO_WAITING_TIME = 0.04  # Used to wait longer at larger intersections (s/m)

    INT_CONF_OPP1 = {'ego': RED, 'ref': RED, 'left': RED, 'right': RED, 'opposite': GREEN}
    INT_CONF_OPP2 = {'ego': GREEN, 'ref': GREEN, 'left': RED, 'right': RED, 'opposite': GREEN}
    INT_CONF_LFT1 = {'ego': RED, 'ref': RED, 'left': GREEN, 'right': RED, 'opposite': RED}
    INT_CONF_LFT2 = {'ego': GREEN, 'ref': GREEN, 'left': GREEN, 'right': RED, 'opposite': RED}
    INT_CONF_RGT1 = {'ego': RED, 'ref': RED, 'left': RED, 'right': GREEN, 'opposite': RED}
    INT_CONF_RGT2 = {'ego': GREEN, 'ref': GREEN, 'left': RED, 'right': GREEN, 'opposite': RED}

    INT_CONF_REF1 = {'ego': GREEN, 'ref': GREEN, 'left': RED, 'right': RED, 'opposite': RED}
    INT_CONF_REF2 = {'ego': YELLOW, 'ref': YELLOW, 'left': RED, 'right': RED, 'opposite': RED}

    # Depending on the scenario, IN ORDER OF IMPORTANCE, the traffic light changed
    # The list has to contain only items of the INT_CONF
    SUBTYPE_CONFIG_TRANSLATION = {
        'S7left': ['left', 'opposite', 'right'],
        'S7right': ['left', 'opposite'],
        'S7opposite': ['right', 'left', 'opposite'],
        'S8left': ['opposite'],
        'S9right': ['left', 'opposite']
    }

    CONFIG_TLM_TRANSLATION = {
        'left': [INT_CONF_LFT1, INT_CONF_LFT2],
        'right': [INT_CONF_RGT1, INT_CONF_RGT2],
        'opposite': [INT_CONF_OPP1, INT_CONF_OPP2]
    }

    def __init__(self, ego_vehicle, subtype, debug=False, name="TrafficLightManipulator"):
        super(TrafficLightManipulator, self).__init__(name)
        self.ego_vehicle = ego_vehicle
        self.subtype = subtype
        self.current_step = 1
        self.debug = debug

        self.traffic_light = None
        self.annotations = None
        self.configuration = None
        self.prev_junction_state = None
        self.junction_location = None
        self.seconds_waited = 0
        self.prev_time = None
        self.max_trigger_distance = None
        self.waiting_time = None
        self.inside_junction = False

        self.logger.debug("%s.__init__()" % (self.__class__.__name__))

    def update(self):

        new_status = py_trees.common.Status.RUNNING

        # 1) Set up the parameters
        if self.current_step == 1:

            # Traffic light affecting the ego vehicle
            self.traffic_light = CarlaDataProvider.get_next_traffic_light(self.ego_vehicle, use_cached_location=False)
            if not self.traffic_light:
                # nothing else to do in this iteration...
                return new_status

            # "Topology" of the intersection
            self.annotations = CarlaDataProvider.annotate_trafficlight_in_group(self.traffic_light)

            # Which traffic light will be modified (apart from the ego lane)
            self.configuration = self.get_traffic_light_configuration(self.subtype, self.annotations)
            if self.configuration is None:
                self.current_step = 0  # End the behavior
                return new_status

            # Modify the intersection. Store the previous state
            self.prev_junction_state = self.set_intersection_state(self.INT_CONF_REF1)

            self.current_step += 1
            if self.debug:
                print("--- All set up")

        # 2) Modify the ego lane to yellow when closeby
        elif self.current_step == 2:

            ego_location = CarlaDataProvider.get_location(self.ego_vehicle)

            if self.junction_location is None:
                ego_waypoint = CarlaDataProvider.get_map().get_waypoint(ego_location)
                junction_waypoint = ego_waypoint.next(0.5)[0]
                while not junction_waypoint.is_junction:
                    next_wp = junction_waypoint.next(0.5)[0]
                    junction_waypoint = next_wp
                self.junction_location = junction_waypoint.transform.location

            distance = ego_location.distance(self.junction_location)

            # Failure check
            if self.max_trigger_distance is None:
                self.max_trigger_distance = distance + 1
            if distance > self.max_trigger_distance:
                self.current_step = 0

            elif distance < self.TRIGGER_DISTANCE:
                _ = self.set_intersection_state(self.INT_CONF_REF2)
                self.current_step += 1

            if self.debug:
                print("--- Distance until traffic light changes: {}".format(distance))

        # 3) Modify the ego lane to red and the chosen one to green after several seconds
        elif self.current_step == 3:

            if self.passed_enough_time(self.YELLOW_TIME):
                _ = self.set_intersection_state(self.CONFIG_TLM_TRANSLATION[self.configuration][0])

                self.current_step += 1

        # 4) Wait a bit to let vehicles enter the intersection, then set the ego lane to green
        elif self.current_step == 4:

            # Get the time in red, dependent on the intersection dimensions
            if self.waiting_time is None:
                self.waiting_time = self.get_waiting_time(self.annotations, self.configuration)

            if self.passed_enough_time(self.waiting_time):
                _ = self.set_intersection_state(self.CONFIG_TLM_TRANSLATION[self.configuration][1])

                self.current_step += 1

        # 5) Wait for the end of the intersection
        elif self.current_step == 5:
            # the traffic light has been manipulated, wait until the vehicle finsihes the intersection
            ego_location = CarlaDataProvider.get_location(self.ego_vehicle)
            ego_waypoint = CarlaDataProvider.get_map().get_waypoint(ego_location)

            if not self.inside_junction:
                if ego_waypoint.is_junction:
                    # Wait for the ego_vehicle to enter a junction
                    self.inside_junction = True
                else:
                    if self.debug:
                        print("--- Waiting to ENTER a junction")

            else:
                if ego_waypoint.is_junction:
                    if self.debug:
                        print("--- Waiting to EXIT a junction")
                else:
                    # And to leave it
                    self.inside_junction = False
                    self.current_step += 1

        # 6) At the end (or if something failed), reset to the previous state
        else:
            if self.prev_junction_state:
                CarlaDataProvider.reset_lights(self.prev_junction_state)
                if self.debug:
                    print("--- Returning the intersection to its previous state")

            self.variable_cleanup()
            new_status = py_trees.common.Status.SUCCESS

        return new_status

    def passed_enough_time(self, time_limit):
        """
        Returns true or false depending on the time that has passed from the
        first time this function was called
        """
        # Start the timer
        if self.prev_time is None:
            self.prev_time = GameTime.get_time()

        timestamp = GameTime.get_time()
        self.seconds_waited += (timestamp - self.prev_time)
        self.prev_time = timestamp

        if self.debug:
            print("--- Waited seconds: {}".format(self.seconds_waited))

        if self.seconds_waited >= time_limit:
            self.seconds_waited = 0
            self.prev_time = None

            return True
        return False

    def set_intersection_state(self, choice):
        """
        Changes the intersection to the desired state
        """
        prev_state = CarlaDataProvider.update_light_states(
            self.traffic_light,
            self.annotations,
            choice,
            freeze=True)

        return prev_state

    def get_waiting_time(self, annotation, direction):
        """
        Calculates the time the ego traffic light will remain red
        to let vehicles enter the junction
        """

        tl = annotation[direction][0]
        ego_tl = annotation["ref"][0]

        tl_location = CarlaDataProvider.get_trafficlight_trigger_location(tl)
        ego_tl_location = CarlaDataProvider.get_trafficlight_trigger_location(ego_tl)

        distance = ego_tl_location.distance(tl_location)

        return self.RED_TIME + distance * self.DIST_TO_WAITING_TIME

    def get_traffic_light_configuration(self, subtype, annotations):
        """
        Checks the list of possible altered traffic lights and gets
        the first one that exists in the intersection

        Important parameters:
        - subtype: Subtype of the scenario
        - annotations: list of the traffic light of the junction, with their direction (right, left...)
        """
        configuration = None

        if subtype in self.SUBTYPE_CONFIG_TRANSLATION:
            possible_configurations = self.SUBTYPE_CONFIG_TRANSLATION[self.subtype]
            while possible_configurations:
                # Chose the first one and delete it
                configuration = possible_configurations[0]
                possible_configurations = possible_configurations[1:]
                if configuration in annotations:
                    if annotations[configuration]:
                        # Found a valid configuration
                        break
                    else:
                        # The traffic light doesn't exist, get another one
                        configuration = None
                else:
                    if self.debug:
                        print("This configuration name is wrong")
                    configuration = None

            if configuration is None and self.debug:
                print("This subtype has no traffic light available")
        else:
            if self.debug:
                print("This subtype is unknown")

        return configuration

    def variable_cleanup(self):
        """
        Resets all variables to the intial state
        """
        self.current_step = 1
        self.traffic_light = None
        self.annotations = None
        self.configuration = None
        self.prev_junction_state = None
        self.junction_location = None
        self.max_trigger_distance = None
        self.waiting_time = None
        self.inside_junction = False


class ScenarioTriggerer(AtomicBehavior):

    """
    Handles the triggering of the scenarios that are part of a route.

    Initializes a list of blackboard variables to False, and only sets them to True when
    the ego vehicle is very close to the scenarios
    """

    WINDOWS_SIZE = 5

    def __init__(self, actor, route, blackboard_list, distance,
                 repeat_scenarios=False, debug=False, name="ScenarioTriggerer"):
        """
        Setup class members
        """
        super(ScenarioTriggerer, self).__init__(name)
        self._world = CarlaDataProvider.get_world()
        self._map = CarlaDataProvider.get_map()
        self._repeat = repeat_scenarios
        self._debug = debug

        self._actor = actor
        self._route = route
        self._distance = distance
        self._blackboard_list = blackboard_list
        self._triggered_scenarios = []  # List of already done scenarios

        self._current_index = 0
        self._route_length = len(self._route)
        self._waypoints, _ = zip(*self._route)

    def update(self):
        new_status = py_trees.common.Status.RUNNING

        location = CarlaDataProvider.get_location(self._actor)
        if location is None:
            return new_status

        lower_bound = self._current_index
        upper_bound = min(self._current_index + self.WINDOWS_SIZE + 1, self._route_length)

        shortest_distance = float('inf')
        closest_index = -1

        for index in range(lower_bound, upper_bound):
            ref_waypoint = self._waypoints[index]
            ref_location = ref_waypoint.location

            dist_to_route = ref_location.distance(location)
            if dist_to_route <= shortest_distance:
                closest_index = index
                shortest_distance = dist_to_route

        if closest_index == -1 or shortest_distance == float('inf'):
            return new_status

        # Update the ego position at the route
        self._current_index = closest_index

        route_location = self._waypoints[closest_index].location

        # Check which scenarios can be triggered
        blackboard = py_trees.blackboard.Blackboard()
        for black_var_name, scen_location in self._blackboard_list:

            # Close enough
            scen_distance = route_location.distance(scen_location)
            condition1 = bool(scen_distance < self._distance)

            # Not being currently done
            value = blackboard.get(black_var_name)
            condition2 = bool(not value)

            # Already done, if needed
            condition3 = bool(self._repeat or black_var_name not in self._triggered_scenarios)

            if condition1 and condition2 and condition3:
                _ = blackboard.set(black_var_name, True)
                self._triggered_scenarios.append(black_var_name)

                if self._debug:
                    self._world.debug.draw_point(
                        scen_location + carla.Location(z=4),
                        size=0.5,
                        life_time=0.5,
                        color=carla.Color(255, 255, 0)
                    )
                    self._world.debug.draw_string(
                        scen_location + carla.Location(z=5),
                        str(black_var_name),
                        False,
                        color=carla.Color(0, 0, 0),
                        life_time=1000
                    )

        return new_status


class KeepLongitudinalGap(AtomicBehavior):
    """
    This class contains an atomic behavior to maintain a set gap with leading/adjacent vehicle.

    Important parameters:
    - actor: CARLA actor to execute the behavior
    - reference_actor: Reference actor the distance shall be kept to.
    - distance: target gap between the two actors in meters
    - distance_type: Specifies how distance should be calculated between the two actors

    The behavior terminates after overwritten by other events / when target distance is reached(if continues).
    """

    def __init__(self, actor, reference_actor, gap, gap_type="distance", max_speed=None, continues=False,
                 freespace=False, name="AutoKeepDistance"):
        """
        Setup parameters
        """
        super(KeepLongitudinalGap, self).__init__(name, actor)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))
        self._reference_actor = reference_actor
        self._gap = gap
        self._gap_type = gap_type
        self._continues = continues
        self._freespace = freespace
        self._global_rp = None
        max_speed_limit = 100
        self.max_speed = max_speed_limit if max_speed is None else float(max_speed)
        if freespace and self._gap_type == "distance":
            self._gap += self._reference_actor.bounding_box.extent.x + self._actor.bounding_box.extent.x

        self._start_time = None

    def initialise(self):
        actor_dict = {}

        try:
            check_actors = operator.attrgetter("ActorsWithController")
            actor_dict = check_actors(py_trees.blackboard.Blackboard())
        except AttributeError:
            pass

        if not actor_dict or self._actor.id not in actor_dict:
            raise RuntimeError("Actor not found in ActorsWithController BlackBoard")

        self._start_time = GameTime.get_time()
        actor_dict[self._actor.id].update_target_speed(self.max_speed, start_time=self._start_time)

        self._global_rp = GlobalRoutePlanner(CarlaDataProvider.get_world().get_map(), 1.0)

        super(KeepLongitudinalGap, self).initialise()

    def update(self):
        """
        keeps track of gap and update the controller accordingly
        """
        try:
            check_actors = operator.attrgetter("ActorsWithController")
            actor_dict = check_actors(py_trees.blackboard.Blackboard())
        except AttributeError:
            pass

        if not actor_dict or self._actor.id not in actor_dict:
            return py_trees.common.Status.FAILURE

        if actor_dict[self._actor.id].get_last_longitudinal_command() != self._start_time:
            return py_trees.common.Status.SUCCESS

        new_status = py_trees.common.Status.RUNNING

        actor_velocity = CarlaDataProvider.get_velocity(self._actor)
        reference_velocity = CarlaDataProvider.get_velocity(self._reference_actor)

        gap = sr_tools.scenario_helper.get_distance_between_actors(self._actor, self._reference_actor,
                                                                   distance_type="longitudinal",
                                                                   freespace=self._freespace,
                                                                   global_planner=self._global_rp)
        actor_transform = CarlaDataProvider.get_transform(self._actor)
        ref_actor_transform = CarlaDataProvider.get_transform(self._reference_actor)
        if is_within_distance(ref_actor_transform, actor_transform, float('inf'), [0, 90]) and \
                operator.le(gap, self._gap):
            try:
                factor = abs(actor_velocity - reference_velocity)/actor_velocity
                if actor_velocity > reference_velocity:
                    actor_velocity = actor_velocity - (factor*actor_velocity)
                elif actor_velocity < reference_velocity and operator.gt(gap, self._gap):
                    actor_velocity = actor_velocity + (factor*actor_velocity)
            except ZeroDivisionError:
                pass
            actor_dict[self._actor.id].update_target_speed(actor_velocity)

            if not self._continues:
                if operator.le(gap, self._gap):
                    new_status = py_trees.common.Status.SUCCESS
        else:
            actor_dict[self._actor.id].update_target_speed(self.max_speed)

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))
        return new_status
