#!/usr/bin/env python
"""
Entry point for carla simulator ROS bridge
"""

import rospy

import carla

from carla_ros_bridge.bridge import CarlaRosBridge
from carla_ros_bridge.bridge_with_rosbag import CarlaRosBridgeWithBag


def main():
    """
    main function for carla simulator ROS bridge maintaiing the communication client and the CarlaRosBridge objects
    """
    rospy.init_node("carla_client", anonymous=True)

    params = rospy.get_param('carla')
    host = params['host']
    port = params['port']

    rospy.loginfo("Trying to connect to {host}:{port}".format(
        host=host, port=port))

    try:
        carla_client = carla.Client(host=host, port=port)
        carla_client.set_timeout(2000)

        carla_world = carla_client.get_world()

        rospy.loginfo("Connected")

        bridge_cls = CarlaRosBridgeWithBag if rospy.get_param(
            'rosbag_fname', '') else CarlaRosBridge
        carla_ros_bridge = bridge_cls(
            carla_world=carla_client.get_world(), params=params)
        carla_ros_bridge.run()
        carla_ros_bridge = None

        rospy.logdebug("Delete world and client")
        del carla_world
        del carla_client

    finally:
        rospy.loginfo("Done")

if __name__ == "__main__":
    main()
