# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""CARLA Client."""

import logging
import struct

from contextlib import contextmanager

from . import sensor
from . import tcp
from . import util

try:
    from . import carla_server_pb2 as carla_protocol
except ImportError:
    raise RuntimeError('cannot import "carla_server_pb2.py", run the protobuf compiler to generate this file')

try:
    import numpy
except ImportError:
    raise RuntimeError('cannot import numpy, make sure numpy package is installed.')


VehicleControl = carla_protocol.Control


@contextmanager
def make_carla_client(host, world_port, timeout=15):
    """Context manager for creating and connecting a CarlaClient."""
    with util.make_connection(CarlaClient, host, world_port, timeout) as client:
        yield client


class CarlaClient(object):
    """The CARLA client. Manages communications with the CARLA server."""

    def __init__(self, host, world_port, timeout=15):
        self._world_client = tcp.TCPClient(host, world_port, timeout)
        self._stream_client = tcp.TCPClient(host, world_port + 1, timeout)
        self._control_client = tcp.TCPClient(host, world_port + 2, timeout)
        self._current_settings = None
        self._is_episode_requested = False
        self._sensors = {}

    def connect(self, connection_attempts=10):
        """
        Try to establish a connection to a CARLA server at the given host:port.
        """
        self._world_client.connect(connection_attempts)

    def disconnect(self):
        """Disconnect from server."""
        self._control_client.disconnect()
        self._stream_client.disconnect()
        self._world_client.disconnect()

    def connected(self):
        """Return whether there is an active connection."""
        return self._world_client.connected()

    def load_settings(self, carla_settings):
        """
        Load new settings and request a new episode based on these settings.
        carla_settings object must be convertible to a str holding the contents
        of a CarlaSettings.ini file.

        Return a protobuf object holding the scene description.
        """
        self._current_settings = carla_settings
        return self._request_new_episode(carla_settings)

    def start_episode(self, player_start_index):
        """
        Start the new episode at the player start given by the
        player_start_index. The list of player starts is retrieved by
        "load_settings".

        The new episode is started based on the last settings loaded by
        "load_settings".

        This function waits until the server answers with an EpisodeReady.
        """
        if self._current_settings is None:
            raise RuntimeError('no settings loaded, cannot start episode')

        # if no new settings are loaded, request new episode with previous
        if not self._is_episode_requested:
            self._request_new_episode(self._current_settings)

        try:
            pb_message = carla_protocol.EpisodeStart()
            pb_message.player_start_spot_index = player_start_index
            self._world_client.write(pb_message.SerializeToString())
            # Wait for EpisodeReady.
            data = self._world_client.read()
            if not data:
                raise RuntimeError('failed to read data from server')
            pb_message = carla_protocol.EpisodeReady()
            pb_message.ParseFromString(data)
            if not pb_message.ready:
                raise RuntimeError('cannot start episode: server failed to start episode')
            # We can start the agent clients now.
            self._stream_client.connect()
            self._control_client.connect()
            # Set again the status for no episode requested
        finally:
            self._is_episode_requested = False

    def read_data(self):
        """
        Read the data sent from the server this frame. The episode must be
        started. Return a pair containing the protobuf object containing the
        measurements followed by the raw data of the sensors.
        """
        # Read measurements.
        data = self._stream_client.read()
        if not data:
            raise RuntimeError('failed to read data from server')
        pb_message = carla_protocol.Measurements()
        pb_message.ParseFromString(data)
        # Read sensor data.
        return pb_message, dict(x for x in self._read_sensor_data())

    def send_control(self, *args, **kwargs):
        """
        Send the VehicleControl to be applied this frame.

        If synchronous mode was requested, the server will pause the simulation
        until this message is received.
        """
        if isinstance(args[0] if args else None, carla_protocol.Control):
            pb_message = args[0]
        else:
            pb_message = carla_protocol.Control()
            pb_message.steer = kwargs.get('steer', 0.0)
            pb_message.throttle = kwargs.get('throttle', 0.0)
            pb_message.brake = kwargs.get('brake', 0.0)
            pb_message.hand_brake = kwargs.get('hand_brake', False)
            pb_message.reverse = kwargs.get('reverse', False)
        self._control_client.write(pb_message.SerializeToString())

    def _request_new_episode(self, carla_settings):
        """
        Internal function to request a new episode. Prepare the client for a new
        episode by disconnecting agent clients.
        """
        # Disconnect agent clients.
        self._stream_client.disconnect()
        self._control_client.disconnect()
        # Send new episode request.
        pb_message = carla_protocol.RequestNewEpisode()
        pb_message.ini_file = str(carla_settings)
        self._world_client.write(pb_message.SerializeToString())
        # Read scene description.
        data = self._world_client.read()
        if not data:
            raise RuntimeError('failed to read data from server')
        pb_message = carla_protocol.SceneDescription()
        pb_message.ParseFromString(data)
        self._sensors = dict((sensor.id, sensor) \
            for sensor in _make_sensor_parsers(pb_message.sensors))
        self._is_episode_requested = True
        return pb_message

    def _read_sensor_data(self):
        while True:
            data = self._stream_client.read()
            if not data:
                raise StopIteration
            yield self._parse_sensor_data(data)

    def _parse_sensor_data(self, data):
        sensor_id = struct.unpack('<L', data[0:4])[0]
        parser = self._sensors[sensor_id]
        return parser.name, parser.parse_raw_data(data[4:])


def _make_sensor_parsers(sensors):
    image_types = ['None', 'SceneFinal', 'Depth', 'SemanticSegmentation']
    getimgtype = lambda id: image_types[id] if len(image_types) > id else 'Unknown'
    getint32 = lambda data, index: struct.unpack('<L', data[index*4:index*4+4])[0]
    getint64 = lambda data, index: struct.unpack('<Q', data[index*4:index*4+8])[0]
    getfloat = lambda data, index: struct.unpack('<f', data[index*4:index*4+4])[0]

    def parse_image(data):
        frame_number = getint64(data, 0)
        width = getint32(data, 2)
        height = getint32(data, 3)
        image_type = getimgtype(getint32(data, 4))
        fov = getfloat(data, 5)
        return sensor.Image(frame_number, width, height, image_type, fov, data[24:])

    def parse_lidar(data):
        frame_number = getint64(data, 0)
        horizontal_angle = getfloat(data, 2)
        channels = getint32(data, 3)
        header_size = 16
        point_count_by_channel = numpy.frombuffer(
            data[header_size:header_size+channels*4],
            dtype=numpy.dtype('uint32'))
        points = numpy.frombuffer(
            data[header_size+channels*4:],
            dtype=numpy.dtype('f4'))
        points = numpy.reshape(points, (int(points.shape[0]/3), 3))
        return sensor.LidarMeasurement(
            frame_number,
            horizontal_angle,
            channels,
            point_count_by_channel,
            sensor.PointCloud(frame_number, points))

    class SensorDefinition(object):
        def __init__(self, s):
            self.id = s.id
            self.name = s.name
            self.type = s.type
            self.parse_raw_data = lambda x: x

    for s in sensors:
        sensor_def = SensorDefinition(s)
        if sensor_def.type == carla_protocol.Sensor.CAMERA:
            sensor_def.parse_raw_data = parse_image
        elif sensor_def.type == carla_protocol.Sensor.LIDAR_RAY_CAST:
            sensor_def.parse_raw_data = parse_lidar
        else:
            logging.error('unknown sensor type %s', sensor_def.type)
        yield sensor_def
