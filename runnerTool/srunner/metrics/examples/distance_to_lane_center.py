#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
This metric calculates the distance between the ego vehicle and
the center of the lane, dumping it to a json file.

It is meant to serve as an example of how to use the map API
"""

import math
import json

from srunner.metrics.examples.basic_metric import BasicMetric


class DistanceToLaneCenter(BasicMetric):
    """
    Metric class DistanceToLaneCenter
    """

    def _create_metric(self, town_map, log, criteria):
        """
        Implementation of the metric.
        """

        # Get ego vehicle id
        ego_id = log.get_ego_vehicle_id()

        dist_list = []
        frames_list = []

        # Get the frames the ego actor was alive and its transforms
        start, end = log.get_actor_alive_frames(ego_id)

        # Get the projected distance vector to the center of the lane
        for i in range(start, end + 1):

            ego_location = log.get_actor_transform(ego_id, i).location
            ego_waypoint = town_map.get_waypoint(ego_location)

            # Get the distance vector and project it
            a = ego_location - ego_waypoint.transform.location      # Ego to waypoint vector
            b = ego_waypoint.transform.get_right_vector()           # Waypoint perpendicular vector
            b_norm = math.sqrt(b.x * b.x + b.y * b.y + b.z * b.z)

            ab_dot = a.x * b.x + a.y * b.y + a.z * b.z
            dist_v = ab_dot/(b_norm*b_norm)*b
            dist = math.sqrt(dist_v.x * dist_v.x + dist_v.y * dist_v.y + dist_v.z * dist_v.z)

            # Get the sign of the distance (left side is positive)
            c = ego_waypoint.transform.get_forward_vector()         # Waypoint forward vector
            ac_cross = c.x * a.y - c.y * a.x
            if ac_cross < 0:
                dist *= -1

            dist_list.append(dist)
            frames_list.append(i)

        # Save the results to a file
        results = {'frames': frames_list, 'distance': dist_list}
        with open('srunner/metrics/data/DistanceToLaneCenter_results.json', 'w') as fw:
            json.dump(results, fw, sort_keys=False, indent=4)
