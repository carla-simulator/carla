#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
This metric filters the useful information of the criteria (sucess / fail ...),
and dump it into a json file

It is meant to serve as an example of how to use the criteria
"""

import json

from srunner.metrics.examples.basic_metric import BasicMetric


class CriteriaFilter(BasicMetric):
    """
    Metric class CriteriaFilter
    """

    def _create_metric(self, town_map, log, criteria):
        """
        Implementation of the metric. This is an example to show how to use the criteria
        """

        ### Parse the criteria information, filtering only the useful information, and dump it into a json ###

        results = {}
        for criterion_name in criteria:
            criterion = criteria[criterion_name]
            results.update({criterion_name:
                {
                    "test_status": criterion["test_status"],
                    "actual_value": criterion["actual_value"],
                    "success_value": criterion["expected_value_success"]
                }
            }
        )

        with open('srunner/metrics/data/CriteriaFilter_results.json', 'w') as fw:
            json.dump(results, fw, sort_keys=False, indent=4)
