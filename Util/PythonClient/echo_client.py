#!/usr/bin/env python2

# CARLA, Copyright (C) 2017 Computer Vision Center (CVC)


"""Implements TCP client that connects, reads, and sends back what it receives."""


import argparse
import logging
import time

from lib.tcp_client import TCPClient
from lib.util import make_client


def launch_echo_client(host, port):
    while True:
        try:
            logging.debug('connecting...')
            with make_client(TCPClient, host, port, timeout=20) as client:
                while True:
                    logging.debug('reading...')
                    data = client.read()
                    if data == '':
                        logging.debug('no data received!')
                        break
                    # logging.info('received: %s', data)
                    logging.debug('writing...')
                    client.write(data)
        except Exception as e:
            logging.error('exception: %s', e)
            time.sleep(1)


def main():
    argparser = argparse.ArgumentParser(description=__doc__)
    argparser.add_argument(
        '-v', '--verbose',
        action='store_true',
        help='print debug information to console instead of log file')
    argparser.add_argument(
        '--host',
        metavar='H',
        default='127.0.0.1',
        help='IP of the host server')
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=4000,
        type=int,
        help='TCP port to listen to')

    args = argparser.parse_args()

    logging_config = {
        'format': 'echo_client:%(levelname)s: %(message)s',
        'level': logging.DEBUG
    }
    if not args.verbose:
        logging_config['filename'] = 'echo_client.log'
        logging_config['filemode'] = 'w+'
    logging.basicConfig(**logging_config)

    launch_echo_client(args.host, args.port)


if __name__ == '__main__':

    main()
