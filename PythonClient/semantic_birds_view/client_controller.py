from __future__ import print_function

import os
import argparse
import logging
import random
import time
import pandas as pd
import numpy as np
from scipy.interpolate import splprep, splev

# I need to prepend `sys.path` with '..' to get to the carla module there.
# I'm pre-pending `sys.path` because there are other carla modules specified
# in PYTHONPATH already
import sys
sys.path = ['..'] + sys.path

from carla.client import make_carla_client
from carla.sensor import Camera, Lidar
from carla.settings import CarlaSettings
from carla.tcp import TCPConnectionError

import config as CONFIG
from gamepad_controller import PadController


DTYPE = 'uint8'
MIN_SPEED = 5


def zero_frame(width, height, frames_per_episode, num_channels):
    return np.zeros((
        height,
        width,
        num_channels,
        frames_per_episode,
    )).astype(DTYPE)


def run_carla_client(args):
    frames_per_episode = 1000

    steer = 0.0
    throttle = 0.5

    if args.controller_name == 'pad':
        weather_id = 1
        controller = PadController()
    else:
        raise NotImplementedError('Only the "pad" controller is supported')

    with make_carla_client(args.host, args.port) as client:
        print('CarlaClient connected')
        episode = 100
        num_fails = 0

        while episode < args.num_episodes:
            # Start a new episode

            storage = {}
            log_dicts = frames_per_episode * [None]

            # Create a CarlaSettings object. This object is a wrapper around
            # the CarlaSettings.ini file. Here we set the configuration we
            # want for the new episode.
            settings = CarlaSettings()

            settings.set(
                SynchronousMode=True,
                SendNonPlayerAgentsInfo=False,
                NumberOfVehicles=20,
                NumberOfPedestrians=0,
                WeatherId=weather_id,
                QualityLevel=args.quality_level
            )

            settings.randomize_seeds()

            # Now we want to add a couple of cameras to the player vehicle.
            # We will collect the images produced by these cameras every
            # frame.

            # Let's add another camera producing ground-truth depth.
            camera_specs = [
                {
                    'id': 'TopSS',
                    'post_processing': 'SemanticSegmentation',
                    'width': 200,
                    'height': 300,
                    'fov': 30,
                    'position': {'x': 0.0, 'y': 0.0, 'z': 100.0},
                    'rotation': {'pitch': -90.0, 'yaw': 0.0, 'roll': 0.0},
                },
            ]
            if args.which_camera_specs == 'carla_challenge':
                camera_specs.extend([
                    {
                        'id': 'FrontSS',
                        'post_processing': 'SemanticSegmentation',
                        'width': 300,
                        'height': 200,
                        'fov': 100,
                        'position': {'x': 0.7, 'y': 0.0, 'z': 1.6},
                        'rotation': {'pitch': 0.0, 'yaw': 0.0, 'roll': 0.0},
                    },
                    {
                        'id': 'LeftSS',
                        'post_processing': 'SemanticSegmentation',
                        'width': 300,
                        'height': 200,
                        'fov': 100,
                        'position': {'x': 0.7, 'y': -0.4, 'z': 1.6},
                        'rotation': {'pitch': 0.0, 'yaw': -45.0, 'roll': 0.0},
                    },
                    {
                        'id': 'RightSS',
                        'post_processing': 'SemanticSegmentation',
                        'width': 300,
                        'height': 200,
                        'fov': 100,
                        'position': {'x': 0.7, 'y': 0.4, 'z': 1.6},
                        'rotation': {'pitch': 0.0, 'yaw': 45.0, 'roll': 0.0},
                    },
                    {
                        'id': 'RearSS',
                        'post_processing': 'SemanticSegmentation',
                        'width': 300,
                        'height': 200,
                        'fov': 130,
                        'position': {'x': -1.8, 'y': 0.0, 'z': 1.6},
                        'rotation': {'pitch': 0.0, 'yaw': 180.0, 'roll': 0.0},
                    },
                ])
            else:
                camera_specs.extend([
                    {
                        'id': 'FrontLeftSS',
                        'post_processing': 'SemanticSegmentation',
                        'width': 300,
                        'height': 200,
                        'fov': 100,
                        'position': {'x': 0.1, 'y': -0.1, 'z': 1.6},
                        'rotation': {'pitch': 0.0, 'yaw': -45.0, 'roll': 0.0},
                    },
                    {
                        'id': 'FrontRightSS',
                        'post_processing': 'SemanticSegmentation',
                        'width': 300,
                        'height': 200,
                        'fov': 100,
                        'position': {'x': 0.1, 'y': 0.1, 'z': 1.6},
                        'rotation': {'pitch': 0.0, 'yaw': 45.0, 'roll': 0.0},
                    },
                    {
                        'id': 'RearLeftSS',
                        'post_processing': 'SemanticSegmentation',
                        'width': 300,
                        'height': 200,
                        'fov': 100,
                        'position': {'x': -0.1, 'y': -0.1, 'z': 1.6},
                        'rotation': {'pitch': 0.0, 'yaw': -45.0, 'roll': 0.0},
                    },
                    {
                        'id': 'RearRightSS',
                        'post_processing': 'SemanticSegmentation',
                        'width': 300,
                        'height': 200,
                        'fov': 100,
                        'position': {'x': -0.1, 'y': 0.1, 'z': 1.6},
                        'rotation': {'pitch': 0.0, 'yaw': 45.0, 'roll': 0.0},
                    },
                ])


            for spec in camera_specs:
                camera = Camera(spec['id'], PostProcessing=spec['post_processing'])
                camera.set(FOV=spec['fov'])
                camera.set_image_size(spec['width'], spec['height'])
                camera.set_position(**spec['position'])
                camera.set_rotation(**spec['rotation'])
                settings.add_sensor(camera)

                num_channels = 3 if 'RGB' in spec['id'] else 1
                storage[spec['id']] = zero_frame(
                        spec['width'],
                        spec['height'],
                        frames_per_episode,
                        num_channels
                )

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
            print('Starting new episode...', )
            client.start_episode(player_start)

            status, storage, log_dicts = run_episode(
                client,
                controller,
                storage,
                log_dicts,
                frames_per_episode,
            )

            if 'FAIL' in status:
                print(status)
                continue
            else:
                print('SUCCESS: ' + str(episode))
                for id_ in storage.keys():
                    np.save('camera_storage/{}_{}_{}.npy'.format(id_, args.racetrack, episode), storage[id_])
                    pd.DataFrame(log_dicts).to_csv('logs/{}_{}.txt'.format(args.racetrack, episode), index=False)
                episode += 1


def run_episode(client, controller, storage, log_dicts, frames_per_episode):
    num_laps = 0
    curr_closest_waypoint = None
    prev_closest_waypoint = None
    num_steps_below_min_speed = 0

    # Iterate every frame in the episode.
    for frame in range(frames_per_episode):
        # Read the data produced by the server this frame.
        measurements, sensor_data = client.read_data()

        if measurements.player_measurements.forward_speed * 3.6 < MIN_SPEED:
            num_steps_below_min_speed += 1
        else:
            num_steps_below_min_speed = 0

        too_many_collisions = (measurements.player_measurements.collision_other > 100000)
        too_long_in_one_place = (num_steps_below_min_speed > 100)
        if too_many_collisions:
            return 'FAIL: too many collisions', None, None
        if too_long_in_one_place:
            return 'FAIL: too long in one place', None, None

        one_log_dict = controller.control(
            None,
            measurements,
            None,
        )

        steer, throttle = one_log_dict['steer'], one_log_dict['throttle']

        brake = 0
        reverse = False

        #if throttle < 0:
        #    brake = -throttle
        #    throttle = 0
        if throttle < 0:
            reverse = True
            throttle = -throttle

        client.send_control(
            steer=steer,
            throttle=throttle,
            brake=brake,
            hand_brake=False,
            reverse=reverse,
        )

        for id_ in storage.keys():
            data = sensor_data[id_].data
            if 'SS' in id_:
                data = np.expand_dims(data, 3)
            storage[id_][..., frame] = data
        log_dicts[frame] = one_log_dict

    return 'SUCCESS', storage, log_dicts


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
        '-e', '--num_episodes',
        default=10,
        type=int,
        dest='num_episodes',
        help='Number of episodes')
    argparser.add_argument(
        '-cont', '--controller_name',
        default='mpc',
        dest='controller_name',
        help='Controller name')
    argparser.add_argument(
        '-r', '--racetrack',
        default='Town01',
        dest='racetrack',
        help='Racetrack / Town')
    argparser.add_argument(
        '-w', '--which_camera_specs',
        choices=['carla_challenge', 'custom'],
        default='carla_challenge',
        dest='which_camera_specs',
        help='Which camera specification should be used')

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
