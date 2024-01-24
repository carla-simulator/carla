#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
This module provide BasicMetric, the basic class of all the metrics.
"""

class BasicMetric(object):
    """
    Base class of all the metrics.
    """

    def __init__(self, town_map, log, criteria=None):
        """
        Initialization of the metric class. This calls the metrics log and creates the metrics

        Args:
            town_map (carla.Map): Map of the simulation. Used to access the Waypoint API.
            log (srunner.metrics.tools.Metricslog): instance of a class used to access the recorder information
            criteria (dict): list of dictionaries with all the criteria information
        """

        # Create the metrics of the simulation. This part is left to the user
        self._create_metric(town_map, log, criteria)

    def _create_metric(self, town_map, log, criteria):
        """
        Pure virtual function to setup the metrics by the user.

        Args:
            town_map (carla.Map): Map of the simulation. Used to access the Waypoint API.
            log (srunner.metrics.tools.Metricslog): instance of a class used to access the recorder information
            criteria (dict): dictionaries with all the criteria information
        """
        raise NotImplementedError(
            "This function should be re-implemented by all metrics"
            "If this error becomes visible the class hierarchy is somehow broken")
