#!/usr/bin/env python3

# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# Broadcasts the map-><vehicle> TF for a CARLA vehicle.
#
# The native ROS2 interface publishes the sensor TFs (<vehicle>-><sensor>) but
# no transform for the vehicle itself. This helper reads the vehicle transform
# through the CARLA Python API every simulation tick and broadcasts it as a
# ROS 2 TF map-><frame>, completing the chain so RViz can combine the latched
# map markers with live sensor data under the 'map' fixed frame. Stamps use
# simulation time to line up with the native sensor headers.

import argparse
import math
import time

import carla
import rclpy
from rclpy.node import Node
from geometry_msgs.msg import TransformStamped
from tf2_ros import TransformBroadcaster


def _quaternion_from_rpy(roll, pitch, yaw):
    cr, sr = math.cos(roll * 0.5), math.sin(roll * 0.5)
    cp, sp = math.cos(pitch * 0.5), math.sin(pitch * 0.5)
    cy, sy = math.cos(yaw * 0.5), math.sin(yaw * 0.5)
    return (
        sr * cp * cy - cr * sp * sy,
        cr * sp * cy + sr * cp * sy,
        cr * cp * sy - sr * sp * cy,
        cr * cp * cy + sr * sp * sy)


def _find_vehicle(world, role_name, node):
    while rclpy.ok():
        for actor in world.get_actors().filter('vehicle.*'):
            if actor.attributes.get('role_name') == role_name:
                return actor
        node.get_logger().info("Waiting for a vehicle with role_name '{}'...".format(role_name))
        time.sleep(1.0)
    return None


def main(args):
    rclpy.init()
    node = Node('carla_ego_tf_broadcaster')
    broadcaster = TransformBroadcaster(node)

    client = carla.Client(args.host, args.port)
    client.set_timeout(10.0)
    world = client.get_world()

    vehicle = _find_vehicle(world, args.role_name, node)
    if vehicle is None:
        return
    node.get_logger().info('Broadcasting TF map->{} for actor {}'.format(args.frame, vehicle.id))

    try:
        while rclpy.ok():
            # Short timeout: signals cannot be delivered while blocked inside
            # the call, so a long wait would hang shutdown when the ticking
            # client stops before this helper.
            try:
                snapshot = world.wait_for_tick(seconds=1.0)
            except RuntimeError:
                continue
            # A signal received while blocked in wait_for_tick may have shut
            # down the ROS context; publishing on it would raise.
            if not rclpy.ok():
                break
            transform = vehicle.get_transform()

            tf_msg = TransformStamped()
            elapsed = snapshot.timestamp.elapsed_seconds
            tf_msg.header.stamp.sec = int(elapsed)
            tf_msg.header.stamp.nanosec = int((elapsed - int(elapsed)) * 1e9)
            tf_msg.header.frame_id = 'map'
            tf_msg.child_frame_id = args.frame
            # CARLA left-handed (x, y, z) to ROS right-handed: negate y;
            # rotation as roll, -pitch, -yaw in radians.
            tf_msg.transform.translation.x = transform.location.x
            tf_msg.transform.translation.y = -transform.location.y
            tf_msg.transform.translation.z = transform.location.z
            qx, qy, qz, qw = _quaternion_from_rpy(
                math.radians(transform.rotation.roll),
                -math.radians(transform.rotation.pitch),
                -math.radians(transform.rotation.yaw))
            tf_msg.transform.rotation.x = qx
            tf_msg.transform.rotation.y = qy
            tf_msg.transform.rotation.z = qz
            tf_msg.transform.rotation.w = qw
            broadcaster.sendTransform(tf_msg)
    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')
    finally:
        node.destroy_node()
        rclpy.try_shutdown()


if __name__ == '__main__':
    argparser = argparse.ArgumentParser(description='CARLA map->vehicle TF broadcaster')
    argparser.add_argument('--host', metavar='H', default='localhost', help='IP of the host CARLA Simulator (default: localhost)')
    argparser.add_argument('--port', metavar='P', default=2000, type=int, help='TCP port of CARLA Simulator (default: 2000)')
    argparser.add_argument('--role-name', metavar='NAME', default='hero', help='role_name of the vehicle to track (default: hero)')
    argparser.add_argument('--frame', metavar='FRAME', default='hero', help='TF child frame, must match the vehicle ros_name (default: hero)')

    main(argparser.parse_args())
