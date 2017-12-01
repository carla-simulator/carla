#!/usr/bin/env python3

# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB), and the INTEL Visual Computing Lab.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Basic CARLA to generate point cloud in PLY format."""

from __future__ import print_function

import argparse
import logging
import os
import random
import sys
import time

from carla.client import make_carla_client
from carla.sensor import Camera
from carla.settings import CarlaSettings
from carla.tcp import TCPConnectionError
from carla.util import print_over_same_line, StopWatch
from carla.image_converter import depth_to_local_point_cloud


def run_carla_client(host, port, save_images_to_disk, image_filename_format):
    # Here we will run 3 episodes with 300 frames each.
    number_of_frames = 300
    fov = 70.0
    output_folder = '_out'

    # We assume the CARLA server is already waiting for a client to connect at
    # host:port. To create a connection we can use the `make_carla_client`
    # context manager, it creates a CARLA client object and starts the
    # connection. It will throw an exception if something goes wrong. The
    # context manager makes sure the connection is always cleaned up on exit.
    with make_carla_client(host, port) as client:
        print('CarlaClient connected')

        # Start a new episode.
        # Here we load the settings
        settings = CarlaSettings()
        settings.set(
            SynchronousMode=True,
            SendNonPlayerAgentsInfo=False,
            NumberOfVehicles=20,
            NumberOfPedestrians=40,
            WeatherId=random.choice([1, 3, 7, 8, 14]))
        settings.randomize_seeds()
        camera1 = Camera('CameraDepth', PostProcessing='Depth', CameraFOV=fov)
        camera1.set_image_size(800, 600)
        camera1.set_position(30, 0, 130)
        settings.add_sensor(camera1)
        scene = client.load_settings(settings)

        # Start at location index id '0'
        client.start_episode(0)

        # Iterate every frame in the episode.
        for frame in range(0, number_of_frames):

            # Read the data produced by the server this frame.
            measurements, sensor_data = client.read_data()

            # Save the images to disk if requested.
            if save_images_to_disk:
                for name, image in sensor_data.items():
                    image.save_to_disk(
                        image_filename_format.format(episode, name, frame))

            print('Frame: {}'.format(frame))

            t = StopWatch()
            points3D = depth_to_local_point_cloud(
                sensor_data['CameraDepth'], fov)
            t.stop()

            print('Transformation took {:.0f} ms'.format(t.milliseconds()))

            t = StopWatch()
            num_pixels = sensor_data[
                'CameraDepth'].width*sensor_data['CameraDepth'].height
            pixel_array = points3D.reshape(num_pixels, 3)
            ply = '\n'.join([' '.join(map(str, i)) for i in pixel_array])

            if not os.path.isdir(output_folder):
                os.makedirs(output_folder)

            with open('{}/{:0>4}.ply'.format(output_folder, frame), 'w+') as f:
                f.write(construct_ply_header(num_pixels) + ply)
            t.stop()

            print('Saving to disk took {:.2f} s'.format(t.seconds()))
            print()

            client.send_control(
                measurements.player_measurements.autopilot_control)


def construct_ply_header(n_points):
    return ('ply\n'
            'format ascii 1.0\n'
            'element vertex ' + str(n_points) + '\n'
            'property float32 x\n'
            'property float32 y\n'
            'property float32 z\n'
            'end_header\n')


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
        '-i', '--images-to-disk',
        action='store_true',
        help='save images to disk')

    args = argparser.parse_args()

    log_level = logging.DEBUG if args.debug else logging.INFO
    logging.basicConfig(format='%(levelname)s: %(message)s', level=log_level)

    logging.info('listening to server %s:%s', args.host, args.port)

    while True:
        try:

            run_carla_client(
                host=args.host,
                port=args.port,
                save_images_to_disk=args.images_to_disk,
                image_filename_format='_images/episode_{:0>3d}/{:s}/image_{:0>5d}.png')

            print('Done.')
            return

        except TCPConnectionError as error:
            logging.error(error)
            time.sleep(1)
        except Exception as exception:
            logging.exception(exception)
            sys.exit(1)


if __name__ == '__main__':

    try:
        main()
    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')
