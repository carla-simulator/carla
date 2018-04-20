import os
import numpy as np
import json


def print_summary(metrics_summary, weathers, path):
    """
        We plot the summary of the testing for the set selected weathers.

        We take the raw data and print the way it was described on CORL 2017 paper

    """

    # Improve readability by adding a weather dictionary
    weather_name_dict = {1: 'Clear Noon', 3: 'After Rain Noon',
                         6: 'Heavy Rain Noon', 8: 'Clear Sunset',
                         4: 'Cloudy After Rain', 14: 'Soft Rain Sunset'}

    # First we write the entire dictionary on the benchmark folder.
    with open(os.path.join(path, 'metrics.json'), 'w') as fo:
        fo.write(json.dumps(metrics_summary))

    # Second we plot the metrics that are already ready by averaging

    metrics_to_average = [
        'episodes_fully_completed',
        'episodes_completion'

    ]
    # We compute the number  of episodes based on size of average completion
    number_of_episodes = len(list(metrics_summary['episodes_fully_completed'].items())[0][1])

    for metric in metrics_to_average:

        if metric == 'episodes_completion':
            print ("Average Percentage of Distance to Goal Travelled ")
        else:
            print ("Percentage of Successful Episodes")

        print ("")
        values = metrics_summary[metric]

        metric_sum_values = np.zeros(number_of_episodes)
        for weather, tasks in values.items():
            if weather in set(weathers):
                print('  Weather: ', weather_name_dict[weather])
                count = 0
                for t in tasks:
                    # if isinstance(t, np.ndarray) or isinstance(t, list):
                    if t == []:
                        print('    Metric Not Computed')
                    else:
                        print('    Task:', count, ' -> ', float(sum(t)) / float(len(t)))
                        metric_sum_values[count] += (float(sum(t)) / float(len(t))) * 1.0 / float(
                            len(weathers))

                    count += 1

        print ('  Average Between Weathers')
        for i in range(len(metric_sum_values)):
            print('    Task ', i, ' -> ', metric_sum_values[i])
        print ("")

    infraction_metrics = [
        'collision_pedestrians',
        'collision_vehicles',
        'collision_other',
        'intersection_offroad',
        'intersection_otherlane'

    ]

    # We need to collect the total number of kilometers for each task

    for metric in infraction_metrics:
        values_driven = metrics_summary['driven_kilometers']
        values = metrics_summary[metric]
        metric_sum_values = np.zeros(number_of_episodes)
        summed_driven_kilometers = np.zeros(number_of_episodes)

        if metric == 'collision_pedestrians':
            print ('Avg. Kilometers driven before a collision to a PEDESTRIAN')
        elif metric == 'collision_vehicles':
            print('Avg. Kilometers driven before a collision to a VEHICLE')
        elif metric == 'collision_other':
            print('Avg. Kilometers driven before a collision to a STATIC OBSTACLE')
        elif metric == 'intersection_offroad':
            print('Avg. Kilometers driven before going OUTSIDE OF THE ROAD')
        else:
            print('Avg. Kilometers driven before invading the OPPOSITE LANE')

        # print (zip(values.items(), values_driven.items()))
        for items_metric, items_driven in zip(values.items(), values_driven.items()):
            weather = items_metric[0]
            tasks = items_metric[1]
            tasks_driven = items_driven[1]

            if weather in set(weathers):
                print('  Weather: ', weather_name_dict[weather])
                count = 0
                for t, t_driven in zip(tasks, tasks_driven):
                    # if isinstance(t, np.ndarray) or isinstance(t, list):
                    if t == []:
                        print('Metric Not Computed')
                    else:
                        if sum(t) > 0:
                            print('    Task ', count, ' -> ', t_driven / float(sum(t)))
                        else:
                            print('    Task ', count, ' -> more than', t_driven)

                        metric_sum_values[count] += float(sum(t))
                        summed_driven_kilometers[count] += t_driven

                    count += 1
        print ('  Average Between Weathers')
        for i in range(len(metric_sum_values)):
            if metric_sum_values[i] == 0:
                print('    Task ', i, ' -> more than ', summed_driven_kilometers[i])
            else:
                print('    Task ', i, ' -> ', summed_driven_kilometers[i] / metric_sum_values[i])
        print ("")

    print("")
    print("")
