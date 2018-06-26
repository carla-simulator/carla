import os
import unittest
from carla.driving_benchmark.recording import Recording


class testRecording(unittest.TestCase):

    def test_init(self):
        """
            The recording should have a reasonable full name
        """

        recording = Recording(name_to_save='Test1',
                              continue_experiment=False, save_images=True)

        _ = open(os.path.join(recording._path, 'summary.csv'), 'r')
        _ = open(os.path.join(recording._path, 'measurements.csv'), 'r')

        # There should be three files in any newly created case
        self.assertEqual(len(os.listdir(recording._path)), 3)

    def test_write_summary_results(self):
        """
            Test writting summary results.
        """
        from carla.driving_benchmark.experiment import Experiment

        recording = Recording(name_to_save='Test1',
                              continue_experiment=False, save_images=True)

        recording.write_summary_results(experiment=Experiment(), pose=[24, 32], rep=1,
                                        path_distance=200, remaining_distance=0,
                                        final_time=0.2, time_out=49, result=1)

        with open(os.path.join(recording._path, 'summary.csv'), 'r') as f:
            header = f.readline().split(',')
            # Assert if header is header
            self.assertIn('exp_id', header)

            self.assertEqual(len(header), len(recording._dict_summary))
            # Assert if there is something writen in the row

            written_row = f.readline().split(',')

            # Assert if the number of collums is correct
            self.assertEqual(len(written_row), len(recording._dict_summary))

    def test_write_summary_results_good_order(self):
        """
            Test if the summary results are writen in the same order on a new process
        """

        from carla.driving_benchmark.experiment import Experiment

        recording = Recording(name_to_save='Test_good_order',
                              continue_experiment=False, save_images=True)

        for _ in range(0, 10):
            recording.write_summary_results(experiment=Experiment(), pose=[24, 32], rep=1,
                                            path_distance=200, remaining_distance=0,
                                            final_time=0.2, time_out=49, result=1)

        recording = Recording(name_to_save='Test_good_order',
                              continue_experiment=True, save_images=True)

        for _ in range(0, 10):
            recording.write_summary_results(experiment=Experiment(), pose=[24, 32], rep=1,
                                            path_distance=200, remaining_distance=0,
                                            final_time=0.2, time_out=49, result=1)

        recording = Recording(name_to_save='Test_good_order',
                              continue_experiment=True, save_images=True)

        for _ in range(0, 10):
            recording.write_summary_results(experiment=Experiment(), pose=[24, 32], rep=1,
                                            path_distance=200, remaining_distance=0,
                                            final_time=0.2, time_out=49, result=1)

        recording = Recording(name_to_save='Test_good_order',
                              continue_experiment=True, save_images=True)

        for _ in range(0, 10):
            recording.write_summary_results(experiment=Experiment(), pose=[24, 32], rep=1,
                                            path_distance=200, remaining_distance=0,
                                            final_time=0.2, time_out=49, result=1)

        # Check if the the test_good_order summaries have all the same files.

    def test_write_measurements_results(self):
        """
            Test writing a few measurements into the log
        """

        import os
        from carla.driving_benchmark.experiment import Experiment
        from carla.carla_server_pb2 import Measurements
        from carla.carla_server_pb2 import Control

        recording = Recording(name_to_save='Test1',
                              continue_experiment=False, save_images=True)

        reward_vec = [Measurements().player_measurements for _ in range(20)]
        control_vec = [Control() for _ in range(25)]

        recording.write_measurements_results(experiment=Experiment(),
                                             rep=1, pose=[24, 32], reward_vec=reward_vec,
                                             control_vec=control_vec)

        with open(os.path.join(recording._path, 'measurements.csv'), 'r') as f:
            header = f.readline().split(',')
            # Assert if header is header
            self.assertIn('exp_id', header)

            self.assertEqual(len(header), len(recording._dict_measurements))
            # Assert if there is something writen in the row

            written_row = f.readline().split(',')

            # Assert if the number of collums is correct
            self.assertEqual(len(written_row), len(recording._dict_measurements))

    def test_continue_experiment(self):
        """
            Test if you are able to continue an experiment after restarting the process
        """
        recording = Recording(name_to_save='Test1',
                              continue_experiment=False, save_images=True)

        # A just started case should return the continue experiment case
        self.assertEqual(recording._continue_experiment(True)[1], 1)
        # If you don't want to continue, should return also one
        self.assertEqual(recording._continue_experiment(False)[1], 1)

        from carla.driving_benchmark.experiment import Experiment

        recording.write_summary_results(experiment=Experiment(), pose=[24, 32], rep=1,
                                        path_distance=200, remaining_distance=0,
                                        final_time=0.2, time_out=49, result=1)
        recording.write_summary_results(experiment=Experiment(), pose=[24, 32], rep=1,
                                        path_distance=200, remaining_distance=0,
                                        final_time=0.2, time_out=49, result=1)

        # After writing two experiments it should return 2, so you could start writing os pos 3
        self.assertEqual(recording._continue_experiment(True)[1], 3)
        # If you dont want to continue, should return also one
        self.assertEqual(recording._continue_experiment(False)[1], 1)

    def test_get_pose_and_experiment(self):
        """
            Test getting the pose and the experiment from a previous executed benchmark
        """

        recording = Recording(name_to_save='Test1',
                              continue_experiment=False, save_images=True)

        from carla.driving_benchmark.experiment import Experiment

        pose, experiment = recording.get_pose_and_experiment(25)

        # An starting experiment should return zero zero

        self.assertEqual(pose, 0)
        self.assertEqual(experiment, 0)

        recording.write_summary_results(experiment=Experiment(), pose=[24, 32], rep=1,
                                        path_distance=200, remaining_distance=0,
                                        final_time=0.2, time_out=49, result=1)
        recording.write_summary_results(experiment=Experiment(), pose=[24, 32], rep=1,
                                        path_distance=200, remaining_distance=0,
                                        final_time=0.2, time_out=49, result=1)

        pose, experiment = recording.get_pose_and_experiment(25)
        self.assertEqual(pose, 2)
        self.assertEqual(experiment, 0)

        for _ in range(23):
            recording.write_summary_results(experiment=Experiment(), pose=[24, 32], rep=1,
                                            path_distance=200, remaining_distance=0,
                                            final_time=0.2, time_out=49, result=1)

        pose, experiment = recording.get_pose_and_experiment(25)
        self.assertEqual(pose, 0)
        self.assertEqual(experiment, 1)

        for _ in range(23):
            recording.write_summary_results(experiment=Experiment(), pose=[24, 32], rep=1,
                                            path_distance=200, remaining_distance=0,
                                            final_time=0.2, time_out=49, result=1)

        pose, experiment = recording.get_pose_and_experiment(25)
        self.assertEqual(pose, 23)
        self.assertEqual(experiment, 1)

    def test_get_pose_and_experiment_corner(self):
        """
            Test getting the pose from multiple cases.
        """
        from carla.driving_benchmark.experiment import Experiment

        recording = Recording(name_to_save='Test1',
                              continue_experiment=False, save_images=True)

        pose, experiment = recording.get_pose_and_experiment(1)

        # An starting experiment should return one

        self.assertEqual(pose, 0)
        self.assertEqual(experiment, 0)

        pose, experiment = recording.get_pose_and_experiment(2)
        self.assertEqual(pose, 0)
        self.assertEqual(experiment, 0)

        recording.write_summary_results(experiment=Experiment(), pose=[24, 32], rep=1,
                                        path_distance=200, remaining_distance=0,
                                        final_time=0.2, time_out=49, result=1)

        pose, experiment = recording.get_pose_and_experiment(1)

        print(pose, experiment)
        self.assertEqual(pose, 0)
        self.assertEqual(experiment, 1)

        pose, experiment = recording.get_pose_and_experiment(2)

        print(pose, experiment)
        # An starting experiment should return one
        self.assertEqual(pose, 1)
        self.assertEqual(experiment, 0)

        pose, experiment = recording.get_pose_and_experiment(3)

        print(pose, experiment)
        # An starting experiment should return one
        self.assertEqual(pose, 1)
        self.assertEqual(experiment, 0)

        recording.write_summary_results(experiment=Experiment(), pose=[24, 32], rep=1,
                                        path_distance=200, remaining_distance=0,
                                        final_time=0.2, time_out=49, result=1)

        pose, experiment = recording.get_pose_and_experiment(2)

        self.assertEqual(pose, 0)
        self.assertEqual(experiment, 1)

        pose, experiment = recording.get_pose_and_experiment(3)

        self.assertEqual(pose, 2)
        self.assertEqual(experiment, 0)


if __name__ == '__main__':
    unittest.main()
