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
import random
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
        '-n', '--number-of-vehicles',
        metavar='N',
        default=10,
        type=int,
        help='number of vehicles (default: 10)')
    argparser.add_argument(
        '-d', '--delay',
        metavar='D',
        default=2.0,
        type=float,
        help='delay in seconds between spawns (default: 2.0)')
    argparser.add_argument(
        '--safe',
        action='store_true',
        help='avoid spawning vehicles prone to accidents')
    argparser.add_argument(
        '-f', '--recorder_filename',
        metavar='F',
        default="test1.log",
        help='recorder filename (test1.log)')
    argparser.add_argument(
        '-t', '--recorder_time',
        metavar='T',
        default=0,
        type=int,
        help='recorder duration (auto-stop)')
    args = argparser.parse_args()

    actor_list = []

    try:

        client = carla.Client(args.host, args.port)
        client.set_timeout(2.0)
        world = client.get_world()
        blueprints = world.get_blueprint_library().filter('vehicle.*')

        if args.safe:
            blueprints = [x for x in blueprints if int(x.get_attribute('number_of_wheels')) == 4]
            blueprints = [x for x in blueprints if not x.id.endswith('isetta')]

        def try_spawn_random_vehicle_at(transform):
            blueprint = random.choice(blueprints)
            if blueprint.has_attribute('color'):
                color = random.choice(blueprint.get_attribute('color').recommended_values)
                blueprint.set_attribute('color', color)
            blueprint.set_attribute('role_name', 'autopilot')
            vehicle = world.try_spawn_actor(blueprint, transform)
            if vehicle is not None:
                actor_list.append(vehicle)
                vehicle.set_autopilot()
                print('spawned %r at %s' % (vehicle.type_id, transform.location))
                return True
            return False

        # @todo Needs to be converted to list to be shuffled.
        spawn_points = list(world.get_map().get_spawn_points())
        random.shuffle(spawn_points)

        print('found %d spawn points.' % len(spawn_points))

        count = args.number_of_vehicles

        print("Recording on file:", client.start_recorder(args.recorder_filename))

        for spawn_point in spawn_points:
            if try_spawn_random_vehicle_at(spawn_point):
                count -= 1
            if count <= 0:
                break

        while count > 0:
            time.sleep(args.delay)
            if try_spawn_random_vehicle_at(random.choice(spawn_points)):
                count -= 1

        print('spawned %d vehicles, press Ctrl+C to exit.' % args.number_of_vehicles)

        if (args.recorder_time > 0):
            time.sleep(args.recorder_time)
        else:
            while True:
                time.sleep(0.1)

    finally:

        print('\ndestroying %d actors' % len(actor_list))
        for actor in actor_list:
            actor.destroy()

        print("Stop recording")
        client.stop_recorder()


if __name__ == '__main__':

    try:
        main()
    except KeyboardInterrupt:
        pass
    finally:
        print('\ndone.')
