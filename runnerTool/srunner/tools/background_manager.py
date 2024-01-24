#!/usr/bin/env python

#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Several atomic behaviors to help with the communication with the background activity,
removing its interference with other scenarios
"""

import py_trees
from srunner.scenariomanager.scenarioatomics.atomic_behaviors import AtomicBehavior


class RoadBehaviorManager(AtomicBehavior):
    """
    Updates the blackboard to change the parameters of the road behavior.
    None values imply that these values won't be changed

    Args:
        num_front_vehicles (int): Amount of vehicles in front of the ego. Can't be negative
        num_back_vehicles (int): Amount of vehicles behind it. Can't be negative
        vehicle_dist (float): Minimum distance between the road vehicles. Must between 0 and 'spawn_dist'
        spawn_dist (float): Minimum distance between spawned vehicles. Must be positive
    """

    def __init__(self, num_front_vehicles=None, num_back_vehicles=None,
                 vehicle_dist=None, spawn_dist=None, name="RoadBehaviorManager"):
        self._num_front_vehicles = num_front_vehicles
        self._num_back_vehicles = num_back_vehicles
        self._vehicle_dist = vehicle_dist
        self._spawn_dist = spawn_dist
        super(RoadBehaviorManager, self).__init__(name)

    def update(self):
        py_trees.blackboard.Blackboard().set(
            "BA_RoadBehavior",
            [self._num_front_vehicles, self._num_back_vehicles, self._vehicle_dist, self._spawn_dist],
            overwrite=True
        )
        return py_trees.common.Status.SUCCESS


class OppositeBehaviorManager(AtomicBehavior):
    """
    Updates the blackboard to change the parameters of the opposite road behavior.
    None values imply that these values won't be changed

    Args:
        source_dist (float): Distance between the opposite sources and the ego vehicle. Must be positive
        vehicle_dist (float) Minimum distance between the opposite vehicles. Must between 0 and 'spawn_dist'
        spawn_dist (float): Minimum distance between spawned vehicles. Must be positive
        max_actors (int): Max amount of concurrent alive actors spawned by the same source. Can't be negative
    """

    def __init__(self, source_dist=None, vehicle_dist=None, spawn_dist=None,
                 max_actors=None, name="OppositeBehaviorManager"):
        self._source_dist = source_dist
        self._vehicle_dist = vehicle_dist
        self._spawn_dist = spawn_dist
        self._max_actors = max_actors
        super(OppositeBehaviorManager, self).__init__(name)

    def update(self):
        py_trees.blackboard.Blackboard().set(
            "BA_OppositeBehavior",
            [self._source_dist, self._vehicle_dist, self._spawn_dist, self._max_actors],
            overwrite=True
        )
        return py_trees.common.Status.SUCCESS


class JunctionBehaviorManager(AtomicBehavior):
    """
    Updates the blackboard to change the parameters of the junction behavior.
    None values imply that these values won't be changed

    Args:
        source_dist (float): Distance between the junctiob sources and the junction entry. Must be positive
        vehicle_dist (float) Minimum distance between the junction vehicles. Must between 0 and 'spawn_dist'
        spawn_dist (float): Minimum distance between spawned vehicles. Must be positive
        max_actors (int): Max amount of concurrent alive actors spawned by the same source. Can't be negative

    """

    def __init__(self, source_dist=None, vehicle_dist=None, spawn_dist=None,
                 max_actors=None, name="JunctionBehaviorManager"):
        self._source_dist = source_dist
        self._vehicle_dist = vehicle_dist
        self._spawn_dist = spawn_dist
        self._max_actors = max_actors
        super(JunctionBehaviorManager, self).__init__(name)

    def update(self):
        py_trees.blackboard.Blackboard().set(
            "BA_JunctionBehavior",
            [self._source_dist, self._vehicle_dist, self._spawn_dist, self._max_actors],
            overwrite=True
        )
        return py_trees.common.Status.SUCCESS


class Scenario2Manager(AtomicBehavior):
    """
    Updates the blackboard to tell the background activity that a Scenario2 has to be triggered.
    'stop_duration' is the amount of time, in seconds, the vehicles will be stopped
    """

    def __init__(self, stop_duration=10, name="Scenario2Manager"):
        self._stop_duration = stop_duration
        super(Scenario2Manager, self).__init__(name)

    def update(self):
        py_trees.blackboard.Blackboard().set("BA_Scenario2", self._stop_duration, overwrite=True)
        return py_trees.common.Status.SUCCESS


class Scenario4Manager(AtomicBehavior):
    """
    Updates the blackboard to tell the background activity that a Scenario4 has been triggered.
    'crossing_dist' is the distance between the crossing actor and the junction
    """

    def __init__(self, crossing_dist=10, name="Scenario4Manager"):
        self._crossing_dist = crossing_dist
        super(Scenario4Manager, self).__init__(name)

    def update(self):
        """Updates the blackboard and succeds"""
        py_trees.blackboard.Blackboard().set("BA_Scenario4", self._crossing_dist, overwrite=True)
        return py_trees.common.Status.SUCCESS


class Scenario7Manager(AtomicBehavior):
    """
    Updates the blackboard to tell the background activity that a Scenario7 has been triggered
    'entry_direction' is the direction from which the incoming traffic enters the junction. It should be
    something like 'left', 'right' or 'opposite'
    """

    def __init__(self, entry_direction, name="Scenario7Manager"):
        self._entry_direction = entry_direction
        super(Scenario7Manager, self).__init__(name)

    def update(self):
        """Updates the blackboard and succeds"""
        py_trees.blackboard.Blackboard().set("BA_Scenario7", self._entry_direction, overwrite=True)
        return py_trees.common.Status.SUCCESS


class Scenario8Manager(AtomicBehavior):
    """
    Updates the blackboard to tell the background activity that a Scenario8 has been triggered
    'entry_direction' is the direction from which the incoming traffic enters the junction. It should be
    something like 'left', 'right' or 'opposite'
    """

    def __init__(self, entry_direction, name="Scenario8Manager"):
        self._entry_direction = entry_direction
        super(Scenario8Manager, self).__init__(name)

    def update(self):
        """Updates the blackboard and succeds"""
        py_trees.blackboard.Blackboard().set("BA_Scenario8", self._entry_direction, overwrite=True)
        return py_trees.common.Status.SUCCESS


class Scenario9Manager(AtomicBehavior):
    """
    Updates the blackboard to tell the background activity that a Scenario9 has been triggered
    'entry_direction' is the direction from which the incoming traffic enters the junction. It should be
    something like 'left', 'right' or 'opposite'
    """

    def __init__(self, entry_direction, name="Scenario9Manager"):
        self._entry_direction = entry_direction
        super(Scenario9Manager, self).__init__(name)

    def update(self):
        """Updates the blackboard and succeds"""
        py_trees.blackboard.Blackboard().set("BA_Scenario9", self._entry_direction, overwrite=True)
        return py_trees.common.Status.SUCCESS


class Scenario10Manager(AtomicBehavior):
    """
    Updates the blackboard to tell the background activity that a Scenario10 has been triggered
    'entry_direction' is the direction from which the incoming traffic enters the junction. It should be
    something like 'left', 'right' or 'opposite'
    """

    def __init__(self, entry_direction, name="Scenario10Manager"):
        self._entry_direction = entry_direction
        super(Scenario10Manager, self).__init__(name)

    def update(self):
        """Updates the blackboard and succeds"""
        py_trees.blackboard.Blackboard().set("BA_Scenario10", self._entry_direction, overwrite=True)
        return py_trees.common.Status.SUCCESS
