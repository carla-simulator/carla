# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
This module provides implementation for GlobalRoutePlannerDAO
"""

import carla


class GlobalRoutePlannerDAO(object):
    """
    This class is the data access layer for fetching data
    from the carla server instance for GlobalRoutePlanner
    """

    def __init__(self, wmap):
        """ Constructor """
        self.wmap = wmap    # Carla world map

    def get_topology(self):
        """ Accessor for topology """
        topology = []
        # Retrieving waypoints to construct a detailed topology
        for segment in self.wmap.get_topology():
            x1 = segment[0].transform.location.x
            y1 = segment[0].transform.location.y
            x2 = segment[1].transform.location.x
            y2 = segment[1].transform.location.y
            seg_dict = dict()
            seg_dict['entry'] = (x1, y1)
            seg_dict['exit'] = (x2, y2)
            seg_dict['path'] = []
            wp1 = segment[0]
            wp2 = segment[1]
            seg_dict['intersection'] = True if wp1.is_intersection else False
            endloc = wp2.transform.location
            w = wp1.next(1)[0]
            while w.transform.location.distance(endloc) > 1:
                x = w.transform.location.x
                y = w.transform.location.y
                seg_dict['path'].append((x, y))
                w = w.next(1)[0]

            topology.append(seg_dict)
        return topology
