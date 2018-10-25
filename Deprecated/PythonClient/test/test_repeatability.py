#!/usr/bin/env python3

# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB), and the INTEL Visual Computing Lab.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Client that runs two servers simultaneously to test repeatability."""

import argparse
import logging
import os
import random
import sys
import time

sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

from carla.client import make_carla_client
from carla.sensor import Camera, Image
from carla.settings import CarlaSettings
from carla.tcp import TCPConnectionError


def run_carla_clients(args):
    filename = '_images_repeatability/server{:d}/{:0>6d}.png'
    with make_carla_client(args.host1, args.port1) as client1:
        logging.info('1st client connected')
        with make_carla_client(args.host2, args.port2) as client2:
            logging.info('2nd client connected')

            settings = CarlaSettings()
            settings.set(
                SynchronousMode=True,
                SendNonPlayerAgentsInfo=True,
                NumberOfVehicles=50,
                NumberOfPedestrians=50,
                WeatherId=random.choice([1, 3, 7, 8, 14]))
            settings.randomize_seeds()

            if args.images_to_disk:
                camera = Camera('DefaultCamera')
                camera.set_image_size(800, 600)
                settings.add_sensor(camera)

            scene1 = client1.load_settings(settings)
            scene2 = client2.load_settings(settings)

            number_of_player_starts = len(scene1.player_start_spots)
            assert number_of_player_starts == len(scene2.player_start_spots)
            player_start = random.randint(0, max(0, number_of_player_starts - 1))
            logging.info(
                'start episode at %d/%d player start (run forever, press ctrl+c to cancel)',
                player_start,
                number_of_player_starts)

            client1.start_episode(player_start)
            client2.start_episode(player_start)

            frame = 0
            while True:
                frame += 1

                meas1, sensor_data1 = client1.read_data()
                meas2, sensor_data2 = client2.read_data()

                player1 = meas1.player_measurements
                player2 = meas2.player_measurements

                images1 = [x for x in sensor_data1.values() if isinstance(x, Image)]
                images2 = [x for x in sensor_data2.values() if isinstance(x, Image)]

                control1 = player1.autopilot_control
                control2 = player2.autopilot_control

                try:
                    assert len(images1) == len(images2)
                    assert len(meas1.non_player_agents) == len(meas2.non_player_agents)
                    assert player1.transform.location.x == player2.transform.location.x
                    assert player1.transform.location.y == player2.transform.location.y
                    assert player1.transform.location.z == player2.transform.location.z
                    assert control1.steer == control2.steer
                    assert control1.throttle == control2.throttle
                    assert control1.brake == control2.brake
                    assert control1.hand_brake == control2.hand_brake
                    assert control1.reverse == control2.reverse
                except AssertionError:
                    logging.exception('assertion failed')

                if args.images_to_disk:
                    assert len(images1) == 1
                    images1[0].save_to_disk(filename.format(1, frame))
                    images2[0].save_to_disk(filename.format(2, frame))

                client1.send_control(control1)
                client2.send_control(control2)


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
        '--host1',
        metavar='H',
        default='127.0.0.1',
        help='IP of the first host server (default: 127.0.0.1)')
    argparser.add_argument(
        '-p1', '--port1',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to the first server (default: 2000)')
    argparser.add_argument(
        '--host2',
        metavar='H',
        default='127.0.0.1',
        help='IP of the second host server (default: 127.0.0.1)')
    argparser.add_argument(
        '-p2', '--port2',
        metavar='P',
        default=3000,
        type=int,
        help='TCP port to listen to the second server (default: 3000)')
    argparser.add_argument(
        '-i', '--images-to-disk',
        action='store_true',
        help='save images to disk')

    args = argparser.parse_args()

    logging_config = {
        'format': '%(levelname)s: %(message)s',
        'level': logging.DEBUG if args.debug else logging.INFO
    }
    if args.log:
        logging_config['filename'] = args.log
        logging_config['filemode'] = 'w+'
    logging.basicConfig(**logging_config)

    logging.info('listening to 1st server at %s:%s', args.host1, args.port1)
    logging.info('listening to 2nd server at %s:%s', args.host2, args.port2)

    while True:
        try:

            run_carla_clients(args)

        except TCPConnectionError as error:
            logging.error(error)
            time.sleep(1)


if __name__ == '__main__':

    try:
        main()
    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')
