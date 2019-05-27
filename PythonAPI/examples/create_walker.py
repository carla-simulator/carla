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
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla

import argparse
import math
import random
import time

red = carla.Color(255, 0, 0)
green = carla.Color(0, 255, 0)
blue = carla.Color(47, 210, 231)
cyan = carla.Color(0, 255, 255)
yellow = carla.Color(255, 255, 0)
orange = carla.Color(255, 162, 0)
white = carla.Color(255, 255, 255)

def main():
    argparser = argparse.ArgumentParser()
    argparser.add_argument(
        '--host',
        metavar='H',
        default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)')
    args = argparser.parse_args()

    try:
        client = carla.Client(args.host, args.port)
        client.set_timeout(2.0)

        world = client.get_world()
        m = world.get_map()
        debug = world.debug
        a = carla.Location(-141.789, -153.839, 0.139954)
        b = carla.Location(-29.7504, -26.8764, 0.270432)
        points = []
        points = client.create_walker(a, b)
        # points = [
        #         (-141.789, -153.839, 0.139954),
        #         (-119.9, -134.6, 0.178125),
        #         (-112.1, -109.7, 0.178125),
        #         (-71.6, -67.4, 0.178125),
        #         (-40.1, -55.4, 0.178125),
        #         (-39.8, -54.8, 0.178125),
        #         (-29.7504, -26.8764, 0.270432)
        #         ]
        for i in range(len(points)-1):
            a1 = carla.Location(points[i][0], points[i][1], points[i][2])
            b1 = carla.Location(points[i+1][0], points[i+1][1], points[i+1][2])
            # print(a.x, a.y, a.z)
            debug.draw_line(a1, b1, color=orange, thickness=0.5, life_time=12)
            # debug.draw_line(a, a+carla.Location(z=1000), color=orange, thickness=0.2, life_time=12)
            debug.draw_point(a1, 1, red, 12)

        time.sleep(5)

    finally:
        # print("Total: ", total)
        pass

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        pass
    finally:
        pass
