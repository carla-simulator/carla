#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Spawn NPCs into the simulation"""

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
import logging
import random


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
        '-w', '--number-of-walkers',
        metavar='W',
        default=50,
        type=int,
        help='number of walkers (default: 50)')
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
        '--filter',
        metavar='PATTERN',
        default='vehicle.*',
        help='actor filter (default: "vehicle.*")')
    args = argparser.parse_args()

    logging.basicConfig(format='%(levelname)s: %(message)s', level=logging.INFO)

    actor_list = []
    client = carla.Client(args.host, args.port)
    client.set_timeout(2.0)

    try:

        world = client.get_world()
        blueprints = world.get_blueprint_library().filter(args.filter)

        if args.safe:
            blueprints = [x for x in blueprints if int(x.get_attribute('number_of_wheels')) == 4]
            blueprints = [x for x in blueprints if not x.id.endswith('isetta')]
            blueprints = [x for x in blueprints if not x.id.endswith('carlacola')]

        spawn_points = world.get_map().get_spawn_points()
        number_of_spawn_points = len(spawn_points)

        if args.number_of_vehicles < number_of_spawn_points:
            random.shuffle(spawn_points)
        elif args.number_of_vehicles > number_of_spawn_points:
            msg = 'requested %d vehicles, but could only find %d spawn points'
            logging.warning(msg, args.number_of_vehicles, number_of_spawn_points)
            args.number_of_vehicles = number_of_spawn_points

        # @todo cannot import these directly.
        SpawnActor = carla.command.SpawnActor
        SetAutopilot = carla.command.SetAutopilot
        FutureActor = carla.command.FutureActor

        batch = []
        for n, transform in enumerate(spawn_points):
            if n >= args.number_of_vehicles:
                break
            blueprint = random.choice(blueprints)
            if blueprint.has_attribute('color'):
                color = random.choice(blueprint.get_attribute('color').recommended_values)
                blueprint.set_attribute('color', color)
            if blueprint.has_attribute('driver_id'):
                driver_id = random.choice(blueprint.get_attribute('driver_id').recommended_values)
                blueprint.set_attribute('driver_id', driver_id)
            blueprint.set_attribute('role_name', 'autopilot')
            batch.append(SpawnActor(blueprint, transform).then(SetAutopilot(FutureActor, True)))

        for response in client.apply_batch_sync(batch):
            if response.error:
                logging.error(response.error)
            else:
                actor_list.append(response.actor_id)

        print('spawned %d vehicles, press Ctrl+C to exit.' % len(actor_list))

        # Spawn Walkers
        print (args.number_of_walkers)
        spawn_points = []
        for i in range(args.number_of_walkers):
            spawn_point = carla.Transform()
            spawn_point.location = world.get_random_location_from_navigation()
            spawn_points.append(spawn_point)
        batch = []
        info = []
        for spawn_point in spawn_points:
            walker_bp = random.choice(world.get_blueprint_library().filter('walker.pedestrian.*'))
            batch.append(SpawnActor(walker_bp, spawn_point))

        # apply
        results = client.apply_batch_sync(batch, True)
        for i in range(len(results)):
            if results[i].error:
                logging.error(results[i].error)
            else:
                info.append({ "id":results[i].actor_id, "trans":spawn_points[i], "con":None })

        # Spawn walker controller
        batch = []
        walker_controller_bp = world.get_blueprint_library().find('controller.ai.walker')
        for i in range(len(info)):
            batch.append(SpawnActor(walker_controller_bp, carla.Transform(), info[i]["id"]))
        # apply
        results = client.apply_batch_sync(batch, True)
        for i in range(len(results)):
            if results[i].error:
                logging.error(results[i].error)
            else:
                info[i]["con"] = results[i].actor_id

        # get whole list of actors (child and parents)
        all_id = []
        for i in range(len(info)):
            all_id.append(info[i]["id"])
            all_id.append(info[i]["con"])
        all_actors = world.get_actors(all_id)

        # initialize each controller and set target to walk to
        for i in range(len(all_id)):
            # check it if it is a controller or a walker
            index = -1
            for j in range(len(info)):
                if (info[j]["con"] == all_id[i]):
                    index = j
                    break
            if (index != -1):
                # init
                all_actors[i].start(info[index]["trans"].location)
                # walk to random point
                target = world.get_random_location_from_navigation()
                all_actors[i].go_to_location(target)

        print('spawned %d walkers, press Ctrl+C to exit.' % len(info))

        while True:
            world.wait_for_tick()

    finally:

        print('\ndestroying %d vehicles' % len(actor_list))
        client.apply_batch([carla.command.DestroyActor(x) for x in actor_list])

        print('\ndestroying %d walkers' % len(info))
        client.apply_batch([carla.command.DestroyActor(x) for x in all_id])

if __name__ == '__main__':

    try:
        main()
    except KeyboardInterrupt:
        pass
    finally:
        print('\ndone.')
