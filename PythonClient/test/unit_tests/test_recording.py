
import unittest
from carla.benchmarks.recording import Recording

class testRecording(unittest.TestCase):


    def test_write_summary_results(self):

        import os

        recording = Recording(city_name='Town01', name_to_save='Test1')




        with open(os.path.join(recording._full_name, recording._suffix_name), 'r') as f:

            header = f.readline().split(',')
            #Assert if header is header
            self.assertIn('exp_id', header)

            self.assertEqual(len(header), len(recording.__dict_stats))
            #Assert if there is something writen in the row
            #Assert if the number of collums is correct






if __name__ == '__main__':
    unittest.main()
    