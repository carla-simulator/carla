#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
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


def print_formated_dict(dic):
    for k, v in dic.items():
        print(' - "' + str(k) + '"' + ": " + str(v))
    print()


def bold(text):
    return ''.join([docker_utils.BOLD, text, docker_utils.ENDC])


def bold_underline(text):
    return ''.join([docker_utils.UNDERLINE, bold(text)])


def parse_args():
    argparser = argparse.ArgumentParser(
        description=__doc__)
    argparser.add_argument(
        '-i', '--input',
        type=str,
        help='Path of all the assets to convert')
    argparser.add_argument(
        '-o', '--output',
        type=str,
        help='Path where all the assets will be after conversion. Default: current directory.')
    argparser.add_argument(
        '--packages',
        type=str,
        help='(Optional) Packages to generate. Usage: "--packages=PkgeName1,PkgeName2"')
    argparser.add_argument(
        '-v', '--verbose',
        action='store_true',
        default=False,
        help='Prints extra information')
    argparser.add_argument(
        '--image',
        type=str,
        help='Use a specific Carla image. Default: "carla:latest"')
    args = argparser.parse_args()

    if not args.output:
        args.output = os.getcwd()

    if args.packages is not None and args.packages and not args.input:
        print(
            docker_utils.RED +
            "[Error] The Input Path [-i|--input] must be specified "
            "if you are processing individual packages." + docker_utils.ENDC)
        exit(1)

    print()

    print(bold("- ") + bold_underline("Params:"))
    print(" - Output path: " + str(args.output))
    print(" - Packages:    " + str(args.packages))
    print(" - Input path:  " + str(args.input))
    print(" - Verbose:     " + str(args.verbose))
    print()

    return args


def main():

    args = parse_args()
    carla_image_name = "carla:latest"
    inbox_assets_path = '/home/carla/carla/Import'
    client = docker.from_env()

    # All possible Docker arguments are here:
    # https://docker-py.readthedocs.io/en/stable/containers.html
    container_args = {
        "image": carla_image_name,
        "user": 'carla',
        "auto_remove": True,
        "stdin_open": True,
        "tty": True,
        "detach": True}

    if args.packages:
        container_args["volumes"] = {
            args.input: {'bind': inbox_assets_path, 'mode': 'rw'}}

    print(bold("- ") + bold_underline("Docker arguments:"))
    print_formated_dict(container_args)

    try:

        print("Runnig Docker...")
        carla_container = client.containers.run(**container_args)

        if args.packages:
            # If there is packages, import them first and package them
            docker_utils.exec_command(
                carla_container,
                'make import',
                user='carla', verbose=args.verbose, ignore_error=False)

            docker_utils.exec_command(
                carla_container,
                'make package ARGS="--packages=' + str(args.packages) + '"',
                user='carla', verbose=args.verbose, ignore_error=False)
        else:
            # Just create a package of the whole project
            docker_utils.exec_command(
                carla_container,
                'make package',
                user='carla', verbose=args.verbose, ignore_error=False)

        # Get the files routes to export
        files_to_copy = docker_utils.get_file_paths(
            carla_container,
            '/home/carla/carla/Dist/*.tar.gz',
            user='carla', verbose=args.verbose)

        # Copy these fles to the output folder
        docker_utils.extract_files(carla_container, files_to_copy, args.output)

    finally:

        print("Closing container " + carla_image_name)
        carla_container.stop()


if __name__ == '__main__':
    main()
