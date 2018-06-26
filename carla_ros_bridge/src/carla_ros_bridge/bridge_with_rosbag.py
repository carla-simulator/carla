"""
RosBridge class with rosbag support
"""

import time

from tf2_msgs.msg import TFMessage
import rosbag
import rospy
import os
from datetime import datetime

from carla_ros_bridge.bridge import CarlaRosBridge


class CarlaRosBridgeWithBag(CarlaRosBridge):
    def __init__(self, *args, **kwargs):
        super(CarlaRosBridgeWithBag, self).__init__(*args, **kwargs)

        fname = '{:%m%d_%H%M}'.format(datetime.now())
        rosbag_fname = os.path.join(rospy.get_param('rosbag_fname'), 
                                    'recording_episode'+fname)
        self.bag = rosbag.Bag(rosbag_fname, mode='w')

    def send_msgs(self):
        for publisher, msg in self.msgs_to_publish:
            self.bag.write(publisher.name, msg, self.cur_time)

        tf_msg = TFMessage(self.tf_to_publish)
        self.bag.write('tf', tf_msg, self.cur_time)

        self.bag.write('steer', self.steer, self.cur_time)
        self.bag.write('brake', self.brake, self.cur_time)
        self.bag.write('gas', self.throttle, self.cur_time)

        super(CarlaRosBridgeWithBag, self).send_msgs()

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        rospy.loginfo("Closing the bag file")
        self.bag.close()
        super(CarlaRosBridgeWithBag, self).__exit__(exc_type, exc_value,
                                                    traceback)
