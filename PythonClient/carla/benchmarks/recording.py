

import csv
import datetime
import os

class Recording(object):
    def __init__(self
                 , city_name
                 , name_to_save
                 , continue_experiment
                 , save_images):


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

        self._dict_measurements = {'exp_id': -1,
                                   'rep': -1,
                                   'weather': -1,
                                   'start_point': -1,
                                   'end_point': -1,
                                   'collision_general': -1,
                                   'collision_pedestrians': -1,
                                   'collision_vehicles': -1,
                                   'intersection_otherlane': -1,
                                   'intersection_offroad': -1,
                                   'pos_x': -1,
                                   'pos_y': -1,
                                   'steer': -1,
                                   'throttle': -1,
                                   'brake': -1
                                   }

        # Create the log files and get the names
        self._suffix_name, self._full_name = self._create_log_record(name_to_save, self._experiments)
        # Get the line for the experiment to be continued
        self._line_on_file = self._continue_experiment(continue_experiment)


        # A log with a date file: to show when was the last access and log what was tested,

        now = datetime.datetime.now()
        self._internal_log_name = os.path.join(self._full_name, 'log_' + now.strftime("%Y%m%d%H%M"))

        self._save_images = save_images
        self._image_filename_format = os.path.join(
            self._full_name, '_images/episode_{:s}/{:s}/image_{:0>5d}.jpg')



    """
        Methods to log status of a certain benchmark execution.
        This serves to keep track of which episodes were in fact benchmarked.
    """


    def log_poses(self, start_index, end_index, weather_id):
        with open(self._internal_log_name, 'a+') as log:
            log.write(' Start Poses  (%d  %d ) on weather %d \n ' %
                      (start_index, end_index, weather_id))

    def log_poses_finish(self):
        with open(self._internal_log_name, 'a+') as log:
            log.write('Finished Experiment')

    def log_start(self, id_experiment):

        with open(self._internal_log_name, 'a+') as log:
            log.write('Start Experiment %d \n' % id_experiment)

    def log_end(self):
        with open(self._recording._internal_log_name, 'a+') as log:
            log.write('====== Finished Entire Benchmark ======')


    """
        Methods to record the measurements, sensors,
        controls and status of the entire benchmark.

    """

    def write_summary_results(self, experiment, pose, rep,
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


    def write_details_results(self, experiment, rep, pose, reward_vec, control_vec):
        with open(os.path.join(self._full_name,
                               'details_' + self._suffix_name), 'a+') as rfd:
            rw = csv.DictWriter(rfd, self._dict_measurements.keys())

            for i in range(len(reward_vec)):
                self._dict_measurements['exp_id'] = experiment.id
                self._dict_measurements['rep'] = rep
                self._dict_measurements['start_point'] = pose[0]
                self._dict_measurements['end_point'] = pose[1]
                self._dict_measurements['weather'] = experiment.Conditions.WeatherId
                self._dict_measurements['collision_general'] = reward_vec[
                    i].collision_other
                self._dict_measurements['collision_pedestrians'] = reward_vec[
                    i].collision_pedestrians
                self._dict_measurements['collision_vehicles'] = reward_vec[
                    i].collision_vehicles
                self._dict_measurements['intersection_otherlane'] = reward_vec[
                    i].intersection_otherlane
                self._dict_measurements['intersection_offroad'] = reward_vec[
                    i].intersection_offroad
                self._dict_measurements['pos_x'] = reward_vec[
                    i].transform.location.x
                self._dict_measurements['pos_y'] = reward_vec[
                    i].transform.location.y
                self._dict_measurements['steer'] = control_vec[
                    i].steer
                self._dict_measurements['throttle'] = control_vec[
                    i].throttle
                self._dict_measurements['brake'] = control_vec[
                    i].brake

                rw.writerow(self._dict_measurements)


    def _create_log_record(self, base_name, experiments):
        """
        This function creates the log files for the benchmark.

        """
        suffix_name = self._get_experiments_names(experiments)
        full_name = os.path.join('_benchmarks_results',
                                 base_name + '_'
                                 + self._get_details() + '/')

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


    def save_images(self, sensor_data, episode_name, frame):
        if self._save_images:
            for name, image in sensor_data.items():
                image.save_to_disk(self._image_filename_format.format(
                    episode_name, name, frame))


    def get_pose_and_experiment(self,number_poses_task):
        """
            Based on the line in log file, return the current pose and experiment.
            If the line is zero, create new log files.


        """
        """
            Warning: assumes that all tasks have the same size
        """


        if self._line_on_file == 0:
            # The fixed name considering all the experiments being run
            with open(os.path.join(self._full_name,
                                   self._suffix_name), 'w') as ofd:

                w = csv.DictWriter(ofd, self._dict_stats.keys())
                w.writeheader()

            with open(os.path.join(self._full_name,
                                   'details_' + self._suffix_name), 'w') as rfd:

                rw = csv.DictWriter(rfd, self._dict_measurements.keys())
                rw.writeheader()
            return 0, 0
        else:

            return self._line_on_file / number_poses_task, self._line_on_file % number_poses_task



    def _experiment_exist(self):
        return os.path.exists(self._full_name)


    def _get_last_position(self):
        """
            Get the last position on the experiment file
            With this you are able to continue from there
        :return int, position:
        """

        # Try to open, if the file is not found
        try:
            with open(os.path.join(self._full_name, self._suffix_name)) as f:
                return sum(1 for _ in f)
        except IOError:
            return 0




    @staticmethod
    def _get_experiments_names(experiments):

        name_cat = 'w'

        for experiment in experiments:

            name_cat += str(experiment.Conditions.WeatherId) + '.'

        return name_cat

