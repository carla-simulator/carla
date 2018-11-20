"""
Classes to handle Carla traffic objects
"""

import rospy
import carla 

from carla_ros_bridge.actor import Actor

class Traffic(Actor):
    """
    Generic Actor Implementation for traffic objects
    """
    def __init__(self, carla_actor, actor_parent, topic_prefix):
        super(Traffic, self).__init__(carla_actor = carla_actor, actor_parent = actor_parent, topic_prefix = topic_prefix)


class TrafficLight(Traffic):
    """
    Generic Actor Implementation for traffic lights
    """
    def __init__(self, carla_actor, actor_parent, topic_prefix):
        super(TrafficLight, self).__init__(carla_actor = carla_actor, actor_parent = actor_parent, topic_prefix = topic_prefix)
