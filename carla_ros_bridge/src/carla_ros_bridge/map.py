"""
Class to handle the carla map
"""

import rospy

from geometry_msgs.msg import Transform
from carla_ros_bridge.child import Child


class Map(Child):

    """
    Child implementation details for the map
    """

    def __init__(self, carla_world, parent, topic):
        """
        Constructor

        :param carla_world: carla world object
        :type carla_world: carla.World
        :param parent: the parent of this
        :type parent: carla_ros_bridge.Parent
        :param topic_prefix: the topic prefix to be used for this child
        :type topic_prefix: string
        """
        super(Map, self).__init__(
            carla_id=-1, carla_world=carla_world, parent=parent, topic_prefix=topic)
        self.carla_map = self.get_carla_world().get_map()

    def destroy(self):
        """
        Function (override) to destroy this object.

        Remove reference to carla.Map object.
        Finally forward call to super class.

        :return:
        """
        rospy.logdebug("Destroying Map()")
        self.carla_map = None
        super(Map, self).destroy()

    def update(self):
        """
        Function (override) to update this object.

        On update map sends:
        - tf global frame

        :return:
        """
        self.send_tf_msg()
        super(Map).update()

    def get_current_ros_transfrom(self):
        """
        Function (override) to return the ros transform of this.

        The global map frame has an empty transform.

        :return:
        """
        return Transform()

    def send_tf_msg(self):
        """
        Function (override) to send tf messages of the map.

        The camera defines the global fame and has to set frame_id=1

        :return:
        """
        tf_msg = self.get_tf_msg()
        tf_msg.header.frame_id = 1
        self.parent.publish_ros_message('tf', tf_msg)
