#!/usr/bin/env python
"""
Ros Bridge node for carla simulator
"""

import rospy
import time

import carla 

from carla_ros_bridge.bridge import CarlaRosBridge
from carla_ros_bridge.bridge_with_rosbag import CarlaRosBridgeWithBag

def main():
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

        bridge_cls = CarlaRosBridgeWithBag if rospy.get_param('rosbag_fname', '') else CarlaRosBridge
        with bridge_cls(world=carla_client.get_world(), params=params) as carla_ros_bridge:
            rospy.on_shutdown(carla_ros_bridge.on_shutdown)
            carla_ros_bridge.run()
            del carla_ros_bridge

        rospy.logdebug("Delete world and client")
        del carla_world
        del carla_client
    
    finally:
        rospy.loginfo("Done")

if __name__ == "__main__":
    main()
