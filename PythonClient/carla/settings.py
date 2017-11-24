# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB), and the INTEL Visual Computing Lab.
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
        # [CARLA/LevelSettings]
        self.PlayerVehicle = None
        self.NumberOfVehicles = 20
        self.NumberOfPedestrians = 30
        self.WeatherId = -1
        self.SeedVehicles = None
        self.SeedPedestrians = None
        self.randomize_weather()
        self.set(**kwargs)
        self._cameras = []

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
        if isinstance(sensor, carla_sensor.Camera):
            self._cameras.append(sensor)
        else:
            raise ValueError('Sensor not supported')

    def __str__(self):
        """Converts this object to an INI formatted string."""
        ini = ConfigParser()
        ini.optionxform=str
        S_SERVER = 'CARLA/Server'
        S_LEVEL = 'CARLA/LevelSettings'
        S_CAPTURE = 'CARLA/SceneCapture'

        def add_section(section, obj, keys):
            for key in keys:
                if hasattr(obj, key) and getattr(obj, key) is not None:
                    if not ini.has_section(section):
                        ini.add_section(section)
                    ini.set(section, key, str(getattr(obj, key)))

        add_section(S_SERVER, self, [
            'SynchronousMode',
            'SendNonPlayerAgentsInfo'])
        add_section(S_LEVEL, self, [
            'NumberOfVehicles',
            'NumberOfPedestrians',
            'WeatherId',
            'SeedVehicles',
            'SeedPedestrians'])

        ini.add_section(S_CAPTURE)
        ini.set(S_CAPTURE, 'Cameras', ','.join(c.CameraName for c in self._cameras))

        for camera in self._cameras:
            add_section(S_CAPTURE + '/' + camera.CameraName, camera, [
                'PostProcessing',
                'ImageSizeX',
                'ImageSizeY',
                'CameraFOV',
                'CameraPositionX',
                'CameraPositionY',
                'CameraPositionZ',
                'CameraRotationPitch',
                'CameraRotationRoll',
                'CameraRotationYaw'])

        if sys.version_info >= (3, 0):
            text = io.StringIO()
        else:
            text = io.BytesIO()

        ini.write(text)
        return text.getvalue().replace(' = ', '=')


def _get_sensor_names(settings):
    """
    Return a list with the names of the sensors defined in the settings object.
    The settings object can be a CarlaSettings or an INI formatted string.
    """
    if isinstance(settings, CarlaSettings):
        return [camera.CameraName for camera in settings._cameras]
    ini = ConfigParser()
    if sys.version_info >= (3, 0):
        ini.readfp(io.StringIO(settings))
    else:
        ini.readfp(io.BytesIO(settings))

    section_name = 'CARLA/SceneCapture'
    option_name = 'Cameras'

    if ini.has_section(section_name) and ini.has_option(section_name, option_name):
        cameras = ini.get(section_name, option_name)
        return cameras.split(',')
    return []
