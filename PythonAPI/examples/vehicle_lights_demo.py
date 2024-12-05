#!/usr/bin/env python

# Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import argparse
import math

import carla

"""Visualize the different vehicle lights"""

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
        '-p', '--port', metavar='P', default=2000, type=int,
        help='TCP port to listen to (default: 2000)')
    argparser.add_argument(
        '-b', '--blueprint', default='lincoln',
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
            if bp.has_attribute('has_lights') and bp.get_attribute('has_lights'):
                bps.append(bp)
    else:
        bps.append(bp_lib.filter(args.blueprint)[0])

    vehicle = None
    try:
        for bp in bps:
            vehicle = world.spawn_actor(bp, carla.Transform(carla.Location(-47, 20, 0.3)))
            spectator = world.get_spectator()
            world.wait_for_tick()

            lights = vehicle.get_light_state()
            lights |= carla.VehicleLightState.Interior          # Add a specific light
            lights |= carla.VehicleLightState.LowBeam           # Add a specific light
            lights |= carla.VehicleLightState.Position          # Add a specific light
            lights |= carla.VehicleLightState.Reverse           # Add a specific light
            
            vehicle.set_light_state(carla.VehicleLightState(lights))

            angle_315 = False
            angle_675 = False

            angle = -45
            while angle < 1035:
                timestamp = world.wait_for_tick().timestamp
                angle += timestamp.delta_seconds * args.speed
                spectator.set_transform(get_transform(vehicle, angle))

                if not angle_315 and angle >= 315:
                    lights = vehicle.get_light_state()
                    lights &= ~carla.VehicleLightState.LowBeam          # Remove specific light
                    lights &= ~carla.VehicleLightState.Position         # Remove specific light
                    lights &= ~carla.VehicleLightState.Reverse         # Remove specific light
                    lights |= carla.VehicleLightState.HighBeam          # Add a specific light
                    lights |= carla.VehicleLightState.Brake             # Add a specific light
                    vehicle.set_light_state(carla.VehicleLightState(lights))
                    angle_315 = True

                if not angle_675 and angle >= 675:
                    lights = vehicle.get_light_state()
                    lights &= ~carla.VehicleLightState.HighBeam         # Remove specific light
                    lights &= ~carla.VehicleLightState.Brake            # Remove specific light
                    lights |= carla.VehicleLightState.Fog             # Add a specific light
                    lights |= carla.VehicleLightState.RightBlinker  # Add a specific light
                    lights |= carla.VehicleLightState.LeftBlinker   # Add a specific light
                    lights |= carla.VehicleLightState.Special1  # Add a specific light
                    lights |= carla.VehicleLightState.Special2   # Add a specific light
                    vehicle.set_light_state(carla.VehicleLightState(lights))
                    angle_675 = True

            lights = vehicle.get_light_state()
            lights |= carla.VehicleLightState.HighBeam      # Add a specific light
            lights |= carla.VehicleLightState.Brake      # Add a specific light
            lights |= carla.VehicleLightState.RightBlinker      # Add a specific light
            lights |= carla.VehicleLightState.LeftBlinker      # Add a specific light

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
