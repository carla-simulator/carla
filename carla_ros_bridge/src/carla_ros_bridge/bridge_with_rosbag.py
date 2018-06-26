"""
RosBridge class with rosbag support
"""

import time

from tf2_msgs.msg import TFMessage
import rosbag
import rospy
import os

from carla_ros_bridge.bridge import CarlaRosBridge


class CarlaRosBridgeWithBag(CarlaRosBridge):
    def __init__(self, *args, **kwargs):
        super(CarlaRosBridgeWithBag, self).__init__(*args, **kwargs)
        rosbag_fname = rospy.get_param('rosbag_fname')
        self.bag = rosbag.Bag(rosbag_fname, mode='w')

    def send_msgs(self):
        for publisher, msg in self.msgs_to_publish:
            self.bag.write(publisher.name, msg, self.cur_time)

        tf_msg = TFMessage(self.tf_to_publish)
        self.bag.write('tf', tf_msg, self.cur_time)
        super(CarlaRosBridgeWithBag, self).send_msgs()

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        rospy.loginfo("Closing the bag file")
        self.bag.close()
        super(CarlaRosBridgeWithBag, self).__exit__(exc_type, exc_value,
                                                    traceback)
