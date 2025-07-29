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

def test_location(carlamap, location):
    """
    Test if the location is correctly converted to geolocation and back.
    """
    carlageolocation = carlamap.transform_to_geolocation(location)
    testresult = carlamap.geolocation_to_transform(carlageolocation)
    if location == testresult:
        print("geolocation_to_transform works correctly")
    else:
        print("geolocation_to_transform does not work correctly")
        print("carlalocation: ", location)
        print("testresult.location: ", testresult)

def test_geolocation(carlamap, geolocation):
    """
    Test if the geolocation is correctly converted to location and back.
    """
    carlalocation = carlamap.geolocation_to_transform(geolocation)
    testresult = carlamap.transform_to_geolocation(carlalocation)
    if geolocation == testresult:
        print("transform_to_geolocation works correctly")
    else:
        print("transform_to_geolocation does not work correctly")
        print("carlalocation: ", geolocation)
        print("testresult: ", testresult)

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
        world = client.get_world()
        carlamap = world.get_map()

        print("Testing transform to geolocation...")

        test_location(carlamap, carla.Location(0, 0, 0))
        test_location(carlamap, carla.Location(100, 100, 100))
        test_location(carlamap, carla.Location(-100, -100, -100))   
        test_location(carlamap, carla.Location(100, 0, 0))
        test_location(carlamap, carla.Location(0, 100, 0))
        test_location(carlamap, carla.Location(0, 0, 100))
        test_location(carlamap, carla.Location(-100, 0, 0))
        test_location(carlamap, carla.Location(0, -100, 0))
        test_location(carlamap, carla.Location(1000, 1000, 1000))


        print("Testing geolocation to transform...")

        test_geolocation(carlamap, carla.GeoLocation(41.5, 2.1, 150.0) )
        test_geolocation(carlamap, carla.GeoLocation(-33.9, 151.2, 0.0))
        test_geolocation(carlamap, carla.GeoLocation(0, 120, -100))
        test_geolocation(carlamap, carla.GeoLocation(90.0, 180.0, 1000.0))
        test_geolocation(carlamap, carla.GeoLocation(-90.0, -180.0, -100.0))
        test_geolocation(carlamap, carla.GeoLocation(0, 0, 100))
        test_geolocation(carlamap, carla.GeoLocation(-20, 0, 0))
        test_geolocation(carlamap, carla.GeoLocation(0, -20, 0))
        test_geolocation(carlamap, carla.GeoLocation(20, 20, 1000))

    finally:
        pass


if __name__ == '__main__':

    try:
        main()
    except KeyboardInterrupt:
        pass
    finally:
        print('\ndone.')
