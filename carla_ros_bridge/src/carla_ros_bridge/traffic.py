"""
Classes to handle Carla traffic objects
"""

import rospy

from carla_ros_bridge.actor import Actor


class Traffic(Actor):

    """
    Actor implementation details for traffic objects
    """

    @staticmethod
    def create_actor(carla_actor, parent):
        """
        Static factory method to create traffic actors

        :param carla_actor: carla actor object
        :type carla_actor: carla.Actor
        :param parent: the parent of the new traffic actor
        :type parent: carla_ros_bridge.Parent
        :return: the created traffic actor
        :rtype: carla_ros_bridge.Traffic or derived type
        """
        if carla_actor.type_id == "traffic.traffic_light":
            return TrafficLight(carla_actor=carla_actor, parent=parent)
        else:
            return Traffic(carla_actor=carla_actor, parent=parent)

    def __init__(self, carla_actor, parent, topic_prefix=None, append_role_name_topic_postfix=True):
        """
        Constructor

        :param carla_actor: carla actor object
        :type carla_actor: carla.Actor
        :param parent: the parent of this
        :type parent: carla_ros_bridge.Parent
        :param topic_prefix: the topic prefix to be used for this actor
        :type topic_prefix: string
        """
        if topic_prefix is None:
            topic_prefix = 'traffic'
        super(Traffic, self).__init__(carla_actor=carla_actor,
                                      parent=parent,
                                      topic_prefix=topic_prefix,
                                      append_role_name_topic_postfix=append_role_name_topic_postfix)
        if self.__class__.__name__ == "Traffic":
            rospy.logwarn("Created Unsupported Traffic Actor"
                          "(id={}, parent_id={}, type={}, attributes={})".format(
                              self.get_id(), self.get_parent_id(),
                              self.carla_actor.type_id, self.carla_actor.attributes))


class TrafficLight(Traffic):

    """
    Traffic implementation details for traffic lights
    """

    def __init__(self, carla_actor, parent):
        """
        Constructor

        :param carla_actor: carla actor object
        :type carla_actor: carla.TrafficLight
        :param parent: the parent of this
        :type parent: carla_ros_bridge.Parent
        """
        topic_prefix = 'traffic.traffic_light'
        super(TrafficLight, self).__init__(carla_actor=carla_actor,
                                           parent=parent, topic_prefix=topic_prefix)
        rospy.logwarn("Created Traffic-Light Actor(id={}, parent_id={}, type={}, attributes={}). "
                      "Not yet fully implemented!".format(
                          self.get_id(), self.get_parent_id(),
                          self.carla_actor.type_id, self.carla_actor.attributes))
