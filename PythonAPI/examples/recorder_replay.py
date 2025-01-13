#!/usr/bin/env python

# Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Script that helps users to manually replay a log previously recorded"""

import argparse
import threading
import sys
import os
import pygame

import carla

FPS = 20
FIRST_FRAME = 0
LAST_FRAME = False
REPLAY_SPEED = 1
TIME = 0

def tick(world):
    global TIME, REPLAY_SPEED
    world.tick()
    TIME += world.get_snapshot().delta_seconds * REPLAY_SPEED

def recorder_utilities(client):
    global LAST_FRAME, REPLAY_SPEED
    stop = False

    while not stop and not LAST_FRAME:
        data = input("\nInput the next action: ")
        try:
            int_data = float(data)
            print("  Setting the replayer factor to {}".format(int_data))
            client.set_replayer_time_factor(int_data)
            REPLAY_SPEED = int_data
        except ValueError:
            if data not in ("S", "R"):
                print("\033[93mIgnoring unknown command '{}'\033[0m".format(data))
                continue

            print("  Time: {}".format(round(TIME, 3)))
            if data == 'S':
                stop = True
    LAST_FRAME = True

def main():
    argparser = argparse.ArgumentParser(description=__doc__)
    argparser.add_argument(
        '--host', metavar='H', default='localhost',
        help='IP of the host CARLA Simulator (default: localhost)')
    argparser.add_argument(
        '--port', metavar='P', default=2000, type=int,
        help='TCP port of CARLA Simulator (default: 2000)')
    argparser.add_argument(
        '-f', '--file', default='', required=True,
        help='File to be executed')
    argparser.add_argument(
        '--start-time', default=0, type=float,
        help='Start time of the recorder')
    argparser.add_argument(
        '--end-time', default=0, type=float,
        help='End time of the recorder')
    argparser.add_argument(
        '--follow-id', default=0, type=int,
        help='ID to follow')
    argparser.add_argument(
        '--follow-ego', action="store_true",
        help='follow the ego vehicle')
    argparser.add_argument(
        '--factor', default=1, type=float,
        help='Initial recorder factor')

    args = argparser.parse_args()

    if args.follow_id and args.follow_ego:
        print("Choose to either follow an id, or the ego vehicle, but not both")
        sys.exit(0)

    global TIME, LAST_FRAME, REPLAY_SPEED

    TIME = args.start_time
    REPLAY_SPEED = args.factor

    client = None
    world = None

    if not os.path.exists(args.file):
        print("WARNING: The specified '.log' file does not exist. Shutting down")
        sys.exit(-1)

    # Get the client
    print("\n\033[1m> Setting the simulation\033[0m")
    client = carla.Client(args.host, args.port)
    client.set_timeout(200.0)
    file_info = client.show_recorder_file_info(args.file, True)

    # Synchronous mode provides a smoother motion of the camera that follows the ego
    world = client.get_world()
    settings = world.get_settings()
    settings.synchronous_mode = True
    settings.fixed_delta_seconds = 1/FPS
    world.apply_settings(settings)

    # Get the ego vehicle id so that the spectator focuses on it
    follow_id = args.follow_id
    if args.follow_ego:
        file_split = file_info.split("Create ")
        for data in file_split:
            if not 'hero' in data:
                continue
            follow_id = int(data.split(": ")[0])
            print("Detected an ego vehicle with id '{}'".format(follow_id))
            break

    # Get the duration of the recorder (only if the end time is 0, aka until the recorder end)
    duration = args.end_time
    if not duration:
        duration = float(file_info.split("Duration: ")[-1].split(" ")[0])

    print("\033[1m> Starting the replayer\033[0m")
    client.replay_file(args.file, args.start_time, args.end_time, follow_id)
    client.set_replayer_time_factor(args.factor)

    tick(world)

    try:
        print("\033[1m> Running the recorder. Use\033[0m")
        print("\033[1m  - R: to record the replayer timestamp data\033[0m")
        print("\033[1m  - S: to stop the script\033[0m")
        print("\033[1m  - A number: to change the speed's factor of the replayer\033[0m")

        t1 = threading.Thread(target=recorder_utilities, args=(client, ))
        t1.start()

        clock = pygame.time.Clock()
        while not LAST_FRAME:
            clock.tick_busy_loop(20)
            tick(world)
            if TIME >= duration:
                LAST_FRAME = True

    except KeyboardInterrupt:
        pass
    finally:

        if world is not None:
            settings = world.get_settings()
            settings.synchronous_mode = False
            settings.fixed_delta_seconds = None
            world.apply_settings(settings)

        if client is not None:
            client.stop_replayer(True)
            client.set_replayer_time_factor(1)

if __name__ == '__main__':
    main()

