#!/usr/bin/env python
"""OpenDRIVE Map viewer
"""
# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import glob
import os
import sys
import argparse
import math
import pygame
import time

# ==============================================================================
# -- find carla module ---------------------------------------------------------
# ==============================================================================

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

# ==============================================================================
# -- Constants -----------------------------------------------------------------
# ==============================================================================

COLOR_BLACK = pygame.Color(0, 0, 0)
COLOR_GREEN = pygame.Color(0, 255, 0)
COLOR_RED = pygame.Color(255, 0, 0)
COLOR_BLUE = pygame.Color(0, 0, 255)
COLOR_WHITE = pygame.Color(255, 255, 255)
COLOR_PINK = pygame.Color(255, 0, 255)


def world_to_pixel(location, pixels_per_meter, scale, world_offset, offset=(0, 0)):
    """Converts the world coordinates to pixel coordinates"""
    pixel_x = scale * pixels_per_meter * (location.x - world_offset[0])
    pixel_y = scale * pixels_per_meter * (location.y - world_offset[1])
    return [int(pixel_x - offset[0]), int(pixel_y - offset[1])]


def pixel_to_world(pixel_x, pixel_y, pixels_per_meter, scale, world_offset, offset=(0, 0)):
    """Converts the pixel coordinates to world coordinates"""
    location_x = float(pixel_x + offset[0]) / (scale * pixels_per_meter) + world_offset[0]
    location_y = float(pixel_y + offset[1]) / (scale * pixels_per_meter) + world_offset[1]
    return carla.Location(location_x, location_y, 0)

# ==============================================================================
# -- Main --------------------------------------------------------------------
# ==============================================================================

HEIGHT = 1024
WIDTH = 1024

def main():
    """Runs the 2D map viewer. Shows the map and the closest point of the mouse to the road
       Prints the required time to build the map structure and the average time of the query
       nearest point to the road.
    """

    argparser = argparse.ArgumentParser()
    argparser.add_argument(
        '-f', '--file',
        metavar='F',
        default="",
        type=str,
        help='Path to the OpenDRIVE file')
    args = argparser.parse_args()

    pygame.init()

    display = pygame.display.set_mode(
        (WIDTH, HEIGHT),
        pygame.HWSURFACE | pygame.DOUBLEBUF)

    # Place a title to game window
    pygame.display.set_caption("map viewer")

    filename = args.file
    f_odr = open(filename, "r")
    opendrive = f_odr.read()
    f_odr.close()

    start_map = time.time()
    carla_map = carla.Map("MapViewer", str(opendrive))
    end_map = time.time()
    print("Map load time: " + str(end_map - start_map) + " s")

    waypoints = carla_map.generate_waypoints(1)
    points = []
    x_list = []
    y_list = []
    for waypoint in waypoints:
        transf = waypoint.transform
        if math.isnan(transf.location.x) | math.isnan(transf.location.y):
            print("nan here: lane id " + str(waypoint.lane_id) +
                  " road id " + str(waypoint.road_id))
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
    road_mid = (0.5 * (x_max + x_min), 0.5 * (y_max + y_min))
    scale = 0.99
    pixels_per_meter = min(WIDTH / road_width, HEIGHT / road_height)
    world_offset = road_mid

    for point in points:
        pygame.draw.circle(
            display, COLOR_GREEN,
            world_to_pixel(point.location, pixels_per_meter, scale, world_offset,
                           (-WIDTH / 2, -HEIGHT / 2)), 0)
    road = display.convert()

    avg_query_time = 0
    counter = 0
    while True:
        event = pygame.event.poll()
        if event.type == pygame.QUIT:
            break

        display.blit(road, (0,0))

        mouse = pygame.mouse.get_pos()
        mouse_position = pixel_to_world(mouse[0], mouse[1], pixels_per_meter,
                                        scale, world_offset, (-WIDTH / 2, -HEIGHT / 2))
        mouse_waypoint = carla_map.get_waypoint(mouse_position)

        query_start = time.time()
        waypoint_position = world_to_pixel(mouse_waypoint.transform.location,
                                           pixels_per_meter, scale, world_offset,
                                           (-WIDTH / 2, -HEIGHT / 2))
        query_end = time.time()
        counter += 1
        avg_query_time = (avg_query_time * counter + query_end - query_start) / counter
        if (counter == 10):
            print("Query time: " + str(avg_query_time) + " s \r"),
            counter = 0
            avg_query_time = 0
        pygame.draw.line(display, COLOR_WHITE, mouse, waypoint_position, 1)
        pygame.display.flip()

    pygame.quit()


if __name__ == '__main__':
    main()
