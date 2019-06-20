#!/usr/bin/env python

# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Helper script to generate consumables for Carla in Docker
"""

from __future__ import print_function

import argparse
import docker
import docker_utils
import os


def parse_args():
    argparser = argparse.ArgumentParser(
        description=__doc__)
    argparser.add_argument(
        '-v', '--verbose',
        action='store_true',
        default=False,
        help='Prints extra information')
    argparser.add_argument(
        '-i', '--input',
        type=str,
        help='Path of all the assets to convert',
        required=True)
    argparser.add_argument(
        '-o', '--output',
        type=str,
        help='Path where all the assets will be after conversion')
    args = argparser.parse_args()

    if not args.output:
        args.output = os.getcwd()

    print("- Params: -------------------------------------------")
    print(" - Verbose:     " + str(args.verbose))
    print(" - Input path:  " + str(args.input))
    print(" - Output path: " + str(args.output))
    print("-----------------------------------------------------")

    return args


def main():
    args = parse_args()
    carla_container_name = "carla:latest"
    inbox_assets_path = '/home/ue4/carla/Import'
    client = docker.from_env()

    try:

        if args.verbose:
            print("Running: docker run -v " + args.input + ":" + inbox_assets_path + " --rm -it " + carla_container_name + " /bin/bash")

        carla_container = client.containers.run(
            carla_container_name,
            user='ue4',
            auto_remove=True,
            stdin_open=True,
            tty=True,
            detach=True,
            volumes={args.input: {'bind': inbox_assets_path, 'mode': 'ro'}})

        docker_utils.exec_command(carla_container, 'make import', user='ue4', verbose=args.verbose)
        files_to_copy = docker_utils.get_file_names(carla_container, '/home/ue4/carla/Dist/*.tar.gz', user='ue4', verbose=args.verbose)
        docker_utils.extract_files(carla_container, files_to_copy, args.output)

    finally:

        print("Closing container " + carla_container_name)
        carla_container.stop()


if __name__ == '__main__':
    main()
