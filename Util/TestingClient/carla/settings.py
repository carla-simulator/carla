# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB), and the INTEL Visual Computing Lab.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""CARLA Settings"""

import configparser
import io
import random


MAX_NUMBER_OF_WEATHER_IDS = 14


class Camera(object):
    """Camera description. To be added to CarlaSettings."""
    def __init__(self, name, **kwargs):
        self.CameraName = name
        self.PostProcessing = 'SceneFinal'
        self.ImageSizeX = 800
        self.ImageSizeY = 600
        self.CameraFOV = 90
        self.CameraPositionX = 15
        self.CameraPositionY = 0
        self.CameraPositionZ = 123
        self.CameraRotationPitch = 8
        self.CameraRotationRoll = 0
        self.CameraRotationYaw = 0
        self.set(**kwargs)

    def set(self, **kwargs):
        for key, value in kwargs.items():
            if not hasattr(self, key):
                raise ValueError('CarlaSettings.Camera: no key named %r' % key)
            setattr(self, key, value)

    def set_image_size(self, pixels_x, pixels_y):
        self.ImageSizeX = pixels_x
        self.ImageSizeY = pixels_y

    def set_position(self, x, y, z):
        self.CameraPositionX = x
        self.CameraPositionY = y
        self.CameraPositionZ = z

    def set_rotation(self, pitch, roll, yaw):
        self.CameraRotationPitch = pitch
        self.CameraRotationRoll = roll
        self.CameraRotationYaw = yaw


class CarlaSettings(object):
    """CARLA settings object. Convertible to str as CarlaSettings.ini."""
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

    def get_number_of_agents(self):
        if not self.SendNonPlayerAgentsInfo:
            return 0
        return self.NumberOfVehicles + self.NumberOfPedestrians

    def randomize_seeds(self):
        self.SeedVehicles = random.getrandbits(16)
        self.SeedPedestrians = random.getrandbits(16)

    def randomize_weather(self):
        self.WeatherId = random.randint(0, MAX_NUMBER_OF_WEATHER_IDS)

    def add_camera(self, camera):
        if not isinstance(camera, Camera):
            raise ValueError('Can only add instances of carla_settings.Camera')
        self._cameras.append(camera)

    def __str__(self):
        ini = configparser.ConfigParser()
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

        text = io.StringIO()
        ini.write(text)
        return text.getvalue().replace(' = ', '=')
