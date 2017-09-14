#!/usr/bin/env python2

# CARLA, Copyright (C) 2017 Computer Vision Center (CVC)


"""Basic CARLA client for testing."""


import argparse
import logging
import time


from lib.carla_util import TestCarlaClientBase
from lib.util import make_client


def main():
    argparser = argparse.ArgumentParser(description=__doc__)
    argparser.add_argument(
        '-v', '--verbose',
        action='store_true',
        help='print debug information to console instead of log file')
    argparser.add_argument(
        '-d', '--debug',
        action='store_true',
        help='print debug extra information to log')
    argparser.add_argument(
        '--host',
        metavar='H',
        default='127.0.0.1',
        help='IP of the host server')
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to')

    args = argparser.parse_args()

    logging_config = {
        'format': 'carla_client:%(levelname)s: %(message)s',
        'level': logging.DEBUG if args.debug else logging.INFO
    }
    if not args.verbose:
        logging_config['filename'] = 'carla_client.log'
        logging_config['filemode'] = 'w+'
    logging.basicConfig(**logging_config)

    while True:
        try:
            with make_client(TestCarlaClientBase, args) as client:
                while True:
                    client.start_episode()
                    client.loop_on_agent_client(iterations=10)

        except Exception as exception:
            logging.error('exception: %s', exception)
            time.sleep(1)


if __name__ == '__main__':

    main()
