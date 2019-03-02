# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
This module provides implementation for GlobalRoutePlannerDAO
"""

import numpy as np


class GlobalRoutePlannerDAO(object):
    """
    This class is the data access layer for fetching data
    from the carla server instance for GlobalRoutePlanner
    """

    def __init__(self, wmap):
        """get_topology
        Constructor

        wmap    :   carl world map object
        """
        self._wmap = wmap

    def get_topology(self):
        """
        Accessor for topology.
        This function retrieves topology from the server as a list of
        road segments as pairs of waypoint objects, and processes the
        topology into a list of dictionary objects.

        return: list of dictionary objects with the following attributes
                entry   -   (x,y) of entry point of road segment
                exit    -   (x,y) of exit point of road segment
                path    -   list of waypoints separated by 1m from entry
                            to exit
                intersection    -   Boolean indicating if the road segment
                                    is an intersection
                roadid  - unique id common for all lanes of a road segment
        """
        topology = []
        # Retrieving waypoints to construct a detailed topology
        for segment in self._wmap.get_topology():
            wp1, wp2 = segment[0], segment[1]
            l1, l2 = wp1.transform.location, wp2.transform.location
            l1.x, l1.y, l1.z, l2.x, l2.y, l2.z = np.round([l1.x, l1.y, l1.z, l2.x, l2.y, l2.z], 2)
            wp1.transform.location, wp2.transform.location = l1, l2
            seg_dict = dict()
            seg_dict['entry'] = wp1
            seg_dict['exit'] = wp2
            seg_dict['path'] = []
            endloc = wp2.transform.location
            w = wp1.next(1)[0]
            while w.transform.location.distance(endloc) > 1:
                seg_dict['path'].append(w)
                w = w.next(1)[0]
            topology.append(seg_dict)
        return topology

    def get_waypoint(self, location):
        """
        The method returns waytpoint at given location
        """
        return self._wmap.get_waypoint(location)
