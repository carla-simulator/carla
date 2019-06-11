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
    argparser.add_argument(
        '-n', '--number-of-vehicles',
        metavar='N',
        default=50,
        type=int,
        help='number of vehicles (default: 10)')
    args = argparser.parse_args()

    try:
      client = carla.Client(args.host, args.port)
      client.set_timeout(2.0)

      world = client.get_world()
      m = world.get_map()
      debug = world.debug

      # get random points to spawn
      spawn_points = []
      for i in range(args.number_of_vehicles):
          spawn_point = carla.Transform()
          spawn_point.location = world.get_random_location_from_navigation()
          spawn_points.append(spawn_point)


      SpawnActor = carla.command.SpawnActor
      SetAutopilot = carla.command.SetAutopilot
      FutureActor = carla.command.FutureActor

      # Spawn walker
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

      # wait
      while (1):
          time.sleep(1);

    finally:
        print('\ndestroying %d actors' % len(all_id))
        client.apply_batch([carla.command.DestroyActor(x) for x in all_id])

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        pass
    finally:
        pass
