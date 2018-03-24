# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.


import numpy as np
import math
import os


sldist = lambda c1, c2: math.sqrt((c2[0] - c1[0])**2 + (c2[1] - c1[1])**2)
flatten = lambda l: [item for sublist in l for item in sublist]


"""
The metrics code takes the driving details.
Based on that it calculates some specific metrics



"""


def divide_by_episodes(metrics_matrix, header):


    prev_x = metrics_matrix[0, header.index('pos_x')]
    prev_y = metrics_matrix[0, header.index('pos_y')]

    i = 1
    prev_i_position = 0

    episode_matrix_metrics = []

    while i < metrics_matrix.shape[0]:

        x = metrics_matrix[i, header.index('pos_x')]
        y = metrics_matrix[i, header.index('pos_y')]
        # Here we defined a maximum distance in a tick, this case 7 meters, if it is bigger


        if sldist((x, y), (prev_x, prev_y)) > 7:
            episode_matrix_metrics.append(metrics_matrix[prev_i_position:i, :])
            prev_i_position = i

        prev_x = x
        prev_y = y

        i += 1


    episode_matrix_metrics.append(metrics_matrix[prev_i_position:-1, :])

    return episode_matrix_metrics



def get_colisions(selected_matrix, header, parameters):

    count_collisions_general = 0
    count_collisions_pedestrian = 0
    count_collisions_vehicle = 0
    i = 1

    while i < selected_matrix.shape[0]:
        if (selected_matrix[i, header.index('collision_gen')]
                - selected_matrix[(i-parameters['collision_general']['frames_skip']), header.index('collision_gen')]) > \
                parameters['collision_general']['threshold']:
            count_collisions_general += 1
            i +=  parameters['collision_general']['frames_recount']
        i += 1

    i = 1
    while i < selected_matrix.shape[0]:
        if (selected_matrix[i, header.index('collision_car')]
                - selected_matrix[(i-parameters['collision_vehicles']['frames_skip']), header.index('collision_car')]) > \
                parameters['collision_vehicles']['threshold']:
            count_collisions_vehicle += 1
            i += parameters['collision_vehicles']['frames_recount']
        i += 1

    i = 1
    while i < selected_matrix.shape[0]:
        if (selected_matrix[i, header.index('collision_ped')]
                - selected_matrix[i-parameters['collision_pedestrians']['frames_skip'],
                                  header.index('collision_ped')]) > \
                parameters['collision_pedestrians']['threshold']:
            count_collisions_pedestrian += 1
            i += parameters['collision_pedestrians']['frames_recount']
        i += 1

    return count_collisions_general, count_collisions_vehicle, count_collisions_pedestrian


def get_distance_traveled(selected_matrix, header):

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





def get_out_of_road_lane(selected_matrix, header, parameters):


    """

    :param selected_matrix: The matrix with all the experiments details
    :param header: The header , to know the positions of details
    :param parameters:
    :return:
    """

    count_sidewalk_intersect = 0
    count_lane_intersect = 0

    i = 0

    while i < selected_matrix.shape[0]:
        # print selected_matrix[i,6]
        if (selected_matrix[i, header.index('sidewalk_intersect')]
            - selected_matrix[(i-parameters['intersection_offroad']['frames_skip']),
                                header.index('sidewalk_intersect')]) \
                > parameters['intersection_offroad']['threshold']:

            count_sidewalk_intersect += 1
            i += parameters['intersection_offroad']['frames_recount']
        if i >= selected_matrix.shape[0]:
            break

        if (selected_matrix[i, header.index('lane_intersect')]
            - selected_matrix[(i-parameters['intersection_otherlane']['frames_skip']),
                                 header.index('lane_intersect')]) \
                > parameters['intersection_otherlane']['threshold']:
            count_lane_intersect += 1
            i += parameters['intersection_otherlane']['frames_recount']

        i += 1

    return count_lane_intersect, count_sidewalk_intersect



def compute_summary(filename, parameters):

    """

    :param filename:
    :param dynamic_episodes:
    :param parameterss:
    :return:
    """

    # Separate the PATH and the basename
    path = os.path.dirname(filename)
    base_name = os.path.basename(filename)



    f = open(filename, "rb")
    header = f.readline()
    header = header.split(',')
    header[-1] = header[-1][:-2]
    f.close()

    f = open(os.path.join(path, 'details_' + base_name), "rb")
    header_metrics = f.readline()
    header_metrics = header_metrics.split(',')
    header_metrics[-1] = header_metrics[-1][:-2]
    f.close()

    result_matrix = np.loadtxt(open(filename, "rb"), delimiter=",", skiprows=1)

    # Corner Case: The presented test just had one episode
    if result_matrix.ndim == 1:
        result_matrix = np.expand_dims(result_matrix, axis=0)


    tasks = np.unique(result_matrix[:, header.index('exp_id')])


    all_weathers = np.unique(result_matrix[:, header.index('weather')])

    metrics_matrix = np.loadtxt(open(os.path.join(
        path, 'details_' + base_name), "rb"), delimiter=",", skiprows=1)

    metrics_dictionary = {'average_completion': {w: [0]*len(tasks) for w in all_weathers},
                          'intersection_offroad': {w: [[] for i in range(len(tasks))] for w in all_weathers},
                          'intersection_otherlane': {w: [[] for i in range(len(tasks))] for w in all_weathers},
                          'collision_pedestrians': {w: [[] for i in range(len(tasks))] for w in all_weathers},
                          'collision_vehicles': {w: [[] for i in range(len(tasks))] for w in all_weathers},
                          'collision_other': {w: [[] for i in range(len(tasks))] for w in all_weathers},
                          'average_fully_completed': {w: [0]*len(tasks) for w in all_weathers},
                          'average_speed': {w: [0]*len(tasks) for w in all_weathers},
                          'driven_kilometers': {w: [0]*len(tasks) for w in all_weathers}
                          }

    for t in tasks:
        experiment_results_matrix = result_matrix[
            result_matrix[:, header.index('exp_id')] == t]
        weathers = np.unique(experiment_results_matrix[:, header.index('weather')])




        for w in weathers:
            t = int(t)

            experiment_results_matrix = result_matrix[np.logical_and(result_matrix[:, header.index(
                'exp_id')] == t, result_matrix[:, header.index('weather')] == w)]


            experiment_metrics_matrix = metrics_matrix[np.logical_and(metrics_matrix[:, header_metrics.index(
                'exp_id')] == float(t), metrics_matrix[:, header_metrics.index('weather')] == float(w))]






            metrics_dictionary['average_fully_completed'][w][t] = \
                    experiment_results_matrix[:, header.index('result')]


            metrics_dictionary['average_completion'][w][t] = sum(
                (experiment_results_matrix[:, header.index('initial_distance')]
                 - experiment_results_matrix[:, header.index('final_distance')])
                / experiment_results_matrix[:, header.index('initial_distance')]) \
                / len(experiment_results_matrix[:, header.index('final_distance')])



            # Now we divide the experiment metrics matrix


            episode_experiment_metrics_matrix = divide_by_episodes(experiment_metrics_matrix, header_metrics)

            count = 0


            for episode_experiment_metrics in episode_experiment_metrics_matrix:

                km_run_episodes = get_distance_traveled(
                    episode_experiment_metrics, header_metrics)
                metrics_dictionary['driven_kilometers'][w][t] = km_run_episodes
                metrics_dictionary['average_speed'][w][t] =\
                        km_run_episodes\
                        /(experiment_results_matrix[count,
                                                  header.index('final_time')]/3600.0)
                count += 1


            if list(tasks).index(t) in set(parameters['dynamic_episodes']):

                for episode_experiment_metrics in episode_experiment_metrics_matrix:
                    lane_road = get_out_of_road_lane(
                        episode_experiment_metrics, header_metrics, parameters)
                    colisions = get_colisions(episode_experiment_metrics, header_metrics, parameters)


                    metrics_dictionary['intersection_offroad'][
                        w][t].append(lane_road[0])
                    metrics_dictionary['intersection_otherlane'][
                        w][t].append(lane_road[1])
                    metrics_dictionary['collision_pedestrians'][
                        w][t].append(colisions[2])
                    metrics_dictionary['collision_vehicles'][
                        w][t].append(colisions[1])
                    metrics_dictionary['collision_other'][
                        w][t].append(colisions[0])


    return metrics_dictionary
