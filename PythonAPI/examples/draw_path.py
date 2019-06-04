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
import math
import random
import time
import logging

red = carla.Color(255, 0, 0)
green = carla.Color(0, 255, 0)
blue = carla.Color(47, 210, 231)
cyan = carla.Color(0, 255, 255)
yellow = carla.Color(255, 255, 0)
orange = carla.Color(255, 162, 0)
white = carla.Color(255, 255, 255)

def main():
    argparser = argparse.ArgumentParser()
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
        m = world.get_map()
        debug = world.debug

        # FOR SPAWNING WALKERS IN BATCH
        spawn_points = []
        for i in range(50):
            spawn_point = carla.Transform()
            spawn_point.location = world.get_random_location_from_navigation()
            spawn_points.append(spawn_point)


        SpawnActor = carla.command.SpawnActor
        SetAutopilot = carla.command.SetAutopilot
        FutureActor = carla.command.FutureActor


        # Spawn walker
        print ("SPAWNING WALKERS")
        actor_list = []
        batch = []
        for spawn_point in spawn_points:
            walker_bp = random.choice(world.get_blueprint_library().filter('walker.pedestrian.*'))
            batch.append(SpawnActor(walker_bp, spawn_point))

        for response in client.apply_batch_sync(batch):
            if response.error:
                logging.error(response.error)
            else:
                actor_list.append(response.actor_id)


        # Spawn walker controller
        time.sleep(2.0)
        print ("SPAWNING CONTROLLERS")
        del batch[:]
        batch = []
        controller_list = []
        walker_controller_bp = random.choice(world.get_blueprint_library().filter('controller.ai.walker'))
        for actor_id in actor_list:
            batch.append(SpawnActor(walker_controller_bp, carla.Transform(), actor_id))

        for response in client.apply_batch_sync(batch):
            if response.error:
                logging.error(response.error)
            else:
                controller_list.append(response.actor_id)
        time.sleep(2.0)

        # Set a target for each pedestrian
        # walkers = world.get_actors(actor_list)
        walker_controllers = world.get_actors(controller_list)
        for controller in walker_controllers:
            print ("Starting controller: ", controller.id)
            controller.start()

            target = world.get_random_location_from_navigation()
            controller.go_to_location(target)

        print('spawned %d walkers, press Ctrl+C to exit.' % len(actor_list))

        # FOR SPAWNING WALKERS SEQUENTIALLY
        # for i in range(50):
        #     print ("-----------------------------------------")
        #     spawn_point = carla.Transform()
        #     spawn_point.location = world.get_random_location_from_navigation()
        #     print (i, " SPAWN AT:", spawn_point.location.x, spawn_point.location.y, spawn_point.location.z)
        #     blueprint = random.choice(world.get_blueprint_library().filter('walker.pedestrian.*'))
        #     player = None
        #     while player is None:
        #         player = world.try_spawn_actor(blueprint, spawn_point)

        #     blueprint = random.choice(world.get_blueprint_library().filter('controller.ai.walker'))
        #     walker_controller = world.spawn_actor(blueprint, carla.Transform(), attach_to=player)
        #     walker_controller.start()

        #     time.sleep(0.4)


        #     target = world.get_random_location_from_navigation()
        #     print ("TARGET AT:", target.x, target.y, target.z)

        #     walker_controller.go_to_location(target)
        #     time.sleep(0.5)

        while (1):
            time.sleep(1);

    finally:
        pass

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        pass
    finally:
        pass
