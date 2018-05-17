#!/usr/bin/env python3

# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Connects with a CARLA simulator and displays the available start positions
for the current map."""

from __future__ import print_function

import argparse
import logging
import sys
import time

import matplotlib.image as mpimg
import matplotlib.pyplot as plt

from matplotlib.patches import Circle

from carla.client import make_carla_client
from carla.planner.map import CarlaMap
from carla.settings import CarlaSettings
from carla.tcp import TCPConnectionError


def view_start_positions(args):
    # We assume the CARLA server is already waiting for a client to connect at
    # host:port. The same way as in the client example.
    with make_carla_client(args.host, args.port) as client:
        print('CarlaClient connected')

        # We load the default settings to the client.
        scene = client.load_settings(CarlaSettings())
        print("Received the start positions")

        try:
            image = mpimg.imread('carla/planner/%s.png' % scene.map_name)
            carla_map = CarlaMap(scene.map_name, 0.1653, 50)
        except IOError as exception:
            logging.error(exception)
            logging.error('Cannot find map "%s"', scene.map_name)
            sys.exit(1)

        fig, ax = plt.subplots(1)

        ax.imshow(image)

        if args.positions == 'all':
            positions_to_plot = range(len(scene.player_start_spots))
        else:
            positions_to_plot = map(int, args.positions.split(','))

        for position in positions_to_plot:
            # Check if position is valid
            if position >= len(scene.player_start_spots):
                raise RuntimeError('Selected position is invalid')

            # Convert world to pixel coordinates
            pixel = carla_map.convert_to_pixel([scene.player_start_spots[position].location.x,
                                                scene.player_start_spots[position].location.y,
                                                scene.player_start_spots[position].location.z])

            circle = Circle((pixel[0], pixel[1]), 12, color='r', label='A point')
            ax.add_patch(circle)

            if not args.no_labels:
                plt.text(pixel[0], pixel[1], str(position), size='x-small')

        plt.axis('off')
        plt.show()

        fig.savefig('town_positions.pdf', orientation='landscape', bbox_inches='tight')


def main():
    argparser = argparse.ArgumentParser(description=__doc__)
    argparser.add_argument(
        '-v', '--verbose',
        action='store_true',
        dest='debug',
        help='print debug information')
    argparser.add_argument(
        '--host',
        metavar='H',
        default='localhost',
        help='IP of the host server (default: localhost)')
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)')
    argparser.add_argument(
        '-pos', '--positions',
        metavar='P',
        default='all',
        help='Indices of the positions that you want to plot on the map. '
             'The indices must be separated by commas (default = all positions)')
    argparser.add_argument(
        '--no-labels',
        action='store_true',
        help='do not display position indices')

    args = argparser.parse_args()

    log_level = logging.DEBUG if args.debug else logging.INFO
    logging.basicConfig(format='%(levelname)s: %(message)s', level=log_level)

    logging.info('listening to server %s:%s', args.host, args.port)

    while True:
        try:

            view_start_positions(args)
            print('Done.')
            return

        except TCPConnectionError as error:
            logging.error(error)
            time.sleep(1)
        except RuntimeError as error:
            logging.error(error)
            break


if __name__ == '__main__':

    try:
        main()
    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')
