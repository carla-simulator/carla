#!/usr/bin/env python

# Copyright (c) 2015 Daniel Stonier
# Copyright (c) 2020 Intel Corporation
#
# License: BSD
#   https://raw.githubusercontent.com/splintered-reality/py_trees/devel/LICENSE

"""
This module provides a backport from newer py_trees releases (> 1.0)
To use certain features also within ScenarioRunner, which uses py_trees
version 0.8.x
"""

import py_trees


class Decorator(py_trees.behaviour.Behaviour):

    """
    A decorator is responsible for handling the lifecycle of a single
    child beneath

    This is taken from py_trees 1.2 to work with our current implementation
    that uses py_trees 0.8.2
    """

    def __init__(self, child, name):
        """
        Common initialisation steps for a decorator - type checks and
        name construction (if None is given).
        Args:
            name (:obj:`str`): the decorator name
            child (:class:`~py_trees.behaviour.Behaviour`): the child to be decorated
        Raises:
            TypeError: if the child is not an instance of :class:`~py_trees.behaviour.Behaviour`
        """
        # Checks
        if not isinstance(child, py_trees.behaviour.Behaviour):
            raise TypeError("A decorator's child must be an instance of py_trees.behaviours.Behaviour")
        # Initialise
        super(Decorator, self).__init__(name=name)
        self.children.append(child)
        # Give a convenient alias
        self.decorated = self.children[0]
        self.decorated.parent = self

    def tick(self):
        """
        A decorator's tick is exactly the same as a normal proceedings for
        a Behaviour's tick except that it also ticks the decorated child node.
        Yields:
            :class:`~py_trees.behaviour.Behaviour`: a reference to itself or one of its children
        """
        self.logger.debug("%s.tick()" % self.__class__.__name__)
        # initialise just like other behaviours/composites
        if self.status != py_trees.common.Status.RUNNING:
            self.initialise()
        # interrupt proceedings and process the child node
        # (including any children it may have as well)
        for node in self.decorated.tick():
            yield node
        # resume normal proceedings for a Behaviour's tick
        new_status = self.update()
        if new_status not in list(py_trees.common.Status):
            self.logger.error(
                "A behaviour returned an invalid status, setting to INVALID [%s][%s]" % (new_status, self.name))
            new_status = py_trees.common.Status.INVALID
        if new_status != py_trees.common.Status.RUNNING:
            self.stop(new_status)
        self.status = new_status
        yield self

    def stop(self, new_status=py_trees.common.Status.INVALID):
        """
        As with other composites, it checks if the child is running
        and stops it if that is the case.
        Args:
            new_status (:class:`~py_trees.common.Status`): the behaviour is transitioning to this new status
        """
        self.logger.debug("%s.stop(%s)" % (self.__class__.__name__, new_status))
        self.terminate(new_status)
        # priority interrupt handling
        if new_status == py_trees.common.Status.INVALID:
            self.decorated.stop(new_status)
        # if the decorator returns SUCCESS/FAILURE and should stop the child
        if self.decorated.status == py_trees.common.Status.RUNNING:
            self.decorated.stop(py_trees.common.Status.INVALID)
        self.status = new_status

    def tip(self):
        """
        Get the *tip* of this behaviour's subtree (if it has one) after it's last
        tick. This corresponds to the the deepest node that was running before the
        subtree traversal reversed direction and headed back to this node.
        """
        if self.decorated.status != py_trees.common.Status.INVALID:
            return self.decorated.tip()

        return super(Decorator, self).tip()


def oneshot_behavior(variable_name, behaviour, name=None):
    """
    This is taken from py_trees.idiom.oneshot.
    """
    if not name:
        name = behaviour.name

    subtree_root = py_trees.composites.Selector(name=name)

    # Initialize the variables
    blackboard = py_trees.blackboard.Blackboard()
    _ = blackboard.set(variable_name, False)

    # Wait until the scenario has ended
    check_flag = py_trees.blackboard.CheckBlackboardVariable(
        name=variable_name + " Done?",
        variable_name=variable_name,
        expected_value=True,
        clearing_policy=py_trees.common.ClearingPolicy.ON_INITIALISE
    )
    set_flag = py_trees.blackboard.SetBlackboardVariable(
        name="Mark Done",
        variable_name=variable_name,
        variable_value=True
    )
    # If it's a sequence, don't double-nest it in a redundant manner
    if isinstance(behaviour, py_trees.composites.Sequence):
        behaviour.add_child(set_flag)
        sequence = behaviour
    else:
        sequence = py_trees.composites.Sequence(name="OneShot")
        sequence.add_children([behaviour, set_flag])

    subtree_root.add_children([check_flag, sequence])
    return subtree_root
