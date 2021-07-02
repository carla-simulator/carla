# Copyright (c) # Copyright (c) 2018-2020 CVC.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.


"""
This module provides GlobalRoutePlanner implementation.
"""
import math
import numpy as np

from agents.navigation.local_planner import RoadOption
from agents.tools.route_helper import trace_route, initialize_map
import ad_map_access as ad

class GlobalRoutePlanner(object):
    """
    This class provides a high level route planner.
    """

    def __init__(self, wmap, sampling_resolution):
        """
        Constructor
        """
        self._previous_decision = RoadOption.VOID
        self._sampling_resolution = sampling_resolution
        self._wmap = wmap
        initialize_map(self._wmap)

    def trace_route(self, origin, destination, with_options=True):
        """
        This method traces a route between a starting and ending point.
        IF 'with_options' is True (deafult behavior), returns a list of [carla.Waypoint, RoadOption],
        being RoadOption a high level represnetation of what the direction of the route.
        If 'with_options' is False, the road options are computed, returning a list of carla.Waypoint

        :param origin (carla.Waypoint): starting point of the route
        :param destination (carla.Waypoint): ending point of the route
        :param with_options (bool): whether or not the road options will be added to the route info
        """
        route = trace_route(origin, destination, self._wmap, self._sampling_resolution)
        if not route:
            return []

        # Add options, or not
        if with_options:
            return self.add_options_to_route(route)
        else:
            return route

    def add_options_to_route(self, route):
        """
        This method adds the road options to a route, returning a list of [carla.Waypoint, RoadOption].
        CHANGELANELEFT and CHANGELANERIGHT are used to signalize a lane change. For the other options,
        LEFT, RIGHT and STRAIGHT indicate the specific lane chosen at an intersection and outside those,
        LANEFOLLOW is always used.

        This has been tested for sampling resolutions up to ~7 meters, and might fail for higher values.

        :param route (list of carla.Waypoint): list of waypoints representing the route
        """
        route_with_options = []
        route_with_lane_changes = []

        # Part 1: Add road options, excluding lane changes
        if route[0].is_junction:
            self._prev_at_junction = True
            entry_index = 0
        else:
            self._prev_at_junction = False
            entry_index = None

        for i, waypoint in enumerate(route):
            at_junction = waypoint.is_junction
            if not at_junction and self._prev_at_junction:
                # Just exited a junction, get all of its data
                road_option = self._compute_options(route[entry_index], waypoint)
                for j in range(entry_index, i):
                    route_with_options.append([route[j], road_option])
                entry_index = None
                route_with_options.append([waypoint, RoadOption.LANEFOLLOW])
            elif not at_junction and not self._prev_at_junction:
                # Outside a junction, always LANEFOLLOW
                route_with_options.append([waypoint, RoadOption.LANEFOLLOW])
            elif not self._prev_at_junction:
                # Just entered a junction, save its entrypoint and wait for the exit
                entry_index = i

            self._prev_at_junction = at_junction

        # Route ended at a junction
        if self._prev_at_junction:
            road_option = self._compute_options(route[entry_index], route[-1])
            for j in range(entry_index, len(route)):
                route_with_options.append([route[j], road_option])
            entry_index = None

        # Part 2: Add lane changes
        lane_change_type = None

        for i in range(0, len(route_with_options)):
            waypoint, option = route_with_options[i]

            # Start and end cases
            if i == len(route_with_options) - 1:
                route_with_lane_changes.append([waypoint, lane_change_type if lane_change_type else option])
                continue
            if i == 0:
                prev_direction = waypoint.transform.get_forward_vector()
                np_prev_direction = np.array([prev_direction.x, prev_direction.y, prev_direction.z])

            # Check the dot product between the two consecutive waypoint
            route_direction = route_with_options[i+1][0].transform.location - waypoint.transform.location
            np_route_direction = np.array([route_direction.x, route_direction.y, route_direction.z])

            if np.linalg.norm(np_route_direction) and np.linalg.norm(np_prev_direction):
                dot = np.dot(np_prev_direction, np_route_direction)
                dot /= np.linalg.norm(np_prev_direction) * np.linalg.norm(np_route_direction)
            else:
                dot = 1

            if 0 < dot < math.cos(math.radians(45)):
                if lane_change_type:
                    # Last lane change waypoint
                    new_option = lane_change_type
                    lane_change_type = None
                else:
                    # First lane change waypoint
                    cross = np.cross(np_prev_direction, np_route_direction)
                    lane_change_type = RoadOption.CHANGELANERIGHT if cross[2] > 0 else RoadOption.CHANGELANELEFT
                    new_option = lane_change_type

            else:
                if lane_change_type:
                    new_option = lane_change_type
                else:
                    new_option = option

            route_with_lane_changes.append([waypoint, new_option])
            np_prev_direction = np_route_direction

        return route_with_lane_changes

    def _compute_options(self, entry_waypoint, exit_waypoint):
        """
        Computes the road option of a
        """
        diff = (exit_waypoint.transform.rotation.yaw - entry_waypoint.transform.rotation.yaw) % 360
        if diff > 315.0:
            option = RoadOption.STRAIGHT
        elif diff > 180.0:
            option = RoadOption.LEFT
        elif diff > 45.0:
            option = RoadOption.RIGHT
        else:
            option = RoadOption.STRAIGHT

        return option
