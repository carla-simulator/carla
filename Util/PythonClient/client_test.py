#!/usr/bin/env python3

# CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

"""Basic CARLA client."""

import argparse
import logging
import random
import time

import carla

from carla.client import CarlaClient
from carla.settings import CarlaSettings, Camera
from carla.tcp import TCPClient
from carla.util import make_connection


def run_carla_server(host, port):
    with make_connection(CarlaClient, host, port, timeout=15) as client:
        logging.info('CarlaClient connected')
        frames_per_episode = 300
        while True:
            settings = CarlaSettings()
            settings.set(SendNonPlayerAgentsInfo=True)
            settings.randomize_seeds()
            settings.add_camera(Camera('DefaultCamera'))

            logging.debug('sending CarlaSettings:\n%s', settings)
            logging.info('new episode requested')

            scene = client.request_new_episode(settings)

            number_of_player_starts = len(scene.player_start_spots)
            player_start = random.randint(0, max(0, number_of_player_starts - 1))
            logging.info(
                'start episode at %d/%d player start (%d frames)',
                player_start,
                number_of_player_starts,
                frames_per_episode)

            client.start_episode(player_start)

            autopilot = (random.random() < 0.5)
            reverse = (random.random() < 0.2)

            for _ in range(0, frames_per_episode):
                logging.debug('reading measurements...')
                measurements, images = client.read_measurements()

                logging.debug('received data of %d agents', len(measurements.non_player_agents))
                logging.debug('received %d bytes of images', len(images))

                logging.debug('sending control...')
                client.send_control(
                    steer=random.uniform(-1.0, 1.0),
                    throttle=0.3,
                    reverse=reverse,
                    autopilot=autopilot)


def main():
    argparser = argparse.ArgumentParser(description=__doc__)
    argparser.add_argument(
        '-v', '--verbose',
        action='store_true',
        dest='debug',
        help='print debug information')
    argparser.add_argument(
        '--log',
        metavar='LOG_FILE',
        default=None,
        help='print output to file')
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
        '--echo',
        action='store_true',
        help='start a client that just echoes what the server sends')

    args = argparser.parse_args()

    name = 'echo_client: ' if args.echo else 'carla_client: '
    logging_config = {
        'format': name + '%(levelname)s: %(message)s',
        'level': logging.DEBUG if args.debug else logging.INFO
    }
    if args.log:
        logging_config['filename'] = args.log
        logging_config['filemode'] = 'w+'
    logging.basicConfig(**logging_config)

    logging.info('listening to server %s:%s', args.host, args.port)

    while True:
        try:

            if args.echo:

                with make_connection(TCPClient, args.host, args.port, timeout=15) as client:
                    while True:
                        logging.info('reading...')
                        data = client.read()
                        if not data:
                            raise RuntimeError('failed to read data from server')
                        logging.info('writing...')
                        client.write(data)

            else:

                run_carla_server(args.host, args.port)

        except Exception as exception:
            logging.error('exception: %s', exception)
            time.sleep(1)


if __name__ == '__main__':

    try:
        main()
    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')
