#!/usr/bin/env python

# Copyright (c) 2018-2020 Intel Corporation
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
This module provides all atomic evaluation criteria required to analyze if a
scenario was completed successfully or failed.

Criteria should run continuously to monitor the state of a single actor, multiple
actors or environmental parameters. Hence, a termination is not required.

The atomic criteria are implemented with py_trees.
"""

import weakref
import math
import numpy as np
import py_trees
import shapely.geometry

import carla

from srunner.scenariomanager.carla_data_provider import CarlaDataProvider
from srunner.scenariomanager.timer import GameTime
from srunner.scenariomanager.traffic_events import TrafficEvent, TrafficEventType


class Criterion(py_trees.behaviour.Behaviour):

    """
    Base class for all criteria used to evaluate a scenario for success/failure

    Important parameters (PUBLIC):
    - name: Name of the criterion
    - expected_value_success:    Result in case of success
                                 (e.g. max_speed, zero collisions, ...)
    - expected_value_acceptable: Result that does not mean a failure,
                                 but is not good enough for a success
    - actual_value: Actual result after running the scenario
    - test_status: Used to access the result of the criterion
    - optional: Indicates if a criterion is optional (not used for overall analysis)
    """

    def __init__(self,
                 name,
                 actor,
                 expected_value_success,
                 expected_value_acceptable=None,
                 optional=False,
                 terminate_on_failure=False):
        super(Criterion, self).__init__(name)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))
        self._terminate_on_failure = terminate_on_failure

        self.name = name
        self.actor = actor
        self.test_status = "INIT"
        self.expected_value_success = expected_value_success
        self.expected_value_acceptable = expected_value_acceptable
        self.actual_value = 0
        self.optional = optional
        self.list_traffic_events = []

    def initialise(self):
        """
        Initialise the criterion. Can be extended by the user-derived class
        """
        self.logger.debug("%s.initialise()" % (self.__class__.__name__))

    def terminate(self, new_status):
        """
        Terminate the criterion. Can be extended by the user-derived class
        """
        if self.test_status in ('RUNNING', 'INIT'):
            self.test_status = "SUCCESS"

        self.logger.debug("%s.terminate()[%s->%s]" % (self.__class__.__name__, self.status, new_status))


class MaxVelocityTest(Criterion):

    """
    This class contains an atomic test for maximum velocity.

    Important parameters:
    - actor: CARLA actor to be used for this test
    - max_velocity_allowed: maximum allowed velocity in m/s
    - optional [optional]: If True, the result is not considered for an overall pass/fail result
    """

    def __init__(self, actor, max_velocity_allowed, optional=False, name="CheckMaximumVelocity"):
        """
        Setup actor and maximum allowed velovity
        """
        super(MaxVelocityTest, self).__init__(name, actor, max_velocity_allowed, None, optional)

    def update(self):
        """
        Check velocity
        """
        new_status = py_trees.common.Status.RUNNING

        if self.actor is None:
            return new_status

        velocity = CarlaDataProvider.get_velocity(self.actor)

        self.actual_value = max(velocity, self.actual_value)

        if velocity > self.expected_value_success:
            self.test_status = "FAILURE"
        else:
            self.test_status = "SUCCESS"

        if self._terminate_on_failure and (self.test_status == "FAILURE"):
            new_status = py_trees.common.Status.FAILURE

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))

        return new_status


class DrivenDistanceTest(Criterion):

    """
    This class contains an atomic test to check the driven distance

    Important parameters:
    - actor: CARLA actor to be used for this test
    - distance_success: If the actor's driven distance is more than this value (in meters),
                        the test result is SUCCESS
    - distance_acceptable: If the actor's driven distance is more than this value (in meters),
                           the test result is ACCEPTABLE
    - optional [optional]: If True, the result is not considered for an overall pass/fail result
    """

    def __init__(self,
                 actor,
                 distance_success,
                 distance_acceptable=None,
                 optional=False,
                 name="CheckDrivenDistance"):
        """
        Setup actor
        """
        super(DrivenDistanceTest, self).__init__(name, actor, distance_success, distance_acceptable, optional)
        self._last_location = None

    def initialise(self):
        self._last_location = CarlaDataProvider.get_location(self.actor)
        super(DrivenDistanceTest, self).initialise()

    def update(self):
        """
        Check distance
        """
        new_status = py_trees.common.Status.RUNNING

        if self.actor is None:
            return new_status

        location = CarlaDataProvider.get_location(self.actor)

        if location is None:
            return new_status

        if self._last_location is None:
            self._last_location = location
            return new_status

        self.actual_value += location.distance(self._last_location)
        self._last_location = location

        if self.actual_value > self.expected_value_success:
            self.test_status = "SUCCESS"
        elif (self.expected_value_acceptable is not None and
              self.actual_value > self.expected_value_acceptable):
            self.test_status = "ACCEPTABLE"
        else:
            self.test_status = "RUNNING"

        if self._terminate_on_failure and (self.test_status == "FAILURE"):
            new_status = py_trees.common.Status.FAILURE

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))

        return new_status

    def terminate(self, new_status):
        """
        Set final status
        """
        if self.test_status != "SUCCESS":
            self.test_status = "FAILURE"
        self.actual_value = round(self.actual_value, 2)
        super(DrivenDistanceTest, self).terminate(new_status)


class AverageVelocityTest(Criterion):

    """
    This class contains an atomic test for average velocity.

    Important parameters:
    - actor: CARLA actor to be used for this test
    - avg_velocity_success: If the actor's average velocity is more than this value (in m/s),
                            the test result is SUCCESS
    - avg_velocity_acceptable: If the actor's average velocity is more than this value (in m/s),
                               the test result is ACCEPTABLE
    - optional [optional]: If True, the result is not considered for an overall pass/fail result
    """

    def __init__(self,
                 actor,
                 avg_velocity_success,
                 avg_velocity_acceptable=None,
                 optional=False,
                 name="CheckAverageVelocity"):
        """
        Setup actor and average velovity expected
        """
        super(AverageVelocityTest, self).__init__(name, actor,
                                                  avg_velocity_success,
                                                  avg_velocity_acceptable,
                                                  optional)
        self._last_location = None
        self._distance = 0.0

    def initialise(self):
        self._last_location = CarlaDataProvider.get_location(self.actor)
        super(AverageVelocityTest, self).initialise()

    def update(self):
        """
        Check velocity
        """
        new_status = py_trees.common.Status.RUNNING

        if self.actor is None:
            return new_status

        location = CarlaDataProvider.get_location(self.actor)

        if location is None:
            return new_status

        if self._last_location is None:
            self._last_location = location
            return new_status

        self._distance += location.distance(self._last_location)
        self._last_location = location

        elapsed_time = GameTime.get_time()
        if elapsed_time > 0.0:
            self.actual_value = self._distance / elapsed_time

        if self.actual_value > self.expected_value_success:
            self.test_status = "SUCCESS"
        elif (self.expected_value_acceptable is not None and
              self.actual_value > self.expected_value_acceptable):
            self.test_status = "ACCEPTABLE"
        else:
            self.test_status = "RUNNING"

        if self._terminate_on_failure and (self.test_status == "FAILURE"):
            new_status = py_trees.common.Status.FAILURE

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))

        return new_status

    def terminate(self, new_status):
        """
        Set final status
        """
        if self.test_status == "RUNNING":
            self.test_status = "FAILURE"
        super(AverageVelocityTest, self).terminate(new_status)


class CollisionTest(Criterion):

    """
    This class contains an atomic test for collisions.

    Args:
    - actor (carla.Actor): CARLA actor to be used for this test
    - other_actor (carla.Actor): only collisions with this actor will be registered
    - other_actor_type (str): only collisions with actors including this type_id will count.
        Additionally, the "miscellaneous" tag can also be used to include all static objects in the scene
    - terminate_on_failure [optional]: If True, the complete scenario will terminate upon failure of this test
    - optional [optional]: If True, the result is not considered for an overall pass/fail result
    """

    MIN_AREA_OF_COLLISION = 3       # If closer than this distance, the collision is ignored
    MAX_AREA_OF_COLLISION = 5       # If further than this distance, the area is forgotten
    MAX_ID_TIME = 5                 # Amount of time the last collision if is remembered

    def __init__(self, actor, other_actor=None, other_actor_type=None,
                 optional=False, name="CollisionTest", terminate_on_failure=False):
        """
        Construction with sensor setup
        """
        super(CollisionTest, self).__init__(name, actor, 0, None, optional, terminate_on_failure)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))

        world = self.actor.get_world()
        blueprint = world.get_blueprint_library().find('sensor.other.collision')
        self._collision_sensor = world.spawn_actor(blueprint, carla.Transform(), attach_to=self.actor)
        self._collision_sensor.listen(lambda event: self._count_collisions(weakref.ref(self), event))

        self.other_actor = other_actor
        self.other_actor_type = other_actor_type
        self.registered_collisions = []
        self.last_id = None
        self.collision_time = None

    def update(self):
        """
        Check collision count
        """
        new_status = py_trees.common.Status.RUNNING

        if self._terminate_on_failure and (self.test_status == "FAILURE"):
            new_status = py_trees.common.Status.FAILURE

        actor_location = CarlaDataProvider.get_location(self.actor)
        new_registered_collisions = []

        # Loops through all the previous registered collisions
        for collision_location in self.registered_collisions:

            # Get the distance to the collision point
            distance_vector = actor_location - collision_location
            distance = math.sqrt(math.pow(distance_vector.x, 2) + math.pow(distance_vector.y, 2))

            # If far away from a previous collision, forget it
            if distance <= self.MAX_AREA_OF_COLLISION:
                new_registered_collisions.append(collision_location)

        self.registered_collisions = new_registered_collisions

        if self.last_id and GameTime.get_time() - self.collision_time > self.MAX_ID_TIME:
            self.last_id = None

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))

        return new_status

    def terminate(self, new_status):
        """
        Cleanup sensor
        """
        if self._collision_sensor is not None:
            self._collision_sensor.destroy()
        self._collision_sensor = None

        super(CollisionTest, self).terminate(new_status)

    @staticmethod
    def _count_collisions(weak_self, event):     # pylint: disable=too-many-return-statements
        """
        Callback to update collision count
        """
        self = weak_self()
        if not self:
            return

        actor_location = CarlaDataProvider.get_location(self.actor)

        # Ignore the current one if it is the same id as before
        if self.last_id == event.other_actor.id:
            return

        # Filter to only a specific actor
        if self.other_actor and self.other_actor.id != event.other_actor.id:
            return

        # Filter to only a specific type
        if self.other_actor_type:
            if self.other_actor_type == "miscellaneous":
                if "traffic" not in event.other_actor.type_id \
                        and "static" not in event.other_actor.type_id:
                    return
            else:
                if self.other_actor_type not in event.other_actor.type_id:
                    return

        # Ignore it if its too close to a previous collision (avoid micro collisions)
        for collision_location in self.registered_collisions:

            distance_vector = actor_location - collision_location
            distance = math.sqrt(math.pow(distance_vector.x, 2) + math.pow(distance_vector.y, 2))

            if distance <= self.MIN_AREA_OF_COLLISION:
                return

        if ('static' in event.other_actor.type_id or 'traffic' in event.other_actor.type_id) \
                and 'sidewalk' not in event.other_actor.type_id:
            actor_type = TrafficEventType.COLLISION_STATIC
        elif 'vehicle' in event.other_actor.type_id:
            actor_type = TrafficEventType.COLLISION_VEHICLE
        elif 'walker' in event.other_actor.type_id:
            actor_type = TrafficEventType.COLLISION_PEDESTRIAN
        else:
            return

        collision_event = TrafficEvent(event_type=actor_type)
        collision_event.set_dict({
            'type': event.other_actor.type_id,
            'id': event.other_actor.id,
            'x': actor_location.x,
            'y': actor_location.y,
            'z': actor_location.z})
        collision_event.set_message(
            "Agent collided against object with type={} and id={} at (x={}, y={}, z={})".format(
                event.other_actor.type_id,
                event.other_actor.id,
                round(actor_location.x, 3),
                round(actor_location.y, 3),
                round(actor_location.z, 3)))

        self.test_status = "FAILURE"
        self.actual_value += 1
        self.collision_time = GameTime.get_time()

        self.registered_collisions.append(actor_location)
        self.list_traffic_events.append(collision_event)

        # Number 0: static objects -> ignore it
        if event.other_actor.id != 0:
            self.last_id = event.other_actor.id


class ActorSpeedAboveThresholdTest(Criterion):

    """
    This test will fail if the actor has had its linear velocity lower than a specific value for
    a specific amount of time
    Important parameters:
    - actor: CARLA actor to be used for this test
    - speed_threshold: speed required
    - below_threshold_max_time: Maximum time (in seconds) the actor can remain under the speed threshold
    - terminate_on_failure [optional]: If True, the complete scenario will terminate upon failure of this test
    """

    def __init__(self, actor, speed_threshold, below_threshold_max_time,
                 name="ActorSpeedAboveThresholdTest", terminate_on_failure=False):
        """
        Class constructor.
        """
        super(ActorSpeedAboveThresholdTest, self).__init__(name, actor, 0, terminate_on_failure=terminate_on_failure)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))
        self._actor = actor
        self._speed_threshold = speed_threshold
        self._below_threshold_max_time = below_threshold_max_time
        self._time_last_valid_state = None

    def update(self):
        """
        Check if the actor speed is above the speed_threshold
        """
        new_status = py_trees.common.Status.RUNNING

        linear_speed = CarlaDataProvider.get_velocity(self._actor)
        if linear_speed is not None:
            if linear_speed < self._speed_threshold and self._time_last_valid_state:
                if (GameTime.get_time() - self._time_last_valid_state) > self._below_threshold_max_time:
                    # Game over. The actor has been "blocked" for too long
                    self.test_status = "FAILURE"

                    # record event
                    vehicle_location = CarlaDataProvider.get_location(self._actor)
                    blocked_event = TrafficEvent(event_type=TrafficEventType.VEHICLE_BLOCKED)
                    ActorSpeedAboveThresholdTest._set_event_message(blocked_event, vehicle_location)
                    ActorSpeedAboveThresholdTest._set_event_dict(blocked_event, vehicle_location)
                    self.list_traffic_events.append(blocked_event)
            else:
                self._time_last_valid_state = GameTime.get_time()

        if self._terminate_on_failure and (self.test_status == "FAILURE"):
            new_status = py_trees.common.Status.FAILURE
        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))

        return new_status

    @staticmethod
    def _set_event_message(event, location):
        """
        Sets the message of the event
        """

        event.set_message('Agent got blocked at (x={}, y={}, z={})'.format(round(location.x, 3),
                                                                           round(location.y, 3),
                                                                           round(location.z, 3)))

    @staticmethod
    def _set_event_dict(event, location):
        """
        Sets the dictionary of the event
        """
        event.set_dict({
            'x': location.x,
            'y': location.y,
            'z': location.z,
        })


class KeepLaneTest(Criterion):

    """
    This class contains an atomic test for keeping lane.

    Important parameters:
    - actor: CARLA actor to be used for this test
    - optional [optional]: If True, the result is not considered for an overall pass/fail result
    """

    def __init__(self, actor, optional=False, name="CheckKeepLane"):
        """
        Construction with sensor setup
        """
        super(KeepLaneTest, self).__init__(name, actor, 0, None, optional)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))

        world = self.actor.get_world()
        blueprint = world.get_blueprint_library().find('sensor.other.lane_invasion')
        self._lane_sensor = world.spawn_actor(blueprint, carla.Transform(), attach_to=self.actor)
        self._lane_sensor.listen(lambda event: self._count_lane_invasion(weakref.ref(self), event))

    def update(self):
        """
        Check lane invasion count
        """
        new_status = py_trees.common.Status.RUNNING

        if self.actual_value > 0:
            self.test_status = "FAILURE"
        else:
            self.test_status = "SUCCESS"

        if self._terminate_on_failure and (self.test_status == "FAILURE"):
            new_status = py_trees.common.Status.FAILURE

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))

        return new_status

    def terminate(self, new_status):
        """
        Cleanup sensor
        """
        if self._lane_sensor is not None:
            self._lane_sensor.destroy()
        self._lane_sensor = None
        super(KeepLaneTest, self).terminate(new_status)

    @staticmethod
    def _count_lane_invasion(weak_self, event):
        """
        Callback to update lane invasion count
        """
        self = weak_self()
        if not self:
            return
        self.actual_value += 1


class ReachedRegionTest(Criterion):

    """
    This class contains the reached region test
    The test is a success if the actor reaches a specified region

    Important parameters:
    - actor: CARLA actor to be used for this test
    - min_x, max_x, min_y, max_y: Bounding box of the checked region
    """

    def __init__(self, actor, min_x, max_x, min_y, max_y, name="ReachedRegionTest"):
        """
        Setup trigger region (rectangle provided by
        [min_x,min_y] and [max_x,max_y]
        """
        super(ReachedRegionTest, self).__init__(name, actor, 0)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))
        self._actor = actor
        self._min_x = min_x
        self._max_x = max_x
        self._min_y = min_y
        self._max_y = max_y

    def update(self):
        """
        Check if the actor location is within trigger region
        """
        new_status = py_trees.common.Status.RUNNING

        location = CarlaDataProvider.get_location(self._actor)
        if location is None:
            return new_status

        in_region = False
        if self.test_status != "SUCCESS":
            in_region = (location.x > self._min_x and location.x < self._max_x) and (
                location.y > self._min_y and location.y < self._max_y)
            if in_region:
                self.test_status = "SUCCESS"
            else:
                self.test_status = "RUNNING"

        if self.test_status == "SUCCESS":
            new_status = py_trees.common.Status.SUCCESS

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))

        return new_status


class OffRoadTest(Criterion):

    """
    Atomic containing a test to detect when an actor deviates from the driving lanes. This atomic can
    fail when actor has spent a specific time outside driving lanes (defined by OpenDRIVE). Simplified
    version of OnSidewalkTest, and doesn't relly on waypoints with *Sidewalk* lane types

    Args:
        actor (carla.Actor): CARLA actor to be used for this test
        duration (float): Time spent at sidewalks before the atomic fails.
            If terminate_on_failure isn't active, this is ignored.
        optional (bool): If True, the result is not considered for an overall pass/fail result
            when using the output argument
        terminate_on_failure (bool): If True, the atomic will fail when the duration condition has been met.
    """

    def __init__(self, actor, duration=0, optional=False, terminate_on_failure=False, name="OffRoadTest"):
        """
        Setup of the variables
        """
        super(OffRoadTest, self).__init__(name, actor, 0, None, optional, terminate_on_failure)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))

        self._map = CarlaDataProvider.get_map()
        self._offroad = False

        self._duration = duration
        self._prev_time = None
        self._time_offroad = 0

    def update(self):
        """
        First, transforms the actor's current position to its corresponding waypoint. This is
        filtered to only use waypoints of type Driving or Parking. Depending on these results,
        the actor will be considered to be outside (or inside) driving lanes.

        returns:
            py_trees.common.Status.FAILURE: when the actor has spent a given duration outside driving lanes
            py_trees.common.Status.RUNNING: the rest of the time
        """
        new_status = py_trees.common.Status.RUNNING

        current_location = CarlaDataProvider.get_location(self.actor)

        # Get the waypoint at the current location to see if the actor is offroad
        drive_waypoint = self._map.get_waypoint(
            current_location,
            project_to_road=False
        )
        park_waypoint = self._map.get_waypoint(
            current_location,
            project_to_road=False,
            lane_type=carla.LaneType.Parking
        )
        if drive_waypoint or park_waypoint:
            self._offroad = False
        else:
            self._offroad = True

        # Counts the time offroad
        if self._offroad:
            if self._prev_time is None:
                self._prev_time = GameTime.get_time()
            else:
                curr_time = GameTime.get_time()
                self._time_offroad += curr_time - self._prev_time
                self._prev_time = curr_time
        else:
            self._prev_time = None

        if self._time_offroad > self._duration:
            self.test_status = "FAILURE"

        if self._terminate_on_failure and self.test_status == "FAILURE":
            new_status = py_trees.common.Status.FAILURE

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))

        return new_status


class EndofRoadTest(Criterion):

    """
    Atomic containing a test to detect when an actor has changed to a different road

    Args:
        actor (carla.Actor): CARLA actor to be used for this test
        duration (float): Time spent after ending the road before the atomic fails.
            If terminate_on_failure isn't active, this is ignored.
        optional (bool): If True, the result is not considered for an overall pass/fail result
            when using the output argument
        terminate_on_failure (bool): If True, the atomic will fail when the duration condition has been met.
    """

    def __init__(self, actor, duration=0, optional=False, terminate_on_failure=False, name="EndofRoadTest"):
        """
        Setup of the variables
        """
        super(EndofRoadTest, self).__init__(name, actor, 0, None, optional, terminate_on_failure)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))

        self._map = CarlaDataProvider.get_map()
        self._end_of_road = False

        self._duration = duration
        self._start_time = None
        self._time_end_road = 0
        self._road_id = None

    def update(self):
        """
        First, transforms the actor's current position to its corresponding waypoint. Then the road id
        is compared with the initial one and if that's the case, a time is started

        returns:
            py_trees.common.Status.FAILURE: when the actor has spent a given duration outside driving lanes
            py_trees.common.Status.RUNNING: the rest of the time
        """
        new_status = py_trees.common.Status.RUNNING

        current_location = CarlaDataProvider.get_location(self.actor)
        current_waypoint = self._map.get_waypoint(current_location)

        # Get the current road id
        if self._road_id is None:
            self._road_id = current_waypoint.road_id

        else:
            # Wait until the actor has left the road
            if self._road_id != current_waypoint.road_id or self._start_time:

                # Start counting
                if self._start_time is None:
                    self._start_time = GameTime.get_time()
                    return new_status

                curr_time = GameTime.get_time()
                self._time_end_road = curr_time - self._start_time

                if self._time_end_road > self._duration:
                    self.test_status = "FAILURE"
                    self.actual_value += 1
                    return py_trees.common.Status.SUCCESS

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))

        return new_status


class OnSidewalkTest(Criterion):

    """
    Atomic containing a test to detect sidewalk invasions of a specific actor. This atomic can
    fail when actor has spent a specific time outside driving lanes (defined by OpenDRIVE).

    Args:
        actor (carla.Actor): CARLA actor to be used for this test
        duration (float): Time spent at sidewalks before the atomic fails.
            If terminate_on_failure isn't active, this is ignored.
        optional (bool): If True, the result is not considered for an overall pass/fail result
            when using the output argument
        terminate_on_failure (bool): If True, the atomic will fail when the duration condition has been met.
    """

    def __init__(self, actor, duration=0, optional=False, terminate_on_failure=False, name="OnSidewalkTest"):
        """
        Construction with sensor setup
        """
        super(OnSidewalkTest, self).__init__(name, actor, 0, None, optional, terminate_on_failure)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))

        self._actor = actor
        self._map = CarlaDataProvider.get_map()
        self._onsidewalk_active = False
        self._outside_lane_active = False

        self._actor_location = self._actor.get_location()
        self._wrong_sidewalk_distance = 0
        self._wrong_outside_lane_distance = 0
        self._sidewalk_start_location = None
        self._outside_lane_start_location = None
        self._duration = duration
        self._prev_time = None
        self._time_outside_lanes = 0

    def update(self):
        """
        First, transforms the actor's current position as well as its four corners to their
        corresponding waypoints. Depending on their lane type, the actor will be considered to be
        outside (or inside) driving lanes.

        returns:
            py_trees.common.Status.FAILURE: when the actor has spent a given duration outside
                driving lanes and terminate_on_failure is active
            py_trees.common.Status.RUNNING: the rest of the time
        """
        new_status = py_trees.common.Status.RUNNING

        if self._terminate_on_failure and self.test_status == "FAILURE":
            new_status = py_trees.common.Status.FAILURE

        # Some of the vehicle parameters
        current_tra = CarlaDataProvider.get_transform(self._actor)
        current_loc = current_tra.location
        current_wp = self._map.get_waypoint(current_loc, lane_type=carla.LaneType.Any)

        # Case 1) Car center is at a sidewalk
        if current_wp.lane_type == carla.LaneType.Sidewalk:
            if not self._onsidewalk_active:
                self._onsidewalk_active = True
                self._sidewalk_start_location = current_loc

        # Case 2) Not inside allowed zones (Driving and Parking)
        elif current_wp.lane_type not in (carla.LaneType.Driving, carla.LaneType.Parking):

            # Get the vertices of the vehicle
            heading_vec = current_tra.get_forward_vector()
            heading_vec.z = 0
            heading_vec = heading_vec / math.sqrt(math.pow(heading_vec.x, 2) + math.pow(heading_vec.y, 2))
            perpendicular_vec = carla.Vector3D(-heading_vec.y, heading_vec.x, 0)

            extent = self.actor.bounding_box.extent
            x_boundary_vector = heading_vec * extent.x
            y_boundary_vector = perpendicular_vec * extent.y

            bbox = [
                current_loc + carla.Location(x_boundary_vector - y_boundary_vector),
                current_loc + carla.Location(x_boundary_vector + y_boundary_vector),
                current_loc + carla.Location(-1 * x_boundary_vector - y_boundary_vector),
                current_loc + carla.Location(-1 * x_boundary_vector + y_boundary_vector)]

            bbox_wp = [
                self._map.get_waypoint(bbox[0], lane_type=carla.LaneType.Any),
                self._map.get_waypoint(bbox[1], lane_type=carla.LaneType.Any),
                self._map.get_waypoint(bbox[2], lane_type=carla.LaneType.Any),
                self._map.get_waypoint(bbox[3], lane_type=carla.LaneType.Any)]

            lane_type_list = [bbox_wp[0].lane_type, bbox_wp[1].lane_type, bbox_wp[2].lane_type, bbox_wp[3].lane_type]

            # Case 2.1) Not quite outside yet
            if bbox_wp[0].lane_type == (carla.LaneType.Driving or carla.LaneType.Parking) \
                or bbox_wp[1].lane_type == (carla.LaneType.Driving or carla.LaneType.Parking) \
                or bbox_wp[2].lane_type == (carla.LaneType.Driving or carla.LaneType.Parking) \
                    or bbox_wp[3].lane_type == (carla.LaneType.Driving or carla.LaneType.Parking):

                self._onsidewalk_active = False
                self._outside_lane_active = False

            # Case 2.2) At the mini Shoulders between Driving and Sidewalk
            elif carla.LaneType.Sidewalk in lane_type_list:
                if not self._onsidewalk_active:
                    self._onsidewalk_active = True
                    self._sidewalk_start_location = current_loc

            else:
                distance_vehicle_wp = current_loc.distance(current_wp.transform.location)

                # Case 2.3) Outside lane
                if distance_vehicle_wp >= current_wp.lane_width / 2:

                    if not self._outside_lane_active:
                        self._outside_lane_active = True
                        self._outside_lane_start_location = current_loc

                # Case 2.4) Very very edge case (but still inside driving lanes)
                else:
                    self._onsidewalk_active = False
                    self._outside_lane_active = False

        # Case 3) Driving and Parking conditions
        else:
            # Check for false positives at junctions
            if current_wp.is_junction:
                distance_vehicle_wp = math.sqrt(
                    math.pow(current_wp.transform.location.x - current_loc.x, 2) +
                    math.pow(current_wp.transform.location.y - current_loc.y, 2))

                if distance_vehicle_wp <= current_wp.lane_width / 2:
                    self._onsidewalk_active = False
                    self._outside_lane_active = False
                # Else, do nothing, the waypoint is too far to consider it a correct position
            else:

                self._onsidewalk_active = False
                self._outside_lane_active = False

        # Counts the time offroad
        if self._onsidewalk_active or self._outside_lane_active:
            if self._prev_time is None:
                self._prev_time = GameTime.get_time()
            else:
                curr_time = GameTime.get_time()
                self._time_outside_lanes += curr_time - self._prev_time
                self._prev_time = curr_time
        else:
            self._prev_time = None

        if self._time_outside_lanes > self._duration:
            self.test_status = "FAILURE"

        # Update the distances
        distance_vector = CarlaDataProvider.get_location(self._actor) - self._actor_location
        distance = math.sqrt(math.pow(distance_vector.x, 2) + math.pow(distance_vector.y, 2))

        if distance >= 0.02:  # Used to avoid micro-changes adding to considerable sums
            self._actor_location = CarlaDataProvider.get_location(self._actor)

            if self._onsidewalk_active:
                self._wrong_sidewalk_distance += distance
            elif self._outside_lane_active:
                # Only add if car is outside the lane but ISN'T in a junction
                self._wrong_outside_lane_distance += distance

        # Register the sidewalk event
        if not self._onsidewalk_active and self._wrong_sidewalk_distance > 0:

            self.actual_value += 1

            onsidewalk_event = TrafficEvent(event_type=TrafficEventType.ON_SIDEWALK_INFRACTION)
            self._set_event_message(
                onsidewalk_event, self._sidewalk_start_location, self._wrong_sidewalk_distance)
            self._set_event_dict(
                onsidewalk_event, self._sidewalk_start_location, self._wrong_sidewalk_distance)

            self._onsidewalk_active = False
            self._wrong_sidewalk_distance = 0
            self.list_traffic_events.append(onsidewalk_event)

        # Register the outside of a lane event
        if not self._outside_lane_active and self._wrong_outside_lane_distance > 0:

            self.actual_value += 1

            outsidelane_event = TrafficEvent(event_type=TrafficEventType.OUTSIDE_LANE_INFRACTION)
            self._set_event_message(
                outsidelane_event, self._outside_lane_start_location, self._wrong_outside_lane_distance)
            self._set_event_dict(
                outsidelane_event, self._outside_lane_start_location, self._wrong_outside_lane_distance)

            self._outside_lane_active = False
            self._wrong_outside_lane_distance = 0
            self.list_traffic_events.append(outsidelane_event)

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))

        return new_status

    def terminate(self, new_status):
        """
        If there is currently an event running, it is registered
        """
        # If currently at a sidewalk, register the event
        if self._onsidewalk_active:

            self.actual_value += 1

            onsidewalk_event = TrafficEvent(event_type=TrafficEventType.ON_SIDEWALK_INFRACTION)
            self._set_event_message(
                onsidewalk_event, self._sidewalk_start_location, self._wrong_sidewalk_distance)
            self._set_event_dict(
                onsidewalk_event, self._sidewalk_start_location, self._wrong_sidewalk_distance)

            self._onsidewalk_active = False
            self._wrong_sidewalk_distance = 0
            self.list_traffic_events.append(onsidewalk_event)

        # If currently outside of our lane, register the event
        if self._outside_lane_active:

            self.actual_value += 1

            outsidelane_event = TrafficEvent(event_type=TrafficEventType.OUTSIDE_LANE_INFRACTION)
            self._set_event_message(
                outsidelane_event, self._outside_lane_start_location, self._wrong_outside_lane_distance)
            self._set_event_dict(
                outsidelane_event, self._outside_lane_start_location, self._wrong_outside_lane_distance)

            self._outside_lane_active = False
            self._wrong_outside_lane_distance = 0
            self.list_traffic_events.append(outsidelane_event)

        super(OnSidewalkTest, self).terminate(new_status)

    def _set_event_message(self, event, location, distance):
        """
        Sets the message of the event
        """
        if event.get_type() == TrafficEventType.ON_SIDEWALK_INFRACTION:
            message_start = 'Agent invaded the sidewalk'
        else:
            message_start = 'Agent went outside the lane'

        event.set_message(
            '{} for about {} meters, starting at (x={}, y={}, z={})'.format(
                message_start,
                round(distance, 3),
                round(location.x, 3),
                round(location.y, 3),
                round(location.z, 3)))

    def _set_event_dict(self, event, location, distance):
        """
        Sets the dictionary of the event
        """
        event.set_dict({
            'x': location.x,
            'y': location.y,
            'z': location.z,
            'distance': distance})


class OutsideRouteLanesTest(Criterion):

    """
    Atomic to detect if the vehicle is either on a sidewalk or at a wrong lane. The distance spent outside
    is computed and it is returned as a percentage of the route distance traveled.

    Args:
        actor (carla.ACtor): CARLA actor to be used for this test
        route (list [carla.Location, connection]): series of locations representing the route waypoints
        optional (bool): If True, the result is not considered for an overall pass/fail result
    """

    ALLOWED_OUT_DISTANCE = 1.3          # At least 0.5, due to the mini-shoulder between lanes and sidewalks
    MAX_ALLOWED_VEHICLE_ANGLE = 120.0   # Maximum angle between the yaw and waypoint lane
    MAX_ALLOWED_WAYPOINT_ANGLE = 150.0  # Maximum change between the yaw-lane angle between frames
    WINDOWS_SIZE = 3                    # Amount of additional waypoints checked (in case the first on fails)

    def __init__(self, actor, route, optional=False, name="OutsideRouteLanesTest"):
        """
        Constructor
        """
        super(OutsideRouteLanesTest, self).__init__(name, actor, 0, None, optional)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))

        self._actor = actor
        self._route = route
        self._current_index = 0
        self._route_length = len(self._route)
        self._waypoints, _ = zip(*self._route)

        self._map = CarlaDataProvider.get_map()
        self._pre_ego_waypoint = self._map.get_waypoint(self._actor.get_location())

        self._outside_lane_active = False
        self._wrong_lane_active = False
        self._last_road_id = None
        self._last_lane_id = None
        self._total_distance = 0
        self._wrong_distance = 0

    def update(self):
        """
        Transforms the actor location and its four corners to waypoints. Depending on its types,
        the actor will be considered to be at driving lanes, sidewalk or offroad.

        returns:
            py_trees.common.Status.FAILURE: when the actor has left driving and terminate_on_failure is active
            py_trees.common.Status.RUNNING: the rest of the time
        """
        new_status = py_trees.common.Status.RUNNING

        if self._terminate_on_failure and (self.test_status == "FAILURE"):
            new_status = py_trees.common.Status.FAILURE

        # Some of the vehicle parameters
        location = CarlaDataProvider.get_location(self._actor)
        if location is None:
            return new_status

        # 1) Check if outside route lanes
        self._is_outside_driving_lanes(location)
        self._is_at_wrong_lane(location)

        if self._outside_lane_active or self._wrong_lane_active:
            self.test_status = "FAILURE"

        # 2) Get the traveled distance
        for index in range(self._current_index + 1,
                           min(self._current_index + self.WINDOWS_SIZE + 1, self._route_length)):
            # Get the dot product to know if it has passed this location
            index_location = self._waypoints[index]
            index_waypoint = self._map.get_waypoint(index_location)

            wp_dir = index_waypoint.transform.get_forward_vector()  # Waypoint's forward vector
            wp_veh = location - index_location  # vector waypoint - vehicle
            dot_ve_wp = wp_veh.x * wp_dir.x + wp_veh.y * wp_dir.y + wp_veh.z * wp_dir.z

            if dot_ve_wp > 0:
                # Get the distance traveled
                index_location = self._waypoints[index]
                current_index_location = self._waypoints[self._current_index]
                new_dist = current_index_location.distance(index_location)

                # Add it to the total distance
                self._current_index = index
                self._total_distance += new_dist

                # And to the wrong one if outside route lanes
                if self._outside_lane_active or self._wrong_lane_active:
                    self._wrong_distance += new_dist

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))

        return new_status

    def _is_outside_driving_lanes(self, location):
        """
        Detects if the ego_vehicle is outside driving lanes
        """

        current_driving_wp = self._map.get_waypoint(location, lane_type=carla.LaneType.Driving, project_to_road=True)
        current_parking_wp = self._map.get_waypoint(location, lane_type=carla.LaneType.Parking, project_to_road=True)

        driving_distance = location.distance(current_driving_wp.transform.location)
        if current_parking_wp is not None:  # Some towns have no parking
            parking_distance = location.distance(current_parking_wp.transform.location)
        else:
            parking_distance = float('inf')

        if driving_distance >= parking_distance:
            distance = parking_distance
            lane_width = current_parking_wp.lane_width
        else:
            distance = driving_distance
            lane_width = current_driving_wp.lane_width

        self._outside_lane_active = bool(distance > (lane_width / 2 + self.ALLOWED_OUT_DISTANCE))

    def _is_at_wrong_lane(self, location):
        """
        Detects if the ego_vehicle has invaded a wrong lane
        """

        current_waypoint = self._map.get_waypoint(location, lane_type=carla.LaneType.Driving, project_to_road=True)
        current_lane_id = current_waypoint.lane_id
        current_road_id = current_waypoint.road_id

        # Lanes and roads are too chaotic at junctions
        if current_waypoint.is_junction:
            self._wrong_lane_active = False
        elif self._last_road_id != current_road_id or self._last_lane_id != current_lane_id:

            # Route direction can be considered continuous, except after exiting a junction.
            if self._pre_ego_waypoint.is_junction:
                yaw_waypt = current_waypoint.transform.rotation.yaw % 360
                yaw_actor = self._actor.get_transform().rotation.yaw % 360

                vehicle_lane_angle = (yaw_waypt - yaw_actor) % 360

                if vehicle_lane_angle < self.MAX_ALLOWED_VEHICLE_ANGLE \
                        or vehicle_lane_angle > (360 - self.MAX_ALLOWED_VEHICLE_ANGLE):
                    self._wrong_lane_active = False
                else:
                    self._wrong_lane_active = True

            else:
                # Check for a big gap in waypoint directions.
                yaw_pre_wp = self._pre_ego_waypoint.transform.rotation.yaw % 360
                yaw_cur_wp = current_waypoint.transform.rotation.yaw % 360

                waypoint_angle = (yaw_pre_wp - yaw_cur_wp) % 360

                if waypoint_angle >= self.MAX_ALLOWED_WAYPOINT_ANGLE \
                        and waypoint_angle <= (360 - self.MAX_ALLOWED_WAYPOINT_ANGLE):  # pylint: disable=chained-comparison

                    # Is the ego vehicle going back to the lane, or going out? Take the opposite
                    self._wrong_lane_active = not bool(self._wrong_lane_active)
                else:

                    # Changing to a lane with the same direction
                    self._wrong_lane_active = False

        # Remember the last state
        self._last_lane_id = current_lane_id
        self._last_road_id = current_road_id
        self._pre_ego_waypoint = current_waypoint

    def terminate(self, new_status):
        """
        If there is currently an event running, it is registered
        """

        if self._wrong_distance > 0:

            percentage = self._wrong_distance / self._total_distance * 100

            outside_lane = TrafficEvent(event_type=TrafficEventType.OUTSIDE_ROUTE_LANES_INFRACTION)
            outside_lane.set_message(
                "Agent went outside its route lanes for about {} meters "
                "({}% of the completed route)".format(
                    round(self._wrong_distance, 3),
                    round(percentage, 2)))

            outside_lane.set_dict({
                'distance': self._wrong_distance,
                'percentage': percentage
            })

            self._wrong_distance = 0
            self.list_traffic_events.append(outside_lane)
            self.actual_value = round(percentage, 2)

        super(OutsideRouteLanesTest, self).terminate(new_status)


class WrongLaneTest(Criterion):

    """
    This class contains an atomic test to detect invasions to wrong direction lanes.

    Important parameters:
    - actor: CARLA actor to be used for this test
    - optional [optional]: If True, the result is not considered for an overall pass/fail result
    """
    MAX_ALLOWED_ANGLE = 120.0
    MAX_ALLOWED_WAYPOINT_ANGLE = 150.0

    def __init__(self, actor, optional=False, name="WrongLaneTest"):
        """
        Construction with sensor setup
        """
        super(WrongLaneTest, self).__init__(name, actor, 0, None, optional)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))

        self._actor = actor
        self._map = CarlaDataProvider.get_map()
        self._last_lane_id = None
        self._last_road_id = None

        self._in_lane = True
        self._wrong_distance = 0
        self._actor_location = self._actor.get_location()
        self._previous_lane_waypoint = self._map.get_waypoint(self._actor.get_location())
        self._wrong_lane_start_location = None

    def update(self):
        """
        Check lane invasion count
        """

        new_status = py_trees.common.Status.RUNNING

        if self._terminate_on_failure and (self.test_status == "FAILURE"):
            new_status = py_trees.common.Status.FAILURE

        lane_waypoint = self._map.get_waypoint(self._actor.get_location())
        current_lane_id = lane_waypoint.lane_id
        current_road_id = lane_waypoint.road_id

        if (self._last_road_id != current_road_id or self._last_lane_id != current_lane_id) \
                and not lane_waypoint.is_junction:
            next_waypoint = lane_waypoint.next(2.0)[0]

            if not next_waypoint:
                return new_status

            # The waypoint route direction can be considered continuous.
            # Therefore just check for a big gap in waypoint directions.
            previous_lane_direction = self._previous_lane_waypoint.transform.get_forward_vector()
            current_lane_direction = lane_waypoint.transform.get_forward_vector()

            p_lane_vector = np.array([previous_lane_direction.x, previous_lane_direction.y])
            c_lane_vector = np.array([current_lane_direction.x, current_lane_direction.y])

            waypoint_angle = math.degrees(
                math.acos(np.clip(np.dot(p_lane_vector, c_lane_vector) /
                                  (np.linalg.norm(p_lane_vector) * np.linalg.norm(c_lane_vector)), -1.0, 1.0)))

            if waypoint_angle > self.MAX_ALLOWED_WAYPOINT_ANGLE and self._in_lane:

                self.test_status = "FAILURE"
                self._in_lane = False
                self.actual_value += 1
                self._wrong_lane_start_location = self._actor_location

            else:
                # Reset variables
                self._in_lane = True

            # Continuity is broken after a junction so check vehicle-lane angle instead
            if self._previous_lane_waypoint.is_junction:

                vector_wp = np.array([next_waypoint.transform.location.x - lane_waypoint.transform.location.x,
                                      next_waypoint.transform.location.y - lane_waypoint.transform.location.y])

                vector_actor = np.array([math.cos(math.radians(self._actor.get_transform().rotation.yaw)),
                                         math.sin(math.radians(self._actor.get_transform().rotation.yaw))])

                vehicle_lane_angle = math.degrees(
                    math.acos(np.clip(np.dot(vector_actor, vector_wp) / (np.linalg.norm(vector_wp)), -1.0, 1.0)))

                if vehicle_lane_angle > self.MAX_ALLOWED_ANGLE:

                    self.test_status = "FAILURE"
                    self._in_lane = False
                    self.actual_value += 1
                    self._wrong_lane_start_location = self._actor.get_location()

        # Keep adding "meters" to the counter
        distance_vector = self._actor.get_location() - self._actor_location
        distance = math.sqrt(math.pow(distance_vector.x, 2) + math.pow(distance_vector.y, 2))

        if distance >= 0.02:  # Used to avoid micro-changes adding add to considerable sums
            self._actor_location = CarlaDataProvider.get_location(self._actor)

            if not self._in_lane and not lane_waypoint.is_junction:
                self._wrong_distance += distance

        # Register the event
        if self._in_lane and self._wrong_distance > 0:

            wrong_way_event = TrafficEvent(event_type=TrafficEventType.WRONG_WAY_INFRACTION)
            self._set_event_message(wrong_way_event, self._wrong_lane_start_location,
                                    self._wrong_distance, current_road_id, current_lane_id)
            self._set_event_dict(wrong_way_event, self._wrong_lane_start_location,
                                 self._wrong_distance, current_road_id, current_lane_id)

            self.list_traffic_events.append(wrong_way_event)
            self._wrong_distance = 0

        # Remember the last state
        self._last_lane_id = current_lane_id
        self._last_road_id = current_road_id
        self._previous_lane_waypoint = lane_waypoint

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))

        return new_status

    def terminate(self, new_status):
        """
        If there is currently an event running, it is registered
        """
        if not self._in_lane:

            lane_waypoint = self._map.get_waypoint(self._actor.get_location())
            current_lane_id = lane_waypoint.lane_id
            current_road_id = lane_waypoint.road_id

            wrong_way_event = TrafficEvent(event_type=TrafficEventType.WRONG_WAY_INFRACTION)
            self._set_event_message(wrong_way_event, self._wrong_lane_start_location,
                                    self._wrong_distance, current_road_id, current_lane_id)
            self._set_event_dict(wrong_way_event, self._wrong_lane_start_location,
                                 self._wrong_distance, current_road_id, current_lane_id)

            self._wrong_distance = 0
            self._in_lane = True
            self.list_traffic_events.append(wrong_way_event)

        super(WrongLaneTest, self).terminate(new_status)

    def _set_event_message(self, event, location, distance, road_id, lane_id):
        """
        Sets the message of the event
        """

        event.set_message(
            "Agent invaded a lane in opposite direction for {} meters, starting at (x={}, y={}, z={}). "
            "road_id={}, lane_id={}".format(
                round(distance, 3),
                round(location.x, 3),
                round(location.y, 3),
                round(location.z, 3),
                road_id,
                lane_id))

    def _set_event_dict(self, event, location, distance, road_id, lane_id):
        """
        Sets the dictionary of the event
        """
        event.set_dict({
            'x': location.x,
            'y': location.y,
            'z': location.y,
            'distance': distance,
            'road_id': road_id,
            'lane_id': lane_id})


class InRadiusRegionTest(Criterion):

    """
    The test is a success if the actor is within a given radius of a specified region

    Important parameters:
    - actor: CARLA actor to be used for this test
    - x, y, radius: Position (x,y) and radius (in meters) used to get the checked region
    """

    def __init__(self, actor, x, y, radius, name="InRadiusRegionTest"):
        """
        """
        super(InRadiusRegionTest, self).__init__(name, actor, 0)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))
        self._actor = actor
        self._x = x     # pylint: disable=invalid-name
        self._y = y     # pylint: disable=invalid-name
        self._radius = radius

    def update(self):
        """
        Check if the actor location is within trigger region
        """
        new_status = py_trees.common.Status.RUNNING

        location = CarlaDataProvider.get_location(self._actor)
        if location is None:
            return new_status

        if self.test_status != "SUCCESS":
            in_radius = math.sqrt(((location.x - self._x)**2) + ((location.y - self._y)**2)) < self._radius
            if in_radius:
                route_completion_event = TrafficEvent(event_type=TrafficEventType.ROUTE_COMPLETED)
                route_completion_event.set_message("Destination was successfully reached")
                self.list_traffic_events.append(route_completion_event)
                self.test_status = "SUCCESS"
            else:
                self.test_status = "RUNNING"

        if self.test_status == "SUCCESS":
            new_status = py_trees.common.Status.SUCCESS

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))

        return new_status


class InRouteTest(Criterion):

    """
    The test is a success if the actor is never outside route. The actor can go outside of the route
    but only for a certain amount of distance

    Important parameters:
    - actor: CARLA actor to be used for this test
    - route: Route to be checked
    - offroad_max: Maximum distance (in meters) the actor can deviate from the route
    - offroad_min: Maximum safe distance (in meters). Might eventually cause failure
    - terminate_on_failure [optional]: If True, the complete scenario will terminate upon failure of this test
    """
    MAX_ROUTE_PERCENTAGE = 30  # %
    WINDOWS_SIZE = 5  # Amount of additional waypoints checked

    def __init__(self, actor, route, offroad_min=-1, offroad_max=30, name="InRouteTest", terminate_on_failure=False):
        """
        """
        super(InRouteTest, self).__init__(name, actor, 0, terminate_on_failure=terminate_on_failure)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))
        self._actor = actor
        self._route = route
        self._offroad_max = offroad_max
        # Unless specified, halve of the max value
        if offroad_min == -1:
            self._offroad_min = self._offroad_max / 2
        else:
            self._offroad_min = self._offroad_min

        self._world = CarlaDataProvider.get_world()
        self._waypoints, _ = zip(*self._route)
        self._route_length = len(self._route)
        self._current_index = 0
        self._out_route_distance = 0
        self._in_safe_route = True

        self._accum_meters = []
        prev_wp = self._waypoints[0]
        for i, wp in enumerate(self._waypoints):
            d = wp.distance(prev_wp)
            if i > 0:
                accum = self._accum_meters[i - 1]
            else:
                accum = 0

            self._accum_meters.append(d + accum)
            prev_wp = wp

        # Blackboard variable
        blackv = py_trees.blackboard.Blackboard()
        _ = blackv.set("InRoute", True)

    def update(self):
        """
        Check if the actor location is within trigger region
        """
        new_status = py_trees.common.Status.RUNNING

        location = CarlaDataProvider.get_location(self._actor)
        if location is None:
            return new_status

        if self._terminate_on_failure and (self.test_status == "FAILURE"):
            new_status = py_trees.common.Status.FAILURE

        elif self.test_status in ('RUNNING', 'INIT'):

            off_route = True

            shortest_distance = float('inf')
            closest_index = -1

            # Get the closest distance
            for index in range(self._current_index,
                               min(self._current_index + self.WINDOWS_SIZE + 1, self._route_length)):
                ref_waypoint = self._waypoints[index]
                distance = math.sqrt(((location.x - ref_waypoint.x) ** 2) + ((location.y - ref_waypoint.y) ** 2))
                if distance <= shortest_distance:
                    closest_index = index
                    shortest_distance = distance

            if closest_index == -1 or shortest_distance == float('inf'):
                return new_status

            # Check if the actor is out of route
            if shortest_distance < self._offroad_max:
                off_route = False
                self._in_safe_route = bool(shortest_distance < self._offroad_min)

            # If actor advanced a step, record the distance
            if self._current_index != closest_index:

                new_dist = self._accum_meters[closest_index] - self._accum_meters[self._current_index]

                # If too far from the route, add it and check if its value
                if not self._in_safe_route:
                    self._out_route_distance += new_dist
                    out_route_percentage = 100 * self._out_route_distance / self._accum_meters[-1]
                    if out_route_percentage > self.MAX_ROUTE_PERCENTAGE:
                        off_route = True

                self._current_index = closest_index

            if off_route:
                # Blackboard variable
                blackv = py_trees.blackboard.Blackboard()
                _ = blackv.set("InRoute", False)

                route_deviation_event = TrafficEvent(event_type=TrafficEventType.ROUTE_DEVIATION)
                route_deviation_event.set_message(
                    "Agent deviated from the route at (x={}, y={}, z={})".format(
                        round(location.x, 3),
                        round(location.y, 3),
                        round(location.z, 3)))
                route_deviation_event.set_dict({
                    'x': location.x,
                    'y': location.y,
                    'z': location.z})

                self.list_traffic_events.append(route_deviation_event)

                self.test_status = "FAILURE"
                self.actual_value += 1
                new_status = py_trees.common.Status.FAILURE

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))

        return new_status


class RouteCompletionTest(Criterion):

    """
    Check at which stage of the route is the actor at each tick

    Important parameters:
    - actor: CARLA actor to be used for this test
    - route: Route to be checked
    - terminate_on_failure [optional]: If True, the complete scenario will terminate upon failure of this test
    """
    DISTANCE_THRESHOLD = 10.0  # meters
    WINDOWS_SIZE = 2

    def __init__(self, actor, route, name="RouteCompletionTest", terminate_on_failure=False):
        """
        """
        super(RouteCompletionTest, self).__init__(name, actor, 100, terminate_on_failure=terminate_on_failure)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))
        self._actor = actor
        self._route = route
        self._map = CarlaDataProvider.get_map()

        self._wsize = self.WINDOWS_SIZE
        self._current_index = 0
        self._route_length = len(self._route)
        self._waypoints, _ = zip(*self._route)
        self.target = self._waypoints[-1]

        self._accum_meters = []
        prev_wp = self._waypoints[0]
        for i, wp in enumerate(self._waypoints):
            d = wp.distance(prev_wp)
            if i > 0:
                accum = self._accum_meters[i - 1]
            else:
                accum = 0

            self._accum_meters.append(d + accum)
            prev_wp = wp

        self._traffic_event = TrafficEvent(event_type=TrafficEventType.ROUTE_COMPLETION)
        self.list_traffic_events.append(self._traffic_event)
        self._percentage_route_completed = 0.0

    def update(self):
        """
        Check if the actor location is within trigger region
        """
        new_status = py_trees.common.Status.RUNNING

        location = CarlaDataProvider.get_location(self._actor)
        if location is None:
            return new_status

        if self._terminate_on_failure and (self.test_status == "FAILURE"):
            new_status = py_trees.common.Status.FAILURE

        elif self.test_status in ('RUNNING', 'INIT'):

            for index in range(self._current_index, min(self._current_index + self._wsize + 1, self._route_length)):
                # Get the dot product to know if it has passed this location
                ref_waypoint = self._waypoints[index]
                wp = self._map.get_waypoint(ref_waypoint)
                wp_dir = wp.transform.get_forward_vector()          # Waypoint's forward vector
                wp_veh = location - ref_waypoint                    # vector waypoint - vehicle
                dot_ve_wp = wp_veh.x * wp_dir.x + wp_veh.y * wp_dir.y + wp_veh.z * wp_dir.z

                if dot_ve_wp > 0:
                    # good! segment completed!
                    self._current_index = index
                    self._percentage_route_completed = 100.0 * float(self._accum_meters[self._current_index]) \
                        / float(self._accum_meters[-1])
                    self._traffic_event.set_dict({
                        'route_completed': self._percentage_route_completed})
                    self._traffic_event.set_message(
                        "Agent has completed > {:.2f}% of the route".format(
                            self._percentage_route_completed))

            if self._percentage_route_completed > 99.0 and location.distance(self.target) < self.DISTANCE_THRESHOLD:
                route_completion_event = TrafficEvent(event_type=TrafficEventType.ROUTE_COMPLETED)
                route_completion_event.set_message("Destination was successfully reached")
                self.list_traffic_events.append(route_completion_event)
                self.test_status = "SUCCESS"
                self._percentage_route_completed = 100

        elif self.test_status == "SUCCESS":
            new_status = py_trees.common.Status.SUCCESS

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))

        return new_status

    def terminate(self, new_status):
        """
        Set test status to failure if not successful and terminate
        """
        self.actual_value = round(self._percentage_route_completed, 2)

        if self.test_status == "INIT":
            self.test_status = "FAILURE"
        super(RouteCompletionTest, self).terminate(new_status)


class RunningRedLightTest(Criterion):

    """
    Check if an actor is running a red light

    Important parameters:
    - actor: CARLA actor to be used for this test
    - terminate_on_failure [optional]: If True, the complete scenario will terminate upon failure of this test
    """
    DISTANCE_LIGHT = 15  # m

    def __init__(self, actor, name="RunningRedLightTest", terminate_on_failure=False):
        """
        Init
        """
        super(RunningRedLightTest, self).__init__(name, actor, 0, terminate_on_failure=terminate_on_failure)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))
        self._actor = actor
        self._world = actor.get_world()
        self._map = CarlaDataProvider.get_map()
        self._list_traffic_lights = []
        self._last_red_light_id = None
        self.actual_value = 0
        self.debug = False

        all_actors = self._world.get_actors()
        for _actor in all_actors:
            if 'traffic_light' in _actor.type_id:
                center, waypoints = self.get_traffic_light_waypoints(_actor)
                self._list_traffic_lights.append((_actor, center, waypoints))

    # pylint: disable=no-self-use
    def is_vehicle_crossing_line(self, seg1, seg2):
        """
        check if vehicle crosses a line segment
        """
        line1 = shapely.geometry.LineString([(seg1[0].x, seg1[0].y), (seg1[1].x, seg1[1].y)])
        line2 = shapely.geometry.LineString([(seg2[0].x, seg2[0].y), (seg2[1].x, seg2[1].y)])
        inter = line1.intersection(line2)

        return not inter.is_empty

    def update(self):
        """
        Check if the actor is running a red light
        """
        new_status = py_trees.common.Status.RUNNING

        transform = CarlaDataProvider.get_transform(self._actor)
        location = transform.location
        if location is None:
            return new_status

        veh_extent = self._actor.bounding_box.extent.x

        tail_close_pt = self.rotate_point(carla.Vector3D(-0.8 * veh_extent, 0.0, location.z), transform.rotation.yaw)
        tail_close_pt = location + carla.Location(tail_close_pt)

        tail_far_pt = self.rotate_point(carla.Vector3D(-veh_extent - 1, 0.0, location.z), transform.rotation.yaw)
        tail_far_pt = location + carla.Location(tail_far_pt)

        for traffic_light, center, waypoints in self._list_traffic_lights:

            if self.debug:
                z = 2.1
                if traffic_light.state == carla.TrafficLightState.Red:
                    color = carla.Color(155, 0, 0)
                elif traffic_light.state == carla.TrafficLightState.Green:
                    color = carla.Color(0, 155, 0)
                else:
                    color = carla.Color(155, 155, 0)
                self._world.debug.draw_point(center + carla.Location(z=z), size=0.2, color=color, life_time=0.01)
                for wp in waypoints:
                    text = "{}.{}".format(wp.road_id, wp.lane_id)
                    self._world.debug.draw_string(
                        wp.transform.location + carla.Location(x=1, z=z), text, color=color, life_time=0.01)
                    self._world.debug.draw_point(
                        wp.transform.location + carla.Location(z=z), size=0.1, color=color, life_time=0.01)

            center_loc = carla.Location(center)

            if self._last_red_light_id and self._last_red_light_id == traffic_light.id:
                continue
            if center_loc.distance(location) > self.DISTANCE_LIGHT:
                continue
            if traffic_light.state != carla.TrafficLightState.Red:
                continue

            for wp in waypoints:

                tail_wp = self._map.get_waypoint(tail_far_pt)

                # Calculate the dot product (Might be unscaled, as only its sign is important)
                ve_dir = CarlaDataProvider.get_transform(self._actor).get_forward_vector()
                wp_dir = wp.transform.get_forward_vector()
                dot_ve_wp = ve_dir.x * wp_dir.x + ve_dir.y * wp_dir.y + ve_dir.z * wp_dir.z

                # Check the lane until all the "tail" has passed
                if tail_wp.road_id == wp.road_id and tail_wp.lane_id == wp.lane_id and dot_ve_wp > 0:
                    # This light is red and is affecting our lane
                    yaw_wp = wp.transform.rotation.yaw
                    lane_width = wp.lane_width
                    location_wp = wp.transform.location

                    lft_lane_wp = self.rotate_point(carla.Vector3D(0.4 * lane_width, 0.0, location_wp.z), yaw_wp + 90)
                    lft_lane_wp = location_wp + carla.Location(lft_lane_wp)
                    rgt_lane_wp = self.rotate_point(carla.Vector3D(0.4 * lane_width, 0.0, location_wp.z), yaw_wp - 90)
                    rgt_lane_wp = location_wp + carla.Location(rgt_lane_wp)

                    # Is the vehicle traversing the stop line?
                    if self.is_vehicle_crossing_line((tail_close_pt, tail_far_pt), (lft_lane_wp, rgt_lane_wp)):

                        self.test_status = "FAILURE"
                        self.actual_value += 1
                        location = traffic_light.get_transform().location
                        red_light_event = TrafficEvent(event_type=TrafficEventType.TRAFFIC_LIGHT_INFRACTION)
                        red_light_event.set_message(
                            "Agent ran a red light {} at (x={}, y={}, z={})".format(
                                traffic_light.id,
                                round(location.x, 3),
                                round(location.y, 3),
                                round(location.z, 3)))
                        red_light_event.set_dict({
                            'id': traffic_light.id,
                            'x': location.x,
                            'y': location.y,
                            'z': location.z})

                        self.list_traffic_events.append(red_light_event)
                        self._last_red_light_id = traffic_light.id
                        break

        if self._terminate_on_failure and (self.test_status == "FAILURE"):
            new_status = py_trees.common.Status.FAILURE

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))

        return new_status

    def rotate_point(self, point, angle):
        """
        rotate a given point by a given angle
        """
        x_ = math.cos(math.radians(angle)) * point.x - math.sin(math.radians(angle)) * point.y
        y_ = math.sin(math.radians(angle)) * point.x + math.cos(math.radians(angle)) * point.y
        return carla.Vector3D(x_, y_, point.z)

    def get_traffic_light_waypoints(self, traffic_light):
        """
        get area of a given traffic light
        """
        base_transform = traffic_light.get_transform()
        base_rot = base_transform.rotation.yaw
        area_loc = base_transform.transform(traffic_light.trigger_volume.location)

        # Discretize the trigger box into points
        area_ext = traffic_light.trigger_volume.extent
        x_values = np.arange(-0.9 * area_ext.x, 0.9 * area_ext.x, 1.0)  # 0.9 to avoid crossing to adjacent lanes

        area = []
        for x in x_values:
            point = self.rotate_point(carla.Vector3D(x, 0, area_ext.z), base_rot)
            point_location = area_loc + carla.Location(x=point.x, y=point.y)
            area.append(point_location)

        # Get the waypoints of these points, removing duplicates
        ini_wps = []
        for pt in area:
            wpx = self._map.get_waypoint(pt)
            # As x_values are arranged in order, only the last one has to be checked
            if not ini_wps or ini_wps[-1].road_id != wpx.road_id or ini_wps[-1].lane_id != wpx.lane_id:
                ini_wps.append(wpx)

        # Advance them until the intersection
        wps = []
        for wpx in ini_wps:
            while not wpx.is_intersection:
                next_wp = wpx.next(0.5)[0]
                if next_wp and not next_wp.is_intersection:
                    wpx = next_wp
                else:
                    break
            wps.append(wpx)

        return area_loc, wps


class RunningStopTest(Criterion):

    """
    Check if an actor is running a stop sign

    Important parameters:
    - actor: CARLA actor to be used for this test
    - terminate_on_failure [optional]: If True, the complete scenario will terminate upon failure of this test
    """
    PROXIMITY_THRESHOLD = 50.0  # meters
    SPEED_THRESHOLD = 0.1
    WAYPOINT_STEP = 1.0  # meters

    def __init__(self, actor, name="RunningStopTest", terminate_on_failure=False):
        """
        """
        super(RunningStopTest, self).__init__(name, actor, 0, terminate_on_failure=terminate_on_failure)
        self.logger.debug("%s.__init__()" % (self.__class__.__name__))
        self._actor = actor
        self._world = CarlaDataProvider.get_world()
        self._map = CarlaDataProvider.get_map()
        self._list_stop_signs = []
        self._target_stop_sign = None
        self._stop_completed = False
        self._affected_by_stop = False
        self.actual_value = 0

        all_actors = self._world.get_actors()
        for _actor in all_actors:
            if 'traffic.stop' in _actor.type_id:
                self._list_stop_signs.append(_actor)

    @staticmethod
    def point_inside_boundingbox(point, bb_center, bb_extent):
        """
        X
        :param point:
        :param bb_center:
        :param bb_extent:
        :return:
        """

        # pylint: disable=invalid-name
        A = carla.Vector2D(bb_center.x - bb_extent.x, bb_center.y - bb_extent.y)
        B = carla.Vector2D(bb_center.x + bb_extent.x, bb_center.y - bb_extent.y)
        D = carla.Vector2D(bb_center.x - bb_extent.x, bb_center.y + bb_extent.y)
        M = carla.Vector2D(point.x, point.y)

        AB = B - A
        AD = D - A
        AM = M - A
        am_ab = AM.x * AB.x + AM.y * AB.y
        ab_ab = AB.x * AB.x + AB.y * AB.y
        am_ad = AM.x * AD.x + AM.y * AD.y
        ad_ad = AD.x * AD.x + AD.y * AD.y

        return am_ab > 0 and am_ab < ab_ab and am_ad > 0 and am_ad < ad_ad  # pylint: disable=chained-comparison

    def is_actor_affected_by_stop(self, actor, stop, multi_step=20):
        """
        Check if the given actor is affected by the stop
        """
        affected = False
        # first we run a fast coarse test
        current_location = actor.get_location()
        stop_location = stop.get_transform().location
        if stop_location.distance(current_location) > self.PROXIMITY_THRESHOLD:
            return affected

        stop_t = stop.get_transform()
        transformed_tv = stop_t.transform(stop.trigger_volume.location)

        # slower and accurate test based on waypoint's horizon and geometric test
        list_locations = [current_location]
        waypoint = self._map.get_waypoint(current_location)
        for _ in range(multi_step):
            if waypoint:
                next_wps = waypoint.next(self.WAYPOINT_STEP)
                if not next_wps:
                    break
                waypoint = next_wps[0]
                if not waypoint:
                    break
                list_locations.append(waypoint.transform.location)

        for actor_location in list_locations:
            if self.point_inside_boundingbox(actor_location, transformed_tv, stop.trigger_volume.extent):
                affected = True

        return affected

    def _scan_for_stop_sign(self):
        target_stop_sign = None

        ve_tra = CarlaDataProvider.get_transform(self._actor)
        ve_dir = ve_tra.get_forward_vector()

        wp = self._map.get_waypoint(ve_tra.location)
        wp_dir = wp.transform.get_forward_vector()

        dot_ve_wp = ve_dir.x * wp_dir.x + ve_dir.y * wp_dir.y + ve_dir.z * wp_dir.z

        if dot_ve_wp > 0:  # Ignore all when going in a wrong lane
            for stop_sign in self._list_stop_signs:
                if self.is_actor_affected_by_stop(self._actor, stop_sign):
                    # this stop sign is affecting the vehicle
                    target_stop_sign = stop_sign
                    break

        return target_stop_sign

    def update(self):
        """
        Check if the actor is running a red light
        """
        new_status = py_trees.common.Status.RUNNING

        location = self._actor.get_location()
        if location is None:
            return new_status

        if not self._target_stop_sign:
            # scan for stop signs
            self._target_stop_sign = self._scan_for_stop_sign()
        else:
            # we were in the middle of dealing with a stop sign
            if not self._stop_completed:
                # did the ego-vehicle stop?
                current_speed = CarlaDataProvider.get_velocity(self._actor)
                if current_speed < self.SPEED_THRESHOLD:
                    self._stop_completed = True

            if not self._affected_by_stop:
                stop_location = self._target_stop_sign.get_location()
                stop_extent = self._target_stop_sign.trigger_volume.extent

                if self.point_inside_boundingbox(location, stop_location, stop_extent):
                    self._affected_by_stop = True

            if not self.is_actor_affected_by_stop(self._actor, self._target_stop_sign):
                # is the vehicle out of the influence of this stop sign now?
                if not self._stop_completed and self._affected_by_stop:
                    # did we stop?
                    self.actual_value += 1
                    self.test_status = "FAILURE"
                    stop_location = self._target_stop_sign.get_transform().location
                    running_stop_event = TrafficEvent(event_type=TrafficEventType.STOP_INFRACTION)
                    running_stop_event.set_message(
                        "Agent ran a stop with id={} at (x={}, y={}, z={})".format(
                            self._target_stop_sign.id,
                            round(stop_location.x, 3),
                            round(stop_location.y, 3),
                            round(stop_location.z, 3)))
                    running_stop_event.set_dict({
                        'id': self._target_stop_sign.id,
                        'x': stop_location.x,
                        'y': stop_location.y,
                        'z': stop_location.z})

                    self.list_traffic_events.append(running_stop_event)

                # reset state
                self._target_stop_sign = None
                self._stop_completed = False
                self._affected_by_stop = False

        if self._terminate_on_failure and (self.test_status == "FAILURE"):
            new_status = py_trees.common.Status.FAILURE

        self.logger.debug("%s.update()[%s->%s]" % (self.__class__.__name__, self.status, new_status))

        return new_status
