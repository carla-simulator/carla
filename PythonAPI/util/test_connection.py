#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Blocks until the simulator is ready or the time-out is met."""


import carla

import argparse
import sys
import time


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
        '--timeout',
        metavar='T',
        default=10.0,
        type=float,
        help='time-out in seconds (default: 10)')
    args = argparser.parse_args()

    t0 = time.time()

    while args.timeout > (time.time() - t0):
        try:
            client = carla.Client(args.host, args.port)
            client.set_timeout(0.1)
            print('CARLA %s connected at %s:%d.' % (client.get_server_version(), args.host, args.port))
            return 0
        except RuntimeError:
            pass

    print('Failed to connect to %s:%d.' % (args.host, args.port))
    return 1


if __name__ == '__main__':

    sys.exit(main())
