#!/usr/bin/python
from __future__ import print_function
import numpy as np
import csv

import rospy
from visualization_msgs.msg import Marker
from geometry_msgs.msg import Point
from rospy.numpy_msg import numpy_msg
from rospy_tutorials.msg import Floats


def dist2(x1, y1, x2, y2):
    return (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2)


def sparse_trajectory(xs, ys, yaws, speeds, distance):
    assert len(xs) == len(ys) == len(yaws) == len(speeds)
    length = len(xs)
    distance2 = distance*distance
    sparse = [(xs[0], ys[0], yaws[0], speeds[0])]
    for i in range(length):
        x, y, yaw, speed = xs[i], ys[i], yaws[i], speeds[i]
        if dist2(x, y, sparse[-1][0], sparse[-1][1]) >= distance2:
           sparse.append((x, y, yaw, speed))
    return sparse
# PREVIOUS VERSION:
# def sparse_trajectory(data_zip, distance=1):
#    distance2 = distance*distance
#    sparse = []
#    for x, y, yaw, speed in data_zip:
#        if len(sparse) == 0:
#            sparse.append((x, y, yaw, speed))
#            continue
#        if dist2(x, y, sparse[-1][0], sparse[-1][1]) >= distance2:
#            sparse.append((x,y,yaw,speed))
#    return sparse


def get_marker(sparse):
    marker = Marker()
    marker.header.frame_id = '/map'
    marker.type = marker.LINE_STRIP
    marker.action = marker.ADD

    # marker scale
    marker.scale.x = 0.1
    marker.scale.y = 0.1
    marker.scale.z = 0.1

    # marker color
    marker.color.a = 1.0
    marker.color.r = 0.0
    marker.color.g = 1.0
    marker.color.b = 0.0

    # marker orientaiton
    marker.pose.orientation.x = 0.0
    marker.pose.orientation.y = 0.0
    marker.pose.orientation.z = 0.0
    marker.pose.orientation.w = 1.0

    # marker position
    marker.pose.position.x = 0.0
    marker.pose.position.y = 0.0
    marker.pose.position.z = 0.0

    # marker line points
    marker.points = []

    for x, y, yaw, speed in sparse:
        p = Point()
        p.x = x
        p.y = y
        p.z = 0.0
        marker.points.append(p)

    return marker


def get_waypoints(sparse):
    waypoints = np.zeros((len(sparse), len(sparse[0])), dtype=np.float32)
    for i in range(len(sparse)):
        waypoints[i] = sparse[i]
    return waypoints


if __name__ == '__main__':
    fname = '/home/md/carla/PythonClient/racetrack/ros/wp-2018-12-18-17-35-23.csv'
    xs = []
    ys = []
    yaws = []
    speeds = []
    with open(fname, 'r') as f:
       data = csv.reader(f, delimiter=',')
       for row in data:
           xs.append(float(row[0]))
           ys.append(float(row[1]))
           yaws.append(float(row[2]))
           speeds.append(float(row[3]))

    rospy.init_node('~markers_node')

    publishers = {}
    publishers['centerline'] = rospy.Publisher('/centerline', Marker, queue_size=1)
    publishers['centerline_numpy'] = rospy.Publisher('/centerline_numpy', numpy_msg(Floats), queue_size=1)

    distance = 1
    sparse = sparse_trajectory(xs, ys, yaws, speeds, distance)

    print('Original trajectory length:', len(xs))
    print('  Example:', xs[0], ys[0], yaws[0], speeds[0])
    print('Sparse trajectory (%.1fm) length:' % float(distance), len(sparse))
    print('  Example:', sparse[0])

    marker = get_marker(sparse)
    waypoints_numpy = get_waypoints(sparse)

    while not rospy.is_shutdown():
       # Publish the Marker
       publishers['centerline'].publish(marker)
       publishers['centerline_numpy'].publish(waypoints_numpy.reshape(-1))

       rospy.sleep(0.01)
