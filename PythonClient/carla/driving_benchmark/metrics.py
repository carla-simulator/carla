# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.


import numpy as np
import math
import os

sldist = lambda c1, c2: math.sqrt((c2[0] - c1[0]) ** 2 + (c2[1] - c1[1]) ** 2)
flatten = lambda l: [item for sublist in l for item in sublist]


class Metrics(object):
    """
        The metrics class is made to take the driving measurements
        and calculate some specific performance metrics.

    """

    def __init__(self, parameters, dynamic_tasks):
        """
        Args
            parameters: A dictionary with the used parameters for checking how to count infractions
            dynamic_tasks: A list of the all dynamic tasks (That contain dynamic objects)
        """

        self._parameters = parameters
        self._parameters['dynamic_tasks'] = dynamic_tasks

    def _divide_by_episodes(self, measurements_matrix, header):

        """
            Divides the measurements matrix on different episodes.

            Args:
                measurements_matrix: The full measurements matrix
                header: The header from the measurements matrix

        """

        # Read previous for position zero
        prev_start = measurements_matrix[0, header.index('start_point')]
        prev_end = measurements_matrix[0, header.index('end_point')]
        prev_exp_id = measurements_matrix[0, header.index('exp_id')]

        # Start at the position 1.
        i = 1
        prev_i_position = 0
        episode_matrix_metrics = []

        while i < measurements_matrix.shape[0]:

            current_start = measurements_matrix[i, header.index('start_point')]
            current_end = measurements_matrix[i, header.index('end_point')]
            current_exp_id = measurements_matrix[i, header.index('exp_id')]

            # If there is a change in the position it means it is a new episode for sure.
            if (current_start != prev_start and current_end != prev_end) \
                    or current_exp_id != prev_exp_id:
                episode_matrix_metrics.append(measurements_matrix[prev_i_position:i, :])
                prev_i_position = i

            prev_start = current_start
            prev_end = current_end
            prev_exp_id = current_exp_id

            i += 1

        episode_matrix_metrics.append(measurements_matrix[prev_i_position:-1, :])

        return episode_matrix_metrics

    def _get_collisions(self, selected_matrix, header):
        """
            Get the number of collisions for pedestrians, vehicles or other
        Args:
            selected_matrix: The matrix with all the experiments summary
            header: The header , to know the positions of details


        """
        count_collisions_general = 0
        count_collisions_pedestrian = 0
        count_collisions_vehicle = 0
        i = 1
        # Computing general collisions
        while i < selected_matrix.shape[0]:
            if (selected_matrix[i, header.index('collision_other')]
                - selected_matrix[
                    (i - self._parameters['collision_other']['frames_skip']), header.index(
                        'collision_other')]) > \
                    self._parameters['collision_other']['threshold']:
                count_collisions_general += 1
                i += self._parameters['collision_other']['frames_recount']
            i += 1

        i = 1
        # Computing collisions for vehicles
        while i < selected_matrix.shape[0]:
            if (selected_matrix[i, header.index('collision_vehicles')]
                - selected_matrix[
                    (i - self._parameters['collision_vehicles']['frames_skip']), header.index(
                        'collision_vehicles')]) > \
                    self._parameters['collision_vehicles']['threshold']:
                count_collisions_vehicle += 1
                i += self._parameters['collision_vehicles']['frames_recount']
            i += 1

        i = 1

        # Computing the collisions for pedestrians
        while i < selected_matrix.shape[0]:
            if (selected_matrix[i, header.index('collision_pedestrians')]
                - selected_matrix[i - self._parameters['collision_pedestrians']['frames_skip'],
                                  header.index('collision_pedestrians')]) > \
                    self._parameters['collision_pedestrians']['threshold']:
                count_collisions_pedestrian += 1
                i += self._parameters['collision_pedestrians']['frames_recount']
            i += 1

        return count_collisions_general, count_collisions_vehicle, count_collisions_pedestrian

    def _get_distance_traveled(self, selected_matrix, header):
        """
            Compute the total distance travelled
        Args:
            selected_matrix: The matrix with all the experiments summary
            header: The header , to know the positions of details


        """

        prev_x = selected_matrix[0, header.index('pos_x')]
        prev_y = selected_matrix[0, header.index('pos_y')]

        i = 1
        acummulated_distance = 0

        while i < selected_matrix.shape[0]:
            x = selected_matrix[i, header.index('pos_x')]
            y = selected_matrix[i, header.index('pos_y')]

            acummulated_distance += sldist((x, y), (prev_x, prev_y))

            prev_x = x
            prev_y = y

            i += 1

        return acummulated_distance / (1000.0)

    def _get_out_of_road_lane(self, selected_matrix, header):

        """
            Check for the situations were the agent goes out of the road.
        Args:
            selected_matrix: The matrix with all the experiments summary
            header: The header , to know the positions of details


        """

        count_sidewalk_intersect = 0
        count_lane_intersect = 0

        i = 0

        while i < selected_matrix.shape[0]:

            if (selected_matrix[i, header.index('intersection_offroad')]
                - selected_matrix[(i - self._parameters['intersection_offroad']['frames_skip']),
                                  header.index('intersection_offroad')]) \
                    > self._parameters['intersection_offroad']['threshold']:
                count_sidewalk_intersect += 1
                i += self._parameters['intersection_offroad']['frames_recount']
            if i >= selected_matrix.shape[0]:
                break

            if (selected_matrix[i, header.index('intersection_otherlane')]
                - selected_matrix[(i - self._parameters['intersection_otherlane']['frames_skip']),
                                  header.index('intersection_otherlane')]) \
                    > self._parameters['intersection_otherlane']['threshold']:
                count_lane_intersect += 1
                i += self._parameters['intersection_otherlane']['frames_recount']

            i += 1

        return count_lane_intersect, count_sidewalk_intersect

    def compute(self, path):

        """
            Compute a dictionary containing the following metrics

            * Off Road Intersection: The number of times the agent goes out of the road.
             The intersection is only counted if the area of the vehicle outside
              of the road is bigger than a *threshold*.

            * Other Lane Intersection: The number of times the agent goes to the other
             lane. The intersection is only counted if the area of the vehicle on the
             other lane is bigger than a *threshold*.

            * Vehicle Collisions: The number of collisions with vehicles that have
              an impact bigger than a *threshold*.

            * Pedestrian Collisions: The number of collisions with pedestrians
             that have an impact bigger than a threshold.

            * General Collisions: The number of collisions with all other
            objects.


            Args:
                path: Path where the log files are.

        """

        with open(os.path.join(path, 'summary.csv'), "rU") as f:
            header = f.readline()
            header = header.split(',')
            header[-1] = header[-1][:-1]

        with open(os.path.join(path, 'measurements.csv'), "rU") as f:

            header_metrics = f.readline()
            header_metrics = header_metrics.split(',')
            header_metrics[-1] = header_metrics[-1][:-1]

        result_matrix = np.loadtxt(os.path.join(path, 'summary.csv'), delimiter=",", skiprows=1)

        # Corner Case: The presented test just had one episode
        if result_matrix.ndim == 1:
            result_matrix = np.expand_dims(result_matrix, axis=0)

        tasks = np.unique(result_matrix[:, header.index('exp_id')])

        all_weathers = np.unique(result_matrix[:, header.index('weather')])

        measurements_matrix = np.loadtxt(os.path.join(path, 'measurements.csv'), delimiter=",",
                                         skiprows=1)

        metrics_dictionary = {'episodes_completion': {w: [0] * len(tasks) for w in all_weathers},
                              'intersection_offroad': {w: [[] for i in range(len(tasks))] for w in
                                                       all_weathers},
                              'intersection_otherlane': {w: [[] for i in range(len(tasks))] for w in
                                                         all_weathers},
                              'collision_pedestrians': {w: [[] for i in range(len(tasks))] for w in
                                                        all_weathers},
                              'collision_vehicles': {w: [[] for i in range(len(tasks))] for w in
                                                     all_weathers},
                              'collision_other': {w: [[] for i in range(len(tasks))] for w in
                                                  all_weathers},
                              'episodes_fully_completed': {w: [0] * len(tasks) for w in
                                                           all_weathers},
                              'average_speed': {w: [0] * len(tasks) for w in all_weathers},
                              'driven_kilometers': {w: [0] * len(tasks) for w in all_weathers}
                              }

        for t in range(len(tasks)):
            experiment_results_matrix = result_matrix[
                result_matrix[:, header.index('exp_id')] == tasks[t]]

            weathers = np.unique(experiment_results_matrix[:, header.index('weather')])

            for w in weathers:

                experiment_results_matrix = result_matrix[
                    np.logical_and(result_matrix[:, header.index(
                        'exp_id')] == tasks[t], result_matrix[:, header.index('weather')] == w)]

                experiment_metrics_matrix = measurements_matrix[
                    np.logical_and(measurements_matrix[:, header_metrics.index(
                        'exp_id')] == float(tasks[t]),
                                   measurements_matrix[:, header_metrics.index('weather')] == float(
                                       w))]

                metrics_dictionary['episodes_fully_completed'][w][t] = \
                    experiment_results_matrix[:, header.index('result')].tolist()

                metrics_dictionary['episodes_completion'][w][t] = \
                    ((experiment_results_matrix[:, header.index('initial_distance')]
                      - experiment_results_matrix[:, header.index('final_distance')])
                     / experiment_results_matrix[:, header.index('initial_distance')]).tolist()

                # Now we divide the experiment metrics matrix

                episode_experiment_metrics_matrix = self._divide_by_episodes(
                    experiment_metrics_matrix, header_metrics)

                count = 0

                for episode_experiment_metrics in episode_experiment_metrics_matrix:

                    km_run_episodes = self._get_distance_traveled(
                        episode_experiment_metrics, header_metrics)
                    metrics_dictionary['driven_kilometers'][w][t] += km_run_episodes
                    metrics_dictionary['average_speed'][w][t] = \
                        km_run_episodes / (experiment_results_matrix[count,
                                                                     header.index(
                                                                         'final_time')] / 3600.0)
                    count += 1

                    lane_road = self._get_out_of_road_lane(
                        episode_experiment_metrics, header_metrics)

                    metrics_dictionary['intersection_otherlane'][
                        w][t].append(lane_road[0])
                    metrics_dictionary['intersection_offroad'][
                        w][t].append(lane_road[1])

                    if tasks[t] in set(self._parameters['dynamic_tasks']):

                        collisions = self._get_collisions(episode_experiment_metrics,
                                                          header_metrics)

                        metrics_dictionary['collision_pedestrians'][
                            w][t].append(collisions[2])
                        metrics_dictionary['collision_vehicles'][
                            w][t].append(collisions[1])
                        metrics_dictionary['collision_other'][
                            w][t].append(collisions[0])

                    else:

                        metrics_dictionary['collision_pedestrians'][
                            w][t].append(0)
                        metrics_dictionary['collision_vehicles'][
                            w][t].append(0)
                        metrics_dictionary['collision_other'][
                            w][t].append(0)

        return metrics_dictionary
