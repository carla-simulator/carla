
from carla.settings import CarlaSettings
from carla.sensor import Camera


class Experiment(object):

    def __init__(self, **kwargs):
        self.Id = ''
        self.Conditions = CarlaSettings()
        self.Poses = [[]]
        self.Repetitions = 1

        # self. ,vehicles,pedestrians,weather,cameras

    def set(self, **kwargs):
        for key, value in kwargs.items():
            if not hasattr(self, key):
                raise ValueError('Experiment: no key named %r' % key)
            setattr(self, key, value)

    @property
    def id(self):
        return self.Id

    @property
    def conditions(self):
        return self.Conditions

    @property
    def poses(self):
        return self.Poses

    @property
    def repetitions(self):
        return self.Repetitions
