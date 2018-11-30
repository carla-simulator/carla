"""
Base Classes to handle child objects
"""

from abc import abstractmethod

import rospy

from geometry_msgs.msg import TransformStamped

from carla_ros_bridge.parent import Parent


class Child(Parent):

    """
    Generic base class for all child entities
    """

    def __init__(self, carla_id, carla_world, parent, topic_prefix=''):
        """
        Constructor

        :param carla_id: unique carla_id of this child object
            carla_id > 0: carla actor ids (see also carla.Actor)
            carla_id == 0: resevered for the (root) bridge object; not allowed in here
            carla_id == -1: used by the map object
        :type carla_id: int64
        :param carla_world: carla world object
        :type carla_world: carla.World
        :param parent: the parent of this
        :type parent: carla_ros_bridge.Parent
        :param topic_prefix: the topic prefix to be used for this child
        :type topic_prefix: string
        """
        if carla_id == 0:
            raise ValueError(
                "A child can never have an carla_id of zero. Reserved for the parent root (the bridge object)")
        self.topic_prefix = topic_prefix.replace(".", "/").replace("-", "_")
        # each Child defines its own frame
        super(Child, self).__init__(
            carla_id=carla_id, carla_world=carla_world, frame_id=self.topic_prefix)
        self.parent = parent
        rospy.logdebug("Created {}-Child(id={}, parent_id={}, topic_name={})".format(
            self.__class__.__name__, self.get_id(), self.get_parent_id(), self.topic_name()))

    def destroy(self):
        """
        Function (override) to destroy this object.

        Remove the reference to the carla_ros_bridge.Parent object.
        Finally forward call to super class.

        :return:
        """
        rospy.logdebug(
            "Destroying {}-Child(id={})".format(self.__class__.__name__, self.get_id()))
        self.parent = None
        super(Child, self).destroy()

    def get_current_ros_time(self):
        """
        Function (override) to query the current ROS time.

        Just forwards the request to the parent.

        :return: The latest received ROS time of the bridge
        :rtype: rospy.Time
        """
        return self.parent.get_current_ros_time()

    def publish_ros_message(self, topic, msg):
        """
        Function (override) to publish ROS messages.

        Just forwards the request to the parent.

        :param topic: ROS topic to publish the message on
        :type topic: string
        :param msg: the ROS message to be published
        :type msg: a valid ROS message type
        :return:
        """
        self.parent.publish_ros_message(topic, msg)

    def get_param(self, key):
        """
        Function (override) to query global parameters passed from the outside.

        Just forwards the request to the parent.

        :param key: the key of the parameter
        :type key: string
        :return: the parameter string
        :rtype: string
        """
        return self.parent.get_param(key)

    def topic_name(self):
        """
        Function (override) to get the topic name of the current entity.

        Concatenate the child's onw topic prefix to the the parent topic name if not empty.

        :return: the final topic name of this
        :rtype: string
        """
        if len(self.topic_prefix) > 0:
            return self.parent.topic_name() + "/" + self.topic_prefix
        else:
            return self.parent.topic_name()

    def get_parent_id(self):
        """
        Getter for the carla_id of the parent.

        :return: unique carla_id of the parent of this child
        :rtype: int64
        """
        return self.parent.get_id()

    def get_msg_header(self, use_parent_frame=True):
        """
        Helper function to create a ROS message Header

        :param use_parent_frame: per default the header.frame_id is set
          to the frame of the Child's parent. If this is set to False,
          the Child's own frame is used as basis.
        :type use_parent_frame: boolean
        :return: prefilled Header object
        """
        header = super(Child, self).get_msg_header()
        if use_parent_frame:
            header.frame_id = self.parent.get_frame_id()
        return header

    def send_tf_msg(self):
        """
        Helper function to send a ROS tf message of this child

        Mainly calls get_tf_msg() and publish_ros_message().

        :return:
        """
        tf_msg = self.get_tf_msg()
        self.publish_ros_message('tf', tf_msg)

    def get_tf_msg(self):
        """
        Helper function to create a ROS tf message of this child

        :return: the filled tf message
        :rtype: geometry_msgs.msg.TransformStamped
        """
        tf_msg = TransformStamped()
        tf_msg.header = self.get_msg_header()
        tf_msg.child_frame_id = self.get_frame_id()
        tf_msg.transform = self.get_current_ros_transfrom()
        return tf_msg

    @abstractmethod
    def get_current_ros_transfrom(self):
        """
        Pure virtual function to query the current ROS transform

        :return: the ROS transform of this entity
        :rtype: geometry_msgs.msg.Transform
        """
        raise NotImplementedError(
            "This function has to be implemented by the derived classes")
