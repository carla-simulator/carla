# CARLA, Copyright (C) 2017 Computer Vision Center (CVC)


"""CARLA Client."""


from tcp_client import TCPClient

import carla_server_pb2 as carla_protocol


class CarlaClient(object):
    def __init__(self, host, world_port, timeout):
        self._world_client = TCPClient(host, world_port, timeout)
        self._stream_client = TCPClient(host, world_port + 1, timeout)
        self._control_client = TCPClient(host, world_port + 2, timeout)

    def connect_world_client(self):
        self._world_client.connect()

    def disconnect_all(self):
        self.disconnect_agent_client()
        self._world_client.disconnect()

    def connect_agent_client(self):
        self._stream_client.connect()
        self._control_client.connect()

    def disconnect_agent_client(self):
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
        pb_message.steer = kwargs.get('steer', 0.0)
        pb_message.throttle = kwargs.get('throttle', 0.0)
        pb_message.brake = kwargs.get('brake', 0.0)
        pb_message.hand_brake = kwargs.get('hand_brake', False)
        pb_message.reverse = kwargs.get('reverse', False)
        pb_message.autopilot = kwargs.get('autopilot', False)
        self._control_client.write(pb_message.SerializeToString())
