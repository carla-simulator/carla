# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
This module provides implementation for GlobalRoutePlannerDAO
"""


class GlobalRoutePlannerDAO(object):
    """
    This class is the data access layer for fetching data
    from the carla server instance for GlobalRoutePlanner
    """

    def __init__(self, wmap):
        """
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
        """
        topology = []
        # Retrieving waypoints to construct a detailed topology
        for segment in self._wmap.get_topology():
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
