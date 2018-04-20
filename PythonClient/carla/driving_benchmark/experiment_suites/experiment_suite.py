# To be redefined on subclasses on how to calculate timeout for an episode
import abc


class ExperimentSuite(object):

    def __init__(self, city_name):

        self._city_name = city_name
        self._experiments = self.build_experiments()

    def calculate_time_out(self, path_distance):
        """
        Function to return the timeout ,in milliseconds,
        that is calculated based on distance to goal.
        This is the same timeout as used on the CoRL paper.
        """
        return ((path_distance / 1000.0) / 10.0) * 3600.0 + 10.0

    def get_number_of_poses_task(self):
        """
            Get the number of poses a task have for this benchmark
        """

        # Warning: assumes that all tasks have the same size

        return len(self._experiments[0].poses)

    def get_experiments(self):
        """
        Getter for the experiment set.
        """
        return self._experiments

    @property
    def dynamic_tasks(self):
        """
        Returns the episodes that contain dynamic obstacles
        """
        dynamic_tasks = set()
        for exp in self._experiments:
            if exp.conditions.NumberOfVehicles > 0 or exp.conditions.NumberOfPedestrians > 0:
                dynamic_tasks.add(exp.task)

        return list(dynamic_tasks)

    @property
    def metrics_parameters(self):
        """
        Property to return the parameters for the metric module
        Could be redefined depending on the needs of the user.
        """
        return {

            'intersection_offroad': {'frames_skip': 10,
                                     'frames_recount': 20,
                                     'threshold': 0.3
                                     },
            'intersection_otherlane': {'frames_skip': 10,
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

        }

    @property
    def weathers(self):
        weathers = set(self.train_weathers)
        weathers.update(self.test_weathers)
        return weathers

    @abc.abstractmethod
    def build_experiments(self):
        """
        Returns a set of experiments to be evaluated
        Must be redefined in an inherited class.

        """

    @abc.abstractproperty
    def train_weathers(self):
        """
        Return the weathers that are considered as training conditions
        """

    @abc.abstractproperty
    def test_weathers(self):
        """
        Return the weathers that are considered as testing conditions
        """
