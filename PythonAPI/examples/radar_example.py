#!/usr/bin/env python

# Copyright (c) 2019 Marc G Puig. All rights reserved.
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
import math
import time
import weakref


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
    args = argparser.parse_args()

    try:

        client = carla.Client(args.host, args.port)
        client.set_timeout(2.0)
        world = client.get_world()
        debug = world.debug

        bp_lb = world.get_blueprint_library()
        radar_bp = bp_lb.filter('sensor.other.radar')[0]

        print('------------------------------------')
        radar_bp.set_attribute('resolution', '5')
        print(radar_bp.id)
        print(radar_bp.tags)
        print(radar_bp.has_tag('resolution'))
        print(radar_bp.has_attribute('resolution'))
        print(radar_bp.get_attribute('resolution'))
        print('------------------------------------')

        mustang_bp = bp_lb.filter('vehicle.ford.mustang')[0]


        start_location = carla.Transform(carla.Location(-47.7, -83.9, 5.5))
        # start_location = world.get_map().get_spawn_points()[0].location

        print(world.get_map())
        # print(world.get_map().get_spawn_points())

        mustang = world.spawn_actor(
            mustang_bp,
            start_location)

        radar = world.spawn_actor(
            radar_bp,
            carla.Transform(
                location=carla.Location(y=1.5, z=1.5),
                rotation=carla.Rotation()),
                # rotation=carla.Rotation(pitch=45.0)),
                # rotation=carla.Rotation(pitch=90.0)),
            mustang)

        mustang.apply_control(carla.VehicleControl(throttle=0.3, steer=-0.0))

        def radar_callback(weak_radar, sensor):
            self = weak_radar()
            if not self:
                return
            print(f"sensor:                 {sensor}")
            print(f"len(sensor):            {len(sensor)}")
            print(f"sensor.get_point_count: {sensor.get_point_count()}")
            print("Detections:")
            [print(f"  - {s}") for s in sensor]
            [print(f"  - {s.velocity}") for s in sensor]
            [print(f"  - {s.azimuth}") for s in sensor]
            [print(f"  - {s.altitude}") for s in sensor]
            [print(f"  - {s.depth}") for s in sensor]
            print("------------------------------------------------------------------------------------------")

        weak_radar = weakref.ref(radar)
        radar.listen(lambda sensor: radar_callback(weak_radar, sensor))

        time_to_run = 3.0 # in seconds

        close_time = time.time() + time_to_run

        while time.time() < close_time:
            _ = world.wait_for_tick(2.0)

        cd = carla.RadarDetection()
        cd.velocity = 4.0
        cd.azimuth = 4.0
        cd.altitude = 4.0
        cd.depth = 4.0
        print(cd)

    finally:
        print('')
        if radar is not None:
            radar.destroy()
        if mustang is not None:
            mustang.destroy()


if __name__ == '__main__':

    main()
