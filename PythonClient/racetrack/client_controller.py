from __future__ import print_function

import argparse
import logging
import random
import time
import pandas as pd
import numpy as np
from scipy.interpolate import splprep, splev

import sys
sys.path.append('..')
from carla.client import make_carla_client
from carla.sensor import Camera, Lidar
from carla.settings import CarlaSettings
from carla.tcp import TCPConnectionError

from config import (
    IMAGE_SIZE,
    DTYPE,
    STEER_NOISE,
    THROTTLE_NOISE,
    IMAGE_CLIP_LOWER,
    IMAGE_CLIP_UPPER
)

from utils import clip_throttle, print_measurements

from model_predictive_control import MPCController
from proportion_derivative_control import PDController
from gamepad_controller import PadController


def run_carla_client(args):
    frames_per_episode = 10000
    spline_points = 10000

    track_DF = pd.read_csv('racetrack{}.txt'.format(args.racetrack), header=None)
    # The track data are rescaled by 100x with relation to Carla measurements
    track_DF = track_DF / 100

    pts_2D = track_DF.loc[:, [0, 1]].values
    tck, u = splprep(pts_2D.T, u=None, s=2.0, per=1, k=3)
    u_new = np.linspace(u.min(), u.max(), spline_points)
    x_new, y_new = splev(u_new, tck, der=0)
    pts_2D = np.c_[x_new, y_new]

    steer = 0.0
    throttle = 0.5

    depth_array = None

    num_laps = 0

    if args.controller_name == 'mpc':
        weather_id = 2
        controller = MPCController(args.target_speed)
    elif args.controller_name == 'pd':
        weather_id = 1
        controller = PDController(args.target_speed)
    elif args.controller_name == 'pad':
        weather_id = 5
        controller = PadController()
    elif args.controller_name == 'nn':
        # Import it here because importing TensorFlow is time consuming
        from neural_network_controller import NNController  # noqa
        weather_id = 11
        controller = NNController(
            args.target_speed,
            args.model_dir_name,
            args.which_model,
            args.throttle_coeff_A,
            args.throttle_coeff_B,
        )

    with make_carla_client(args.host, args.port) as client:
        print('CarlaClient connected')
        episode = 0
        while episode < args.num_of_episodes:
            # Start a new episode.

            store_data = not args.dont_store_data
            if store_data:
                depth_storage = np.zeros((IMAGE_CLIP_LOWER-IMAGE_CLIP_UPPER, IMAGE_SIZE[1], frames_per_episode)).astype(DTYPE)
                log_dicts = frames_per_episode * [None]

            if args.settings_filepath is None:

                # Create a CarlaSettings object. This object is a wrapper around
                # the CarlaSettings.ini file. Here we set the configuration we
                # want for the new episode.
                settings = CarlaSettings()

                settings.set(
                    SynchronousMode=True,
                    SendNonPlayerAgentsInfo=False,
                    NumberOfVehicles=0,
                    NumberOfPedestrians=0,
                    WeatherId=weather_id,
                    QualityLevel=args.quality_level
                )

                settings.randomize_seeds()

                # Now we want to add a couple of cameras to the player vehicle.
                # We will collect the images produced by these cameras every
                # frame.

                # Let's add another camera producing ground-truth depth.
                camera = Camera('CameraDepth', PostProcessing='Depth')
                camera.set_image_size(IMAGE_SIZE[1], IMAGE_SIZE[0])
                camera.set_position(2.30, 0, 1.30)
                settings.add_sensor(camera)

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
            num_of_player_starts = len(scene.player_start_spots)
            player_start = random.randint(0, max(0, num_of_player_starts - 1))

            # Notify the server that we want to start the episode at the
            # player_start index. This function blocks until the server is ready
            # to start the episode.
            print('Starting new episode...')
            client.start_episode(player_start)

            status = run_episode(
                client,
                controller,
                pts_2D,
                depth_storage,
                log_dicts,
                frames_per_episode,
                args.controller_name,
                store_data
            )

            if status == 'FAIL':
                continue
            else:
                episode += 1
                if store_data:
                    np.save('depth_data/{}_racetrack{}_depth_data{}.npy'.format(args.controller_name, args.racetrack, episode), depth_storage)
                    pd.DataFrame(log_dicts).to_csv('logs/{}_racetrack{}_log{}.txt'.format(args.controller_name, args.racetrack, episode), index=False)


def run_episode(client, controller, pts_2D, depth_storage, log_dicts, frames_per_episode, controller_name, store_data):
    # Iterate every frame in the episode.
    for frame in range(frames_per_episode):

        # Read the data produced by the server this frame.
        measurements, sensor_data = client.read_data()
        if measurements.player_measurements.collision_other > 10000:
            return 'FAIL'

        depth_array = np.log(sensor_data['CameraDepth'].data).astype(DTYPE)
        depth_array = depth_array[IMAGE_CLIP_UPPER:IMAGE_CLIP_LOWER, :]

        one_log_dict, which_closest  = controller.control(
            pts_2D,
            measurements,
            depth_array,
        )

        steer, throttle = one_log_dict['steer'], one_log_dict['throttle']

        if controller_name != 'nn':
            # Add noise to "augment" the race
            steer += STEER_NOISE()
            throttle += THROTTLE_NOISE()

        client.send_control(
            steer=steer,
            throttle=throttle,
            brake=0.0,
            hand_brake=False,
            reverse=False
        )

        if store_data:
            depth_storage[..., frame] = depth_array
            one_log_dict['frame'] = frame
            log_dicts[frame] = one_log_dict

    return 'SUCCESS'


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
        '-r', '--racetrack',
        default='01',
        dest='racetrack',
        help='Racetrack number (but with a zero: 01, 02, etc.)')
    argparser.add_argument(
        '-e', '--num_of_episodes',
        default=10,
        type=int,
        dest='num_of_episodes',
        help='Number of episodes')
    argparser.add_argument(
        '-s', '--speed',
        default=45,
        type=float,
        dest='target_speed',
        help='Target speed')
    argparser.add_argument(
        '-cont', '--controller_name',
        default='mpc',
        dest='controller_name',
        help='Controller name')
    argparser.add_argument(
        '-d', '--dont_store_data',
        default=False,
        type=bool,
        dest='dont_store_data',
        help='Should data be stored')

    # For the NN controller
    argparser.add_argument(
        '-mf', '--model_dir_name',
        default=None,
        dest='model_dir_name',
        help='NN model directory name')
    argparser.add_argument(
        '-w', '--which_model',
        default='best',
        dest='which_model',
        help='Which model to load (5, 10, 15, ..., or: "best")')
    argparser.add_argument(
        '-tca', '--throttle_coeff_A',
        default=1.0,
        type=float,
        dest='throttle_coeff_A',
        help='Coefficient by which NN throttle predictions will be multiplied by')
    argparser.add_argument(
        '-tcb', '--throttle_coeff_B',
        default=0.0,
        type=float,
        dest='throttle_coeff_B',
        help='Coefficient by which NN throttle predictions will be shifted by')

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
