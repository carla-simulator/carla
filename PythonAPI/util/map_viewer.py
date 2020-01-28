#!/usr/bin/env python
"""OpenDRIVE Map viewer
"""
# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# ==============================================================================
# -- find carla module ---------------------------------------------------------
# ==============================================================================

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

# ==============================================================================
# -- imports -------------------------------------------------------------------
# ==============================================================================

import carla

import argparse
import logging
import datetime
import weakref
import math
import random
import hashlib
import pygame
import time

# ==============================================================================
# -- Constants -----------------------------------------------------------------
# ==============================================================================

COLOR_GREEN = pygame.Color(0, 255, 0)
COLOR_RED = pygame.Color(255, 0, 0)
COLOR_BLUE = pygame.Color(0, 0, 255)
COLOR_WHITE = pygame.Color(255, 255, 255)

def world_to_pixel(location, pixels_per_meter, scale,  world_offset, offset=(0,0)):
    """Converts the world coordinates to pixel coordinates"""
    x = scale * pixels_per_meter * (location.x - world_offset[0])
    y = scale * pixels_per_meter * (location.y - world_offset[1])
    return [int(x - offset[0]), int(y - offset[1])]

# ==============================================================================
# -- Main --------------------------------------------------------------------
# ==============================================================================

def main():
    argparser = argparse.ArgumentParser()
    argparser.add_argument(
        '-f', '--file',
        metavar='F',
        default="",
        type=str,
        help='Path to the OpenDRIVE file')
    argparser.add_argument(
        '-o', '--output',
        metavar='O',
        default="map_viewer.png",
        type=str,
        help='Output file for the map image')
    args = argparser.parse_args()

    pygame.init()
    height = 1024
    width = 1024
    display = pygame.display.set_mode(
        (width, height),
        pygame.HWSURFACE | pygame.DOUBLEBUF)

    # Place a title to game window
    pygame.display.set_caption("map viewer")

    filename = args.file
    f_odr = open(filename, "r")
    opendrive = f_odr.read()
    map = carla.Map("TestParser", str(opendrive))
    waypoints = map.generate_waypoints(10)
    points = []
    x_list = []
    y_list = []
    for w in waypoints:

        transf = w.transform
        if math.isnan(transf.location.x) | math.isnan(transf.location.y):
            print("nan here: lane id " + str(w.lane_id) + " road id " + str(w.road_id))
        else:
            x_list.append(transf.location.x)
            y_list.append(transf.location.y)
            points.append(transf)
    x_min = min(x_list)
    x_max = max(x_list)
    y_min = min(y_list)
    y_max = max(y_list)

    road_width = x_max - x_min
    road_height = y_max - y_min
    road_mid = (0.5*(x_max + x_min), 0.5*(y_max + y_min))
    scale = 1
    pixels_per_meter = width/road_width
    world_offset = road_mid
    print("Num points: " + str(len(waypoints)))
    print("road width: " + str(road_width))
    print("road height: " + str(road_height))
    print("road mid: " + str(road_mid))
    print("scale: " + str(scale))

    for point in points:
        pygame.draw.circle(
            display, COLOR_GREEN,
            world_to_pixel(point.location, pixels_per_meter, scale, world_offset, (-width / 2, -height / 2)), 0)

    """waypoints = map.get_topology()
    topopoints = []
    for w in waypoints:
        pos = w[0].transform.location
        topopoints.append(pos)
        pos = w[1].transform.location
        topopoints.append(pos)

    for point in topopoints:
        pygame.draw.circle(
            display, COLOR_BLUE,
            world_to_pixel(point, pixels_per_meter, scale, world_offset, (-width / 2, -height / 2)), 3)
"""
    pygame.display.flip()
    if args.output != "":
        pygame.image.save(display, args.output)
    time.sleep(100000)




if __name__ == '__main__':
    main()