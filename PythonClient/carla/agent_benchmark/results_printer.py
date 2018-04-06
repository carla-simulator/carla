
def print_summary(metrics_summary, weathers, path):
    """
        We plot the summary of the testing for the set selected weathers.

        We take the raw data and print the way it was described on CORL 2017 paper

    """


    # First we write the entire dictionary on the benchmark folder.
    fout = os.path.join(self._recording._path, 'metrics.txt')
    fo = open(fout, "w")

    for k, v in metrics_summary.items():
        fo.write(str(k) + ' >>> ' + str(v) + '\n\n')

    fo.close()
    # Second we plot the metrics that are already ready by averaging

    metrics_to_average = [
        'average_fully_completed',
        'average_completion'

    ]

    for metric in metrics_to_average:

        if metric == 'average_completion':
            print ("Average Percentage of Distance to Goal Travelled ")
        else:
            print ("Percentage of Successful Episodes")

        print ("")
        values = metrics_summary[metric]

        metric_sum_values = np.zeros(len(self._poses_town01()))
        for weather, tasks in values.items():
            if weather in set(weathers):
                print('  Weather: ', self._weather_name_dict[weather])
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
        metric_sum_values = np.zeros(len(self._poses_town01()))
        summed_driven_kilometers = np.zeros(len(self._poses_town01()))

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
                print('  Weather: ', weather)
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
            print('    Task ', i, ' -> ', summed_driven_kilometers[i] / metric_sum_values[i])
        print ("")

    print("")
    print("")
