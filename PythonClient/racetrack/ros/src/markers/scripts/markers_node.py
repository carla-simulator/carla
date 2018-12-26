#!/usr/bin/python
from __future__ import print_function

import csv
import rospy
from visualization_msgs.msg import Marker
from geometry_msgs.msg import Point


def dist2(x1, y1, x2, y2):
    return (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2)


def sparseTrajectory(xs, ys, yaws, speeds, distance=1):
    assert len(xs) == len(ys) == len(yaws) == len(speeds)
    length = len(xs)
    distance2 = distance*distance
    sparse = [(xs[0], ys[0], yaws[0], speeds[0])]
    for i in range(length):
        x, y, yaw, speed = xs[i], ys[i], yaws[i], speeds[i]
        if dist2(x, y, sparse[-1][0], sparse[-1][1]) >= distance2:
           sparse.append((x, y, yaw, speed))
    return sparse


if __name__ == '__main__':
    fname = '/home/md/carla/PythonClient/racetrack/racetrack01.txt'
    xs = []
    ys = []
    yaws = []
    speeds = []
    with open(fname, 'r') as f:
       data = csv.reader(f, delimiter=',')
       for row in data:
           xs.append(float(row[0]))
           ys.append(float(row[1]))
           # TODO(MD): this is a hack, I don't have the yaw nor the speed in the CSV file
           yaws.append(-1000000000000000000)
           speeds.append(-1000000000000000000)

    rospy.init_node('markers_node')
    pub_line_min_dist = rospy.Publisher('/centerline', Marker, queue_size=1)

    distance = 1
    sparse = sparseTrajectory(xs, ys, yaws, speeds, distance)

    print('Original trajectory length:', len(xs))
    print('  Example:', xs[0], ys[0], yaws[0], speeds[0])
    print('Sparse trajectory (%.1fm) length:' % float(distance), len(sparse))
    print('  Example:', sparse[0])


    while not rospy.is_shutdown():
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

       # Publish the Marker
       pub_line_min_dist.publish(marker)

       rospy.sleep(0.01)
