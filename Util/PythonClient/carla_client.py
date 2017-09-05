#!/usr/bin/env python

# CARLA, Copyright (C) 2017 Computer Vision Center (CVC)


"""CARLA Client."""


import argparse
import logging
import random
import time

from tcp_client import TCPClient

import carla_server_pb2 as carla_protocol


CarlaSettings = """
[CARLA/Server]
UseNetworking=true
WorldPort=2000
ServerTimeOut=10000
SynchronousMode=false
SendNonPlayerAgentsInfo=true

[CARLA/LevelSettings]
PlayerVehicle=
NumberOfVehicles=20
NumberOfPedestrians=30
WeatherId=1
SeedVehicles=123456789
SeedPedestrians=123456789

[CARLA/SceneCapture]
Cameras=MyCamera

[CARLA/SceneCapture/MyCamera]
PostProcessing=SceneFinal
ImageSizeX=800
ImageSizeY=600
CameraFOV=90
CameraPositionX=15
CameraPositionY=0
CameraPositionZ=123
CameraRotationPitch=8
CameraRotationRoll=0
CameraRotationYaw=0
"""

class CarlaClient(object):
    def __init__(self, host, world_port, timeout):
        self._timeout = timeout
        self._world_client = TCPClient(host, world_port)
        self._stream_client = TCPClient(host, world_port + 1)
        self._control_client = TCPClient(host, world_port + 2)
        self._world_client.connect(timeout)

    def disconnect_all(self):
        self.disconnect_secondary_clients()
        self._world_client.disconnect()

    def connect_secondary_clients(self):
        self._stream_client.connect(self._timeout)
        self._control_client.connect(self._timeout)

    def disconnect_secondary_clients(self):
        self._stream_client.disconnect()
        self._control_client.disconnect()

    def write_request_new_episode(self, ini_file):
        pb_message = carla_protocol.RequestNewEpisode()
        pb_message.ini_file = ini_file
        self._world_client.write(pb_message.SerializeToString())

    def read_scene_description(self):
        data = self._world_client.read()
        if not data:
            return None
        pb_message = carla_protocol.SceneDescription()
        pb_message.ParseFromString(data)
        return pb_message

    def write_episode_start(self, player_start_location_index):
        pb_message = carla_protocol.EpisodeStart()
        pb_message.player_start_spot_index = player_start_location_index
        self._world_client.write(pb_message.SerializeToString())

    def read_episode_ready(self):
        data = self._world_client.read()
        if not data:
            return None
        pb_message = carla_protocol.EpisodeReady()
        pb_message.ParseFromString(data)
        return pb_message

    def read_measurements(self):
        data = self._stream_client.read()
        if not data:
            return None
        pb_message = carla_protocol.Measurements()
        pb_message.ParseFromString(data)
        return pb_message

    def read_images(self):
        data = self._stream_client.read()
        if not data:
            return None
        return data

    def write_control(self, **kwargs):
        pb_message = carla_protocol.Control()
        pb_message.steer = kwargs.get('steer', 0.3)
        pb_message.throttle = kwargs.get('throttle', 1.0)
        pb_message.brake = kwargs.get('brake', 0.0)
        pb_message.hand_brake = kwargs.get('hand_brake', False)
        pb_message.reverse = kwargs.get('reverse', False)
        pb_message.autopilot = kwargs.get('autopilot', False)
        self._control_client.write(pb_message.SerializeToString())


def test_carla_client():
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
        default=2000,
        type=int,
        help='TCP port to listen to')
    argparser.add_argument(
        '--ini-file',
        metavar='FILE',
        default='CarlaSettings.ini',
        help='path to a CarlaSettings.ini')

    args = argparser.parse_args()

    logging_config = {
        'format': 'carla_client:%(levelname)s: %(message)s',
        'level': logging.DEBUG
    }
    if not args.verbose:
        logging_config['filename'] = 'carla_client.log'
        logging_config['filemode'] = 'w+'
    logging.basicConfig(**logging_config)

    import os

    while True:
        client = None
        try:

            logging.info('connecting carla client to %s:%d', args.host, args.port)
            client = CarlaClient(args.host, args.port, timeout=15)

            if os.path.isfile(args.ini_file):
                logging.info('sending file %s', args.ini_file)
                with open(args.ini_file, 'r') as fd:
                    client.write_request_new_episode(fd.read())
            else:
                logging.info('sending empty ini file')
                client.write_request_new_episode(CarlaSettings)

            while True:

                logging.info('waiting for the scene description')
                data = client.read_scene_description()
                number_of_start_spots = len(data.player_start_spots)
                logging.info('received %d player start locations', number_of_start_spots)
                # for spot in data.player_start_spots:
                #     logging.info(spot)

                logging.info('sending episode start')
                client.write_episode_start(random.randint(0, max(0, number_of_start_spots - 1)))

                logging.info('waiting for episode to be ready')
                data = client.read_episode_ready()
                if not data.ready:
                    logging.error('received episode not ready')
                    break

                time.sleep(1) # Give some time to the server ;)
                logging.info('connecting secondary clients')
                client.connect_secondary_clients()

                autopilot = random.choice([True, False])

                for x in xrange(0, 1000):
                    logging.info('waiting for measurements')
                    data = client.read_measurements()
                    if not data:
                        logging.info('received empty measurements')
                    if not data.IsInitialized():
                        logging.info('received non-initialized measurements')
                    else:
                        logging.info('received valid measurements')
                        logging.info('received info of %d agents', len(data.non_player_agents))
                        # for agent in data.non_player_agents:
                        #     logging.info(agent)
                    logging.info('waiting for images')
                    data = client.read_images()
                    logging.info('received %d bytes of images', len(data) if data is not None else 0)

                    # if (x+1) % 100 == 0:
                    #     logging.info('Taking a nap...')
                    #     time.sleep(2)

                    logging.info('sending control')
                    if autopilot:
                        client.write_control(autopilot=True)
                    else:
                        client.write_control(steer=random.uniform(-1.0, 1.0), throttle=0.3, reverse=False)

                if os.path.isfile(args.ini_file):
                    logging.info('sending file %s', args.ini_file)
                    with open(args.ini_file, 'r') as fd:
                        client.write_request_new_episode(fd.read())
                else:
                    logging.info('sending empty ini file')
                    client.write_request_new_episode('Dummy empty ini file')

                client.disconnect_secondary_clients()

        except Exception as e:

            logging.error('exception: %s', e)
            time.sleep(1)

        finally:

            if client is not None:
                client.disconnect_all()


if __name__ == '__main__':

    test_carla_client()
