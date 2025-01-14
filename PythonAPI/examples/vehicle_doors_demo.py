#!/usr/bin/env python

# Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Example visualization of vehicle doors opening"""

import argparse
import math

import carla

def get_transform(vehicle, angle, d=2.5):

    vehicle_location = vehicle.get_location()
    radius = vehicle.bounding_box.extent.x * d
    height = vehicle.bounding_box.extent.x * 0.8
    a = math.radians(angle)
    location = carla.Location(radius * math.cos(a), radius * math.sin(a), height) + vehicle_location
    return carla.Transform(location, carla.Rotation(yaw=180 + angle, pitch=-15))

def main():
    argparser = argparse.ArgumentParser(
        description='CARLA Manual Control Client')
    argparser.add_argument(
        '--host', metavar='H', default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    argparser.add_argument(
        '-p', '--port', metavar='P', default=2000,
        type=int, help='TCP port to listen to (default: 2000)')
    argparser.add_argument(
        '-b', '--blueprint', default='all',
        help="Blueprint to be used. 'all' cycles through all vehicles")
    argparser.add_argument(
        '--speed', default=60, type=int,
        help="Camera rotation speed")
    args = argparser.parse_args()

    client = carla.Client(args.host, args.port)
    client.set_timeout(10)
    world = client.get_world()

    bp_lib = world.get_blueprint_library()

    bps = []
    if args.blueprint == 'all':
        all_vehicle_bps = bp_lib.filter('vehicle')
        for bp in all_vehicle_bps:
            if bp.has_attribute('has_dynamic_doors') and bp.get_attribute('has_dynamic_doors'):
                bps.append(bp)
    else:
        bps.append(bp_lib.filter(args.blueprint)[0])

    vehicle = None
    try:
        for bp in bps:
            vehicle = world.spawn_actor(bp, carla.Transform(carla.Location(-47, 20, 0.3)))
            spectator = world.get_spectator()

            angle_0 = False
            angle_90 = False
            angle_180 = False
            angle_270 = False
            angle_360 = False

            angle = -45
            while angle < 675:
                timestamp = world.wait_for_tick().timestamp
                angle += timestamp.delta_seconds * args.speed
                spectator.set_transform(get_transform(vehicle, angle))

                if not angle_0 and angle >= 0:
                    vehicle.open_door(carla.VehicleDoor.FR)
                    angle_0 = True
                if not angle_90 and angle >= 90:
                    vehicle.close_door(carla.VehicleDoor.FR)
                    world.wait_for_tick()
                    vehicle.open_door(carla.VehicleDoor.RR)
                    angle_90 = True
                if not angle_180 and angle >= 180:
                    vehicle.close_door(carla.VehicleDoor.RR)
                    world.wait_for_tick()
                    vehicle.open_door(carla.VehicleDoor.RL)
                    angle_180 = True
                if not angle_270 and angle >= 270:
                    vehicle.close_door(carla.VehicleDoor.RL)
                    world.wait_for_tick()
                    vehicle.open_door(carla.VehicleDoor.FL)
                    angle_270 = True
                if not angle_360 and angle >= 360:
                    vehicle.close_door(carla.VehicleDoor.FL)
                    world.wait_for_tick()
                    vehicle.open_door(carla.VehicleDoor.All)
                    angle_360 = True

            vehicle.destroy()
            vehicle = None
            for _ in range(10):
                world.wait_for_tick()

    except KeyboardInterrupt:
        pass

    finally:
        if vehicle is not None:
            vehicle.destroy()

if __name__ == '__main__':
    main()
