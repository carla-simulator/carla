#!/usr/bin/env python

# Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# Allows controlling a vehicle with a keyboard. For a simpler and more
# documented example, please take a look at tutorial.py.

from __future__ import print_function


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

from carla import ColorConverter as cc

import argparse
import collections
import datetime
import logging
import math
import random
import re
import weakref
import cv2
import numpy as np


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

    # friction_image

    client = carla.Client(args.host, args.port)
    client.set_timeout(10.0)

    world = client.get_world()
    m = world.get_map()
    debug = world.debug
    actors = world.get_actors().filter("static.trigger.*")
    for actor in actors:
        actor.destroy()
    world.wait_for_tick()

    random.seed(0)
    friction_map = []
    friction_map_size_x = 600
    friction_map_size_y = 800
    friction_map = np.ones((friction_map_size_x, friction_map_size_y, 1), np.float32)
    for i in range(0, friction_map_size_x):
        for j in range(0,friction_map_size_y):
            print(i, j)
            friction_map[i][j][0] = random.uniform(0.0,1.0)

    cv2.imshow('friction map', friction_map)

    friction_tile_size = 10
    friction_offset = carla.Location(-100, 100, 0)
    print('Map size:', str((friction_map_size_x, friction_map_size_y)))

    # Find Trigger Friction Blueprint
    friction_bp = world.get_blueprint_library().find('static.trigger.friction')
    drag_bp = world.get_blueprint_library().find('static.trigger.drag')

    extent = carla.Location(friction_tile_size, friction_tile_size, 10000.0)

    friction_bp.set_attribute('friction', str(1.0))
    friction_bp.set_attribute('extent_x', str(extent.x*100))
    friction_bp.set_attribute('extent_y', str(extent.y*100))
    friction_bp.set_attribute('extent_z', str(extent.z*100))

    drag_bp.set_attribute('drag', str(1.0))
    drag_bp.set_attribute('extent_x', str(extent.x*100))
    drag_bp.set_attribute('extent_y', str(extent.y*100))
    drag_bp.set_attribute('extent_z', str(extent.z*100))

    actors = world.get_actors().filter("vehicle.*")
    vehicle = None
    if len(actors) is not 0:
        vehicle = actors[0]

    current_tile_id = None
    current_friction_actor = None
    current_drag_actor = None
    while True:
        cv2.waitKey(1)
        snapshot = world.wait_for_tick()
        # Check if vehicle has been destroyed
        if vehicle is not None:
            if snapshot.find(vehicle.id) is None:
                vehicle = None

        # Search for the ego vehicle if necessary
        if vehicle is None:
            # print('Vehicle not found, searching...')
            vehicle = None
            current_tile = None
            if current_friction_actor is not None:
                current_friction_actor.destroy()
                current_friction_actor = None
            if current_drag_actor is not None:
                current_drag_actor.destroy()
                current_drag_actor = None
            actors = world.get_actors().filter("vehicle.*")
            if len(actors) is not 0:
                vehicle = actors[0]
            else:
                continue

        # Get friction map tile id
        location = vehicle.get_location()
        friction_map_x = int(location.x / friction_tile_size)
        friction_map_y = int(location.y / friction_tile_size)
        tile_id = (friction_map_x, friction_map_y)
        friction_value = friction_map[tile_id[1] % friction_map_size_x][tile_id[0] % friction_map_size_y][0]
        # print('Current tile: ', tile_id, 'friction: ', friction_value*2)
        # Update friction values if the tile has changed
        if current_tile_id != tile_id:

            if current_friction_actor is not None:
                current_friction_actor.destroy()
                current_friction_actor = None
            if current_drag_actor is not None:
                current_drag_actor.destroy()
                current_drag_actor = None

            current_tile_id = tile_id
            # Spawn Trigger Friction
            transform = carla.Transform()
            transform.location = carla.Location(float(friction_tile_size*friction_map_x), float(friction_tile_size*friction_map_y), 0.0)
            drag_value = (friction_value)*2.0
            friction_value = 5.0

            friction_bp.set_attribute('friction', str(friction_value))
            friction_actor = world.spawn_actor(friction_bp, transform)
            drag_bp.set_attribute('drag', str(drag_value))
            drag_actor = world.spawn_actor(drag_bp, transform)
            current_friction_actor = friction_actor
            current_drag_actor = drag_actor

            # Optional for visualizing the tile
            # debug.draw_box(box=carla.BoundingBox(transform.location, extent*0.5), rotation=transform.rotation, life_time=100, thickness=0.5, color=carla.Color(r=255,g=0,b=0))

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print('\nExit by user.')
    finally:
        print('\nExit.')
