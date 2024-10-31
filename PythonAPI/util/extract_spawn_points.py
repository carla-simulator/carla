#!/usr/bin/env python

# Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import argparse
import os

import carla

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
        '-o', '--output-dir', default='',
        help='Folder path where the spawn poitns will be stored (empty string is deactivated)')
    argparser.add_argument(
        '--show', action='store_true',
        help='Show the spawning points in the scene')
    args = argparser.parse_args()

    if args.output_dir and not os.path.exists(args.output_dir):
        print('output directory not found.')
        return

    client = carla.Client(args.host, args.port)
    world = client.get_world()
    tmap = world.get_map()
    spawn_points = tmap.get_spawn_points()

    if args.show:
        print("Showing the spawn points in the scene")
        for i, spawn_point in enumerate(spawn_points):
            world.debug.draw_point(spawn_point.location, life_time=-1, size=0.2, color=carla.Color(0,0,128))
            world.debug.draw_string(spawn_point.location + carla.Location(z=3), str(i), life_time=0, color=carla.Color(0, 0, 0))


    if args.output_dir:
        file_name = args.output_dir + "/spawn_points.csv"
        print(f"Saving the spawn points to {file_name}")
        with open(file_name, "w", encoding='utf8') as file:
            for index, spawn_point in enumerate(spawn_points):
                loc = spawn_point.location
                rot = spawn_point.rotation
                file.write(f'{index},{loc.x},{loc.y},{loc.z},{rot.roll},{rot.pitch},{rot.yaw}\n')


if __name__ == '__main__':
    main()
