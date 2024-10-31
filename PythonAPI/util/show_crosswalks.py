#!/usr/bin/env python

# Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.


import argparse
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
    argparser.add_argument(
        '--host', metavar='H', default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    argparser.add_argument(
        '-p', '--port', metavar='P', default=2000, type=int,
        help='TCP port to listen to (default: 2000)')
    argparser.add_argument(
        '--show', default='100', type=float,
        help='Duration of the visualization of the junctions')
    args = argparser.parse_args()

    client = carla.Client(args.host, args.port)
    world = client.get_world()
    tmap = world.get_map()

    crosswalks = tmap.get_crosswalks()

    for i in range(len(crosswalks)):
        world.debug.draw_string(crosswalks[i] + carla.Location(z=1), str(i), life_time=args.show, color=carla.Color(0, 0, 0))

    for i in range(0, len(crosswalks), 5):
        a = crosswalks[i] + carla.Location(z=0.5)
        b = crosswalks[i+1] + carla.Location(z=0.5)
        c = crosswalks[i+2] + carla.Location(z=0.5)
        d = crosswalks[i+3] + carla.Location(z=0.5)
        e = crosswalks[i+4] + carla.Location(z=0.5)

        world.debug.draw_line(a, b, life_time=args.show, thickness=0.1, color=carla.Color(0, 0, 128))
        world.debug.draw_line(b, c, life_time=args.show, thickness=0.1, color=carla.Color(0, 0, 128))
        world.debug.draw_line(c, d, life_time=args.show, thickness=0.1, color=carla.Color(0, 0, 128))
        world.debug.draw_line(d, e, life_time=args.show, thickness=0.1, color=carla.Color(0, 0, 128))


if __name__ == '__main__':
    main()
