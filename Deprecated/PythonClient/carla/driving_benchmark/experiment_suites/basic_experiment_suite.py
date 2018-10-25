# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.


from __future__ import print_function

from carla.driving_benchmark.experiment import Experiment
from carla.sensor import Camera
from carla.settings import CarlaSettings

from .experiment_suite import ExperimentSuite


class BasicExperimentSuite(ExperimentSuite):

    @property
    def train_weathers(self):
        return [1]

    @property
    def test_weathers(self):
        return [1]

    def build_experiments(self):
        """
            Creates the whole set of experiment objects,
            The experiments created depends on the selected Town.

        """

        # We check the town, based on that we define the town related parameters
        # The size of the vector is related to the number of tasks, inside each
        # task there is also multiple poses ( start end, positions )
        if self._city_name == 'Town01':
            poses_tasks = [[[7, 3]], [[138, 17]], [[140, 134]], [[140, 134]]]
            vehicles_tasks = [0, 0, 0, 20]
            pedestrians_tasks = [0, 0, 0, 50]
        else:
            poses_tasks = [[[4, 2]], [[37, 76]], [[19, 66]], [[19, 66]]]
            vehicles_tasks = [0, 0, 0, 15]
            pedestrians_tasks = [0, 0, 0, 50]

        # We set the camera
        # This single RGB camera is used on every experiment

        camera = Camera('CameraRGB')
        camera.set(FOV=100)
        camera.set_image_size(800, 600)
        camera.set_position(2.0, 0.0, 1.4)
        camera.set_rotation(-15.0, 0, 0)

        # Based on the parameters, creates a vector with experiment objects.
        experiments_vector = []
        for weather in self.weathers:

            for iteration in range(len(poses_tasks)):
                poses = poses_tasks[iteration]
                vehicles = vehicles_tasks[iteration]
                pedestrians = pedestrians_tasks[iteration]

                conditions = CarlaSettings()
                conditions.set(
                    SendNonPlayerAgentsInfo=True,
                    NumberOfVehicles=vehicles,
                    NumberOfPedestrians=pedestrians,
                    WeatherId=weather

                )
                # Add all the cameras that were set for this experiments
                conditions.add_sensor(camera)
                experiment = Experiment()
                experiment.set(
                    Conditions=conditions,
                    Poses=poses,
                    Task=iteration,
                    Repetitions=1
                )
                experiments_vector.append(experiment)

        return experiments_vector
