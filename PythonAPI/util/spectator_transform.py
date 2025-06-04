#!/usr/bin/env python

# Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Script to get the transform of the spectator. Useful when roaming the city to remember specific points of interest"""

import argparse
import sys
import carla

def main():
    argparser = argparse.ArgumentParser(
        description=__doc__)
    argparser.add_argument(
        '--host', metavar='H', default='localhost',
        help='IP of the host CARLA Simulator (default: localhost)')
    argparser.add_argument(
        '--port', metavar='P', default=2000, type=int,
        help='TCP port of CARLA Simulator (default: 2000)')
    argparser.add_argument(
        '-w', '--use-waypoint', action='store_true',
        help='Return the driving waypoint closest to the spectator')
    argparser.add_argument(
        '-p', '--use-projection', action='store_true',
        help="Projects the spectator's location onto the ground")
    argparser.add_argument(
        '-z', '--height-diff', default=0, type=int,
        help="Adds a height diff to the output point. Useful with '-w' or '-p' to get a point z meters above ground")
    argparser.add_argument(
        '-if', '--interpolation-format', action='store_true',
        help="Return the data as an interpolation format, to be used by 'interpolate_camera.py'")

    args = argparser.parse_args()
    if args.use_waypoint and args.use_projection:
        print("Cannot use waypoint and projection at the same time")
        return

    # Get the client
    client = carla.Client(args.host, args.port)
    client.set_timeout(10.0)
    world = client.get_world()
    spectator = world.get_spectator()

    spec_transform = spectator.get_transform()

    if args.use_waypoint:
        tmap = world.get_map()
        wp = tmap.get_waypoint(spec_transform.location)
        transform = wp.transform

    elif args.use_projection:
        ground_loc = world.ground_projection(spec_transform.location, 100)
        if not ground_loc:
            print("Couldn't find a valid projection")
            sys.exit(0)
        transform = carla.Transform(ground_loc.location, spec_transform.rotation)

    else:
        transform = spec_transform

    transform.location.z += args.height_diff
    world.debug.draw_point(transform.location + carla.Location(z=0.2), size=0.3, color=carla.Color(0, 100, 0), life_time=30)

    if args.interpolation_format:
        print(f"<point time=\"0\""
            f" x=\"{round(transform.location.x, 2)}\""
            f" y=\"{round(transform.location.y, 2)}\""
            f" z=\"{round(transform.location.z, 2)}\""
            f" pitch=\"{round(transform.rotation.pitch, 2)}\""
            f" yaw=\"{round(transform.rotation.yaw, 2)}\""
            f" roll=\"{round(transform.rotation.roll, 2)}\"/>")
    else:
        print(transform)

if __name__ == '__main__':
    try:
        main()
    except RuntimeError as e:
        print(e)
