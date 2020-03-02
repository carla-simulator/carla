#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import glob
import os
import sys

try:
    sys.path.append(glob.glob('../../../PythonAPI/carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla
import argparse

argparser = argparse.ArgumentParser()
argparser.add_argument(
    '-f', '--file',
    metavar='F',
    default="",
    type=str,
    help='OpenDRIVE file')
args = argparser.parse_args()

# read the OpenDRIVE
try:
    f = open(args.file, "rt")
except:
    print("OpenDRIVE file not found!")
    exit(1)

data = f.read()
f.close()

# create the map with the OpenDRIVE content
m = carla.Map("t", data)
points = m.get_crosswalks()
for i in range(len(points)):
    print(points[i])

# generate the .OBJ file
f = open("crosswalks.obj", "wt")
faceIndex = 0
vertexIndex = 0
i = 0
totalCrosswalks = 0
totalPoints = 0
while (i < len(points)):
    totalCrosswalks += 1
    # object
    f.write("o crosswalk_%d\n" % totalCrosswalks)
    a = i   # get the starting point index for this object
    startPoint = totalPoints
    while (i < len(points)):
        # vertex
        f.write("v %f %f %f\n" % (points[i].x, points[i].z, points[i].y))
        i +=1
        totalPoints += 1
        if (points[i].x == points[a].x and points[i].y == points[a].y and points[i].z == points[a].z):
            i +=1
            break
    # material (type)
    f.write("usemtl crosswalk\n")
    # faces
    for j in range(startPoint+2, totalPoints):
        f.write("f %d %d %d\n" % (startPoint+1, j-1+1, j+1))

f.close()
