# Copyright (c) # Copyright (c) 2018-2020 CVC.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.


"""
This module provides GlobalRoutePlanner implementation.
"""
import math
import os
import numpy as np
import xml.etree.ElementTree as ET
import carla

from agents.navigation.local_planner import RoadOption
from agents.tools.global_route_planner_helper import trace_route
from agents.tools.misc import vector

import ad_map_access as ad

class GlobalRoutePlanner(object):
    """
    This class provides a high level route planner.
    """

    def __init__(self, wmap, sampling_resolution, world=None):
        """
        Constructor
        """
        self._previous_decision = RoadOption.VOID

        self._sampling_resolution = sampling_resolution
        self._world = world  # TODO: remove it after debugging
        self._wmap = wmap
        self._initialize_map()

    def _initialize_map(self):
        """Initialize the AD map library and, creates the file needed to do so."""
        lat_ref = 0.0
        lon_ref = 0.0

        opendrive_contents = self._wmap.to_opendrive()
        xodr_name = 'RoutePlannerMap.xodr'
        txt_name = 'RoutePlannerMap.txt'

        # Save the opendrive data into a file
        with open(xodr_name, 'w') as f:
            f.write(opendrive_contents)

        # Get geo reference
        xml_tree = ET.parse(xodr_name)
        for geo_elem in xml_tree.find('header').find('geoReference').text.split(' '):
            if geo_elem.startswith('+lat_0'):
                lat_ref = float(geo_elem.split('=')[-1])
            elif geo_elem.startswith('+lon_0'):
                lon_ref = float(geo_elem.split('=')[-1])

        # Save the previous info
        with open(txt_name, 'w') as f:
            txt_content = "[ADMap]\n" \
                          "map=" + xodr_name + "\n" \
                          "[ENUReference]\n" \
                          "default=" + str(lat_ref) + " " + str(lon_ref) + " 0.0"
            f.write(txt_content)

        # Intialize the map and remove created files
        initialized = ad.map.access.init(txt_name)
        if not initialized:
            raise ValueError("Couldn't initialize the map")

        for fname in [txt_name, xodr_name]:
            if os.path.exists(fname):
                os.remove(fname)

    def trace_route(self, origin, destination):
        """
        This method returns list of (carla.Waypoint, RoadOption)
        from origin to destination
        """
        route = trace_route(origin, destination, self._wmap, self._sampling_resolution)

        # Add options
        route_with_options = self.add_options_to_route(route)

        # TODO: remove it
        for w in route_with_options:
            wp = w[0].transform.location + carla.Location(z=0.1)
            if w[1] == RoadOption.LEFT:  # Yellow
                color = carla.Color(255, 255, 0)
            elif w[1] == RoadOption.RIGHT:  # Cyan
                color = carla.Color(0, 255, 255)
            elif w[1] == RoadOption.CHANGELANELEFT:  # Orange
                color = carla.Color(255, 64, 0)
            elif w[1] == RoadOption.CHANGELANERIGHT:  # Dark Cyan
                color = carla.Color(0, 64, 255)
            elif w[1] == RoadOption.STRAIGHT:  # Gray
                color = carla.Color(128, 128, 128)
            else:  # LANEFOLLOW
                color = carla.Color(0, 255, 0) # Green
            self._world.debug.draw_point(wp, size=0.08, color=color, life_time=100)

        return route_with_options

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
            road_option = self._compute_options(route[-1], waypoint)
            for j in range(entry_index, len(route)):
                route_with_options.append([route[j], road_option])
            entry_index = None

        # Part 2: Add lane changes
        prev_lane_change = None

        for i in range(0, len(route_with_options) - 1):
            waypoint, option = route_with_options[i]
            next_waypoint, _ = route_with_options[i+1]

            # Lane changes are set to both lanes
            if prev_lane_change:
                route_with_lane_changes.append([waypoint, prev_lane_change])
                prev_lane_change = None
                continue

            # Check the dot product between the two consecutive waypoint
            direction = waypoint.transform.get_forward_vector()
            np_direction = np.array([direction.x, direction.y, direction.z])

            route_direction = next_waypoint.transform.location - waypoint.transform.location
            np_route_direction = np.array([route_direction.x, route_direction.y, route_direction.z])

            dot = np.dot(np_direction, np_route_direction)
            dot /= np.linalg.norm(np_direction) * np.linalg.norm(np_route_direction)

            if 0 < dot < math.cos(math.radians(45)):
                cross = np.cross(np_direction, np_route_direction)
                prev_lane_change = RoadOption.CHANGELANERIGHT if cross[2] > 0 else RoadOption.CHANGELANELEFT
                route_with_lane_changes.append([waypoint, prev_lane_change])
            else:
                route_with_lane_changes.append([waypoint, option])

        return route_with_lane_changes

    def _compute_options(self, entry_waypoint, exit_waypoint):
        """
        Computes the road option of a
        """
        diff = (exit_waypoint.transform.rotation.yaw - entry_waypoint.transform.rotation.yaw) % 360
        if diff > 315.0:
            option = RoadOption.STRAIGHT
        elif diff > 225.0:
            option = RoadOption.LEFT
        elif diff > 135.0:
            option = RoadOption.HALFTURN
        elif diff > 45.0:
            option = RoadOption.RIGHT
        else:
            option = RoadOption.STRAIGHT

        return option
