# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.


import numpy as np
import math
import os

import matplotlib.pyplot as plt


sldist = lambda c1, c2: math.sqrt((c2[0] - c1[0])**2 + (c2[1] - c1[1])**2)
flatten = lambda l: [item for sublist in l for item in sublist]


"""
The metrics code takes the driving details.
Based on that it calculates some specific metrics



"""



class Metrics(object):


    def __init__(self, parameters):
        """
        Args
            parameters: A dictionary with the used parameters for checking how to count infractions
        """

        self._parameters = parameters




    def _divide_by_episodes(self, metrics_matrix, header):

        prev_start = metrics_matrix[0, header.index('start_point')]
        prev_end = metrics_matrix[0, header.index('end_point')]
        prev_exp_id = metrics_matrix[0, header.index('exp_id')]

        # Start at the position 1.
        i = 1
        prev_i_position = 0
        episode_matrix_metrics = []

        while i < metrics_matrix.shape[0]:

            current_start = metrics_matrix[i, header.index('start_point')]
            current_end = metrics_matrix[i, header.index('end_point')]
            current_exp_id = metrics_matrix[i, header.index('exp_id')]

            # If there is a change in the position it means it is a new episode for sure.
            if (current_start != prev_start and current_end != prev_end) or current_exp_id != prev_exp_id:
                episode_matrix_metrics.append(metrics_matrix[prev_i_position:i, :])
                prev_i_position = i

            prev_start = current_start
            prev_end = current_end
            prev_exp_id = current_exp_id

            i += 1

        episode_matrix_metrics.append(metrics_matrix[prev_i_position:-1, :])

        return episode_matrix_metrics



    def _get_colisions(self, selected_matrix, header):

        count_collisions_general = 0
        count_collisions_pedestrian = 0
        count_collisions_vehicle = 0
        i = 1
        # Computing general collisions
        while i < selected_matrix.shape[0]:
            if (selected_matrix[i, header.index('collision_general')]
                    - selected_matrix[(i-self._parameters['collision_general']['frames_skip']), header.index('collision_general')]) > \
                    self._parameters['collision_general']['threshold']:
                count_collisions_general += 1
                i += self._parameters['collision_general']['frames_recount']
            i += 1

        i = 1
        # Computing collisions for vehicles
        while i < selected_matrix.shape[0]:
            if (selected_matrix[i, header.index('collision_vehicles')]
                    - selected_matrix[(i-self._parameters['collision_vehicles']['frames_skip']), header.index('collision_vehicles')]) > \
                    self._parameters['collision_vehicles']['threshold']:
                count_collisions_vehicle += 1
                i += self._parameters['collision_vehicles']['frames_recount']
            i += 1

        i = 1

        # Computing the collisions for pedestrians
        while i < selected_matrix.shape[0]:
            if (selected_matrix[i, header.index('collision_pedestrians')]
                    - selected_matrix[i-self._parameters['collision_pedestrians']['frames_skip'],
                                      header.index('collision_pedestrians')]) > \
                    self._parameters['collision_pedestrians']['threshold']:
                count_collisions_pedestrian += 1
                i += self._parameters['collision_pedestrians']['frames_recount']
            i += 1

        return count_collisions_general, count_collisions_vehicle, count_collisions_pedestrian


    def _get_distance_traveled(self, selected_matrix, header):

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

        return acummulated_distance/(1000.0)





    def _get_out_of_road_lane(self, selected_matrix, header):


        """
        Args:
            selected_matrix: The matrix with all the experiments details
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
        Arg:
            filename:

        """

        # Separate the PATH and the basename


        with open(os.path.join(path, 'summary.csv'), "r") as f:
            header = f.readline()
            header = header.split(',')
            header[-1] = header[-1][:-2]

        with  open(os.path.join(path, 'measurements.csv'), "r") as f:

            header_metrics = f.readline()
            header_metrics = header_metrics.split(',')
            header_metrics[-1] = header_metrics[-1][:-2]

        result_matrix = np.loadtxt(os.path.join(path, 'summary.csv'), delimiter=",", skiprows=1)

        # Corner Case: The presented test just had one episode
        if result_matrix.ndim == 1:
            result_matrix = np.expand_dims(result_matrix, axis=0)

        tasks = np.unique(result_matrix[:, header.index('exp_id')])


        all_weathers = np.unique(result_matrix[:, header.index('weather')])

        measurements_matrix = np.loadtxt(os.path.join(path, 'measurements.csv'), delimiter=",",
                                         skiprows=1)


        metrics_dictionary = {'average_completion': {w: [0]*len(tasks) for w in all_weathers},
                              'intersection_offroad': {w: [[] for i in range(len(tasks))] for w in all_weathers},
                              'intersection_otherlane': {w: [[] for i in range(len(tasks))] for w in all_weathers},
                              'collision_pedestrians': {w: [[] for i in range(len(tasks))] for w in all_weathers},
                              'collision_vehicles': {w: [[] for i in range(len(tasks))] for w in all_weathers},
                              'collision_general': {w: [[] for i in range(len(tasks))] for w in all_weathers},
                              'average_fully_completed': {w: [0]*len(tasks) for w in all_weathers},
                              'average_speed': {w: [0]*len(tasks) for w in all_weathers},
                              'driven_kilometers': {w: [0]*len(tasks) for w in all_weathers}
                              }


        for t in range(len(tasks)):
            experiment_results_matrix = result_matrix[
                result_matrix[:, header.index('exp_id')] == tasks[t]]

            weathers = np.unique(experiment_results_matrix[:, header.index('weather')])


            for w in weathers:

                experiment_results_matrix = result_matrix[np.logical_and(result_matrix[:, header.index(
                    'exp_id')] == tasks[t], result_matrix[:, header.index('weather')] == w)]

                experiment_metrics_matrix = measurements_matrix[np.logical_and(measurements_matrix[:, header_metrics.index(
                    'exp_id')] == float(tasks[t]), measurements_matrix[:, header_metrics.index('weather')] == float(w))]


                metrics_dictionary['average_fully_completed'][w][t] = \
                        experiment_results_matrix[:, header.index('result')]



                metrics_dictionary['average_completion'][w][t] = sum(
                    (experiment_results_matrix[:, header.index('initial_distance')]
                     - experiment_results_matrix[:, header.index('final_distance')])
                    / experiment_results_matrix[:, header.index('initial_distance')]) \
                    / len(experiment_results_matrix[:, header.index('final_distance')])



                # Now we divide the experiment metrics matrix

                episode_experiment_metrics_matrix = self._divide_by_episodes(experiment_metrics_matrix, header_metrics)


                count = 0


                for episode_experiment_metrics in episode_experiment_metrics_matrix:


                    km_run_episodes = self._get_distance_traveled(
                        episode_experiment_metrics, header_metrics)
                    metrics_dictionary['driven_kilometers'][w][t] = km_run_episodes
                    metrics_dictionary['average_speed'][w][t] =\
                            km_run_episodes\
                            /(experiment_results_matrix[count,
                                                      header.index('final_time')]/3600.0)
                    count += 1



                    lane_road = self._get_out_of_road_lane(
                        episode_experiment_metrics, header_metrics)


                    metrics_dictionary['intersection_otherlane'][
                        w][t].append(lane_road[0])
                    metrics_dictionary['intersection_offroad'][
                        w][t].append(lane_road[1])




                    if tasks[t] in set(self._parameters['dynamic_episodes']):


                        colisions = self._get_colisions(episode_experiment_metrics, header_metrics)

                        metrics_dictionary['collision_pedestrians'][
                            w][t].append(colisions[2])
                        metrics_dictionary['collision_vehicles'][
                            w][t].append(colisions[1])
                        metrics_dictionary['collision_general'][
                            w][t].append(colisions[0])

                    else:

                        metrics_dictionary['collision_pedestrians'][
                            w][t].append(0)
                        metrics_dictionary['collision_vehicles'][
                            w][t].append(0)
                        metrics_dictionary['collision_general'][
                            w][t].append(0)



        return metrics_dictionary
