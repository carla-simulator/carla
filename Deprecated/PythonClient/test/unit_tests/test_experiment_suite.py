import unittest
from carla.driving_benchmark.experiment_suites.experiment_suite import ExperimentSuite

from carla.driving_benchmark.experiment_suites.basic_experiment_suite import BasicExperimentSuite

from carla.driving_benchmark.experiment_suites.corl_2017 import CoRL2017

class testExperimentSuite(unittest.TestCase):


    def test_init(self):

        base_class = ExperimentSuite('Town01')
        subclasses_instanciate = [obj('Town01') for obj in ExperimentSuite.__subclasses__()]


    def test_properties(self):

        all_classes = [obj('Town01') for obj in ExperimentSuite.__subclasses__()]
        print (all_classes)
        for exp_suite in all_classes:
            print(exp_suite.__class__)
            print(exp_suite.dynamic_tasks)
            print(exp_suite.weathers)


