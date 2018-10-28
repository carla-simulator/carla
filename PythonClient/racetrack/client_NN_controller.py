#!/usr/bin/env python3

# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Basic CARLA client example."""

from __future__ import print_function

import argparse
import logging
import random
import time
import pandas as pd
import numpy as np
from scipy import spatial
from scipy.interpolate import splprep, splev

import sys
sys.path.append('..')
from carla.client import make_carla_client
from carla.sensor import Camera, Lidar
from carla.settings import CarlaSettings
from carla.tcp import TCPConnectionError
from carla.util import print_over_same_line

import pickle
import keras


norm = np.linalg.norm


def run_carla_client(args):
    # Here we will run 3 episodes with 3000 frames each.
    number_of_episodes = 1
    frames_per_episode = 10000

    prev_speed = np.nan
    curr_speed = np.nan

    steer = 0
    throttle = 0.5
    target_speed = args.target_speed

    depth_array = None
    prev_depth_array = None

    model = keras.models.load_model('model.h5')

    # We assume the CARLA server is already waiting for a client to connect at
    # host:port. To create a connection we can use the `make_carla_client`
    # context manager, it creates a CARLA client object and starts the
    # connection. It will throw an exception if something goes wrong. The
    # context manager makes sure the connection is always cleaned up on exit.
    with make_carla_client(args.host, args.port) as client:
        print('CarlaClient connected')
        for episode in range(0, number_of_episodes):
            # Start a new episode.

            if args.settings_filepath is None:

                # Create a CarlaSettings object. This object is a wrapper around
                # the CarlaSettings.ini file. Here we set the configuration we
                # want for the new episode.
                settings = CarlaSettings()
                settings.set(
                    SynchronousMode=True,
                    SendNonPlayerAgentsInfo=True,
                    NumberOfVehicles=0,
                    NumberOfPedestrians=40,
                    WeatherId=random.choice([1]),
                    QualityLevel=args.quality_level)
                settings.randomize_seeds()

                # Now we want to add a couple of cameras to the player vehicle.
                # We will collect the images produced by these cameras every
                # frame.

                # The default camera captures RGB images of the scene.
                camera0 = Camera('CameraRGB')
                # Set image resolution in pixels.
                camera0.set_image_size(800, 600)
                # Set its position relative to the car in meters.
                camera0.set_position(0.30, 0, 1.30)
                settings.add_sensor(camera0)

                # Let's add another camera producing ground-truth depth.
                camera1 = Camera('CameraDepth', PostProcessing='Depth')
                camera1.set_image_size(200, 150)
                camera1.set_position(2.30, 0, 1.30)
                settings.add_sensor(camera1)

            else:

                # Alternatively, we can load these settings from a file.
                with open(args.settings_filepath, 'r') as fp:
                    settings = fp.read()

            # Now we load these settings into the server. The server replies
            # with a scene description containing the available start spots for
            # the player. Here we can provide a CarlaSettings object or a
            # CarlaSettings.ini file as string.
            scene = client.load_settings(settings)

            # Choose one player start at random.
            number_of_player_starts = len(scene.player_start_spots)
            player_start = random.randint(0, max(0, number_of_player_starts - 1))

            # Notify the server that we want to start the episode at the
            # player_start index. This function blocks until the server is ready
            # to start the episode.
            print('Starting new episode...')
            client.start_episode(player_start)

            # Iterate every frame in the episode.
            for frame in range(0, frames_per_episode):

                # Read the data produced by the server this frame.
                measurements, sensor_data = client.read_data()

                # Print some of the measurements.
                print_measurements(measurements)

                prev_speed = curr_speed
                curr_speed = measurements.player_measurements.forward_speed * 3.6
                throttle = np.clip(
                    throttle - 0.1 * (curr_speed-target_speed),
                    0.25,
                    1.0
                )

                depth_array = np.log(sensor_data['CameraDepth'].data).astype('float16')

                if prev_depth_array is None:
                    pred = 0
                else:
                    pred = model.predict(
                        np.expand_dims(
                            np.expand_dims(depth_array-prev_depth_array, 0),
                            3
                        )
                    )

                prev_depth_array = depth_array

                client.send_control(
                    steer=pred,
                    throttle=throttle,
                    brake=0.0,
                    hand_brake=False,
                    reverse=False
                )


def print_measurements(measurements):
    number_of_agents = len(measurements.non_player_agents)
    player_measurements = measurements.player_measurements
    message = 'Vehicle at ({pos_x:.1f}, {pos_y:.1f}), '
    message += '{speed:.0f} km/h, '
    message = message.format(
        pos_x=player_measurements.transform.location.x,
        pos_y=player_measurements.transform.location.y,
        speed=player_measurements.forward_speed * 3.6, # m/s -> km/h
    )
    print_over_same_line(message)


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
        '-l', '--lidar',
        action='store_true',
        help='enable Lidar')
    argparser.add_argument(
        '-q', '--quality-level',
        choices=['Low', 'Epic'],
        type=lambda s: s.title(),
        default='Epic',
        help='graphics quality level, a lower level makes the simulation run considerably faster.')
    argparser.add_argument(
        '-c', '--carla-settings',
        metavar='PATH',
        dest='settings_filepath',
        default=None,
        help='Path to a "CarlaSettings.ini" file')

    argparser.add_argument(
        '-s', '--speed',
        default=30,
        type=float,
        dest='target_speed',
        help='Target speed')

    args = argparser.parse_args()

    log_level = logging.DEBUG if args.debug else logging.INFO
    logging.basicConfig(format='%(levelname)s: %(message)s', level=log_level)

    logging.info('listening to server %s:%s', args.host, args.port)

    args.out_filename_format = '_out/episode_{:0>4d}/{:s}/{:0>6d}'

    while True:
        try:

            run_carla_client(args)

            print('Done.')
            return

        except TCPConnectionError as error:
            logging.error(error)
            time.sleep(1)


if __name__ == '__main__':

    try:
        main()
    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')
