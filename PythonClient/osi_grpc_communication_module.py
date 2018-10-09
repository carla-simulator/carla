#!/usr/bin/env python3

# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import argparse
import logging
import random
import time
import math
import grpc
import sys
import os
import numpy as np

from carla.client import make_carla_client
from carla.settings import CarlaSettings
from carla.tcp import TCPConnectionError

sys.path.insert(0,os.path.abspath('proto_generated'))

import osi_grpc_pb2_grpc
import osi_groundtruth_pb2


def run_carla_client(args):
    # Here we will run episodes with frames/episode.
    number_of_episodes = 1
    frames_per_episode = 500
    settings = CarlaSettings()
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

                settings.set(
                    SynchronousMode=True,
                    SendNonPlayerAgentsInfo=True,
                    NumberOfVehicles=100,
                    NumberOfPedestrians=10,
                    WeatherId='Default',
                    QualityLevel=args.quality_level)
                settings.randomize_seeds()

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
            player_start = min(0, max(0,number_of_player_starts-1))
            # Notify the server that we want to start the episode at the
            # player_start index. This function blocks until the server is ready
            # to start the episode.
            print('Starting new episode at %r...' % scene.map_name)
            client.start_episode(player_start)

            # Iterate every frame in the episode.
            for frame in range(0, frames_per_episode):

                # Read the data produced by the server this frame.
                measurements, sensor_data = client.read_data()

                # Start the gRPC communication between server and client
                send_osi_groundtruth(measurements, args)

                # Now we have to send the instructions to control the vehicle.
                # If we are in synchronous mode the server will pause the
                # simulation until we send this control.

                # Together with the measurements, the server has sent the
                # control that the in-game autopilot would do this frame.

                control = measurements.player_measurements.autopilot_control
                client.send_control(control)



def vehicle_to_world_rotation(x_in_vehicle, y_in_vehicle, z_in_vehicle, roll_host, pitch_host, yaw_host):
    """
    Rotates a 3D vector from vehicle to world coordinates
    :param x_in_vehicle: x_value in vehicle coordinate system
    :param y_in_vehicle: y_value in vehicle coordinate system
    :param z_in_vehicle: z_value in vehicle coordinate system
    :param roll_host: roll angle
    :param pitch_host: pitch angle
    :param yaw_host: yaw angle
    :return: x,y,z in world coordinate system
    """

    # Rotation about roll around x axis:
    sin_roll = math.sin(roll_host)
    cos_roll = math.cos(roll_host)
    y1_in_world = y_in_vehicle * cos_roll - z_in_vehicle * sin_roll
    z1_in_world = y_in_vehicle * sin_roll + z_in_vehicle * cos_roll

    # Rotation about pitch around y axis:
    sin_pitch = math.sin(pitch_host)
    cos_pitch = math.cos(pitch_host)
    x1_in_world = x_in_vehicle * cos_pitch + z1_in_world * sin_pitch
    z2_in_world = -x_in_vehicle * sin_pitch + z1_in_world * cos_pitch

    # Rotation about yaw around z axis:
    sin_yaw = math.sin(yaw_host)
    cos_yaw = math.cos(yaw_host)
    x3_in_world = x1_in_world * cos_yaw - y1_in_world * sin_yaw
    y3_in_world = x1_in_world * sin_yaw + y1_in_world * cos_yaw

    return x3_in_world, y3_in_world, z2_in_world


def send_osi_groundtruth(measurements, args):
    """
    Converts the measurements to OSI (Open Simulation Interface) GroundTruth data and sends them using gRPC
    :param measurements: measurements of the agents and the host vehicle in the scene
    :param args: command line arguments
    """
    channel = grpc.insecure_channel('localhost:' + str(args.grpc_port))
    stub = osi_grpc_pb2_grpc.GroundtruthdataStub(channel)
    ground_truth = osi_groundtruth_pb2.GroundTruth()
    host_vehicle = ground_truth.moving_object.add()

    # host_vehicle id
    unique_id = ((2 ** 64) - 1)
    ground_truth.host_vehicle_id.value = unique_id
    host_vehicle.id.value = unique_id

    # host_vehicle location
    host_vehicle.base.position.x = measurements.player_measurements.transform.location.x + \
        measurements.player_measurements.bounding_box.transform.location.x
    host_vehicle.base.position.y = measurements.player_measurements.transform.location.y + \
        measurements.player_measurements.bounding_box.transform.location.y
    host_vehicle.base.position.z = measurements.player_measurements.transform.location.z + \
        measurements.player_measurements.bounding_box.transform.location.z

    # Converting the floating point velocity from CARLA into a 3D OSI Standard Velocity
    yaw_host = measurements.player_measurements.transform.rotation.yaw
    pitch_host = measurements.player_measurements.transform.rotation.pitch
    roll_host = measurements.player_measurements.transform.rotation.roll

    velocity_host_x = measurements.player_measurements.forward_speed
    velocity_host_y = 0
    velocity_host_z = 0

    rotated_host_speed_v_to_w = vehicle_to_world_rotation(velocity_host_x,
                                                          velocity_host_y, velocity_host_z, roll_host,
                                                          pitch_host, yaw_host)
    host_vehicle.base.velocity.x = rotated_host_speed_v_to_w[0]
    host_vehicle.base.velocity.y = rotated_host_speed_v_to_w[1]
    host_vehicle.base.velocity.z = rotated_host_speed_v_to_w[2]

    # host_vehicle acceleration
    host_vehicle.base.acceleration.x = measurements.player_measurements.acceleration.x
    host_vehicle.base.acceleration.y = measurements.player_measurements.acceleration.y
    host_vehicle.base.acceleration.z = measurements.player_measurements.acceleration.z

    # host_vehicle rotation ( CARLA.degrees to OSI.radians)
    host_vehicle.base.orientation.roll = measurements.player_measurements.transform.rotation.roll \
        * (math.pi / 180)
    host_vehicle.base.orientation.pitch = measurements.player_measurements.transform.rotation.pitch \
        * (math.pi / 180)
    host_vehicle.base.orientation.yaw = measurements.player_measurements.transform.rotation.yaw \
        * (math.pi / 180)

    # host_vehicle bb extent ([m] radii dimension of the bounding box(half))
    host_vehicle.base.dimension.length = 2 * measurements.player_measurements.bounding_box.extent.x
    host_vehicle.base.dimension.width = 2 * measurements.player_measurements.bounding_box.extent.y
    host_vehicle.base.dimension.height = 2 * measurements.player_measurements.bounding_box.extent.z

    # timestamp with osi standards
    ground_truth.timestamp.seconds = np.int64(measurements.game_timestamp / 1000)
    last_second = np.double(measurements.game_timestamp / 1000.0)
    ground_truth.timestamp.nanos = np.uint32((10 ** 9) * (last_second - ground_truth.timestamp.seconds))

    for agent in measurements.non_player_agents:

        if agent.HasField('vehicle'):
            yaw_agent = agent.vehicle.transform.rotation.yaw
            pitch_agent = agent.vehicle.transform.rotation.pitch
            roll_agent = agent.vehicle.transform.rotation.roll

            velocity_vehicle_x = agent.vehicle.forward_speed
            velocity_vehicle_y = 0
            velocity_vehicle_z = 0

            mov_obj = ground_truth.moving_object.add()

            # id of the agent
            mov_obj.id.value = agent.id

            # velocity of the agent
            rotated_vehicle_speed_v_to_w = vehicle_to_world_rotation(velocity_vehicle_x,
                                                                     velocity_vehicle_y, velocity_vehicle_z, roll_agent,
                                                                     pitch_agent, yaw_agent)
            mov_obj.base.velocity.x = rotated_vehicle_speed_v_to_w[0]
            mov_obj.base.velocity.y = rotated_vehicle_speed_v_to_w[1]
            mov_obj.base.velocity.z = rotated_vehicle_speed_v_to_w[2]

            # position of the agent  (w.r.t cartisian coordinates)
            mov_obj.base.position.x = agent.vehicle.transform.location.x \
                + agent.vehicle.bounding_box.transform.location.x
            mov_obj.base.position.y = agent.vehicle.transform.location.y \
                + agent.vehicle.bounding_box.transform.location.y
            mov_obj.base.position.z = agent.vehicle.transform.location.z \
                + agent.vehicle.bounding_box.transform.location.z

            # rotation of the agent  ( CARLA.degrees to OSI.radians)
            mov_obj.base.orientation.roll = agent.vehicle.transform.rotation.roll \
                * (math.pi / 180)
            mov_obj.base.orientation.pitch = agent.vehicle.transform.rotation.pitch \
                * (math.pi / 180)
            mov_obj.base.orientation.yaw = agent.vehicle.transform.rotation.yaw \
                * (math.pi / 180)

            # bb extent of the agent([m] radii dimension of the bounding box(half))
            mov_obj.base.dimension.length = 2 * agent.vehicle.bounding_box.extent.x
            mov_obj.base.dimension.width = 2 * agent.vehicle.bounding_box.extent.y
            mov_obj.base.dimension.height = 2 * agent.vehicle.bounding_box.extent.z

    stub.ProcessGroundTruth(ground_truth)


def main():
    """
    Runs the CARLA Client with gRPC communication module.
    Contains the command line arguments.
    """
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
        default='Low',
        help='graphics quality level, a lower level makes the simulation run considerably faster.')
    argparser.add_argument(
        '-gp', '--grpc_port',
        metavar='GP',
        default=63558,
        type=int,
        help='TCP port to send via (default 63558)')
    argparser.add_argument(
        '-c', '--carla-settings',
        metavar='PATH',
        dest='settings_filepath',
        default=None,
        help='Path to a "CarlaSettings.ini" file')

    args = argparser.parse_args()

    log_level = logging.DEBUG if args.debug else logging.INFO
    logging.basicConfig(format='%(levelname)s: %(message)s', level=log_level)

    logging.info('listening to server %s:%s', args.host, args.port)

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
