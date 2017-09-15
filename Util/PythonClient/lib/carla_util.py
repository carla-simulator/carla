# CARLA, Copyright (C) 2017 Computer Vision Center (CVC)


"""Test suite for CARLA Client."""


import logging
import random


from carla_client import CarlaClient


MAX_NUMBER_OF_WEATHER_IDS = 14


def _get_settings_dict(**kwargs):
    return {
        'SynchronousMode':          kwargs.get('SynchronousMode', False),
        'SendNonPlayerAgentsInfo':  kwargs.get('SendNonPlayerAgentsInfo', False),
        'NumberOfVehicles':         max(0, int(kwargs.get('NumberOfVehicles', 20))),
        'NumberOfPedestrians':      max(0, int(kwargs.get('NumberOfPedestrians', 30))),
        'WeatherId':                int(kwargs.get('WeatherId', random.randint(-1, MAX_NUMBER_OF_WEATHER_IDS))),
    }


def get_carla_settings(**kwargs):
    """Create a CarlaSettings.ini based in the arguments provided"""
    return """
[CARLA/Server]
ServerTimeOut=10000
SynchronousMode={SynchronousMode}
SendNonPlayerAgentsInfo={SendNonPlayerAgentsInfo}

[CARLA/LevelSettings]
PlayerVehicle=
NumberOfVehicles={NumberOfVehicles}
NumberOfPedestrians={NumberOfPedestrians}
WeatherId={WeatherId}
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
""".format(**_get_settings_dict(**kwargs))


def get_number_of_agents(**kwargs):
    settings = _get_settings_dict(**kwargs)
    if not settings['SendNonPlayerAgentsInfo']:
        return 0
    else:
        return settings['NumberOfVehicles'] + settings['NumberOfPedestrians']


class TestCarlaClientBase(object):
    def __init__(self, args):
        """Create a TestCarlaClientBase and initialize the CarlaClient"""
        self.args = args
        self.number_of_agents = 0
        logging.info('connecting carla client to %s:%d', self.args.host, self.args.port)
        self.client = CarlaClient(self.args.host, self.args.port, timeout=15)

    def connect(self):
        self.client.connect_world_client()

    def disconnect(self):
        if self.client is not None:
            self.client.disconnect_all()

    def request_new_episode(self, **kwargs):
        """Create and send a CarlaSettigns.ini"""
        self.client.disconnect_agent_client()
        logging.info('requesting new episode')
        carla_settings = get_carla_settings(**kwargs)
        self.number_of_agents = get_number_of_agents(**kwargs)
        logging.debug(carla_settings)
        self.client.write_request_new_episode(carla_settings)

    def start_episode(self, **kwargs):
        """Start a default episode and launch the agent client"""
        self.request_new_episode(**kwargs)
        logging.debug('waiting for the scene description')
        data = self.client.read_scene_description()
        if data is None:
            raise RuntimeError("received empty scene description")
        number_of_start_spots = len(data.player_start_spots)
        if number_of_start_spots == 0:
            raise RuntimeError("received 0 player start spots")
        logging.debug('received %d player start locations', number_of_start_spots)
        if self.args.debug:
            for spot in data.player_start_spots:
                logging.debug(spot)

        logging.debug('sending a random episode start')
        self.client.write_episode_start(random.randint(0, max(0, number_of_start_spots - 1)))

        logging.debug('waiting for episode to be ready')
        data = self.client.read_episode_ready()
        if not data.ready:
            raise RuntimeError('received episode not ready')

        logging.info('connecting secondary clients')
        self.client.connect_agent_client()

    def loop_on_agent_client(self, iterations=100, autopilot=None, control=None):
        """
        Loop on receive measurements and send control. If autopilot or control
        are None they will be chosen at random.
        """
        if control is not None:
            autopilot = False
        if autopilot is None:
            autopilot = random.choice([True, False])

        reverse = (random.random() < 0.2)

        logging.info('running episode with %d iterations', iterations)

        for x in xrange(0, iterations):
            logging.debug('waiting for measurements')
            data = self.client.read_measurements()
            if not data:
                raise RuntimeError('received empty measurements')
            if not data.IsInitialized():
                raise RuntimeError('received non-initialized measurements')
            else:
                logging.debug('received valid measurements')
                number_of_agents = len(data.non_player_agents)
                logging.debug('received data of %d agents', number_of_agents)
                if number_of_agents > self.number_of_agents or number_of_agents + 10 < self.number_of_agents:
                    raise RuntimeError('received data for %d agents, but %d was requested' % (number_of_agents, self.number_of_agents))
                if self.args.debug:
                    for agent in data.non_player_agents:
                        logging.debug(agent)
            logging.debug('waiting for images')
            data = self.client.read_images()
            logging.debug('received %d bytes of images', len(data) if data is not None else 0)

            logging.debug('sending control')
            if autopilot:
                self.client.write_control(autopilot=True)
            else:
                if control is None:
                    self.client.write_control(
                        steer=random.uniform(-1.0, 1.0),
                        throttle=0.3,
                        reverse=reverse)
                else:
                    self.client.write_control(**control)
