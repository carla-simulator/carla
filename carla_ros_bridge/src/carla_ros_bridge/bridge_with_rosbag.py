"""
RosBridge class with rosbag support
"""

import time

from tf2_msgs.msg import TFMessage
import rosbag
import rospy

from carla_ros_bridge.bridge import CarlaRosBridge


class CarlaRosBridgeWithBag(CarlaRosBridge):
    def __init__(self, *args, **kwargs):
        super(CarlaRosBridgeWithBag, self).__init__(*args, **kwargs)
        timestr = time.strftime("%Y%m%d-%H%M%S")
        self.bag = rosbag.Bag('/tmp/output_{}.bag'.format(timestr), mode='w')

    def send_msgs(self):
        for name, msg in self.message_to_publish:
            self.bag.write(name, msg, self.cur_time)

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
