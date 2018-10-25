# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

from carla.settings import CarlaSettings


class Experiment(object):
    """
    Experiment defines a certain task, under conditions
    A task is associated with a set of poses, containing start and end pose.

    Conditions are associated with a carla Settings and describe the following:

    Number Of Vehicles
    Number Of Pedestrians
    Weather
    Random Seed of the agents, describing their behaviour.

    """

    def __init__(self):
        self.Task = 0
        self.Conditions = CarlaSettings()
        self.Poses = [[]]
        self.Repetitions = 1

    def set(self, **kwargs):
        for key, value in kwargs.items():
            if not hasattr(self, key):
                raise ValueError('Experiment: no key named %r' % key)
            setattr(self, key, value)

        if self.Repetitions != 1:
            raise NotImplementedError()

    @property
    def task(self):
        return self.Task

    @property
    def conditions(self):
        return self.Conditions

    @property
    def poses(self):
        return self.Poses

    @property
    def repetitions(self):
        return self.Repetitions
