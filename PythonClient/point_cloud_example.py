#!/usr/bin/env python3

# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB), and the INTEL Visual Computing Lab.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# Basic CARLA client to generate point cloud in PLY format that you
# can visualize with MeshLab (meshlab.net) for instance. Please
# refer to client_example.py for a simpler and more documented example.

from __future__ import print_function

import argparse
import logging
import math
import numpy
import os
import random
import sys
import time
from carla.carla_server_pb2 import Transform

from carla.client import make_carla_client
from carla.sensor import Camera
from carla.settings import CarlaSettings
from carla.tcp import TCPConnectionError
from carla.util import print_over_same_line, StopWatch
from carla.image_converter import depth_to_local_point_cloud, to_rgb_array


def run_carla_client(host, port, save_images_to_disk, image_filename_format):
    # Here we will run 3 episodes with 300 frames each.
    number_of_frames = 3000
    output_folder = '_out'
    image_size = [800, 600]
    camera_local_pos = [30, 0, 130]
    camera_local_rotation = [0, 0, 0]
    fov = 70

    with make_carla_client(host, port) as client:
        print('CarlaClient connected')

        # Here we load the settings.
        settings = CarlaSettings()
        settings.set(
            SynchronousMode=True,
            SendNonPlayerAgentsInfo=False,
            NumberOfVehicles=20,
            NumberOfPedestrians=40,
            WeatherId=random.choice([1, 3, 7, 8, 14]))
        settings.randomize_seeds()

        camera1 = Camera('CameraDepth', PostProcessing='Depth', CameraFOV=fov)
        camera1.set_image_size(*image_size)
        camera1.set_position(*camera_local_pos)
        camera1.set_rotation(*camera_local_rotation)
        settings.add_sensor(camera1)

        camera2 = Camera(
            'CameraRGB', PostProcessing='SceneFinal', CameraFOV=fov)
        camera2.set_image_size(*image_size)
        camera2.set_position(*camera_local_pos)
        camera2.set_rotation(*camera_local_rotation)
        settings.add_sensor(camera2)

        scene = client.load_settings(settings)

        # Start at location index id '0'
        client.start_episode(0)

        # Compute the camera transform matrix
        camera_to_car = Transform()
        camera_to_car.location.x = camera_local_pos[0]
        camera_to_car.location.y = camera_local_pos[1]
        camera_to_car.location.z = camera_local_pos[2]
        camera_to_car.rotation.pitch = camera_local_rotation[0]
        camera_to_car.rotation.yaw = camera_local_rotation[1]
        camera_to_car.rotation.roll = camera_local_rotation[2]

        camera_to_car_matrix = compute_transform_matrix(camera_to_car)

        # Do the custom transformations.
        to_unreal_matrix = compute_transform_matrix_raw(
            roll=-90, yaw=90, sX=-1)

        camera_to_car_matrix = camera_to_car_matrix * to_unreal_matrix

        # Iterate every frame in the episode.
        for frame in range(0, number_of_frames):
            # Read the data produced by the server this frame.
            measurements, sensor_data = client.read_data()

            # Save the images to disk if requested.
            if save_images_to_disk:
                for name, image in sensor_data.items():
                    image.save_to_disk(
                        image_filename_format.format(episode, name, frame))

            # Start transformations time mesure.
            time = StopWatch()

            # RGB image [[[r,g,b],..[r,g,b]],..[[r,g,b],..[r,g,b]]]
            image_RGB = to_rgb_array(sensor_data['CameraRGB'])

            # 2d to (camera) local 3d
            # We use the image_RGB to colorize each 3D point, this is optional.
            # "max_depth" is used to keep only the points that are near to the
            # camera, meaning 1.0 the farest points (sky)
            points_3d = depth_to_local_point_cloud(
                sensor_data['CameraDepth'], image_RGB, max_depth=0.2)
            # This give us the following array of 3D points and colors:
            # [ [X0,Y0,Z0,R0,G0,B0],
            #    ...,
            #   [Xn,Yn,Zn,Rn,Gn,Bn] ]

            # (Camera) local 3d to world 3d.
            # Get the player transformation.
            car_to_world = measurements.player_measurements.transform

            # Compute the final transformation matrix.
            car_to_world_matrix = compute_transform_matrix(
                car_to_world) * camera_to_car_matrix

            # Here we split the 3d point and its corresponding color.
            color_list = points_3d[:, 3:6]
            points_3d = points_3d[:, 0:3]

            # Car to World transformation given the 3D points and the
            # transformation matrix.
            # 3D points must be in format:
            # [ [X0,Y0,Z0],
            #   ...,
            #   [Xn,Yn,Zn] ]
            points_3d = transform_points(car_to_world_matrix, points_3d)

            # Merging again the 3D points (now in world space) with the
            # previous colors
            points_3d = numpy.concatenate((points_3d, color_list), axis=1)
            # Now we have an array of 3D points and colors again:
            # [ [X0,Y0,Z0,R0,G0,B0],
            #    ...,
            #   [Xn,Yn,Zn,Rn,Gn,Bn] ]

            # End transformations time mesure.
            time.stop()

            # Save PLY to disk
            # This generates the PLY string with the 3D points and the RGB colors
            # for each row of the file.
            # ply = '\n'.join(['{:.2f} {:.2f} {:.2f} {:.0f} {:.0f} {:.0f}'.format(
            #     *p) for p in points_3d.tolist()])
            ply = points_3d.tostring()
            # Create folder to save if does not exist.
            if not os.path.isdir(output_folder):
                os.makedirs(output_folder)

            # Open the file and save with the specific PLY format.
            with open('{}/{:0>5}.ply'.format(output_folder, frame), 'w+') as f:
                f.write('\n'.join([construct_ply_header(len(points_3d), True), ply]))

            print_message(time.milliseconds(), len(points_3d), frame)

            client.send_control(
                measurements.player_measurements.autopilot_control)


def print_message(time, point_n, frame):
    message = ' '.join([
        'Transformations took {:>3.0f} ms.',
        'Saved {:>6} points to "{:0>5}.ply."'
    ]).format(time, point_n, frame)
    print_over_same_line(message)


def compute_transform_matrix(transform):
    return compute_transform_matrix_raw(
        transform.location.x,
        transform.location.y,
        transform.location.z,
        transform.rotation.pitch,
        transform.rotation.roll,
        transform.rotation.yaw
    )


def compute_transform_matrix_raw(x=0, y=0, z=0, pitch=0, roll=0, yaw=0, sX=1, sY=1, sZ=1):
    # Yaw
    cy = math.cos(numpy.radians(yaw))
    sy = math.sin(numpy.radians(yaw))
    # Pitch
    cr = math.cos(numpy.radians(roll))
    sr = math.sin(numpy.radians(roll))
    # Roll
    cp = math.cos(numpy.radians(pitch))
    sp = math.sin(numpy.radians(pitch))
    # (4 X 4) Identity matrix
    m = numpy.matrix(numpy.identity(4))
    # Translation
    m[0, 3] = x
    m[1, 3] = y
    m[2, 3] = z
    # Rotation
    m[0, 0] = sX * (cp * cy)
    m[0, 1] = sY * (cy * sp*sr - sy * cr)
    m[0, 2] = -sZ * (cy * sp * cr + sy * sr)
    m[1, 0] = sX * (sy * cp)
    m[1, 1] = sY * (sy * sp * sr + cy * cr)
    m[1, 2] = sZ * (cy * sr - sy * sp * cr)
    m[2, 0] = sX * (sp)
    m[2, 1] = -sY * (cp * sr)
    m[2, 2] = sZ * (cp * cr)

    return m


def transform_points(transform_matrix, points):
    """
    Given a 4x4 transformation matrix, transform an array of 3D points.
    Expected foramt: [[X0,Y0,Z0],..[Xn,Yn,Zn]]
    """
    # Needed foramt: [[X0,..Xn],[Z0,..Zn],[Z0,..Zn]]
    points = points.transpose()
    # Add 0s row: [[X0..,Xn],[Y0..,Yn],[Z0..,Zn],[0,..0]]
    points = numpy.append(points, numpy.ones((1, points.shape[1])), axis=0)
    # Point transformation
    points = transform_matrix * points
    # Return all but last row
    return points[0:3].transpose()


def construct_ply_header(points_number, colors=False):
    """
    Generates a PLY header given a total number of 3D points and
    coloring property if specified
    """
    header = ['ply',
              'format ascii 1.0',
              'element vertex {}',
              'property float32 x',
              'property float32 y',
              'property float32 z',
              'property uchar diffuse_red',
              'property uchar diffuse_green',
              'property uchar diffuse_blue',
              'end_header']
    if not colors:
        return '\n'.join(header[0:6] + [header[-1]]).format(points_number)
    return '\n'.join(header).format(points_number)


def main():
    argparser = argparse.ArgumentParser(description=__doc__)
    argparser.add_argument(
        '-v', '--verbose',
        action='store_true',
        dest='debug',
        help='print debug information')
    argparser.add_argument(
        '-c', '--color',
        action='store_true',
        default='SceneFinal',
        help='Color for the point cloud')
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
        print('\nClient stoped by user.')
