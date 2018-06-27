"""
Class to handle occupancy grid
"""

import numpy as np
import rospy
import tf
from nav_msgs.msg import OccupancyGrid

from carla.planner.map import CarlaMap


class MapHandler(object):
    """
    Convert CarlaMap lane image to ROS OccupancyGrid message
    """

    def __init__(self, map_name, topic='/map', res=0.1643):
        self.map_name = map_name
        self.carla_map = CarlaMap(map_name, res, 50)
        self.map_pub = rospy.Publisher(
            topic, OccupancyGrid, queue_size=10, latch=True)
        self.build_map_message()

    def build_map_message(self):
        self.map_msg = map_msg = OccupancyGrid()

        # form array for map
        map_img = self.carla_map.get_map_lanes()
        # extract green channel, invert, scale to range 0..100, convert to int8
        map_img = (100 - map_img[..., 1] * 100.0 / 255).astype(np.int8)
        map_msg.data = map_img.ravel().tolist()

        # set up general info
        map_msg.info.resolution = self.carla_map._pixel_density
        map_msg.info.width = map_img.shape[1]
        map_msg.info.height = map_img.shape[0]

        # set up origin orientation
        quat = tf.transformations.quaternion_from_euler(0, 0, np.pi)
        map_msg.info.origin.orientation.x = quat[0]
        map_msg.info.origin.orientation.y = quat[1]
        map_msg.info.origin.orientation.z = quat[2]
        map_msg.info.origin.orientation.w = quat[3]

        # set up origin position
        top_right_corner = float(map_img.shape[1]), 0.0
        to_world = self.carla_map.convert_to_world(top_right_corner)
        map_msg.info.origin.position.x = to_world[0]
        map_msg.info.origin.position.y = -to_world[1]

        # FIXME: remove hardcoded values from convert_to_world
        map_msg.info.origin.position.z = -self.carla_map._converter._worldoffset[2]

        # FIXME: height for Town01 is still in centimeters
        if self.map_name == 'Town01':
            map_msg.info.origin.position.z *= 100

    def send_map(self):
        self.map_pub.publish(self.map_msg)
