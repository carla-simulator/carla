# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

from carla.settings import CarlaSettings


class Experiment(object):

    def __init__(self):
        self.Id = ''
        self.Conditions = CarlaSettings()
        self.Poses = [[]]
        self.Repetitions = 1

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
