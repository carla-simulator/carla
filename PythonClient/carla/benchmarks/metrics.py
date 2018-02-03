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


def get_colisions(selected_matrix, header):

    count_gen = 0
    count_ped = 0
    count_car = 0
    i = 1

    while i < selected_matrix.shape[0]:
        if (selected_matrix[i, header.index('collision_gen')]
                - selected_matrix[(i-10), header.index('collision_gen')]) > 40000:
            count_gen += 1
            i += 20
        i += 1

    i = 1
    while i < selected_matrix.shape[0]:
        if (selected_matrix[i, header.index('collision_car')]
                - selected_matrix[(i-10), header.index('collision_car')]) > 40000:
            count_car += 1
            i += 30
        i += 1

    i = 1
    while i < selected_matrix.shape[0]:
        if (selected_matrix[i, header.index('collision_ped')]
                - selected_matrix[i-5, header.index('collision_ped')]) > 30000:
            count_ped += 1
            i += 100
        i += 1

    return count_gen, count_car, count_ped


def get_distance_traveled(selected_matrix, header):

    prev_x = selected_matrix[0, header.index('pos_x')]
    prev_y = selected_matrix[0, header.index('pos_y')]

    i = 1
    acummulated_distance = 0
    while i < selected_matrix.shape[0]:

        x = selected_matrix[i, header.index('pos_x')]
        y = selected_matrix[i, header.index('pos_y')]
        # Here we defined a maximun distance in a tick, this case 8 meters or 288km/h
        if sldist((x, y), (prev_x, prev_y)) < 800:
            acummulated_distance += sldist((x, y), (prev_x, prev_y))


        prev_x = x
        prev_y = y

        i += 1

    return float(acummulated_distance)/float(100*1000)





def get_out_of_road_lane(selected_matrix, header):

    count_road = 0
    count_lane = 0

    i = 0

    while i < selected_matrix.shape[0]:
        # print selected_matrix[i,6]
        if (selected_matrix[i, header.index('sidewalk_intersect')]
                - selected_matrix[(i-10), header.index('sidewalk_intersect')]) > 0.3:
            count_road += 1
            i += 20
        if i >= selected_matrix.shape[0]:
            break

        if (selected_matrix[i, header.index('lane_intersect')]
                - selected_matrix[(i-10), header.index('lane_intersect')]) > 0.4:
            count_lane += 1
            i += 20

        i += 1

    return count_lane, count_road



def compute_summary(filename, dynamic_episodes):

    # Separate the PATH and the basename
    path = os.path.dirname(filename)
    base_name = os.path.basename(filename)



    f = open(filename, "rb")
    header = f.readline()
    header = header.split(',')
    header[-1] = header[-1][:-2]
    f.close()

    f = open(os.path.join(path, 'details_' + base_name), "rb")
    header_details = f.readline()
    header_details = header_details.split(',')
    header_details[-1] = header_details[-1][:-2]
    f.close()

    data_matrix = np.loadtxt(open(filename, "rb"), delimiter=",", skiprows=1)

    # Corner Case: The presented test just had one episode
    if data_matrix.ndim == 1:
        data_matrix = np.expand_dims(data_matrix, axis=0)


    tasks = np.unique(data_matrix[:, header.index('exp_id')])

    all_weathers = np.unique(data_matrix[:, header.index('weather')])

    reward_matrix = np.loadtxt(open(os.path.join(
        path, 'details_' + base_name), "rb"), delimiter=",", skiprows=1)

    metrics_dictionary = {'average_completion': {w: [0.0]*len(tasks) for w in all_weathers},
                          'intersection_offroad': {w: [0.0]*len(tasks) for w in all_weathers},
                          'intersection_otherlane': {w: [0.0]*len(tasks) for w in all_weathers},
                          'collision_pedestrians': {w: [0.0]*len(tasks) for w in all_weathers},
                          'collision_vehicles': {w: [0.0]*len(tasks) for w in all_weathers},
                          'collision_other': {w: [0.0]*len(tasks) for w in all_weathers},
                          'average_fully_completed': {w: [0.0]*len(tasks) for w in all_weathers},
                          'average_speed': {w: [0.0]*len(tasks) for w in all_weathers},
                          'driven_kilometers': {w: [0.0]*len(tasks) for w in all_weathers}
                          }

    for t in tasks:
        task_data_matrix = data_matrix[
            data_matrix[:, header.index('exp_id')] == t]
        weathers = np.unique(task_data_matrix[:, header.index('weather')])



        for w in weathers:
            t = int(t)

            task_data_matrix = data_matrix[np.logical_and(data_matrix[:, header.index(
                'exp_id')] == t, data_matrix[:, header.index('weather')] == w)]


            task_reward_matrix = reward_matrix[np.logical_and(reward_matrix[:, header_details.index(
                'exp_id')] == float(t), reward_matrix[:, header_details.index('weather')] == float(w))]

            km_run = get_distance_traveled(
                task_reward_matrix, header_details)

            metrics_dictionary['average_fully_completed'][w][t] = sum(
                task_data_matrix[:, header.index('result')])/task_data_matrix.shape[0]

            metrics_dictionary['average_completion'][w][t] = sum(
                (task_data_matrix[:, header.index('initial_distance')]
                 - task_data_matrix[:, header.index('final_distance')])
                / task_data_matrix[:, header.index('initial_distance')]) \
                / len(task_data_matrix[:, header.index('final_distance')])


            metrics_dictionary['driven_kilometers'][w][t]= km_run
            metrics_dictionary['average_speed'][w][t]= km_run/ \
                ((sum(task_data_matrix[:, header.index('final_time')]))/3600.0)



            if list(tasks).index(t) in set(dynamic_episodes):

                lane_road = get_out_of_road_lane(
                    task_reward_matrix, header_details)
                colisions = get_colisions(task_reward_matrix, header_details)



                metrics_dictionary['intersection_offroad'][
                    w][t] = lane_road[0]/km_run
                metrics_dictionary['intersection_otherlane'][
                    w][t] = lane_road[1]/km_run
                metrics_dictionary['collision_pedestrians'][
                    w][t] = colisions[2]/km_run
                metrics_dictionary['collision_vehicles'][
                    w][t] = colisions[1]/km_run
                metrics_dictionary['collision_other'][
                    w][t] = colisions[0]/km_run


    return metrics_dictionary
