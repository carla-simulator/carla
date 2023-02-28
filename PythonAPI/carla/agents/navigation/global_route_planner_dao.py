# Copyright (c) # Copyright (c) 2018-2020 CVC.
#
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

    def __init__(self, wmap, sampling_resolution, world=None):
        """
        Constructor method.

            :param wmap: carla.world object
            :param sampling_resolution: sampling distance between waypoints
        """
        self._sampling_resolution = sampling_resolution
        print("sampling in dao", sampling_resolution)
        self._wmap = wmap
        self._world=world

    def get_topology(self):
        """
        Accessor for topology.
        This function retrieves topology from the server as a list of
        road segments as pairs of waypoint objects, and processes the
        topology into a list of dictionary objects.

            :return topology: list of dictionary objects with the following attributes
                entry   -   waypoint of entry point of road segment
                entryxyz-   (x,y,z) of entry point of road segment
                exit    -   waypoint of exit point of road segment
                exitxyz -   (x,y,z) of exit point of road segment
                path    -   list of waypoints separated by 1m from entry
                            to exit
        """
        waypoints_v=False
        topology = []
        # Retrieving waypoints to construct a detailed topology
        print("creating topology")
        print("Sampling resolution  ", self._sampling_resolution)
        #print(len(self._wmap.get_topology()))
        seg_count=0
        for segment in self._wmap.get_topology():
            seg_count+=1
            #print(seg_count)
            #if seg_count<10:
                #continue
            wp1, wp2 = segment[0], segment[1]
            #print("seg 0", segment[0])
            l1, l2 = wp1.transform.location, wp2.transform.location
            if waypoints_v==True:
                self._world.debug.draw_string(wp1.transform.location, 'O', draw_shadow=False, life_time=1200.0,
                                                persistent_lines=True)
                self._world.debug.draw_string(wp2.transform.location, 'X', draw_shadow=False, life_time=1200.0,
                                                persistent_lines=True)
            # Rounding off to avoid floating point imprecision
            x1, y1, z1, x2, y2, z2 = np.round([l1.x, l1.y, l1.z, l2.x, l2.y, l2.z], 0)
            wp1.transform.location, wp2.transform.location = l1, l2
            seg_dict = dict()
            seg_dict['entry'], seg_dict['exit'] = wp1, wp2
            seg_dict['entryxyz'], seg_dict['exitxyz'] = (x1, y1, z1), (x2, y2, z2)
            seg_dict['path'] = []
            endloc = wp2.transform.location
            #print(segment)
            #print(wp2)
            if wp1.transform.location.distance(endloc) > self._sampling_resolution:
                w = wp1.previous(self._sampling_resolution)[0]
                while w.transform.location.distance(endloc) > self._sampling_resolution:
                    seg_dict['path'].append(w)
                    #print( w.transform.location.distance(endloc))
                    #print(w)
                    w = w.previous(self._sampling_resolution)[0]
                    if waypoints_v==True:
                        self._world.debug.draw_string(w.transform.location, '-', draw_shadow=False, life_time=1200.0,
                                                persistent_lines=True)
                    #print("whil...", w.transform.location.distance(endloc))
            else:
                #print("else")
                seg_dict['path'].append(wp1.previous(self._sampling_resolution)[0])
            topology.append(seg_dict)
        print("returning topology")
        return topology

    def get_waypoint(self, location):
        """
        The method returns waypoint at given location

            :param location: vehicle location
            :return waypoint: generated waypoint close to location
        """
        waypoint = self._wmap.get_waypoint(location)
        return waypoint

    def get_resolution(self):
        """ Accessor for self._sampling_resolution """
        return self._sampling_resolution
