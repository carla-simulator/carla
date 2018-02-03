#!/usr/bin/env python3

# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.


import csv
import datetime
import math
import os
import abc
import logging


from builtins import input as input_data


from carla.client import VehicleControl

def sldist(c1, c2):
    return math.sqrt((c2[0] - c1[0])**2 + (c2[1] - c1[1])**2)


class Benchmark(object):

    """
    The Benchmark class, controls the execution of the benchmark by an
    Agent class.
    The benchmark class must be inherited
    """
    def __init__(
            self,
            city_name,
            name_to_save,
            continue_experiment=False,
            save_images=False
            ):


        self.__metaclass__ = abc.ABCMeta

        self._city_name = city_name



        self._base_name = name_to_save
        self._dict_stats = {'exp_id': -1,
                            'rep': -1,
                            'weather': -1,
                            'start_point': -1,
                            'end_point': -1,
                            'result': -1,
                            'initial_distance': -1,
                            'final_distance': -1,
                            'final_time': -1,
                            'time_out': -1
                            }

        self._dict_rewards = {'exp_id': -1,
                              'rep': -1,
                              'weather': -1,
                              'collision_gen': -1,
                              'collision_ped': -1,
                              'collision_car': -1,
                              'lane_intersect': -1,
                              'sidewalk_intersect': -1,
                              'pos_x': -1,
                              'pos_y': -1
                              }


        self._experiments = self._build_experiments()
        # Create the log files and get the names
        self._suffix_name, self._full_name = self._create_log_record(name_to_save, self._experiments)
        # Get the line for the experiment to be continued
        self._line_on_file = self._continue_experiment(continue_experiment)



        self._save_images = save_images
        self._image_filename_format = os.path.join(
            self._full_name, '_images/episode_{:s}/{:s}/image_{:0>5d}.jpg')

    def run_navigation_episode(
            self,
            agent,
            carla,
            time_out,
            target,
            episode_name):

        measurements, sensor_data = carla.read_data()
        carla.send_control(VehicleControl())

        t0 = measurements.game_timestamp
        t1 = t0
        success = False
        measurement_vec = []
        frame = 0
        distance = 10000

        while(t1 - t0) < (time_out * 1000) and not success:
            measurements, sensor_data = carla.read_data()

            control = agent.run_step(measurements, sensor_data, target)

            logging.info("Controller is Inputting:")
            logging.info('Steer = %f Throttle = %f Brake = %f ',
                         control.steer, control.throttle, control.brake)

            carla.send_control(control)

            # measure distance to target
            if self._save_images:
                for name, image in sensor_data.items():
                    image.save_to_disk(self._image_filename_format.format(
                        episode_name, name, frame))

            curr_x = measurements.player_measurements.transform.location.x
            curr_y = measurements.player_measurements.transform.location.y

            measurement_vec.append(measurements.player_measurements)

            t1 = measurements.game_timestamp

            distance = sldist([curr_x, curr_y],
                              [target.location.x, target.location.y])

            logging.info('Status:')
            logging.info(
                '[d=%f] c_x = %f, c_y = %f ---> t_x = %f, t_y = %f',
                float(distance), curr_x, curr_y, target.location.x,
                 target.location.y)

            if distance < 200.0:
                success = True

            frame += 1

        if success:
            return 1, measurement_vec, float(t1 - t0) / 1000.0, distance
        return 0, measurement_vec, time_out, distance

    def benchmark_agent(self, agent, carla):

        if self._line_on_file == 0:
            # The fixed name considering all the experiments being run
            with open(os.path.join(self._full_name,
                                   self._suffix_name), 'w') as ofd:

                w = csv.DictWriter(ofd, self._dict_stats.keys())
                w.writeheader()

            with open(os.path.join(self._full_name,
                                   'details_' + self._suffix_name), 'w') as rfd:

                rw = csv.DictWriter(rfd, self._dict_rewards.keys())
                rw.writeheader()
            start_task = 0
            start_pose = 0
        else:
            (start_task, start_pose) = self._get_pose_and_task(self._line_on_file)

        logging.info(' START ')

        for experiment in self._experiments[start_task:]:

            positions = carla.load_settings(
                experiment.conditions).player_start_spots

            for pose in experiment.poses[start_pose:]:
                for rep in range(experiment.repetitions):

                    start_point = pose[0]
                    end_point = pose[1]

                    carla.start_episode(start_point)

                    logging.info('======== !!!! ==========')
                    logging.info(' Start Position %d End Position %d ',
                                 start_point, end_point)

                    path_distance = agent.get_distance(
                        positions[start_point], positions[end_point])
                    euclidean_distance = \
                        sldist([positions[start_point].location.x, positions[start_point].location.y],
                        [positions[end_point].location.x, positions[end_point].location.y])

                    time_out = self._calculate_time_out(path_distance)
                    # running the agent
                    (result, reward_vec, final_time, remaining_distance) = \
                        self.run_navigation_episode(
                            agent, carla, time_out, positions[end_point],
                            str(experiment.Conditions.WeatherId) + '_'
                        + str(experiment.id) + '_' + str(start_point)
                        + '.' + str(end_point))

                    # compute stats for the experiment

                    self._write_summary_results(
                        experiment, pose, rep, euclidean_distance,
                        remaining_distance, final_time, time_out, result)

                    self._write_details_results(experiment, rep, reward_vec)

                    if(result > 0):
                        logging.info('+++++ Target achieved in %f seconds! +++++',
                              final_time)
                    else:
                        logging.info('----- Timeout! -----')
        return self.get_all_statistics()

    def _write_summary_results(self, experiment, pose, rep,
                               path_distance, remaining_distance,
                               final_time, time_out, result):

        self._dict_stats['exp_id'] = experiment.id
        self._dict_stats['rep'] = rep
        self._dict_stats['weather'] = experiment.Conditions.WeatherId
        self._dict_stats['start_point'] = pose[0]
        self._dict_stats['end_point'] = pose[1]
        self._dict_stats['result'] = result
        self._dict_stats['initial_distance'] = path_distance
        self._dict_stats['final_distance'] = remaining_distance
        self._dict_stats['final_time'] = final_time
        self._dict_stats['time_out'] = time_out

        with open(os.path.join(self._full_name, self._suffix_name), 'a+') as ofd:

            w = csv.DictWriter(ofd, self._dict_stats.keys())

            w.writerow(self._dict_stats)

    def _write_details_results(self, experiment, rep, reward_vec):

        with open(os.path.join(self._full_name,
                               'details_' + self._suffix_name), 'a+') as rfd:

            rw = csv.DictWriter(rfd, self._dict_rewards.keys())

            for i in range(len(reward_vec)):
                self._dict_rewards['exp_id'] = experiment.id
                self._dict_rewards['rep'] = rep
                self._dict_rewards['weather'] = experiment.Conditions.WeatherId
                self._dict_rewards['collision_gen'] = reward_vec[
                    i].collision_other
                self._dict_rewards['collision_ped'] = reward_vec[
                    i].collision_pedestrians
                self._dict_rewards['collision_car'] = reward_vec[
                    i].collision_vehicles
                self._dict_rewards['lane_intersect'] = reward_vec[
                    i].intersection_otherlane
                self._dict_rewards['sidewalk_intersect'] = reward_vec[
                    i].intersection_offroad
                self._dict_rewards['pos_x'] = reward_vec[
                    i].transform.location.x
                self._dict_rewards['pos_y'] = reward_vec[
                    i].transform.location.y

                rw.writerow(self._dict_rewards)

    def _create_log_record(self, base_name, experiments):
        """
        This function creates the log files for the benchmark.

        """
        suffix_name = self._get_experiments_names(experiments)
        full_name = os.path.join('_benchmarks_results',
                                  base_name + '_'
                                  + self._get_details() + '/')

        folder = os.path.dirname(full_name)
        if not os.path.isdir(folder):
            os.makedirs(folder)

        # Make a date file: to show when this was modified,
        # the number of times the experiments were run
        now = datetime.datetime.now()
        open(os.path.join(full_name, now.strftime("%Y%m%d%H%M")),'w').close()

        return suffix_name, full_name


    def _continue_experiment(self, continue_experiment):

        if self._experiment_exist():

            if continue_experiment:
                line_on_file = self._get_last_position()

            else:
                # Ask question, to avoid mistaken override situations
                answer = input_data("The experiment was already found in the files"
                               + ", Do you want to continue (y/n)? \n"
                               )
                if answer == 'Yes' or answer == 'y':
                    line_on_file = self._get_last_position()
                else:
                    line_on_file = 0

        else:
            line_on_file = 0

        return line_on_file



    def _experiment_exist(self):
        return os.path.isfile(self._full_name)

    def _get_last_position(self):

        with open(os.path.join(self._full_name, self._suffix_name)) as f:
            return sum(1 for _ in f)


    # To be redefined on subclasses on how to calculate timeout for an episode
    @abc.abstractmethod
    def _calculate_time_out(self, distance):
        pass

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
    def _get_pose_and_task(self, line_on_file):
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
    @staticmethod
    def _get_experiments_names(experiments):

        name_cat = 'w'

        for experiment in experiments:

            name_cat += str(experiment.Conditions.WeatherId) + '.'

        return name_cat


