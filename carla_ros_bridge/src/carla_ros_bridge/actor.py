"""
Base Classes to handle Actor objects
"""

import rospy

from visualization_msgs.msg import Marker
from std_msgs.msg import ColorRGBA

from carla_ros_bridge.child import Child
from carla_ros_bridge.actor_id_registry import ActorIdRegistry
from carla_ros_bridge.transforms import carla_transform_to_ros_transform, carla_transform_to_ros_pose, carla_velocity_to_ros_twist, carla_acceleration_to_ros_accel


class Actor(Child):

    """
    Generic base class for all carla actors
    """

    global_id_registry = ActorIdRegistry()

    def __init__(self, carla_actor, parent, topic_prefix='', append_role_name_topic_postfix=True):
        """
        Constructor

        :param carla_actor: carla vehicle actor object
        :type carla_actor: carla.Vehicle
        :param parent: the parent of this
        :type parent: carla_ros_bridge.Parent
        :param topic_prefix: the topic prefix to be used for this actor
        :type topic_prefix: string
        :param append_role_name_topic_postfix: if this flag is set True,
            the role_name of the actor is used as topic postfix
        :type append_role_name_topic_postfix: boolean
        """
        # each actor defines its own frame
        if append_role_name_topic_postfix:
            if carla_actor.attributes.has_key('role_name'):
                topic_prefix += '/' + carla_actor.attributes['role_name']
            else:
                topic_prefix += '/' + \
                    str(Actor.global_id_registry.get_id(carla_actor.id))
        super(Actor, self).__init__(
            carla_id=carla_actor.id, carla_world=carla_actor.get_world(), parent=parent, topic_prefix=topic_prefix)
        self.carla_actor = carla_actor
        rospy.logdebug("Created Actor-{}(id={}, parent_id={}, type={}, topic_name={}, attributes={}".format(
            self.__class__.__name__, self.get_id(), self.get_parent_id(),
            self.carla_actor.type_id, self.topic_name(), self.carla_actor.attributes))

        if self.__class__.__name__ == "Actor":
            rospy.logwarn("Created Unsupported Actor(id={}, parent_id={}, type={}, attributes={}".format(
                self.get_id(), self.get_parent_id(), self.carla_actor.type_id, self.carla_actor.attributes))

    def destroy(self):
        """
        Function (override) to destroy this object.

        Remove the reference to the carla.Actor object.
        Finally forward call to super class.

        :return:
        """
        rospy.logdebug(
            "Destroying {}-Actor(id={})".format(self.__class__.__name__, self.get_id()))
        self.carla_actor = None
        super(Actor, self).destroy()

    def get_marker_color(self):
        """
        Virtual (non-abstract) function to get the ROS std_msgs.msg.ColorRGBA
        used for rviz objects of this actor

        Reimplement this in the derived actor class if ROS visualization messages
        (e.g. visualization_msgs.msg.Marker) are sent out and you want a different color than blue.

        :return: blue color object
        :rtype: std_msgs.msg.ColorRGBA
        """
        color = ColorRGBA()
        color.r = 0
        color.g = 0
        color.b = 255
        return color

    def get_marker(self, use_parent_frame=True):
        """
        Helper function to create a ROS visualization_msgs.msg.Marker for the actor

        :param use_parent_frame: per default (True) the header.frame_id
            is set to the frame of the actor's parent.
            If this is set to False, the actor's own frame is used as basis.
        :type use_parent_frame:  boolean
        :return:
        visualization_msgs.msg.Marker
        """
        marker = Marker(
            header=self.get_msg_header(use_parent_frame=use_parent_frame))
        marker.color = self.get_marker_color()
        marker.color.a = 0.3
        marker.id = self.get_global_id()
        marker.text = "id = {}".format(marker.id)
        return marker

    def get_current_ros_transfrom(self):
        """
        Function (override) to provide the current ROS transform

        :return: the ROS transform of this actor
        :rtype: geometry_msgs.msg.Transform
        """
        return carla_transform_to_ros_transform(
            self.carla_actor.get_transform())

    def get_current_ros_pose(self):
        """
        Function to provide the current ROS pose

        :return: the ROS pose of this actor
        :rtype: geometry_msgs.msg.Pose
        """
        return carla_transform_to_ros_pose(
            self.carla_actor.get_transform())

    def get_current_ros_twist(self):
        """
        Function to provide the current ROS twist

        :return: the ROS twist of this actor
        :rtype: geometry_msgs.msg.Twist
        """
        return carla_velocity_to_ros_twist(
            self.carla_actor.get_velocity())

    def get_current_ros_accel(self):
        """
        Function to provide the current ROS acceleration

        :return: the ROS acceleration of this actor
        :rtype: geometry_msgs.msg.Accel
        """
        return carla_acceleration_to_ros_accel(
            self.carla_actor.get_acceleration())

    def get_global_id(self):
        """
        Return a unique global id for the actor used for markers, object ids, etc.

        ros marker id should be int32, carla/unrealengine seems to use int64
        A lookup table is used to remap actor_id to small number between 0 and max_int32

        :return: mapped id of this actor (unique increasing counter value)
        :rtype: uint32
        """
        return Actor.global_id_registry.get_id(self.carla_actor.id)
