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


def main():

    argparser = argparse.ArgumentParser(
        description=__doc__)
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
    argparser.add_argument(
        '-f', '--recorder_filename',
        metavar='F',
        default="test1.rec",
        help='recorder filename (test1.rec)')
    argparser.add_argument(
        '-t', '--time',
        metavar='T',
        default="30",
        type=float,
        help='minimum time to consider it is blocked')
    argparser.add_argument(
        '-d', '--distance',
        metavar='D',
        default="100",
        type=float,
        help='minimum distance to consider it is not moving (in cm)')
    args = argparser.parse_args()

    try:

        client = carla.Client(args.host, args.port)
        client.set_timeout(60.0)

        print(client.show_recorder_actors_blocked(args.recorder_filename, args.time, args.distance))

    finally:
        pass


if __name__ == '__main__':

    try:
        main()
    except KeyboardInterrupt:
        pass
    finally:
        print('\ndone.')
