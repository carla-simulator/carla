#!/usr/bin/env python3

# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""A CARLA client for testing."""

import argparse
import logging
import os
import random
import sys
import time

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

import carla

from carla import sensor
from carla.client import CarlaClient
from carla.settings import CarlaSettings
from carla.tcp import TCPClient
from carla.util import make_connection

import console


def run_carla_client(args):
    with make_connection(CarlaClient, args.host, args.port, timeout=15) as client:
        logging.info('CarlaClient connected')
        filename = '_out/test_episode_{:0>4d}/{:s}/{:0>6d}'
        frames_per_episode = 300
        episode = 0
        while True:
            episode += 1
            settings = CarlaSettings()
            settings.set(SendNonPlayerAgentsInfo=True, SynchronousMode=args.synchronous)
            settings.randomize_seeds()
            camera = sensor.Camera('DefaultCamera')
            camera.set_image_size(300, 200) # Do not change this, hard-coded in test.
            settings.add_sensor(camera)
            lidar = sensor.Lidar('DefaultLidar')
            settings.add_sensor(lidar)

            logging.debug('sending CarlaSettings:\n%s', settings)
            logging.info('new episode requested')

            scene = client.load_settings(settings)

            number_of_player_starts = len(scene.player_start_spots)
            player_start = random.randint(0, max(0, number_of_player_starts - 1))
            logging.info(
                'start episode at %d/%d player start (%d frames)',
                player_start,
                number_of_player_starts,
                frames_per_episode)

            client.start_episode(player_start)

            use_autopilot_control = (random.random() < 0.5)
            reverse = (random.random() < 0.2)

            for frame in range(0, frames_per_episode):
                logging.debug('reading measurements...')
                measurements, sensor_data = client.read_data()
                images = [x for x in sensor_data.values() if isinstance(x, sensor.Image)]

                logging.debug('received data of %d agents', len(measurements.non_player_agents))
                if len(images) > 0:
                    assert (images[0].width, images[0].height) == (camera.ImageSizeX, camera.ImageSizeY)

                if args.images_to_disk:
                    for name, data in sensor_data.items():
                        data.save_to_disk(filename.format(episode, name, frame))

                logging.debug('sending control...')
                control = measurements.player_measurements.autopilot_control
                if not use_autopilot_control:
                    control.steer = random.uniform(-1.0, 1.0)
                    control.throttle = 0.3
                    control.hand_brake = False
                    control.reverse = reverse
                client.send_control(
                    steer=control.steer,
                    throttle=control.throttle,
                    brake=control.brake,
                    hand_brake=control.hand_brake,
                    reverse=control.reverse)


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
        '-s', '--synchronous',
        action='store_true',
        help='enable synchronous mode')
    argparser.add_argument(
        '-i', '--images-to-disk',
        action='store_true',
        help='save images to disk')
    argparser.add_argument(
        '--echo',
        action='store_true',
        help='start a client that just echoes what the server sends')
    argparser.add_argument(
        '-c', '--console',
        action='store_true',
        help='start the client console')

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

    if args.console:
        cmd = console.CarlaClientConsole(args)
        try:
            cmd.cmdloop()
        finally:
            cmd.cleanup()
        return

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

                run_carla_client(args)

        except AssertionError as assertion:
            raise assertion
        except Exception as exception:
            logging.error('exception: %s', exception)
            time.sleep(1)


if __name__ == '__main__':

    try:
        main()
    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')
