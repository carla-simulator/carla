#!/usr/bin/env python

# Copyright (c) 2018-2020 Intel Corporation
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
This module provides all atomic scenario behaviors that reflect
trigger conditions to either activate another behavior, or to stop
another behavior.

For example, such a condition could be "InTriggerRegion", which checks
that a given actor reached a certain region on the map, and then starts/stops
a behavior of this actor.

The atomics are implemented with py_trees and make use of the AtomicCondition
base class
"""

from __future__ import print_function

import operator
import datetime
import math
import py_trees
import carla

from agents.navigation.global_route_planner import GlobalRoutePlanner

from srunner.scenariomanager.scenarioatomics.atomic_behaviors import calculate_distance
from srunner.scenariomanager.carla_data_provider import CarlaDataProvider
from srunner.scenariomanager.timer import GameTime
from srunner.tools.scenario_helper import get_distance_along_route

import srunner.tools as sr_tools

EPSILON = 0.001


class AtomicCondition(py_trees.behaviour.Behaviour):

    """
    Base class for all atomic conditions used to setup a scenario

    *All behaviors should use this class as parent*

    Important parameters:
    - name: Name of the atomic condition
    """

    def __init__(self, name):
        """
        Default init. Has to be called via super from derived class
        """
        super(AtomicCondition, self).__init__(name)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))
        self.name = name

    def setup(self, unused_timeout=15):
        """
        Default setup
        """
        self.logger.debug("%s.setup()" % (self.__class__.__name__))
        return True

    def initialise(self):
        """
        Initialise setup
        """
        self.logger.debug("%s.initialise()" % (self.__class__.__name__))

    def terminate(self, new_status):
        """
        Default terminate. Can be extended in derived class
        """
        self.logger.debug("%s.terminate()[%s->%s]" % (self.__class__.__name__, self.status, new_status))


class InTriggerDistanceToOSCPosition(AtomicCondition):

    """
    OpenSCENARIO atomic
    This class contains the trigger condition for a distance to an OpenSCENARIO position

    Args:
        actor (carla.Actor): CARLA actor to execute the behavior
        osc_position (str): OpenSCENARIO position
        distance (float): Trigger distance between the actor and the target location in meters
        name (str): Name of the condition

    The condition terminates with SUCCESS, when the actor reached the target distance to the openSCENARIO position
    """

    def __init__(self, actor, osc_position, distance, along_route=False,
                 comparison_operator=operator.lt, name="InTriggerDistanceToOSCPosition"):
        """
        Setup parameters
        """
        super(InTriggerDistanceToOSCPosition, self).__init__(name)
        self._actor = actor
        self._osc_position = osc_position
        self._distance = distance
        self._along_route = along_route
        self._comparison_operator = comparison_operator
        self._map = CarlaDataProvider.get_map()

        if self._along_route:
            # Get the global route planner, used to calculate the route
            self._grp = GlobalRoutePlanner(self._map, 0.5)
        else:
            self._grp = None

    def initialise(self):
        if self._distance < 0:
            raise ValueError("distance value must be positive")

    def update(self):
        """
        Check if actor is in trigger distance
        """
        new_status = py_trees.common.Status.RUNNING

        # calculate transform with method in openscenario_parser.py
        osc_transform = sr_tools.openscenario_parser.OpenScenarioParser.convert_position_to_transform(
            self._osc_position)

        if osc_transform is not None:
            osc_location = osc_transform.location
            actor_location = CarlaDataProvider.get_location(self._actor)

            if self._along_route:
                # Global planner needs a location at a driving lane
                actor_location = self._map.get_waypoint(actor_location).transform.location
                osc_location = self._map.get_waypoint(osc_location).transform.location

            distance = calculate_distance(actor_location, osc_location, self._grp)

            if self._comparison_operator(distance, self._distance):
                new_status = py_trees.common.Status.SUCCESS

        return new_status


class InTimeToArrivalToOSCPosition(AtomicCondition):

    """
    OpenSCENARIO atomic
    This class contains a trigger if an actor arrives within a given time to an OpenSCENARIO position

    Important parameters:
    - actor: CARLA actor to execute the behavior
    - osc_position: OpenSCENARIO position
    - time: The behavior is successful, if TTA is less than _time_ in seconds
    - name: Name of the condition

    The condition terminates with SUCCESS, when the actor can reach the position within the given time
    """

    def __init__(self, actor, osc_position, time, along_route=False,
                 comparison_operator=operator.lt, name="InTimeToArrivalToOSCPosition"):
        """
        Setup parameters
        """
        super(InTimeToArrivalToOSCPosition, self).__init__(name)
        self._map = CarlaDataProvider.get_map()
        self._actor = actor
        self._osc_position = osc_position
        self._time = float(time)
        self._along_route = along_route
        self._comparison_operator = comparison_operator

        if self._along_route:
            # Get the global route planner, used to calculate the route
            self._grp = GlobalRoutePlanner(self._map, 0.5)
        else:
            self._grp = None

    def initialise(self):
        if self._time < 0:
            raise ValueError("time value must be positive")

    def update(self):
        """
        Check if actor can arrive within trigger time
        """
        new_status = py_trees.common.Status.RUNNING

        # calculate transform with method in openscenario_parser.py
        try:
            osc_transform = sr_tools.openscenario_parser.OpenScenarioParser.convert_position_to_transform(
                self._osc_position)
        except AttributeError:
            return py_trees.common.Status.FAILURE
        target_location = osc_transform.location
        actor_location = CarlaDataProvider.get_location(self._actor)

        if target_location is None or actor_location is None:
            return new_status

        if self._along_route:
            # Global planner needs a location at a driving lane
            actor_location = self._map.get_waypoint(actor_location).transform.location
            target_location = self._map.get_waypoint(target_location).transform.location

        distance = calculate_distance(actor_location, target_location, self._grp)

        actor_velocity = CarlaDataProvider.get_velocity(self._actor)

        # time to arrival
        if actor_velocity > 0:
            time_to_arrival = distance / actor_velocity
        elif distance == 0:
            time_to_arrival = 0
        else:
            time_to_arrival = float('inf')

        if self._comparison_operator(time_to_arrival, self._time):
            new_status = py_trees.common.Status.SUCCESS
        return new_status


class StandStill(AtomicCondition):

    """
    This class contains a standstill behavior of a scenario

    Important parameters:
    - actor: CARLA actor to execute the behavior
    - name: Name of the condition
    - duration: Duration of the behavior in seconds

    The condition terminates with SUCCESS, when the actor does not move
    """

    def __init__(self, actor, name, duration=float("inf")):
        """
        Setup actor
        """
        super(StandStill, self).__init__(name)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))
        self._actor = actor

        self._duration = duration
        self._start_time = 0

    def initialise(self):
        """
        Initialize the start time of this condition
        """
        self._start_time = GameTime.get_time()
        super(StandStill, self).initialise()

    def update(self):
        """
        Check if the _actor stands still (v=0)
        """
        new_status = py_trees.common.Status.RUNNING

        velocity = CarlaDataProvider.get_velocity(self._actor)

        if velocity > EPSILON:
            self._start_time = GameTime.get_time()

        if GameTime.get_time() - self._start_time > self._duration:
            new_status = py_trees.common.Status.SUCCESS

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))

        return new_status


class RelativeVelocityToOtherActor(AtomicCondition):

    """
    Atomic containing a comparison between an actor's velocity
    and another actor's one. The behavior returns SUCCESS when the
    expected comparison (greater than / less than / equal to) is achieved

    Args:
        actor (carla.Actor): actor from which the velocity is taken
        other_actor (carla.Actor): The actor with the reference velocity
        speed (float): Difference of speed between the actors
        name (string): Name of the condition
        comparison_operator: Type "operator", used to compare the two velocities
    """

    def __init__(self, actor, other_actor, speed, comparison_operator=operator.gt,
                 name="RelativeVelocityToOtherActor"):
        """
        Setup the parameters
        """
        super(RelativeVelocityToOtherActor, self).__init__(name)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))
        self._actor = actor
        self._other_actor = other_actor
        self._relative_speed = speed
        self._comparison_operator = comparison_operator

    def update(self):
        """
        Gets the speed of the two actors and compares them according to the comparison operator

        returns:
            py_trees.common.Status.RUNNING when the comparison fails and
            py_trees.common.Status.SUCCESS when it succeeds
        """
        new_status = py_trees.common.Status.RUNNING

        curr_speed = CarlaDataProvider.get_velocity(self._actor)
        other_speed = CarlaDataProvider.get_velocity(self._other_actor)

        relative_speed = curr_speed - other_speed

        if self._comparison_operator(relative_speed, self._relative_speed):
            new_status = py_trees.common.Status.SUCCESS

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))

        return new_status


class TriggerVelocity(AtomicCondition):

    """
    Atomic containing a comparison between an actor's speed and a reference one.
    The behavior returns SUCCESS when the expected comparison (greater than /
    less than / equal to) is achieved.

    Args:
        actor (carla.Actor): CARLA actor from which the speed will be taken.
        name (string): Name of the atomic
        target_velocity (float): velcoity to be compared with the actor's one
        comparison_operator: Type "operator", used to compare the two velocities
    """

    def __init__(self, actor, target_velocity, comparison_operator=operator.gt, name="TriggerVelocity"):
        """
        Setup the atomic parameters
        """
        super(TriggerVelocity, self).__init__(name)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))
        self._actor = actor
        self._target_velocity = target_velocity
        self._comparison_operator = comparison_operator

    def update(self):
        """
        Gets the speed of the actor and compares it with the reference one

        returns:
            py_trees.common.Status.RUNNING when the comparison fails and
            py_trees.common.Status.SUCCESS when it succeeds
        """
        new_status = py_trees.common.Status.RUNNING

        actor_speed = CarlaDataProvider.get_velocity(self._actor)

        if self._comparison_operator(actor_speed, self._target_velocity):
            new_status = py_trees.common.Status.SUCCESS

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))

        return new_status


class TriggerAcceleration(AtomicCondition):

    """
    Atomic containing a comparison between an actor's acceleration
    and a reference one. The behavior returns SUCCESS when the
    expected comparison (greater than / less than / equal to) is achieved

    Args:
        actor (carla.Actor): CARLA actor to execute the behavior
        name (str): Name of the condition
        target_acceleration (float): Acceleration reference (in m/s^2) on which the success is dependent
        comparison_operator (operator): Type "operator", used to compare the two acceleration
    """

    def __init__(self, actor, target_acceleration, comparison_operator=operator.gt, name="TriggerAcceleration"):
        """
        Setup trigger acceleration
        """
        super(TriggerAcceleration, self).__init__(name)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))
        self._actor = actor
        self._target_acceleration = target_acceleration
        self._comparison_operator = comparison_operator

    def update(self):
        """
        Gets the accleration of the actor and compares it with the reference one

        returns:
            py_trees.common.Status.RUNNING when the comparison fails and
            py_trees.common.Status.SUCCESS when it succeeds
        """
        new_status = py_trees.common.Status.RUNNING

        acceleration = self._actor.get_acceleration()
        linear_accel = math.sqrt(math.pow(acceleration.x, 2) +
                                 math.pow(acceleration.y, 2) +
                                 math.pow(acceleration.z, 2))

        if self._comparison_operator(linear_accel, self._target_acceleration):
            new_status = py_trees.common.Status.SUCCESS

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))

        return new_status


class TimeOfDayComparison(AtomicCondition):

    """
    Atomic containing a comparison between the current time of day of the simulation
    and a given one. The behavior returns SUCCESS when the
    expected comparison (greater than / less than / equal to) is achieved

    Args:
        datetime (datetime): CARLA actor to execute the behavior
        name (str): Name of the condition
        target_acceleration (float): Acceleration reference (in m/s^2) on which the success is dependent
        comparison_operator (operator): Type "operator", used to compare the two acceleration
    """

    def __init__(self, dattime, comparison_operator=operator.gt, name="TimeOfDayComparison"):
        """
        """
        super(TimeOfDayComparison, self).__init__(name)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))
        self._datetime = datetime.datetime.strptime(dattime, "%Y-%m-%dT%H:%M:%S")
        self._comparison_operator = comparison_operator

    def update(self):
        """
        Gets the time of day of the simulation and compares it with the reference one

        returns:
            py_trees.common.Status.RUNNING when the comparison fails and
            py_trees.common.Status.SUCCESS when it succeeds
        """
        new_status = py_trees.common.Status.RUNNING

        try:
            check_dtime = operator.attrgetter("Datetime")
            dtime = check_dtime(py_trees.blackboard.Blackboard())
        except AttributeError:
            pass

        if self._comparison_operator(dtime, self._datetime):
            new_status = py_trees.common.Status.SUCCESS

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))

        return new_status


class OSCStartEndCondition(AtomicCondition):

    """
    This class contains a check if a named story element has started/terminated.

    Important parameters:
    - element_name: The story element's name attribute
    - element_type: The element type [act,scene,maneuver,event,action]
    - rule: Either START or END

    The condition terminates with SUCCESS, when the named story element starts
    """

    def __init__(self, element_type, element_name, rule, name="OSCStartEndCondition"):
        """
        Setup element details
        """
        super(OSCStartEndCondition, self).__init__(name)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))
        self._element_type = element_type.upper()
        self._element_name = element_name
        self._rule = rule.upper()
        self._start_time = None
        self._blackboard = py_trees.blackboard.Blackboard()

    def initialise(self):
        """
        Initialize the start time of this condition
        """
        self._start_time = GameTime.get_time()
        super(OSCStartEndCondition, self).initialise()

    def update(self):
        """
        Check if the specified story element has started/ended since the beginning of the condition
        """
        new_status = py_trees.common.Status.RUNNING

        if new_status == py_trees.common.Status.RUNNING:
            blackboard_variable_name = "({}){}-{}".format(self._element_type, self._element_name, self._rule)
            element_start_time = self._blackboard.get(blackboard_variable_name)
            if element_start_time and element_start_time >= self._start_time:
                new_status = py_trees.common.Status.SUCCESS

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))

        return new_status


class InTriggerRegion(AtomicCondition):

    """
    This class contains the trigger region (condition) of a scenario

    Important parameters:
    - actor: CARLA actor to execute the behavior
    - name: Name of the condition
    - min_x, max_x, min_y, max_y: bounding box of the trigger region

    The condition terminates with SUCCESS, when the actor reached the target region
    """

    def __init__(self, actor, min_x, max_x, min_y, max_y, name="TriggerRegion"):
        """
        Setup trigger region (rectangle provided by
        [min_x,min_y] and [max_x,max_y]
        """
        super(InTriggerRegion, self).__init__(name)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))
        self._actor = actor
        self._min_x = min_x
        self._max_x = max_x
        self._min_y = min_y
        self._max_y = max_y

    def update(self):
        """
        Check if the _actor location is within trigger region
        """
        new_status = py_trees.common.Status.RUNNING

        location = CarlaDataProvider.get_location(self._actor)

        if location is None:
            return new_status

        not_in_region = (location.x < self._min_x or location.x > self._max_x) or \
                        (location.y < self._min_y or location.y > self._max_y)
        if not not_in_region:
            new_status = py_trees.common.Status.SUCCESS

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))

        return new_status


class InTriggerDistanceToVehicle(AtomicCondition):

    """
    This class contains the trigger distance (condition) between to actors
    of a scenario

    Important parameters:
    - actor: CARLA actor to execute the behavior
    - reference_actor: Reference CARLA actor
    - name: Name of the condition
    - distance: Trigger distance between the two actors in meters
    - distance_type: Specifies how distance should be calculated between the two actors
    - freespace: if True distance is calculated between closest boundary points else it will be from center-center
    - dx, dy, dz: distance to reference_location (location of reference_actor)

    The condition terminates with SUCCESS, when the actor reached the target distance to the other actor
    """

    def __init__(self, reference_actor, actor, distance, comparison_operator=operator.lt,
                 distance_type="cartesianDistance", freespace=False, name="TriggerDistanceToVehicle"):
        """
        Setup trigger distance
        """
        super(InTriggerDistanceToVehicle, self).__init__(name)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))
        self._reference_actor = reference_actor
        self._actor = actor
        self._distance = distance
        self._distance_type = distance_type
        self._freespace = freespace
        self._comparison_operator = comparison_operator

        if distance_type == "longitudinal":
            self._global_rp = GlobalRoutePlanner(CarlaDataProvider.get_world().get_map(), 1.0)
        else:
            self._global_rp = None

    def update(self):
        """
        Check if the ego vehicle is within trigger distance to other actor
        """
        new_status = py_trees.common.Status.RUNNING

        location = CarlaDataProvider.get_location(self._actor)
        reference_location = CarlaDataProvider.get_location(self._reference_actor)

        if location is None or reference_location is None:
            return new_status

        distance = sr_tools.scenario_helper.get_distance_between_actors(self._actor,
                                                                        self._reference_actor,
                                                                        distance_type=self._distance_type,
                                                                        freespace=self._freespace,
                                                                        global_planner=self._global_rp)

        if self._comparison_operator(distance, self._distance):
            new_status = py_trees.common.Status.SUCCESS

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))

        return new_status


class InTriggerDistanceToLocation(AtomicCondition):

    """
    This class contains the trigger (condition) for a distance to a fixed
    location of a scenario

    Important parameters:
    - actor: CARLA actor to execute the behavior
    - target_location: Reference location (carla.location)
    - name: Name of the condition
    - distance: Trigger distance between the actor and the target location in meters

    The condition terminates with SUCCESS, when the actor reached the target distance to the given location
    """

    def __init__(self,
                 actor,
                 target_location,
                 distance,
                 comparison_operator=operator.lt,
                 name="InTriggerDistanceToLocation"):
        """
        Setup trigger distance
        """
        super(InTriggerDistanceToLocation, self).__init__(name)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))
        self._target_location = target_location
        self._actor = actor
        self._distance = distance
        self._comparison_operator = comparison_operator

    def update(self):
        """
        Check if the actor is within trigger distance to the target location
        """

        new_status = py_trees.common.Status.RUNNING

        location = CarlaDataProvider.get_location(self._actor)

        if location is None:
            return new_status

        if self._comparison_operator(calculate_distance(
                location, self._target_location), self._distance):
            new_status = py_trees.common.Status.SUCCESS

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))

        return new_status


class InTriggerDistanceToNextIntersection(AtomicCondition):

    """
    This class contains the trigger (condition) for a distance to the
    next intersection of a scenario

    Important parameters:
    - actor: CARLA actor to execute the behavior
    - name: Name of the condition
    - distance: Trigger distance between the actor and the next intersection in meters

    The condition terminates with SUCCESS, when the actor reached the target distance to the next intersection
    """

    def __init__(self, actor, distance, name="InTriggerDistanceToNextIntersection"):
        """
        Setup trigger distance
        """
        super(InTriggerDistanceToNextIntersection, self).__init__(name)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))
        self._actor = actor
        self._distance = distance
        self._map = CarlaDataProvider.get_map()

        waypoint = self._map.get_waypoint(self._actor.get_location())
        while waypoint and not waypoint.is_intersection:
            waypoint = waypoint.next(1)[-1]

        self._final_location = waypoint.transform.location

    def update(self):
        """
        Check if the actor is within trigger distance to the intersection
        """
        new_status = py_trees.common.Status.RUNNING

        current_waypoint = self._map.get_waypoint(CarlaDataProvider.get_location(self._actor))
        distance = calculate_distance(current_waypoint.transform.location, self._final_location)

        if distance < self._distance:
            new_status = py_trees.common.Status.SUCCESS

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))

        return new_status


class InTriggerDistanceToLocationAlongRoute(AtomicCondition):

    """
    Implementation for a behavior that will check if a given actor
    is within a given distance to a given location considering a given route

    Important parameters:
    - actor: CARLA actor to execute the behavior
    - name: Name of the condition
    - distance: Trigger distance between the actor and the next intersection in meters
    - route: Route to be checked
    - location: Location on the route to be checked

    The condition terminates with SUCCESS, when the actor reached the target distance
    along its route to the given location
    """

    def __init__(self, actor, route, location, distance, name="InTriggerDistanceToLocationAlongRoute"):
        """
        Setup class members
        """
        super(InTriggerDistanceToLocationAlongRoute, self).__init__(name)
        self._map = CarlaDataProvider.get_map()
        self._actor = actor
        self._location = location
        self._route = route
        self._distance = distance

        self._location_distance, _ = get_distance_along_route(self._route, self._location)

    def update(self):
        new_status = py_trees.common.Status.RUNNING

        current_location = CarlaDataProvider.get_location(self._actor)

        if current_location is None:
            return new_status

        if current_location.distance(self._location) < self._distance + 20:

            actor_distance, _ = get_distance_along_route(self._route, current_location)

            # If closer than self._distance and hasn't passed the trigger point
            if (self._location_distance < actor_distance + self._distance and
                actor_distance < self._location_distance) or \
                    self._location_distance < 1.0:
                new_status = py_trees.common.Status.SUCCESS

        return new_status


class InTimeToArrivalToLocation(AtomicCondition):

    """
    This class contains a check if a actor arrives within a given time
    at a given location.

    Important parameters:
    - actor: CARLA actor to execute the behavior
    - name: Name of the condition
    - time: The behavior is successful, if TTA is less than _time_ in seconds
    - location: Location to be checked in this behavior

    The condition terminates with SUCCESS, when the actor can reach the target location within the given time
    """

    _max_time_to_arrival = float('inf')  # time to arrival in seconds

    def __init__(self, actor, time, location, comparison_operator=operator.lt, name="TimeToArrival"):
        """
        Setup parameters
        """
        super(InTimeToArrivalToLocation, self).__init__(name)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))
        self._actor = actor
        self._time = time
        self._target_location = location
        self._comparison_operator = comparison_operator

    def update(self):
        """
        Check if the actor can arrive at target_location within time
        """
        new_status = py_trees.common.Status.RUNNING

        current_location = CarlaDataProvider.get_location(self._actor)

        if current_location is None:
            return new_status

        distance = calculate_distance(current_location, self._target_location)
        velocity = CarlaDataProvider.get_velocity(self._actor)

        # if velocity is too small, simply use a large time to arrival
        time_to_arrival = self._max_time_to_arrival
        if velocity > EPSILON:
            time_to_arrival = distance / velocity

        if self._comparison_operator(time_to_arrival, self._time):
            new_status = py_trees.common.Status.SUCCESS

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))

        return new_status


class InTimeToArrivalToVehicle(AtomicCondition):

    """
    This class contains a check if a actor arrives within a given time
    at another actor.

    Important parameters:
    - actor: CARLA actor to execute the behavior
    - name: Name of the condition
    - time: The behavior is successful, if TTA is less than _time_ in seconds
    - other_actor: Reference actor used in this behavior

    The condition terminates with SUCCESS, when the actor can reach the other vehicle within the given time
    """

    _max_time_to_arrival = float('inf')  # time to arrival in seconds

    def __init__(self, actor, other_actor, time, condition_freespace=False,
                 along_route=False, comparison_operator=operator.lt, name="TimeToArrival"):
        """
        Setup parameters
        """
        super(InTimeToArrivalToVehicle, self).__init__(name)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))
        self._map = CarlaDataProvider.get_map()
        self._actor = actor
        self._other_actor = other_actor
        self._time = time
        self._condition_freespace = condition_freespace
        self._along_route = along_route
        self._comparison_operator = comparison_operator

        if self._along_route:
            # Get the global route planner, used to calculate the route
            self._grp = GlobalRoutePlanner(self._map, 0.5)
        else:
            self._grp = None

    def update(self):
        """
        Check if the ego vehicle can arrive at other actor within time
        """
        new_status = py_trees.common.Status.RUNNING

        current_location = CarlaDataProvider.get_location(self._actor)
        other_location = CarlaDataProvider.get_location(self._other_actor)

        # Get the bounding boxes
        if self._condition_freespace:
            if isinstance(self._actor, (carla.Vehicle, carla.Walker)):
                actor_extent = self._actor.bounding_box.extent.x
            else:
                # Patch, as currently static objects have no bounding boxes
                actor_extent = 0

            if isinstance(self._other_actor, (carla.Vehicle, carla.Walker)):
                other_extent = self._other_actor.bounding_box.extent.x
            else:
                # Patch, as currently static objects have no bounding boxes
                other_extent = 0

        if current_location is None or other_location is None:
            return new_status

        current_velocity = CarlaDataProvider.get_velocity(self._actor)
        other_velocity = CarlaDataProvider.get_velocity(self._other_actor)

        if self._along_route:
            # Global planner needs a location at a driving lane
            current_location = self._map.get_waypoint(current_location).transform.location
            other_location = self._map.get_waypoint(other_location).transform.location

        distance = calculate_distance(current_location, other_location, self._grp)

        # if velocity is too small, simply use a large time to arrival
        time_to_arrival = self._max_time_to_arrival

        if current_velocity > other_velocity:
            if self._condition_freespace:
                time_to_arrival = (distance - actor_extent - other_extent) / (current_velocity - other_velocity)
            else:
                time_to_arrival = distance / (current_velocity - other_velocity)

        if self._comparison_operator(time_to_arrival, self._time):
            new_status = py_trees.common.Status.SUCCESS

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))

        return new_status


class InTimeToArrivalToVehicleSideLane(InTimeToArrivalToLocation):

    """
    This class contains a check if a actor arrives within a given time
    at another actor's side lane. Inherits from InTimeToArrivalToLocation

    Important parameters:
    - actor: CARLA actor to execute the behavior
    - name: Name of the condition
    - time: The behavior is successful, if TTA is less than _time_ in seconds
    - cut_in_lane: the lane from where the other_actor will do the cut in
    - other_actor: Reference actor used in this behavior

    The condition terminates with SUCCESS, when the actor can reach the other vehicle within the given time
    """

    _max_time_to_arrival = float('inf')  # time to arrival in seconds

    def __init__(self, actor, other_actor, time, side_lane,
                 comparison_operator=operator.lt, name="InTimeToArrivalToVehicleSideLane"):
        """
        Setup parameters
        """
        self._other_actor = other_actor
        self._side_lane = side_lane

        self._world = CarlaDataProvider.get_world()
        self._map = CarlaDataProvider.get_map(self._world)

        other_location = other_actor.get_transform().location
        other_waypoint = self._map.get_waypoint(other_location)

        if self._side_lane == 'right':
            other_side_waypoint = other_waypoint.get_left_lane()
        elif self._side_lane == 'left':
            other_side_waypoint = other_waypoint.get_right_lane()
        else:
            raise Exception("cut_in_lane must be either 'left' or 'right'")

        other_side_location = other_side_waypoint.transform.location

        super(InTimeToArrivalToVehicleSideLane, self).__init__(
            actor, time, other_side_location, comparison_operator, name)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))

    def update(self):
        """
        Check if the ego vehicle can arrive at other actor within time
        """
        new_status = py_trees.common.Status.RUNNING

        other_location = CarlaDataProvider.get_location(self._other_actor)
        other_waypoint = self._map.get_waypoint(other_location)

        if self._side_lane == 'right':
            other_side_waypoint = other_waypoint.get_left_lane()
        elif self._side_lane == 'left':
            other_side_waypoint = other_waypoint.get_right_lane()
        else:
            raise Exception("cut_in_lane must be either 'left' or 'right'")
        if other_side_waypoint is None:
            return new_status

        self._target_location = other_side_waypoint.transform.location

        if self._target_location is None:
            return new_status

        new_status = super(InTimeToArrivalToVehicleSideLane, self).update()

        return new_status


class WaitUntilInFront(AtomicCondition):

    """
    Behavior that support the creation of cut ins. It waits until the actor has passed another actor

    Parameters:
    - actor: the one getting in front of the other actor
    - other_actor: the reference vehicle that the actor will have to get in front of
    - factor: How much in front the actor will have to get (from 0 to infinity):
        0: They are right next to each other
        1: The front of the other_actor and the back of the actor are right next to each other
    """

    def __init__(self, actor, other_actor, factor=1, check_distance=True, name="WaitUntilInFront"):
        """
        Init
        """
        super(WaitUntilInFront, self).__init__(name)
        self._actor = actor
        self._other_actor = other_actor
        self._distance = 10
        self._factor = max(EPSILON, factor)  # Must be > 0
        self._check_distance = check_distance

        self._world = CarlaDataProvider.get_world()
        self._map = CarlaDataProvider.get_map(self._world)

        actor_extent = self._actor.bounding_box.extent.x
        other_extent = self._other_actor.bounding_box.extent.x
        self._length = self._factor * (actor_extent + other_extent)

        self.logger.debug("%s.__init__()" % (self.__class__.__name__))

    def update(self):
        """
        Checks if the two actors meet the requirements
        """
        new_status = py_trees.common.Status.RUNNING

        in_front = False
        close_by = False

        # Location of our actor
        actor_location = CarlaDataProvider.get_location(self._actor)
        if actor_location is None:
            return new_status

        # Waypoint in front of the other actor
        other_location = CarlaDataProvider.get_location(self._other_actor)
        other_waypoint = self._map.get_waypoint(other_location)
        if other_waypoint is None:
            return new_status
        other_next_waypoints = other_waypoint.next(self._length)
        if other_next_waypoints is None:
            return new_status
        other_next_waypoint = other_next_waypoints[0]

        # Wait for the vehicle to be in front
        other_dir = other_next_waypoint.transform.get_forward_vector()
        act_other_dir = actor_location - other_next_waypoint.transform.location
        dot_ve_wp = other_dir.x * act_other_dir.x + other_dir.y * act_other_dir.y + other_dir.z * act_other_dir.z

        if dot_ve_wp > 0.0:
            in_front = True

        # Wait for it to be close-by
        if not self._check_distance:
            close_by = True
        elif actor_location.distance(other_next_waypoint.transform.location) < self._distance:
            close_by = True

        if in_front and close_by:
            new_status = py_trees.common.Status.SUCCESS

        return new_status


class DriveDistance(AtomicCondition):

    """
    This class contains an atomic behavior to drive a certain distance.

    Important parameters:
    - actor: CARLA actor to execute the condition
    - distance: Distance for this condition in meters

    The condition terminates with SUCCESS, when the actor drove at least the given distance
    """

    def __init__(self, actor, distance, name="DriveDistance"):
        """
        Setup parameters
        """
        super(DriveDistance, self).__init__(name)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))
        self._target_distance = distance
        self._distance = 0
        self._location = None
        self._actor = actor

    def initialise(self):
        self._location = CarlaDataProvider.get_location(self._actor)
        super(DriveDistance, self).initialise()

    def update(self):
        """
        Check driven distance
        """
        new_status = py_trees.common.Status.RUNNING

        new_location = CarlaDataProvider.get_location(self._actor)
        self._distance += calculate_distance(self._location, new_location)
        self._location = new_location

        if self._distance > self._target_distance:
            new_status = py_trees.common.Status.SUCCESS

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))
        return new_status


class AtRightmostLane(AtomicCondition):

    """
    This class contains an atomic behavior to check if the actor is at the rightest driving lane.

    Important parameters:
    - actor: CARLA actor to execute the condition

    The condition terminates with SUCCESS, when the actor enters the rightest lane
    """

    def __init__(self, actor, name="AtRightmostLane"):
        """
        Setup parameters
        """
        super(AtRightmostLane, self).__init__(name)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))
        self._actor = actor
        self._map = CarlaDataProvider.get_map()

    def update(self):
        """
        Check actor waypoints
        """
        new_status = py_trees.common.Status.RUNNING

        location = CarlaDataProvider.get_location(self._actor)
        waypoint = self._map.get_waypoint(location)
        if waypoint is None:
            return new_status
        right_waypoint = waypoint.get_right_lane()
        if right_waypoint is None:
            return new_status
        lane_type = right_waypoint.lane_type

        if lane_type != carla.LaneType.Driving:
            new_status = py_trees.common.Status.SUCCESS

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))
        return new_status


class WaitForTrafficLightState(AtomicCondition):

    """
    This class contains an atomic behavior to wait for a given traffic light
    to have the desired state.

    Args:
        actor (carla.TrafficLight): CARLA traffic light to execute the condition
        state (carla.TrafficLightState): State to be checked in this condition

    The condition terminates with SUCCESS, when the traffic light switches to the desired state
    """

    def __init__(self, actor, state, name="WaitForTrafficLightState"):
        """
        Setup traffic_light
        """
        super(WaitForTrafficLightState, self).__init__(name)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))
        self._actor = actor if "traffic_light" in actor.type_id else None
        self._state = state

    def update(self):
        """
        Set status to SUCCESS, when traffic light state matches the expected one
        """
        if self._actor is None:
            return py_trees.common.Status.FAILURE

        new_status = py_trees.common.Status.RUNNING

        if self._actor.state == self._state:
            new_status = py_trees.common.Status.SUCCESS

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))

        return new_status


class WaitEndIntersection(AtomicCondition):

    """
    Atomic behavior that waits until the vehicles has gone outside the junction.
    If currently inside no intersection, it will wait until one is found
    """

    def __init__(self, actor, debug=False, name="WaitEndIntersection"):
        super(WaitEndIntersection, self).__init__(name)
        self.actor = actor
        self.debug = debug
        self.inside_junction = False
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))

    def update(self):

        new_status = py_trees.common.Status.RUNNING

        location = CarlaDataProvider.get_location(self.actor)
        waypoint = CarlaDataProvider.get_map().get_waypoint(location)

        # Wait for the actor to enter a junction
        if not self.inside_junction and waypoint.is_junction:
            self.inside_junction = True

        # And to leave it
        if self.inside_junction and not waypoint.is_junction:
            if self.debug:
                print("--- Leaving the junction")
            new_status = py_trees.common.Status.SUCCESS

        return new_status


class WaitForBlackboardVariable(AtomicCondition):

    """
    Atomic behavior that keeps running until the blackboard variable is set to the corresponding value.
    Used to avoid returning FAILURE if the blackboard comparison fails.

    It also initially sets the variable to a given value, if given
    """

    def __init__(self, variable_name, variable_value, var_init_value=None,
                 debug=False, name="WaitForBlackboardVariable"):
        super(WaitForBlackboardVariable, self).__init__(name)
        self._debug = debug
        self._variable_name = variable_name
        self._variable_value = variable_value
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))

        if var_init_value is not None:
            blackboard = py_trees.blackboard.Blackboard()
            _ = blackboard.set(variable_name, var_init_value)

    def update(self):

        new_status = py_trees.common.Status.RUNNING

        blackv = py_trees.blackboard.Blackboard()
        value = blackv.get(self._variable_name)
        if value == self._variable_value:
            if self._debug:
                print("Blackboard variable {} set to True".format(self._variable_name))
            new_status = py_trees.common.Status.SUCCESS

        return new_status


class CheckParameter(AtomicCondition):
    """
    Atomic behavior that keeps checking global osc parameter value with the given value.
    The condition terminates with SUCCESS, when the comparison_operator is evaluated successfully.
    """

    def __init__(self, parameter_ref, value, comparison_operator, debug=False, name="CheckParameter"):
        super(CheckParameter, self).__init__(name)
        self._parameter_ref = parameter_ref
        self._value = value
        self._comparison_operator = comparison_operator
        self._debug = debug

    def update(self):
        """
        keeps comparing global osc value with given value till it is successful.
        """
        new_status = py_trees.common.Status.RUNNING
        current_value = CarlaDataProvider.get_osc_global_param_value(self._parameter_ref)
        if self._comparison_operator(current_value, self._value):
            new_status = py_trees.common.Status.SUCCESS

        return new_status
