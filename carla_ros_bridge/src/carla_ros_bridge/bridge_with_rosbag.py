"""
RosBridge class with rosbag support
"""

import os

import rospy
import rosbag

from carla_ros_bridge.bridge import CarlaRosBridge


class CarlaRosBridgeWithBag(CarlaRosBridge):

    """
    Carla Ros bridge with writing ROS bag
    """

    def __init__(self, carla_world, params):
        """
        Constructor

        :param carla_world: carla world object
        :type carla_world: carla.World
        :param params: dict of parameters, see settings.yaml
        :type params: dict
        """
        super(CarlaRosBridgeWithBag, self).__init__(
            carla_world=carla_world, params=params)

        prefix, ext = os.path.splitext(self.get_param('rosbag_fname'))
        rosbag_fname = os.path.abspath(prefix + self.get_param('curr_episode'))
        self.bag = rosbag.Bag(rosbag_fname, mode='w')

    def destroy(self):
        """
        Function (virtual) to destroy this object.

        Close the ROS bag file
        Finally forward call to super class.

        :return:
        """
        rospy.loginfo("Closing the bag file")
        self.bag.close()
        super(CarlaRosBridgeWithBag, self).destroy()

    def send_msgs(self):
        """
        Function (overridden) to actually write the collected ROS messages out
        into the RosBag before sending them via the publisher.

        :return:
        """
        for publisher, msg in self.msgs_to_publish:
            self.bag.write(publisher.name, msg, self.get_current_ros_time())

        super(CarlaRosBridgeWithBag, self).send_msgs()
