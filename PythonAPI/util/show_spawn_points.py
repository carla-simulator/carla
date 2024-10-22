#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# Allows controlling a vehicle with a keyboard. For a simpler and more
# documented example, please take a look at tutorial.py.

import argparse
import sys
import math

import carla

def get_traffic_light_bbs(traffic_light):
    def rotate_point(point, angle):
        """
        rotate a given point by a given angle
        """
        x_ = math.cos(math.radians(angle)) * point.x - math.sin(math.radians(angle)) * point.y
        y_ = math.sin(math.radians(angle)) * point.x - math.cos(math.radians(angle)) * point.y

        return carla.Vector3D(x_, y_, point.z)

    base_transform = traffic_light.get_transform()
    base_rot = base_transform.rotation.yaw

    area_loc = base_transform.transform(traffic_light.trigger_volume.location)
    area_ext = traffic_light.trigger_volume.extent
    point = rotate_point(carla.Vector3D(0, 0, area_ext.z), base_rot)
    point_location = area_loc + carla.Location(x=point.x, y=point.y)

    trigger_volume = carla.BoundingBox(
        carla.Location(point_location.x, point_location.y, point_location.z),
        area_ext
    )
    trigger_volume.rotation = traffic_light.get_transform().rotation

    area_loc = base_transform.transform(traffic_light.bounding_box.location)
    area_ext = traffic_light.bounding_box.extent
    point = rotate_point(carla.Vector3D(0, 0, area_ext.z), base_rot)
    point_location = area_loc + carla.Location(x=point.x, y=point.y)

    bounding_box = carla.BoundingBox(
        carla.Location(point_location.x, point_location.y, point_location.z),
        area_ext
    )
    bounding_box.rotation = traffic_light.get_transform().rotation

    return bounding_box, trigger_volume


def main():
    argparser = argparse.ArgumentParser(
        description='CARLA Manual Control Client')
    argparser.add_argument('--host', metavar='H', default='127.0.0.1', help='IP of the host server (default: 127.0.0.1)')
    argparser.add_argument('-p', '--port', metavar='P', default=2000, type=int, help='TCP port to listen to (default: 2000)')
    argparser.add_argument('--show', default='100', type=float, help='Duration of the visualization of the junctions')
    args = argparser.parse_args()

    client = carla.Client(args.host, args.port)
    world = client.get_world()
    tmap = world.get_map()

    spawn_points = tmap.get_spawn_points()

    for i, spawn_point in enumerate(spawn_points):
        world.debug.draw_string(spawn_point.location + carla.Location(z=1), str(i), life_time=args.show, color=carla.Color(0, 0, 0))
        world.debug.draw_point(spawn_point.location, life_time=args.show, size=0.2, color=carla.Color(0,0,128))


if __name__ == '__main__':
    main()
