# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# CORL experiment set.

from __future__ import print_function

import logging
import time




import unittest

from carla.agent_benchmark.agent import Agent

from carla.agent_benchmark.experiment_suite.sample import SampleBenchmark

from carla.client import make_carla_client, VehicleControl
from carla.tcp import TCPConnectionError



class Manual(Agent):
    """
    Sample redefinition of the Agent,
    An agent that goes straight
    """
    def run_step(self, measurements, sensor_data, directions, target):
        control = VehicleControl()
        control.throttle = 0.9

        return control



class testBenchmark(unittest.TestCase):


    def test_init(self):

        bench = SampleBenchmark(city_name='Town01',
                  name_to_save='Test',
                  continue_experiment=True,
                  save_images=False,
                  distance_for_success=2.0)





    def test_sample_benchmark(self):


        log_level = logging.DEBUG

        logging.basicConfig(format='%(levelname)s: %(message)s', level=log_level)

        logging.info('listening to server %s:%s', '127.0.0.1', '2000')

        while True:
            try:

                with make_carla_client('localhost', 2000) as client:
                    print ('CONNECTED')
                    sample = SampleBenchmark(city_name='Town02', name_to_save='accept_test_benchmark2'
                                             )
                    agent = Manual('Town02')
                    results = sample.benchmark_agent(agent, client)
                    sample.plot_summary_test()
                    sample.plot_summary_train()

                    break

            except TCPConnectionError as error:
                logging.error(error)
                time.sleep(1)


        self.assertEqual(sample._recording._get_last_position(), 5)

        # The manual class only completes the first one
        self.assertEqual(results['average_fully_completed'][1.0][0], 1)

        self.assertEqual(results['average_fully_completed'][1.0][1], 0)

        self.assertEqual(results['average_fully_completed'][1.0][2], 0)

        self.assertEqual(results['average_fully_completed'][1.0][3], 0)

