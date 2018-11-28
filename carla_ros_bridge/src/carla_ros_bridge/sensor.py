"""
Classes to handle Carla sensors
"""
import rospy

from carla_ros_bridge.actor import Actor


class Sensor(Actor):

    """
    Actor implementation details for sensors
    """

    @staticmethod
    def create_actor(carla_actor, parent):
        """
        Static factory method to create vehicle actors

        :param carla_actor: carla sensor actor object
        :type carla_actor: carla.Sensor
        :param parent: the parent of the new traffic actor
        :type parent: carla_ros_bridge.Parent
        :return: the created sensor actor
        :rtype: carla_ros_bridge.Sensor or derived type
        """
        if carla_actor.type_id.startswith("sensor.camera"):
            return Camera.create_actor(carla_actor=carla_actor, parent=parent)
        else:
            return Sensor(carla_actor=carla_actor, parent=parent)

    def __init__(self, carla_actor, parent, topic_prefix=None, append_role_name_topic_postfix=True):
        """
        Constructor

        :param carla_actor: carla actor object
        :type carla_actor: carla.Actor
        :param parent: the parent of this
        :type parent: carla_ros_bridge.Parent
        :param topic_prefix: the topic prefix to be used for this actor
        :type topic_prefix: string
        :param append_role_name_topic_postfix: if this flag is set True,
            the role_name of the actor is used as topic postfix
        :type append_role_name_topic_postfix: boolean
        """
        if topic_prefix is None:
            topic_prefix = 'sensor'
        super(Sensor, self).__init__(carla_actor=carla_actor,
                                     parent=parent,
                                     topic_prefix=topic_prefix,
                                     append_role_name_topic_postfix=append_role_name_topic_postfix)

        if self.__class__.__name__ == "Sensor":
            rospy.logwarn("Created Unsupported Sensor(id={}, parent_id={}, type={}, attributes={}".format(
                self.get_id(), self.get_parent_id(), self.carla_actor.type_id, self.carla_actor.attributes))

    def get_frame_id(self):
        """
        Function (override) to get the frame id of the sensor object.

        Sensor frames respect their respective parent relationship
        within the frame name to prevent from name clashes.

        :return: frame id of the sensor object
        :rtype: string
        """
        return self.parent.get_frame_id() + "/" + super(Sensor, self).get_frame_id()


from carla_ros_bridge.camera import Camera
