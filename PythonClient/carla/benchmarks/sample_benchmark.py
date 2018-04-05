# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# CORL experiment set.

from __future__ import print_function

import os
import numpy as np

from .benchmark import Benchmark
from .experiment import Experiment
from carla.sensor import Camera
from carla.settings import CarlaSettings

from carla.planner.planner import Planner

from .metrics import Metrics


class SampleBenchmark(Benchmark):

    def __init__(self,
                 city_name,
                 name_to_save,
                 continue_experiment=True,
                 save_images=False,
                 distance_for_success=2.0
                 ):

        # Define all the parameters used to compute the driving summary.

        # TODO: this should be taken from the default class

        metrics_parameters = {

            'intersection_offroad': {'frames_skip': 10,  # Check intersection always with 10 frames tolerance
                                     'frames_recount': 20,
                                     'threshold': 0.3
                                     },
            'intersection_otherlane': {'frames_skip': 10,  # Check intersection always with 10 frames tolerance
                                       'frames_recount': 20,
                                       'threshold': 0.4
                                       },
            'collision_other': {'frames_skip': 10,
                                  'frames_recount': 20,
                                  'threshold': 400
                                  },
            'collision_vehicles': {'frames_skip': 10,
                                   'frames_recount': 30,
                                   'threshold': 400
                                   },
            'collision_pedestrians': {'frames_skip': 5,
                                      'frames_recount': 100,
                                      'threshold': 300
                                      },
            'dynamic_episodes': [3]

        }

        self._metrics = Metrics(metrics_parameters)

        # All the weather used on this benchmark
        self._weathers = [1]


        # TODO: Simplify this initalization, Default parameters go to benchmark
        Benchmark.__init__(self, city_name,
                           name_to_save,
                           continue_experiment,
                           save_images,
                           distance_for_success)



    def get_number_of_poses_task(self):

        if self._city_name == 'Town01':
            return len(self._poses_town01()[0])
        else:
            return len(self._poses_town02()[0])


    def get_all_statistics(self): # Weird Function

        summary = self._metrics.compute(self._recording._path)

        return summary

    def plot_summary_train(self):

        self._plot_summary([1.0, 3.0, 6.0, 8.0])

    def plot_summary_test(self):

        self._plot_summary([4.0, 14.0])

    def _plot_summary(self, weathers):
        """
        We plot the summary of the testing for the set selected weathers.
        The test weathers are [4,14]

        """

        metrics_summary = self._metrics.compute(self._recording._path)

        print(" Final Results ! ")

        for metric, values in metrics_summary.items():

            print('Metric : ', metric)
            for weather, tasks in values.items():
                if weather in set(weathers):
                    print('  Weather: ', weather)
                    count = 0
                    for t in tasks:
                        if isinstance(t, np.ndarray) or isinstance(t, list):
                            if t == []:
                                print('Metric Not Computed')
                            else:
                                print('    Task ', count, ' -> ', sum(t) / len(t))
                        else:
                            print('    Task ', count, ' -> ', t)

                        count += 1



    def _calculate_time_out(self, start_point, end_point): #TODO this could come from base class
        """
        Function to return the timeout ( in miliseconds) that is calculated based on distance to goal.
        This is the same timeout as used on the CoRL paper.
        """
        path_distance = self._planner.get_shortest_path_distance(
            [start_point.location.x, start_point.location.y, 0]
            , [start_point.orientation.x, start_point.orientation.y, 0]
            , [end_point.location.x, end_point.location.y, 0]
            , [end_point.orientation.x, end_point.orientation.y, 0])

        return ((path_distance / 1000.0) / 10.0) * 3600.0 + 10.0

    def _poses_town01(self):
        """
            Each matrix is a new task. We have all the four tasks

        """

        def _poses_straight():
            return [[36, 40]]

        def _poses_one_curve():
            return [[138, 17]]

        def _poses_navigation():
            return [[105, 29]]

        return [_poses_straight(),
                _poses_one_curve(),
                _poses_navigation(),
                _poses_navigation()]

    def _poses_town02(self):

        def _poses_straight():
            return [[38, 34]]

        def _poses_one_curve():
            return [[37, 76]]

        def _poses_navigation():
            return [[19, 66]]

        return [_poses_straight(),
                _poses_one_curve(),
                _poses_navigation(),
                _poses_navigation()
                ]

    def _build_experiments(self):
        """
            Creates the whole set of experiment objects,
            The experiments created depends on the selected Town.

        """

        # We set the camera
        # This single RGB camera is used on every experiment

        camera = Camera('CameraRGB')
        camera.set(FOV=100)

        camera.set_image_size(800, 600)

        camera.set_position(2.0, 0.0, 1.4)
        camera.set_rotation(-15.0, 0, 0)

        if self._city_name == 'Town01':
            poses_tasks = self._poses_town01()
            vehicles_tasks = [0, 0, 0, 20]
            pedestrians_tasks = [0, 0, 0, 50]
        else:
            poses_tasks = self._poses_town02()
            vehicles_tasks = [0, 0, 0, 15]
            pedestrians_tasks = [0, 0, 0, 50]

        experiments_vector = []

        for weather in self._weathers:

            for iteration in range(len(poses_tasks)):
                poses = poses_tasks[iteration]
                vehicles = vehicles_tasks[iteration]
                pedestrians = pedestrians_tasks[iteration]

                conditions = CarlaSettings()
                conditions.set(
                    SynchronousMode=True,
                    SendNonPlayerAgentsInfo=True,
                    NumberOfVehicles=vehicles,
                    NumberOfPedestrians=pedestrians,
                    WeatherId=weather,
                    SeedVehicles=123456789,
                    SeedPedestrians=123456789
                )
                # Add all the cameras that were set for this experiments

                conditions.add_sensor(camera)

                experiment = Experiment()
                experiment.set(
                    Conditions=conditions,
                    Poses=poses,
                    Id=iteration,
                    Repetitions=1
                )
                experiments_vector.append(experiment)

        return experiments_vector

    def _get_details(self): #TODO Automatize to get values directly from the master class.

        # Function to get automatic information from the experiment for writing purposes
        return 'sample_' + self._city_name


