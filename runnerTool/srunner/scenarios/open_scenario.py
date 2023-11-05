#!/usr/bin/env python

# Copyright (c) 2019-2020 Intel Corporation
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Basic scenario class using the OpenSCENARIO definition
"""

from __future__ import print_function

import itertools
import os
import py_trees

from srunner.scenariomanager.scenarioatomics.atomic_behaviors import ChangeWeather, ChangeRoadFriction, ChangeParameter
from srunner.scenariomanager.scenarioatomics.atomic_behaviors import ChangeActorControl, ChangeActorTargetSpeed
from srunner.scenariomanager.timer import GameTime
from srunner.scenarios.basic_scenario import BasicScenario
from srunner.tools.openscenario_parser import OpenScenarioParser, oneshot_with_check, ParameterRef
from srunner.tools.py_trees_port import Decorator


def repeatable_behavior(behaviour, name=None):
    """
    This behaviour allows a composite with oneshot ancestors to run multiple
    times, resetting the oneshot variables after each execution
    """
    if not name:
        name = behaviour.name
    clear_descendant_variables = ClearBlackboardVariablesStartingWith(
        name="Clear Descendant Variables of {}".format(name),
        variable_name_beginning=name + ">"
    )
    # If it's a sequence, don't double-nest it in a redundant manner
    if isinstance(behaviour, py_trees.composites.Sequence):
        behaviour.add_child(clear_descendant_variables)
        sequence = behaviour
    else:
        sequence = py_trees.composites.Sequence(name="RepeatableBehaviour of {}".format(name))
        sequence.add_children([behaviour, clear_descendant_variables])
    return sequence


class ClearBlackboardVariablesStartingWith(py_trees.behaviours.Success):

    """
    Clear the values starting with the specified string from the blackboard.

    Args:
        name (:obj:`str`): name of the behaviour
        variable_name_beginning (:obj:`str`): beginning of the names of variable to clear
    """

    def __init__(self,
                 name="Clear Blackboard Variable Starting With",
                 variable_name_beginning="dummy",
                 ):
        super(ClearBlackboardVariablesStartingWith, self).__init__(name)
        self.variable_name_beginning = variable_name_beginning

    def initialise(self):
        """
        Delete the variables from the blackboard.
        """
        blackboard_variables = [key for key, _ in py_trees.blackboard.Blackboard().__dict__.items(
        ) if key.startswith(self.variable_name_beginning)]
        for variable in blackboard_variables:
            delattr(py_trees.blackboard.Blackboard(), variable)


class StoryElementStatusToBlackboard(Decorator):

    """
    Reflect the status of the decorator's child story element to the blackboard.

    Args:
        child: the child behaviour or subtree
        story_element_type: the element type [act,scene,maneuver,event,action]
        element_name: the story element's name attribute
    """

    def __init__(self, child, story_element_type, element_name):
        super(StoryElementStatusToBlackboard, self).__init__(name=child.name, child=child)
        self.story_element_type = story_element_type
        self.element_name = element_name
        self.blackboard = py_trees.blackboard.Blackboard()

    def initialise(self):
        """
        Record the elements's start time on the blackboard
        """
        self.blackboard.set(
            name="({}){}-{}".format(self.story_element_type.upper(),
                                    self.element_name, "START"),
            value=GameTime.get_time(),
            overwrite=True
        )

    def update(self):
        """
        Reflect the decorated child's status
        Returns: the decorated child's status
        """
        return self.decorated.status

    def terminate(self, new_status):
        """
        Terminate and mark Blackboard entry with END
        """
        # Report whether we ended with End or Cancel
        # If we were ended or cancelled, our state will be INVALID and
        # We will have an ancestor (a parallel SUCCESS_ON_ALL) with a successful child/children
        # It's possible we ENDed AND CANCELled if both condition groups were true simultaneously
        # NOTE 'py_trees.common.Status.INVALID' is the status of a behaviur which was terminated by a parent
        rules = []
        if new_status == py_trees.common.Status.INVALID:
            # We were terminated from above unnaturally
            # Figure out if were ended or cancelled
            terminating_ancestor = self.parent
            while terminating_ancestor.status == py_trees.common.Status.INVALID:
                terminating_ancestor = terminating_ancestor.parent
            # We have found an ancestory which was not terminated by a parent
            # Check what caused it to terminate its children
            if terminating_ancestor.status == py_trees.common.Status.SUCCESS:
                successful_children = [
                    child.name
                    for child
                    in terminating_ancestor.children
                    if child.status == py_trees.common.Status.SUCCESS]
                if "StopTrigger" in successful_children:
                    rules.append("END")

        # END is the default status unless we have a more detailed one
        rules = rules or ["END"]

        for rule in rules:
            self.blackboard.set(
                name="({}){}-{}".format(self.story_element_type.upper(),
                                        self.element_name, rule),
                value=GameTime.get_time(),
                overwrite=True
            )


def get_xml_path(tree, node):
    """
    Extract the full path of a node within an XML tree

    Note: Catalogs are pulled from a separate file so the XML tree is split.
          This means that in order to get the XML path, it must be done in 2 steps.
          Some places in this python script do that by concatenating the results
          of 2 get_xml_path calls with another ">".
          Example: "Behavior>AutopilotSequence" + ">" + "StartAutopilot>StartAutopilot>StartAutopilot"
    """

    path = ""
    parent_map = {c: p for p in tree.iter() for c in p}

    cur_node = node
    while cur_node != tree:
        path = "{}>{}".format(cur_node.attrib.get('name'), path)
        cur_node = parent_map[cur_node]

    path = path[:-1]
    return path


class OpenScenario(BasicScenario):

    """
    Implementation of the OpenSCENARIO scenario
    """

    def __init__(self, world, ego_vehicles, config, config_file, debug_mode=False, criteria_enable=True, timeout=300):
        """
        Setup all relevant parameters and create scenario
        """
        self.config = config
        self.route = None
        self.config_file = config_file
        # Timeout of scenario in seconds
        self.timeout = timeout

        super(OpenScenario, self).__init__(self.config.name, ego_vehicles=ego_vehicles, config=config,
                                           world=world, debug_mode=debug_mode,
                                           terminate_on_failure=False, criteria_enable=criteria_enable)

    def _initialize_parameters(self):
        """
        Parse ParameterAction from Init and update global osc parameters.
        """
        param_behavior = py_trees.composites.Parallel(
            policy=py_trees.common.ParallelPolicy.SUCCESS_ON_ALL, name="ParametersInit")
        for i, global_action in enumerate(self.config.init.find('Actions').iter('GlobalAction')):
            maneuver_name = 'InitParams'
            if global_action.find('ParameterAction') is not None:
                parameter_action = global_action.find('ParameterAction')
                parameter_ref = parameter_action.attrib.get('parameterRef')
                if parameter_action.find('ModifyAction') is not None:
                    action_rule = parameter_action.find('ModifyAction').find("Rule")
                    if action_rule.find("AddValue") is not None:
                        rule, value = '+', action_rule.find("AddValue").attrib.get('value')
                    else:
                        rule, value = '*', action_rule.find("MultiplyByValue").attrib.get('value')
                else:
                    rule, value = None, parameter_action.find('SetAction').attrib.get('value')
                parameter_update = ChangeParameter(parameter_ref, value=ParameterRef(value), rule=rule,
                                                   name=maneuver_name + '_%d' % i)
                param_behavior.add_child(oneshot_with_check(variable_name="InitialParameters" + '_%d' % i,
                                                            behaviour=parameter_update))

        return param_behavior

    def _initialize_environment(self, world):
        """
        Initialization of weather and road friction.
        """
        pass

    def _create_environment_behavior(self):
        # Set the appropriate weather conditions

        env_behavior = py_trees.composites.Parallel(
            policy=py_trees.common.ParallelPolicy.SUCCESS_ON_ALL, name="EnvironmentBehavior")

        weather_update = ChangeWeather(
            OpenScenarioParser.get_weather_from_env_action(self.config.init, self.config.catalogs))
        road_friction = ChangeRoadFriction(
            OpenScenarioParser.get_friction_from_env_action(self.config.init, self.config.catalogs))
        env_behavior.add_child(oneshot_with_check(variable_name="InitialWeather", behaviour=weather_update))
        env_behavior.add_child(oneshot_with_check(variable_name="InitRoadFriction", behaviour=road_friction))

        return env_behavior

    def _create_init_behavior(self):

        init_behavior = py_trees.composites.Parallel(
            policy=py_trees.common.ParallelPolicy.SUCCESS_ON_ALL, name="InitBehaviour")

        for actor in self.config.other_actors + self.config.ego_vehicles:
            for carla_actor in self.other_actors + self.ego_vehicles:
                if (carla_actor is not None and 'role_name' in carla_actor.attributes and
                        carla_actor.attributes['role_name'] == actor.rolename):
                    actor_init_behavior = py_trees.composites.Sequence(name="InitActor{}".format(actor.rolename))

                    controller_atomic = None

                    for private in self.config.init.iter("Private"):
                        if private.attrib.get('entityRef', None) == actor.rolename:
                            for private_action in private.iter("PrivateAction"):
                                for controller_action in private_action.iter('ControllerAction'):
                                    module, args = OpenScenarioParser.get_controller(
                                        controller_action, self.config.catalogs)
                                    controller_atomic = ChangeActorControl(
                                        carla_actor, control_py_module=module, args=args,
                                        scenario_file_path=os.path.dirname(self.config.filename))

                    if controller_atomic is None:
                        controller_atomic = ChangeActorControl(carla_actor, control_py_module=None, args={})

                    actor_init_behavior.add_child(controller_atomic)

                    if actor.speed > 0:
                        actor_init_behavior.add_child(ChangeActorTargetSpeed(carla_actor, actor.speed, init_speed=True))

                    init_behavior.add_child(actor_init_behavior)
                    break

        return init_behavior

    def _create_behavior(self):
        """
        Basic behavior do nothing, i.e. Idle
        """

        stories_behavior = py_trees.composites.Parallel(policy=py_trees.common.ParallelPolicy.SUCCESS_ON_ALL,
                                                        name="OSCStories")
        joint_actor_list = self.other_actors + self.ego_vehicles + [None]

        for story in self.config.stories:
            story_name = story.get("name")
            story_behavior = py_trees.composites.Parallel(policy=py_trees.common.ParallelPolicy.SUCCESS_ON_ALL,
                                                          name=story_name)
            for act in story.iter("Act"):

                act_sequence = py_trees.composites.Sequence(
                    name="Act StartConditions and behaviours")

                start_conditions = py_trees.composites.Parallel(
                    policy=py_trees.common.ParallelPolicy.SUCCESS_ON_ONE, name="StartConditions Group")

                parallel_behavior = py_trees.composites.Parallel(
                    policy=py_trees.common.ParallelPolicy.SUCCESS_ON_ONE, name="Maneuver + EndConditions Group")

                parallel_sequences = py_trees.composites.Parallel(
                    policy=py_trees.common.ParallelPolicy.SUCCESS_ON_ALL, name="Maneuvers")

                for sequence in act.iter("ManeuverGroup"):
                    sequence_behavior = py_trees.composites.Sequence(name=sequence.attrib.get('name'))
                    repetitions = sequence.attrib.get('maximumExecutionCount', 1)

                    for _ in range(int(repetitions)):

                        actor_ids = []
                        for actor in sequence.iter("Actors"):
                            for entity in actor.iter("EntityRef"):
                                entity_name = entity.attrib.get('entityRef', None)
                                for k, _ in enumerate(joint_actor_list):
                                    if (joint_actor_list[k] and
                                            entity_name == joint_actor_list[k].attributes['role_name']):
                                        actor_ids.append(k)
                                        break

                        if not actor_ids:
                            print("Warning: Maneuvergroup {} does not use reference actors!".format(
                                sequence.attrib.get('name')))
                            actor_ids.append(len(joint_actor_list) - 1)

                    # Collect catalog reference maneuvers in order to process them at the same time as normal maneuvers
                        catalog_maneuver_list = []
                        for catalog_reference in sequence.iter("CatalogReference"):
                            catalog_maneuver = OpenScenarioParser.get_catalog_entry(self.config.catalogs,
                                                                                    catalog_reference)
                            catalog_maneuver_list.append(catalog_maneuver)
                        all_maneuvers = itertools.chain(iter(catalog_maneuver_list), sequence.iter("Maneuver"))
                        single_sequence_iteration = py_trees.composites.Parallel(
                            policy=py_trees.common.ParallelPolicy.SUCCESS_ON_ALL, name=sequence_behavior.name)
                        for maneuver in all_maneuvers:  # Iterates through both CatalogReferences and Maneuvers
                            maneuver_parallel = py_trees.composites.Parallel(
                                policy=py_trees.common.ParallelPolicy.SUCCESS_ON_ALL,
                                name="Maneuver " + maneuver.attrib.get('name'))
                            for event in maneuver.iter("Event"):
                                event_sequence = py_trees.composites.Sequence(
                                    name="Event " + event.attrib.get('name'))
                                parallel_actions = py_trees.composites.Parallel(
                                    policy=py_trees.common.ParallelPolicy.SUCCESS_ON_ALL, name="Actions")
                                for child in event.iter():
                                    if child.tag == "Action":
                                        for actor_id in actor_ids:
                                            maneuver_behavior = OpenScenarioParser.convert_maneuver_to_atomic(
                                                child, joint_actor_list[actor_id],
                                                joint_actor_list, self.config.catalogs)
                                            maneuver_behavior = StoryElementStatusToBlackboard(
                                                maneuver_behavior, "ACTION", child.attrib.get('name'))
                                            parallel_actions.add_child(
                                                oneshot_with_check(variable_name=  # See note in get_xml_path
                                                                   get_xml_path(story, sequence) + '>' + \
                                                                   get_xml_path(maneuver, child) + '>' + \
                                                                   str(actor_id),
                                                                   behaviour=maneuver_behavior))

                                    if child.tag == "StartTrigger":
                                        # There is always one StartConditions block per Event
                                        parallel_condition_groups = self._create_condition_container(
                                            child, story, "Parallel Condition Groups", sequence, maneuver)
                                        event_sequence.add_child(
                                            parallel_condition_groups)

                                parallel_actions = StoryElementStatusToBlackboard(
                                    parallel_actions, "EVENT", event.attrib.get('name'))
                                event_sequence.add_child(parallel_actions)
                                maneuver_parallel.add_child(
                                    oneshot_with_check(variable_name=get_xml_path(story, sequence) + '>' +
                                                       get_xml_path(maneuver, event),  # See get_xml_path
                                                       behaviour=event_sequence))
                            maneuver_parallel = StoryElementStatusToBlackboard(
                                maneuver_parallel, "MANEUVER", maneuver.attrib.get('name'))
                            single_sequence_iteration.add_child(
                                oneshot_with_check(variable_name=get_xml_path(story, sequence) + '>' +
                                                   maneuver.attrib.get('name'),  # See get_xml_path
                                                   behaviour=maneuver_parallel))

                        # OpenSCENARIO refers to Sequences as Scenes in this instance
                        single_sequence_iteration = StoryElementStatusToBlackboard(
                            single_sequence_iteration, "SCENE", sequence.attrib.get('name'))
                        single_sequence_iteration = repeatable_behavior(
                            single_sequence_iteration, get_xml_path(story, sequence))

                        sequence_behavior.add_child(single_sequence_iteration)

                    if sequence_behavior.children:
                        parallel_sequences.add_child(
                            oneshot_with_check(variable_name=get_xml_path(story, sequence),
                                               behaviour=sequence_behavior))

                if parallel_sequences.children:
                    parallel_sequences = StoryElementStatusToBlackboard(
                        parallel_sequences, "ACT", act.attrib.get('name'))
                    parallel_behavior.add_child(parallel_sequences)

                start_triggers = act.find("StartTrigger")
                if list(start_triggers) is not None:
                    for start_condition in start_triggers:
                        parallel_start_criteria = self._create_condition_container(start_condition,
                                                                                   story,
                                                                                   "StartConditions")
                        if parallel_start_criteria.children:
                            start_conditions.add_child(parallel_start_criteria)
                end_triggers = act.find("StopTrigger")
                if end_triggers is not None and list(end_triggers) is not None:
                    for end_condition in end_triggers:
                        parallel_end_criteria = self._create_condition_container(
                            end_condition, story, "EndConditions", success_on_all=False)
                        if parallel_end_criteria.children:
                            parallel_behavior.add_child(parallel_end_criteria)

                if start_conditions.children:
                    act_sequence.add_child(start_conditions)
                if parallel_behavior.children:
                    act_sequence.add_child(parallel_behavior)

                if act_sequence.children:
                    story_behavior.add_child(act_sequence)

            stories_behavior.add_child(oneshot_with_check(variable_name=get_xml_path(story, story) + '>' +
                                                          story_name,  # See get_xml_path
                                                          behaviour=story_behavior))

        # Build behavior tree
        behavior = py_trees.composites.Parallel(
            policy=py_trees.common.ParallelPolicy.SUCCESS_ON_ALL, name="behavior")

        init_parameters = self._initialize_parameters()
        if init_parameters is not None:
            behavior.add_child(oneshot_with_check(variable_name="InitialParameterSettings", behaviour=init_parameters))

        env_behavior = self._create_environment_behavior()
        if env_behavior is not None:
            behavior.add_child(oneshot_with_check(variable_name="InitialEnvironmentSettings", behaviour=env_behavior))

        init_behavior = self._create_init_behavior()
        if init_behavior is not None:
            behavior.add_child(oneshot_with_check(variable_name="InitialActorSettings", behaviour=init_behavior))

        behavior.add_child(stories_behavior)

        return behavior

    def _create_condition_container(self, node, story, name='Conditions Group', sequence=None,
                                    maneuver=None, success_on_all=True):
        """
        This is a generic function to handle conditions utilising ConditionGroups
        Each ConditionGroup is represented as a Sequence of Conditions
        The ConditionGroups are grouped under a SUCCESS_ON_ONE Parallel
        """

        parallel_condition_groups = py_trees.composites.Parallel(name,
                                                                 policy=py_trees.common.ParallelPolicy.SUCCESS_ON_ONE)

        for condition_group in node.iter("ConditionGroup"):
            if success_on_all:
                condition_group_sequence = py_trees.composites.Parallel(
                    name="Condition Group", policy=py_trees.common.ParallelPolicy.SUCCESS_ON_ALL)
            else:
                condition_group_sequence = py_trees.composites.Parallel(
                    name="Condition Group", policy=py_trees.common.ParallelPolicy.SUCCESS_ON_ONE)
            for condition in condition_group.iter("Condition"):
                criterion = OpenScenarioParser.convert_condition_to_atomic(
                    condition, self.other_actors + self.ego_vehicles)
                if sequence is not None and maneuver is not None:
                    xml_path = get_xml_path(story, sequence) + '>' + \
                        get_xml_path(maneuver, condition)  # See note in get_xml_path
                else:
                    xml_path = get_xml_path(story, condition)
                criterion = oneshot_with_check(variable_name=xml_path, behaviour=criterion)
                condition_group_sequence.add_child(criterion)

            if condition_group_sequence.children:
                parallel_condition_groups.add_child(condition_group_sequence)

        return parallel_condition_groups

    def _create_test_criteria(self):
        """
        A list of all test criteria will be created that is later used
        in parallel behavior tree.
        """
        parallel_criteria = py_trees.composites.Parallel("EndConditions (Criteria Group)",
                                                         policy=py_trees.common.ParallelPolicy.SUCCESS_ON_ONE)

        criteria = []
        for endcondition in self.config.storyboard.iter("StopTrigger"):
            for condition in endcondition.iter("Condition"):
                if condition.attrib.get('name').startswith('criteria_'):
                    criteria.append(condition)

        for condition in criteria:
            criterion = OpenScenarioParser.convert_condition_to_atomic(condition, self.ego_vehicles)
            parallel_criteria.add_child(criterion)

        return parallel_criteria

    def __del__(self):
        """
        Remove all actors upon deletion
        """
        self.remove_all_actors()
