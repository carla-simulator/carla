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


def main():

    argparser = argparse.ArgumentParser(
        description=__doc__)
    argparser.add_argument(
        '--filepath',
        metavar='F',
        default='test.log',
        help='Filepath of the log file')
 

    try:

	print(client.show_recorder_file_info("arg.filepath"))
			
        #client = carla.Client(args.host, args.port)
        #client.set_timeout(60.0)

        # set the time factor for the replayer
        #client.set_replayer_time_factor(args.time_factor)

        # set to ignore the hero vehicles or not
        #client.set_replayer_ignore_hero(args.ignore_hero)

        # replay the session
        #print(client.replay_file(args.recorder_filename, args.start, args.duration, args.camera))

    finally:
        pass


if __name__ == '__main__':

    try:
        main()
    except KeyboardInterrupt:
        pass
    finally:
        print('\ndone.')
