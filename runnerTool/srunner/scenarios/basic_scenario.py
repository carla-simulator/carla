#!/usr/bin/env python

# Copyright (c) 2018-2020 Intel Corporation
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
This module provide BasicScenario, the basic class of all the scenarios.
"""

from __future__ import print_function

import operator
import py_trees

import carla

import srunner.scenariomanager.scenarioatomics.atomic_trigger_conditions as conditions
from srunner.scenariomanager.carla_data_provider import CarlaDataProvider
from srunner.scenariomanager.timer import TimeOut
from srunner.scenariomanager.weather_sim import WeatherBehavior
from srunner.scenariomanager.scenarioatomics.atomic_behaviors import UpdateAllActorControls


class BasicScenario(object):

    """
    Base class for user-defined scenario
    """

    def __init__(self, name, ego_vehicles, config, world,
                 debug_mode=False, terminate_on_failure=False, criteria_enable=False):
        """
        Setup all relevant parameters and create scenario
        and instantiate scenario manager
        """
        self.other_actors = []
        if not self.timeout:     # pylint: disable=access-member-before-definition
            self.timeout = 60    # If no timeout was provided, set it to 60 seconds

        self.criteria_list = []  # List of evaluation criteria
        self.scenario = None
        self.world = world

        self.ego_vehicles = ego_vehicles
        self.name = name
        self.config = config
        self.terminate_on_failure = terminate_on_failure

        self._initialize_environment(world)

        # Initializing adversarial actors
        self._initialize_actors(config)
        if CarlaDataProvider.is_sync_mode():
            world.tick()
        else:
            world.wait_for_tick()

        # Setup scenario
        if debug_mode:
            py_trees.logging.level = py_trees.logging.Level.DEBUG

        behavior = self._create_behavior()

        criteria = None
        if criteria_enable:
            criteria = self._create_test_criteria()

        # Add a trigger condition for the behavior to ensure the behavior is only activated, when it is relevant
        behavior_seq = py_trees.composites.Sequence()
        trigger_behavior = self._setup_scenario_trigger(config)
        if trigger_behavior:
            behavior_seq.add_child(trigger_behavior)

        if behavior is not None:
            behavior_seq.add_child(behavior)
            behavior_seq.name = behavior.name

        end_behavior = self._setup_scenario_end(config)
        if end_behavior:
            behavior_seq.add_child(end_behavior)

        self.scenario = Scenario(behavior_seq, criteria, self.name, self.timeout, self.terminate_on_failure)

    def _initialize_environment(self, world):
        """
        Default initialization of weather and road friction.
        Override this method in child class to provide custom initialization.
        """

        # Set the appropriate weather conditions
        world.set_weather(self.config.weather)

        # Set the appropriate road friction
        if self.config.friction is not None:
            friction_bp = world.get_blueprint_library().find('static.trigger.friction')
            extent = carla.Location(1000000.0, 1000000.0, 1000000.0)
            friction_bp.set_attribute('friction', str(self.config.friction))
            friction_bp.set_attribute('extent_x', str(extent.x))
            friction_bp.set_attribute('extent_y', str(extent.y))
            friction_bp.set_attribute('extent_z', str(extent.z))

            # Spawn Trigger Friction
            transform = carla.Transform()
            transform.location = carla.Location(-10000.0, -10000.0, 0.0)
            world.spawn_actor(friction_bp, transform)

    def _initialize_actors(self, config):
        """
        Default initialization of other actors.
        Override this method in child class to provide custom initialization.
        """
        if config.other_actors:
            new_actors = CarlaDataProvider.request_new_actors(config.other_actors)
            if not new_actors:
                raise Exception("Error: Unable to add actors")

            for new_actor in new_actors:
                self.other_actors.append(new_actor)

    def _setup_scenario_trigger(self, config):
        """
        This function creates a trigger maneuver, that has to be finished before the real scenario starts.
        This implementation focuses on the first available ego vehicle.

        The function can be overloaded by a user implementation inside the user-defined scenario class.
        """
        start_location = None
        if config.trigger_points and config.trigger_points[0]:
            start_location = config.trigger_points[0].location     # start location of the scenario

        ego_vehicle_route = CarlaDataProvider.get_ego_vehicle_route()

        if start_location:
            if ego_vehicle_route:
                if config.route_var_name is None:  # pylint: disable=no-else-return
                    return conditions.InTriggerDistanceToLocationAlongRoute(self.ego_vehicles[0],
                                                                            ego_vehicle_route,
                                                                            start_location,
                                                                            5)
                else:
                    check_name = "WaitForBlackboardVariable: {}".format(config.route_var_name)
                    return conditions.WaitForBlackboardVariable(name=check_name,
                                                                variable_name=config.route_var_name,
                                                                variable_value=True,
                                                                var_init_value=False)

            return conditions.InTimeToArrivalToLocation(self.ego_vehicles[0],
                                                        2.0,
                                                        start_location)

        return None

    def _setup_scenario_end(self, config):
        """
        This function adds and additional behavior to the scenario, which is triggered
        after it has ended.

        The function can be overloaded by a user implementation inside the user-defined scenario class.
        """
        ego_vehicle_route = CarlaDataProvider.get_ego_vehicle_route()

        if ego_vehicle_route:
            if config.route_var_name is not None:
                set_name = "Reset Blackboard Variable: {} ".format(config.route_var_name)
                return py_trees.blackboard.SetBlackboardVariable(name=set_name,
                                                                 variable_name=config.route_var_name,
                                                                 variable_value=False)
        return None

    def _create_behavior(self):
        """
        Pure virtual function to setup user-defined scenario behavior
        """
        raise NotImplementedError(
            "This function is re-implemented by all scenarios"
            "If this error becomes visible the class hierarchy is somehow broken")

    def _create_test_criteria(self):
        """
        Pure virtual function to setup user-defined evaluation criteria for the
        scenario
        """
        raise NotImplementedError(
            "This function is re-implemented by all scenarios"
            "If this error becomes visible the class hierarchy is somehow broken")

    def change_control(self, control):  # pylint: disable=no-self-use
        """
        This is a function that changes the control based on the scenario determination
        :param control: a carla vehicle control
        :return: a control to be changed by the scenario.

        Note: This method should be overriden by the user-defined scenario behavior
        """
        return control

    def remove_all_actors(self):
        """
        Remove all actors
        """
        for i, _ in enumerate(self.other_actors):
            if self.other_actors[i] is not None:
                if CarlaDataProvider.actor_id_exists(self.other_actors[i].id):
                    CarlaDataProvider.remove_actor_by_id(self.other_actors[i].id)
                self.other_actors[i] = None
        self.other_actors = []


class Scenario(object):

    """
    Basic scenario class. This class holds the behavior_tree describing the
    scenario and the test criteria.

    The user must not modify this class.

    Important parameters:
    - behavior: User defined scenario with py_tree
    - criteria_list: List of user defined test criteria with py_tree
    - timeout (default = 60s): Timeout of the scenario in seconds
    - terminate_on_failure: Terminate scenario on first failure
    """

    def __init__(self, behavior, criteria, name, timeout=60, terminate_on_failure=False):
        self.behavior = behavior
        self.test_criteria = criteria
        self.timeout = timeout
        self.name = name

        if self.test_criteria is not None and not isinstance(self.test_criteria, py_trees.composites.Parallel):
            # list of nodes
            for criterion in self.test_criteria:
                criterion.terminate_on_failure = terminate_on_failure

            # Create py_tree for test criteria
            self.criteria_tree = py_trees.composites.Parallel(
                name="Test Criteria",
                policy=py_trees.common.ParallelPolicy.SUCCESS_ON_ONE
            )
            self.criteria_tree.add_children(self.test_criteria)
            self.criteria_tree.setup(timeout=1)
        else:
            self.criteria_tree = criteria

        # Create node for timeout
        self.timeout_node = TimeOut(self.timeout, name="TimeOut")

        # Create overall py_tree
        self.scenario_tree = py_trees.composites.Parallel(name, policy=py_trees.common.ParallelPolicy.SUCCESS_ON_ONE)
        if behavior is not None:
            self.scenario_tree.add_child(self.behavior)
        self.scenario_tree.add_child(self.timeout_node)
        self.scenario_tree.add_child(WeatherBehavior())
        self.scenario_tree.add_child(UpdateAllActorControls())

        if criteria is not None:
            self.scenario_tree.add_child(self.criteria_tree)
        self.scenario_tree.setup(timeout=1)

    def _extract_nodes_from_tree(self, tree):  # pylint: disable=no-self-use
        """
        Returns the list of all nodes from the given tree
        """
        node_list = [tree]
        more_nodes_exist = True
        while more_nodes_exist:
            more_nodes_exist = False
            for node in list(node_list):
                if node.children:
                    node_list.remove(node)
                    more_nodes_exist = True
                    for child in node.children:
                        node_list.append(child)

        if len(node_list) == 1 and isinstance(node_list[0], py_trees.composites.Parallel):
            return []

        return node_list

    def get_criteria(self):
        """
        Return the list of test criteria (all leave nodes)
        """
        criteria_list = self._extract_nodes_from_tree(self.criteria_tree)
        return criteria_list

    def terminate(self):
        """
        This function sets the status of all leaves in the scenario tree to INVALID
        """
        # Get list of all nodes in the tree
        node_list = self._extract_nodes_from_tree(self.scenario_tree)

        # Set status to INVALID
        for node in node_list:
            node.terminate(py_trees.common.Status.INVALID)

        # Cleanup all instantiated controllers
        actor_dict = {}
        try:
            check_actors = operator.attrgetter("ActorsWithController")
            actor_dict = check_actors(py_trees.blackboard.Blackboard())
        except AttributeError:
            pass
        for actor_id in actor_dict:
            actor_dict[actor_id].reset()
        py_trees.blackboard.Blackboard().set("ActorsWithController", {}, overwrite=True)
