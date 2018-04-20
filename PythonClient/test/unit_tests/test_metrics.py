import os
import numpy as np
import unittest
from carla.driving_benchmark.metrics import Metrics
from carla.driving_benchmark.recording import Recording



def sum_matrix(matrix):
    # Line trick to reduce sum a matrix in one line
    return sum(sum(matrix, []))


class testMetrics(unittest.TestCase):

    def __init__(self, *args, **kwargs):
        super(testMetrics, self).__init__(*args, **kwargs)

        self._metrics_parameters = {

            'intersection_offroad': {'frames_skip': 10,  # Check intersection always with 10 frames tolerance
                                     'frames_recount': 20,
                                     'threshold': 0.3
                                     },
            'intersection_otherlane': {'frames_skip': 10,  # Check intersection always with 10 frames tolerance
                                       'frames_recount': 20,
                                       'threshold': 0.4
                                       },
            'collision_other': {'frames_skip': 10,
                                  'frames_recount': 20,
                                  'threshold': 400
                                  },
            'collision_vehicles': {'frames_skip': 10,
                                   'frames_recount': 30,
                                   'threshold': 400
                                   },
            'collision_pedestrians': {'frames_skip': 5,
                                      'frames_recount': 100,
                                      'threshold': 300
                                      },
            'dynamic_episodes': [3]

        }



    def _generate_test_case(self, poses_to_test):



        recording = Recording(name_to_save='TestMetric'
                              , continue_experiment=False, save_images=True
                              )



        from carla.driving_benchmark.experiment import Experiment
        from carla.carla_server_pb2 import Measurements
        from carla.carla_server_pb2 import Control


        for pose in poses_to_test:
            experiment = Experiment()

            recording.write_summary_results(experiment=experiment, pose=pose, rep=1,
                                            path_distance=200, remaining_distance=0,
                                            final_time=0.2, time_out=49, result=1)


            reward_vec = [Measurements().player_measurements for x in range(25)]
            control_vec = [Control() for x in range(25)]

            recording.write_measurements_results(experiment=experiment,
                                                 rep=1, pose=pose, reward_vec=reward_vec,
                                                 control_vec=control_vec)



        return recording._path

    def test_init(self):

        # Metric should instantiate with parameters
        Metrics(self._metrics_parameters,[3])



    def test_divide_by_episodes(self):


        metrics_obj = Metrics(self._metrics_parameters,[3])

        poses_to_test = [[24, 32], [34, 36], [54, 67]]
        path = self._generate_test_case(poses_to_test)

        # We start by reading the summary header file and the measurements header file.
        with open(os.path.join(path, 'summary.csv'), "r") as f:
            header = f.readline()
            header = header.split(',')
            header[-1] = header[-1][:-2]



        with  open(os.path.join(path,'measurements.csv'), "r") as f:

            header_metrics = f.readline()
            header_metrics = header_metrics.split(',')
            header_metrics[-1] = header_metrics[-1][:-2]


        result_matrix = np.loadtxt(os.path.join(path, 'summary.csv'), delimiter=",", skiprows=1)

        # Corner Case: The presented test just had one episode
        if result_matrix.ndim == 1:
            result_matrix = np.expand_dims(result_matrix, axis=0)


        tasks = np.unique(result_matrix[:, header.index('exp_id')])


        all_weathers = np.unique(result_matrix[:, header.index('weather')])

        measurements_matrix = np.loadtxt(os.path.join(path, 'measurements.csv'), delimiter=",", skiprows=1)


        episodes = metrics_obj._divide_by_episodes(measurements_matrix,header_metrics)


        self.assertEqual(len(episodes),3)



    def test_compute(self):

        # This is is the last one, generate many cases, corner cases, to be tested.

        metrics_obj = Metrics(self._metrics_parameters,[3])


        # Lets start testing a general file, not from a real run
        # The case is basically an empty case
        poses_to_test = [[24, 32], [34, 36], [54, 67]]
        path = self._generate_test_case(poses_to_test)



        summary_dict = metrics_obj.compute(path)


        number_of_colisions_vehicles = sum_matrix(summary_dict['collision_vehicles'][1.0])
        number_of_colisions_general = sum_matrix(summary_dict['collision_other'][1.0])
        number_of_colisions_pedestrians = sum_matrix(summary_dict['collision_pedestrians'][1.0])
        number_of_intersection_offroad = sum_matrix(summary_dict['intersection_offroad'][1.0])
        number_of_intersection_otherlane = sum_matrix(summary_dict['intersection_otherlane'][1.0])



        self.assertEqual(number_of_colisions_vehicles, 0)
        self.assertEqual(number_of_colisions_general, 0)
        self.assertEqual(number_of_colisions_pedestrians, 0)
        self.assertEqual(number_of_intersection_offroad, 0)
        self.assertEqual(number_of_intersection_otherlane, 0)


        # Now lets make a collision test on a premade file

        path = 'test/unit_tests/test_data/testfile_collisions'

        summary_dict = metrics_obj.compute(path)

        number_of_colisions_vehicles = sum_matrix(summary_dict['collision_vehicles'][3.0])
        number_of_colisions_general = sum_matrix(summary_dict['collision_other'][3.0])
        number_of_colisions_pedestrians = sum_matrix(summary_dict['collision_pedestrians'][3.0])
        number_of_intersection_offroad = sum_matrix(summary_dict['intersection_offroad'][3.0])
        number_of_intersection_otherlane = sum_matrix(summary_dict['intersection_otherlane'][3.0])



        self.assertEqual(number_of_colisions_vehicles, 2)
        self.assertEqual(number_of_colisions_general, 9)
        self.assertEqual(number_of_colisions_pedestrians, 0)
        self.assertEqual(number_of_intersection_offroad, 1)
        self.assertEqual(number_of_intersection_otherlane, 3)







