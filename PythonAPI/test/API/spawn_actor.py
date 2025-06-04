#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.
import argparse
import carla
import time

def get_transform(transform_data):
    if len(transform_data) != 6:
        raise ValueError("Transform needs 6 parameters but {} were given".format(len(transform_data)))
    location = carla.Location(float(transform_data[0]), float(transform_data[1]), float(transform_data[2]))
    rotation = carla.Rotation(float(transform_data[3]), float(transform_data[4]), float(transform_data[5]))
    return carla.Transform(location, rotation)

def tick_carla(sync, world):
    world.tick() if sync else world.wait_for_tick()

def main():
    argparser = argparse.ArgumentParser(
        description=__doc__)
    argparser.add_argument('--host', default='localhost', help='IP of the host CARLA Simulator (default: localhost)')
    argparser.add_argument('-p', '--port', default=2000, type=int, help='TCP port of CARLA Simulator (default: 2000)')
    argparser.add_argument('-b', '--blueprint', required=True, help='Blueprint to be spawned')
    argparser.add_argument('-t', '--transform', default='0 0 0 0 0 0', nargs="+", help="Transform to be spawned")
    argparser.add_argument('--sync', action='store_true', help='Changes to sync mode')

    args = argparser.parse_args()
    args.transform = get_transform(args.transform)

    actor = None

    # Get the client
    client = carla.Client(args.host, args.port)
    client.set_timeout(100.0)
    world = client.get_world()
    spectator = world.get_spectator()

    if args.sync:
        settings = world.get_settings()
        settings.synchronous_mode = True
        settings.fixed_delta_seconds = 0.05
        world.apply_settings(settings)

    # Spawn the actor
    bp = world.get_blueprint_library().filter(args.blueprint)[0]
    print(f"Spawning blueprint: {bp}")
    actor = world.spawn_actor(bp, args.transform)
    tick_carla(args.sync, world)
    spectator.set_transform(carla.Transform(args.transform.location + carla.Location(z=10), carla.Rotation(pitch=-90)))

    try:
        while True:
            tick_carla(args.sync, world)
            time.sleep(0.03)
    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')
    finally:
        if args.sync:
            settings = world.get_settings()
            settings.synchronous_mode = False
            settings.fixed_delta_seconds = None
            world.apply_settings(settings)

        if actor:
            actor.destroy()


if __name__ == '__main__':
    try:
        main()
    except RuntimeError as e:
        print(e)
