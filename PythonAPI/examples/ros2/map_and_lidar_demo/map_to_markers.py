#!/usr/bin/env python3

# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# Converts the latched /carla/map OpenDRIVE string into lane markers for RViz.
#
# Subscribes to /carla/map (std_msgs/String, transient_local), parses the
# OpenDRIVE with the carla Python package (no simulator connection needed) and
# publishes a latched visualization_msgs/MarkerArray on /carla/map_markers:
#   * lane_boundaries   the lane edges (centerline +/- half the lane width
#                       given by the OpenDRIVE), one continuous polyline per
#                       lane edge
#   * lane_centerlines  the waypoint guide lines (hidden by default in the
#                       bundled RViz preset; enable it under the Map display
#                       Namespaces)
# Markers are re-published whenever a new map sample arrives (e.g. after
# load_world), replacing the previous ones.

import argparse

import carla
import rclpy
from rclpy.executors import ExternalShutdownException
from rclpy.node import Node
from rclpy.qos import QoSProfile, QoSDurabilityPolicy, QoSReliabilityPolicy
from std_msgs.msg import String
from visualization_msgs.msg import Marker, MarkerArray
from geometry_msgs.msg import Point

LATCHED_QOS = QoSProfile(
    depth=1,
    reliability=QoSReliabilityPolicy.RELIABLE,
    durability=QoSDurabilityPolicy.TRANSIENT_LOCAL)

# Safety cap on the waypoints walked per lane, in case a malformed map makes
# next() loop forever. No real town comes close: at the default 2.0 m sampling
# this allows lanes up to 200 km.
MAX_CHAIN_WAYPOINTS = 100000


def _make_line_marker(ns, red, green, blue):
    marker = Marker()
    marker.header.frame_id = 'map'
    marker.ns = ns
    marker.type = Marker.LINE_STRIP
    marker.action = Marker.ADD
    marker.scale.x = 0.2
    marker.color.r = red
    marker.color.g = green
    marker.color.b = blue
    marker.color.a = 1.0
    marker.frame_locked = True
    return marker


def _lane_edge(waypoint, side):
    """Returns the (x, y, z) of the lane edge on the given side (-1 left, +1 right)."""
    location = waypoint.transform.location
    right = waypoint.transform.get_right_vector()
    offset = side * 0.5 * waypoint.lane_width
    return (location.x + right.x * offset,
            location.y + right.y * offset,
            location.z + right.z * offset)


def _lane_chains(carla_map, distance, logger):
    """Yields each lane of the map as one list of consecutive waypoints.

    Walks every lane returned by get_topology() in driving direction at the
    given sampling distance, then appends the first waypoint of the successor
    road so consecutive roads connect without a gap. Walking each lane as a
    single chain (instead of pairing independently sampled waypoints with
    next()) keeps the polyline continuous across lane sections and road seams.
    """
    for start, _ in carla_map.get_topology():
        chain = [start]
        following = start.next(distance)
        while following and following[0].road_id == start.road_id and len(chain) < MAX_CHAIN_WAYPOINTS:
            chain.append(following[0])
            following = chain[-1].next(distance)
        if len(chain) >= MAX_CHAIN_WAYPOINTS:
            logger.warning(
                'Lane chain on road {} truncated at {} waypoints, '
                'its polyline may be incomplete'.format(start.road_id, MAX_CHAIN_WAYPOINTS))
        if following:
            chain.append(following[0])
        yield chain


class MapToMarkers(Node):

    def __init__(self, waypoint_distance):
        super().__init__('carla_map_markers')
        self._waypoint_distance = waypoint_distance
        self._marker_publisher = self.create_publisher(MarkerArray, '/carla/map_markers', LATCHED_QOS)
        self._map_subscription = self.create_subscription(String, '/carla/map', self._on_map, LATCHED_QOS)
        self.get_logger().info('Waiting for latched /carla/map...')

    def _on_map(self, map_msg):
        self.get_logger().info('Received OpenDRIVE ({} bytes), parsing...'.format(len(map_msg.data)))
        carla_map = carla.Map('map_to_markers', map_msg.data)

        # Drop the markers of a previously received map before adding the new ones.
        clear_previous = Marker()
        clear_previous.action = Marker.DELETEALL
        markers = [clear_previous]

        def add_strip(ns, color, points):
            if len(points) < 2:
                return
            marker = _make_line_marker(ns, *color)
            marker.id = len(markers)
            for x, y, z in points:
                # CARLA left-handed (x, y, z) to ROS right-handed: negate y.
                marker.points.append(Point(x=x, y=-y, z=z + 0.1))
            markers.append(marker)

        for chain in _lane_chains(carla_map, self._waypoint_distance, self.get_logger()):
            add_strip('lane_centerlines', (0.2, 0.8, 1.0),
                      [(w.transform.location.x, w.transform.location.y, w.transform.location.z)
                       for w in chain])
            for side in (-1.0, 1.0):
                add_strip('lane_boundaries', (0.9, 0.9, 0.9),
                          [_lane_edge(w, side) for w in chain])

        self._marker_publisher.publish(MarkerArray(markers=markers))
        boundary_count = sum(1 for m in markers if m.ns == 'lane_boundaries')
        centerline_count = sum(1 for m in markers if m.ns == 'lane_centerlines')
        self.get_logger().info(
            'Published {} lane boundary and {} centerline polylines on /carla/map_markers'.format(
                boundary_count, centerline_count))


def main(args):
    rclpy.init()
    node = MapToMarkers(args.waypoint_distance)
    try:
        rclpy.spin(node)
    except (KeyboardInterrupt, ExternalShutdownException):
        print('\nCancelled by user. Bye!')
    finally:
        node.destroy_node()
        rclpy.try_shutdown()


if __name__ == '__main__':
    argparser = argparse.ArgumentParser(description='CARLA latched OpenDRIVE map to RViz lane markers')
    argparser.add_argument('--waypoint-distance', metavar='D', default=2.0, type=float, help='distance in meters between sampled lane points (default: 2.0)')

    main(argparser.parse_args())
