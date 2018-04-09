
import unittest
from carla.driving_benchmark.recording import Recording

class testRecording(unittest.TestCase):


    def test_init(self):
        import os

        """
            The recording should have a reasonable full name

        """


        recording = Recording(name_to_save='Test1'
                              , continue_experiment=False, save_images=True
                              )


        _ = open(os.path.join(recording._path,'summary.csv'), 'r')
        _ = open(os.path.join(recording._path, 'measurements.csv'), 'r')

        # There should be three files in any newly created case
        self.assertEqual(len(os.listdir(recording._path)), 3)


    def test_write_summary_results(self):

        import os
        from carla.driving_benchmark.experiment import Experiment

        recording = Recording(name_to_save='Test1'
                              , continue_experiment=False, save_images=True
                              )

        recording.write_summary_results( experiment=Experiment(), pose=[24,32], rep=1,
                                         path_distance=200, remaining_distance=0,
                                         final_time=0.2, time_out=49, result=1)

        with open(os.path.join(recording._path, 'summary.csv'), 'r') as f:

            header = f.readline().split(',')
            #Assert if header is header
            self.assertIn('exp_id', header)

            self.assertEqual(len(header), len(recording._dict_summary))
            #Assert if there is something writen in the row

            written_row = f.readline().split(',')

            #Assert if the number of collums is correct
            self.assertEqual(len(written_row), len(recording._dict_summary))



    def teste_write_measurements_results(self):

        import os
        from carla.driving_benchmark.experiment import Experiment
        from carla.carla_server_pb2 import Measurements
        from carla.carla_server_pb2 import Control


        recording = Recording(name_to_save='Test1'
                              , continue_experiment=False, save_images=True
                              )


        reward_vec = [Measurements().player_measurements for x in range(20)]
        control_vec = [Control() for x in range(25)]

        recording.write_measurements_results(experiment=Experiment(),
                                             rep=1, pose=[24, 32], reward_vec=reward_vec,
                                             control_vec=control_vec)

        with open(os.path.join(recording._path, 'measurements.csv'), 'r') as f:

            header = f.readline().split(',')
            #Assert if header is header
            self.assertIn('exp_id', header)

            self.assertEqual(len(header), len(recording._dict_measurements))
            #Assert if there is something writen in the row

            written_row = f.readline().split(',')

            #Assert if the number of collums is correct
            self.assertEqual(len(written_row), len(recording._dict_measurements))


    def test_continue_experiment(self):

        recording = Recording( name_to_save='Test1'
                              , continue_experiment=False, save_images=True
                              )

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



        recording = Recording( name_to_save='Test1'
                              , continue_experiment=False, save_images=True
                              )



        from carla.driving_benchmark.experiment import Experiment


        pose, experiment = recording.get_pose_and_experiment(25)

        # An starting experiment should return zero zero

        self.assertEqual(pose, 0)
        self.assertEqual(experiment, 0)


        recording.write_summary_results( experiment=Experiment(), pose=[24,32], rep=1,
                                         path_distance=200, remaining_distance=0,
                                         final_time=0.2, time_out=49, result=1)
        recording.write_summary_results( experiment=Experiment(), pose=[24,32], rep=1,
                                         path_distance=200, remaining_distance=0,
                                         final_time=0.2, time_out=49, result=1)

        pose, experiment = recording.get_pose_and_experiment(25)
        self.assertEqual(pose, 2)
        self.assertEqual(experiment, 0)

        for i in range(23):
            recording.write_summary_results(experiment=Experiment(), pose=[24, 32], rep=1,
                                            path_distance=200, remaining_distance=0,
                                            final_time=0.2, time_out=49, result=1)

        pose, experiment = recording.get_pose_and_experiment(25)
        self.assertEqual(pose, 0)
        self.assertEqual(experiment, 1)

        for i in range(23):
            recording.write_summary_results(experiment=Experiment(), pose=[24, 32], rep=1,
                                            path_distance=200, remaining_distance=0,
                                            final_time=0.2, time_out=49, result=1)

        pose, experiment = recording.get_pose_and_experiment(25)
        self.assertEqual(pose, 23)
        self.assertEqual(experiment, 1)

    def test_get_pose_and_experiment_corner(self):


        from carla.driving_benchmark.experiment import Experiment

        recording = Recording( name_to_save='Test1'
                              , continue_experiment=False, save_images=True
                              )

        pose, experiment = recording.get_pose_and_experiment(1)

        # An starting experiment should return one

        self.assertEqual(pose, 0)
        self.assertEqual(experiment, 0)

        pose, experiment = recording.get_pose_and_experiment(2)
        self.assertEqual(pose, 0)
        self.assertEqual(experiment, 0)


        recording.write_summary_results( experiment=Experiment(), pose=[24, 32], rep=1,
                                         path_distance=200, remaining_distance=0,
                                         final_time=0.2, time_out=49, result=1)


        pose, experiment = recording.get_pose_and_experiment(1)

        print (pose, experiment)
        self.assertEqual(pose, 0)
        self.assertEqual(experiment, 1)


        pose, experiment = recording.get_pose_and_experiment(2)

        print (pose, experiment)
        # An starting experiment should return one
        self.assertEqual(pose, 1)
        self.assertEqual(experiment, 0)

        pose, experiment = recording.get_pose_and_experiment(3)

        print (pose, experiment)
        # An starting experiment should return one
        self.assertEqual(pose, 1)
        self.assertEqual(experiment, 0)

        recording.write_summary_results( experiment=Experiment(), pose=[24, 32], rep=1,
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
    