#!/usr/bin/env python
"""
Ros Bridge node for carla simulator
"""

import rospy

from carla.client import make_carla_client
from carla_ros_bridge.bridge import CarlaRosBridge
from carla_ros_bridge.bridge_with_rosbag import CarlaRosBridgeWithBag


def main():
    rospy.init_node("carla_client", anonymous=True)

    params = rospy.get_param('carla')
    host = params['host']
    port = params['port']

    rospy.loginfo("Trying to connect to {host}:{port}".format(
        host=host, port=port))

    with make_carla_client(host, port) as client:
        rospy.loginfo("Connected")

        bridge_cls = CarlaRosBridgeWithBag if rospy.get_param(
            'rosbag_fname', '') else CarlaRosBridge
        with bridge_cls(client=client, params=params) as carla_ros_bridge:
            rospy.on_shutdown(carla_ros_bridge.on_shutdown)
            carla_ros_bridge.run()


if __name__ == "__main__":
    main()
