# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""CARLA Settings"""

import io
import random
import sys


if sys.version_info >= (3, 0):

    from configparser import ConfigParser

else:

    from ConfigParser import RawConfigParser as ConfigParser


from . import sensor as carla_sensor


MAX_NUMBER_OF_WEATHER_IDS = 14


class CarlaSettings(object):
    """
    The CarlaSettings object controls the settings of an episode.  The __str__
    method retrieves an str with a CarlaSettings.ini file contents.
    """

    def __init__(self, **kwargs):
        # [CARLA/Server]
        self.SynchronousMode = True
        self.SendNonPlayerAgentsInfo = False
        # [CARLA/QualitySettings]
        self.QualityLevel = 'Epic'
        # [CARLA/LevelSettings]
        self.PlayerVehicle = None
        self.NumberOfVehicles = 20
        self.NumberOfPedestrians = 30
        self.WeatherId = 1
        self.SeedVehicles = None
        self.SeedPedestrians = None
        self.DisableTwoWheeledVehicles = False
        self.set(**kwargs)
        self._sensors = []

    def set(self, **kwargs):
        for key, value in kwargs.items():
            if not hasattr(self, key):
                raise ValueError('CarlaSettings: no key named %r' % key)
            setattr(self, key, value)

    def randomize_seeds(self):
        """
        Randomize the seeds of the new episode's pseudo-random number
        generators.
        """
        self.SeedVehicles = random.getrandbits(16)
        self.SeedPedestrians = random.getrandbits(16)

    def randomize_weather(self):
        """Randomized the WeatherId."""
        self.WeatherId = random.randint(0, MAX_NUMBER_OF_WEATHER_IDS)

    def add_sensor(self, sensor):
        """Add a sensor to the player vehicle (see sensor.py)."""
        if not isinstance(sensor, carla_sensor.Sensor):
            raise ValueError('Sensor not supported')
        self._sensors.append(sensor)

    def __str__(self):
        """Converts this object to an INI formatted string."""
        ini = ConfigParser()
        ini.optionxform = str
        S_SERVER = 'CARLA/Server'
        S_QUALITY = 'CARLA/QualitySettings'
        S_LEVEL = 'CARLA/LevelSettings'
        S_SENSOR = 'CARLA/Sensor'

        def get_attribs(obj):
            return [a for a in dir(obj) if not a.startswith('_') and not callable(getattr(obj, a))]

        def add_section(section, obj, keys):
            for key in keys:
                if hasattr(obj, key) and getattr(obj, key) is not None:
                    if not ini.has_section(section):
                        ini.add_section(section)
                    ini.set(section, key, str(getattr(obj, key)))

        add_section(S_SERVER, self, [
            'SynchronousMode',
            'SendNonPlayerAgentsInfo'])
        add_section(S_QUALITY, self, [
            'QualityLevel'])
        add_section(S_LEVEL, self, [
            'NumberOfVehicles',
            'NumberOfPedestrians',
            'WeatherId',
            'SeedVehicles',
            'SeedPedestrians',
            'DisableTwoWheeledVehicles'])

        ini.add_section(S_SENSOR)
        ini.set(S_SENSOR, 'Sensors', ','.join(s.SensorName for s in self._sensors))

        for sensor_def in self._sensors:
            section = S_SENSOR + '/' + sensor_def.SensorName
            add_section(section, sensor_def, get_attribs(sensor_def))

        if sys.version_info >= (3, 0):
            text = io.StringIO()
        else:
            text = io.BytesIO()

        ini.write(text)
        return text.getvalue().replace(' = ', '=')
