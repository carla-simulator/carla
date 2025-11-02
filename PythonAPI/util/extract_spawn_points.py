""" CARLA map spawn points extractor """

from __future__ import print_function
import argparse
import logging
import os
import sys

import carla


def extract(args):
    try:
        client = carla.Client(args.host, args.port, worker_threads=1)
        client.set_timeout(2.0)

        world = client.get_world()
        try:
            _map = world.get_map()
        except RuntimeError as error:
            logging.info('RuntimeError: %s', error)
            sys.exit(1)

        if not _map.get_spawn_points():
            logging.info('There are no spawn points available in your map/town.')
            logging.info('Please add some Vehicle Spawn Point to your UE4 scene.')
            sys.exit(1)
        spawn_points = _map.get_spawn_points()
        with open(args.output_dir + "/spawn_points.csv", "w", encoding='utf8') as file:
            index = 0
            for index, spawn_point in enumerate(spawn_points):
                file.write(f'{index},{spawn_point.location.x},{spawn_point.location.y},{spawn_point.location.z}\n')

    finally:
        world = None

# ==============================================================================
# -- main() --------------------------------------------------------------------
# ==============================================================================


def main():
    argparser = argparse.ArgumentParser(
        description='CARLA map spawn points extractor')
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
        '-o', '--output-dir',
        required=True,
        help='Output directory path for extraction result')
    args = argparser.parse_args()

    if args.output_dir is None or not os.path.exists(args.output_dir):
        print('output directory not found.')

    logging.basicConfig(format='%(levelname)s: %(message)s', level=logging.INFO)

    logging.info('listening to server %s:%s', args.host, args.port)

    print(__doc__)

    try:
        extract(args)
    except:
        print('\nAn error has occurred in extraction.')


if __name__ == '__main__':

    try:
        main()
    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')
    except RuntimeError as e:
        print(e)
