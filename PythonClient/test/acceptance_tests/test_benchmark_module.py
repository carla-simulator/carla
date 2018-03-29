# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# CORL experiment set.

from __future__ import print_function

import os
import numpy as np

from carla.benchmarks.benchmark import Benchmark
from carla.benchmarks.experiment import Experiment
from carla.sensor import Camera
from carla.settings import CarlaSettings

from carla.benchmarks.metrics import compute_summary


class BenchmarkTest(Benchmark):

    def __init__(self,
                 city_name,
                 name_to_save,
                 continue_experiment=True,
                 save_images=False,
                 distance_for_success=2.0
                 ):

        # Define all the parameters used to compute the driving summary.

        self._metrics_parameters = {

            'intersection_offroad': {'frames_skip': 10,  # Check intersection always with 10 frames tolerance
                                     'frames_recount': 20,
                                     'threshold': 0.3
                                     },
            'intersection_otherlane': {'frames_skip': 10,  # Check intersection always with 10 frames tolerance
                                       'frames_recount': 20,
                                       'threshold': 0.4
                                       },
            'collision_general': {'frames_skip': 10,
                                  'frames_recount': 20,
                                  'threshold': 40
                                  },
            'collision_vehicles': {'frames_skip': 10,
                                   'frames_recount': 30,
                                   'threshold': 40
                                   },
            'collision_pedestrians': {'frames_skip': 5,
                                      'frames_recount': 100,
                                      'threshold': 30
                                      },
            'dynamic_episodes': [3]

        }

        Benchmark.__init__(self, city_name,
                           name_to_save,
                           continue_experiment,
                           save_images,
                           distance_for_success)

    def get_all_statistics(self):

        summary = compute_summary(os.path.join(
            self._full_name, self._suffix_name), self._metrics_parameters)

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

        metrics_summary = compute_summary(os.path.join(
            self._full_name, self._suffix_name), self._metrics_parameters)

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



    def _calculate_time_out(self, distance):
        """
        Function to return the timeout ( in miliseconds) that is calculated based on distance to goal.
        This is the same timeout as used on the CoRL paper.
        """

        return ((distance / 1000.0) / 10.0) * 3600.0 + 10.0

    def _poses_town01(self):
        """
        Each matrix is a new task. We have all the four tasks

        """

        def _poses_straight():
            return [[36, 40], [39, 35]]

        def _poses_one_curve():
            return [[138, 17], [47, 16]]

        def _poses_navigation():
            return [[105, 29], [27, 130]]

        return [_poses_straight(),
                _poses_one_curve(),
                _poses_navigation(),
                _poses_navigation()]

    def _poses_town02(self):

        def _poses_straight():
            return [[38, 34], [4, 2]]

        def _poses_one_curve():
            return [[37, 76], [8, 24]]

        def _poses_navigation():
            return [[19, 66], [79, 14]]

        return [_poses_straight(),
                _poses_one_curve(),
                _poses_navigation(),
                _poses_navigation()
                ]

    def _build_experiments(self):
        """
        Creates the whole set of experiment objects,
        The experiments created depend on the selected Town.


        """

        # We set the camera
        # This single RGB camera is used on every experiment

        camera = Camera('CameraRGB')
        camera.set(FOV=100)

        camera.set_image_size(800, 600)

        camera.set_position(2.0, 0.0, 1.4)
        camera.set_rotation(-15.0, 0, 0)

        weathers = [1, 3, 6, 8, 4, 14]
        if self._city_name == 'Town01':
            poses_tasks = self._poses_town01()
            vehicles_tasks = [0, 0, 0, 20]
            pedestrians_tasks = [0, 0, 0, 50]
        else:
            poses_tasks = self._poses_town02()
            vehicles_tasks = [0, 0, 0, 15]
            pedestrians_tasks = [0, 0, 0, 50]

        experiments_vector = []

        for weather in weathers:

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

    def _get_details(self):

        # Function to get automatic information from the experiment for writing purposes
        return 'corl2017_' + self._city_name

    def _get_pose_and_task(self, line_on_file):
        """
        Returns the pose and task this experiment is, based on the line it was
        on the log file.
        """
        """
            Warning: assumes that all tasks have the same size
        """
        if self._city_name == 'Town01':
            return line_on_file / len(self._poses_town01()[0]), line_on_file % len(self._poses_town01()[0])
        else:
            return line_on_file / len(self._poses_town01()[0]), line_on_file % len(self._poses_town02()[0])


import argparse
import logging
import time

from carla.benchmarks.agent import Agent
from carla.benchmarks.corl_2017 import CoRL2017

from carla.client import make_carla_client, VehicleControl
from carla.tcp import TCPConnectionError



class Manual(Agent):
    """
    Sample redefinition of the Agent,
    An agent that goes straight
    """
    def run_step(self, measurements, sensor_data, target):
        control = VehicleControl()
        control.throttle = 0.9

        return control


if __name__ == '__main__':

    argparser = argparse.ArgumentParser(description=__doc__)
    argparser.add_argument(
        '-v', '--verbose',
        action='store_true',
        dest='verbose',
        help='print some extra status information')
    argparser.add_argument(
        '-db', '--debug',
        action='store_true',
        dest='debug',
        help='print debug information')
    argparser.add_argument(
        '--host',
        metavar='H',
        default='localhost',
        help='IP of the host server (default: localhost)')
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)')
    argparser.add_argument(
        '-c', '--city-name',
        metavar='C',
        default='Town01',
        help='The town that is going to be used on benchmark'
        + '(needs to match active town in server, options: Town01 or Town02)')
    argparser.add_argument(
        '-n', '--log_name',
        metavar='T',
        default='test',
        help='The name of the log file to be created by the benchmark'
        )

    args = argparser.parse_args()
    if args.debug:
        log_level = logging.DEBUG
    elif args.verbose:
        log_level = logging.INFO
    else:
        log_level = logging.WARNING

    logging.basicConfig(format='%(levelname)s: %(message)s', level=log_level)

    logging.info('listening to server %s:%s', args.host, args.port)


    while True:
        try:

            with make_carla_client(args.host, args.port) as client:

                corl = BenchmarkTest(city_name=args.city_name, name_to_save=args.log_name+'_metrics')
                agent = Manual(args.city_name)
                results = corl.benchmark_agent(agent, client)
                corl.plot_summary_test()
                corl.plot_summary_train()

                break

        except TCPConnectionError as error:
            logging.error(error)
            time.sleep(1)

