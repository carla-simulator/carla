#!/usr/bin/env python3

# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.


import math
import os
import abc
import logging


from .recording import Recording

from carla.client import VehicleControl

def sldist(c1, c2):
    return math.sqrt((c2[0] - c1[0])**2 + (c2[1] - c1[1])**2)


class Benchmark(object):

    """
    The Benchmark class, controls the execution of the benchmark by an
    Agent class.
    The benchmark class must be inherited with a class that defines the
    all the experiments to be run by the agent
    """
    def __init__(
            self,
            city_name='Town01',
            name_to_save='Test',
            continue_experiment=True,
            save_images=False,
            distance_for_success=2.0
            ):


        self.__metaclass__ = abc.ABCMeta

        self._city_name = city_name
        self._base_name = name_to_save

        # The minimum distance for arriving into the goal point in
        # order to consider ir a success
        self._distance_for_success = distance_for_success
        # build all the experiments to be evaluated, this function is redefined
        # on a benchmark derivate class
        self._experiments = self._build_experiments()
        # The object used to record the benchmark and to able to continue after
        self._recording = Recording(city_name=city_name, name_to_save=name_to_save
                                    , continue_experiment=continue_experiment
                                    , save_images=save_images)





    def _run_navigation_episode(
            self,
            agent,
            client,
            time_out,
            target,
            episode_name):

        """
         Run one episode of the benchmark (Pose) for a certain agent.


        Args:
            agent: the agent object
            client: an object of the carla client to communicate
            with the CARLA simulator
            time_out: the time limit to complete this episode
            target: the target to reach
            episode_name: The name for saving images of this episode

        """

        # Send an initial command.
        measurements, sensor_data = client.read_data()
        client.send_control(VehicleControl())

        initial_timestamp = measurements.game_timestamp
        current_timestamp = initial_timestamp

        # The vector containing all measurements produced on this episode
        measurement_vec = []
        # The vector containing all controls produced on this episode
        control_vec = []
        frame = 0
        distance = 10000
        success = False

        while(current_timestamp - initial_timestamp) < (time_out * 1000) and not success:

            # Read data from server with the client
            measurements, sensor_data = client.read_data()
            # The directions to reach the goal are calculated.
            directions = self._get_directions(measurements.player_measurements.transform, target)
            # Agent process the data.
            control = agent.run_step(measurements, sensor_data, directions, target)
            # Send the control commands to the vehicle
            client.send_control(control)

            # save images if the flag is activated
            self._recording.save_images(sensor_data, episode_name, frame)

            current_x = measurements.player_measurements.transform.location.x
            current_y = measurements.player_measurements.transform.location.y

            logging.info("Controller is Inputting:")
            logging.info('Steer = %f Throttle = %f Brake = %f ',
                           control.steer, control.throttle, control.brake)

            current_timestamp = measurements.game_timestamp
            distance = sldist([current_x, current_y],
                              [target.location.x, target.location.y])

            logging.info('Status:')
            logging.info(
                '[d=%f] c_x = %f, c_y = %f ---> t_x = %f, t_y = %f',
                float(distance), current_x, current_y, target.location.x,
                target.location.y)

            if distance < self._distance_for_success:
                success = True

            # Increment the vectors and append the measurements and controls.
            frame += 1
            measurement_vec.append(measurements.player_measurements)
            control_vec.append(control)

        if success:
            return 1, measurement_vec, control_vec, float(current_timestamp - initial_timestamp) / 1000.0, distance
        return 0, measurement_vec, control_vec, time_out, distance

    def benchmark_agent(self, agent, client):
        """
        Function to benchmark the agent.
        Check the log file for this benchmark.
        Continue from the experiment where it stopped.



        Args:
             agent:
             client:


        Return:
            A dictionary with all the metrics computed from the
            agent running the set of experiments.
        """


        # Function return the current pose and task for this benchmark.
        start_experiment, start_pose = self._recording.get_pose_and_experiment(
                                                       self._get_number_of_poses_task())

        logging.info('START')

        for experiment in self._experiments[start_experiment:]:

            positions = client.load_settings(
                experiment.conditions).player_start_spots


            self._recording.log_start()

            with open(self._internal_log_name, 'a+') as log:
                log.write('Start Experiment %d \n' % experiment.id)

            for pose in experiment.poses[start_pose:]:
                for rep in range(experiment.repetitions):

                    start_point = pose[0]
                    end_point = pose[1]

                    client.start_episode(start_point)

                    logging.info('======== !!!! ==========')
                    logging.info(' Start Position %d End Position %d ',
                                   start_point, end_point)


                    self._recording.log_poses()

                    with open(self._internal_log_name, 'a+') as log:
                        log.write(' Start Poses  (%d  %d ) on weather %d \n ' %
                                  (start_point, end_point, experiment.Conditions.WeatherId))



                    # Calculate the initial distance for this episode
                    euclidean_distance = \
                        sldist([positions[start_point].location.x, positions[start_point].location.y],
                        [positions[end_point].location.x, positions[end_point].location.y])

                    time_out = self._calculate_time_out(positions[start_point], positions[end_point])

                    # running the agent
                    (result, reward_vec, control_vec, final_time, remaining_distance) = \
                        self._run_navigation_episode(
                            agent, client, time_out, positions[end_point],
                            str(experiment.Conditions.WeatherId) + '_'
                            + str(experiment.id) + '_' + str(start_point)
                            + '.' + str(end_point))

                    # compute stats for the experiment

                    self._recording._write_summary_results(
                        experiment, pose, rep, euclidean_distance,
                        remaining_distance, final_time, time_out, result)

                    self._recording._write_details_results(experiment, rep, pose, reward_vec, control_vec)

                    with open(self._internal_log_name, 'a+') as log:
                        log.write('Finished Experiment')

                    if result > 0:
                        logging.info('+++++ Target achieved in %f seconds! +++++',
                                       final_time)
                    else:
                        logging.info('----- Timeout! -----')


        self._recording.log_end()
        with open(self._recording._internal_log_name, 'a+') as log:
            log.write('====== Finished Entire Benchmark ======')
        return self.get_all_statistics()



    # To be redefined on subclasses on how to calculate timeout for an episode


    @abc.abstractmethod
    def _calculate_time_out(self, start_point, end_point):
        pass

    @abc.abstractmethod
    def _get_number_of_poses_task(self):
        """
            Get the number of poses a task have for this benchmark
        """
        """
            Warning: assumes that all tasks have the same size
        """

    @abc.abstractmethod
    def _get_details(self):
        """
        Get details
        :return: a string with name and town of the subclass
        """
    @abc.abstractmethod
    def _build_experiments(self):
        """
        Returns a set of experiments to be evaluated
        Must be redefined in an inherited class.

        """

    @abc.abstractmethod
    def get_all_statistics(self):
        """
        Get the statistics of the evaluated experiments
        :return:
        """
    @abc.abstractmethod
    def _get_directions(self, current_point, end_point):
        """
        Class that should return the directions to reach a certain goal
        """

    @abc.abstractmethod
    def _get_pose_and_experiment(self, line_on_file):
        """
        Parse the experiment depending on number of poses and tasks
        """


    @abc.abstractmethod
    def plot_summary_train(self):
        """
        returns the summary for the train weather/task episodes

        """

    @abc.abstractmethod
    def plot_summary_test(self):
        """
        returns the summary for the test weather/task episodes

        """
