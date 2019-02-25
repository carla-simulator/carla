# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
This module provides GlobalRoutePlanner implementation.
"""

import math
from enum import Enum

import numpy as np
import networkx as nx

import carla
from agents.navigation.local_planner import RoadOption


class GlobalRoutePlanner(object):
    """
    This class provides a very high level route plan.
    Instantiate the calss by passing a reference to
    A GlobalRoutePlannerDAO object.
    """

    def __init__(self, dao):
        """
        Constructor
        """
        self._dao = dao
        self._topology = None
        self._graph = None
        self._id_map = None

    def setup(self):
        """
        Perform initial server data lookup for detailed topology
        and builds graph representation of the world map.
        """
        self._topology = self._dao.get_topology()
        # Creating graph of the world map and also a maping from
        # node co-ordinates to node id
        self._graph, self._id_map = self.build_graph()

    def plan_route(self, origin, destination):
        """
        The following function generates the route plan based on
        origin      : tuple containing x, y of the route's start position
        destination : tuple containing x, y of the route's end position
        return      : list of turn by turn navigation decisions as
        agents.navigation.local_planner.RoadOption elements
        Possible values (for now) are STRAIGHT, LEFT, RIGHT, LANEFOLLOW, VOID
        """

        threshold = math.radians(4.0)
        route = self.path_search(origin, destination)
        plan = []

        # Compare current edge and next edge to decide on action
        for i in range(len(route) - 2):
            current_edge = self._graph.edges[route[i], route[i + 1]]
            next_edge = self._graph.edges[route[i + 1], route[i + 2]]
            cv = current_edge['exit_vector']
            nv = next_edge['net_vector']
            cv, nv = cv + (0,), nv + (0,)  # Making vectors 3D
            num_edges = 0
            cross_list = []
            # Accumulating cross products of all other paths
            for neighbor in self._graph.neighbors(route[i+1]):
                num_edges+=1
                if neighbor != route[i + 2]:
                    select_edge = self._graph.edges[route[i+1], neighbor]
                    sv = select_edge['net_vector']
                    cross_list.append(np.cross(cv, sv)[2])
            # Calculating turn decision
            if next_edge['intersection'] and num_edges > 1:
                next_cross = np.cross(cv, nv)[2]
                deviation = math.acos(np.dot(cv, nv) /\
                    (np.linalg.norm(cv)*np.linalg.norm(nv)))
                if deviation < threshold:
                    action = RoadOption.STRAIGHT
                elif next_cross < min(cross_list):
                    action = RoadOption.LEFT
                elif next_cross > max(cross_list):
                    action = RoadOption.RIGHT
                plan.append(action)
        return plan

    def _distance_heuristic(self, n1, n2):
        """
        Distance heuristic calculator for path searching
        in self._graph
        """
        (x1, y1) = self._graph.nodes[n1]['vertex']
        (x2, y2) = self._graph.nodes[n2]['vertex']
        return ((x1 - x2) ** 2 + (y1 - y2) ** 2) ** 0.5

    def path_search(self, origin, destination):
        """
        This function finds the shortest path connecting origin and destination
        using A* search with distance heuristic.
        origin      :   tuple containing x, y co-ordinates of start position
        desitnation :   tuple containing x, y co-ordinates of end position
        return      :   path as list of node ids (as int) of the graph self._graph
        connecting origin and destination
        """
        xo, yo = origin
        xd, yd = destination
        start = self.localise(xo, yo)
        end = self.localise(xd, yd)

        route = nx.astar_path(
            self._graph, source=self._id_map[start['entry']],
            target=self._id_map[end['exit']],
            heuristic=self._distance_heuristic,
            weight='length')

        return route

    def localise(self, x, y):
        """
        This function finds the road segment closest to (x, y)
        x, y        :   co-ordinates of the point to be localized
        return      :   pair of points, tuple of tuples containing co-ordinates
        of points that represents the road segment closest to x, y
        """
        distance = float('inf')
        nearest = (distance, dict())

        # Measuring distances from segment waypoints and (x, y)
        for segment in self._topology:
            entryxy = segment['entry']
            exitxy = segment['exit']
            path = segment['path']
            for xp, yp in [entryxy] + path + [exitxy]:
                new_distance = self.distance((xp, yp), (x, y))
                if new_distance < nearest[0]:
                    nearest = (new_distance, segment)

        segment = nearest[1]
        return segment

    def build_graph(self):
        """
        This function builds a networkx  graph representation of topology.
        The topology is read from self._topology.
        graph node properties:
            vertex   -   (x,y) of node's position in world map
            num_edges   -   Number of exit edges from the node
        graph edge properties:
            entry_vector    -   unit vector along tangent at entry point
            exit_vector     -   unit vector along tangent at exit point
            net_vector      -   unit vector of the chord from entry to exit
            intersection    -   boolean indicating if the edge belongs to an
                                intersection
        return      :   graph -> networkx graph representing the world map,
                        id_map-> mapping from (x,y) to node id
        """
        graph = nx.DiGraph()
        # Map with structure {(x,y): id, ... }
        id_map = dict()

        for segment in self._topology:

            entryxy = segment['entry']
            exitxy = segment['exit']
            path = segment['path']
            intersection = segment['intersection']
            for vertex in entryxy, exitxy:
                # Adding unique nodes and populating id_map
                if vertex not in id_map:
                    new_id = len(id_map)
                    id_map[vertex] = new_id
                    graph.add_node(new_id, vertex=vertex)

            n1, n2 = id_map[entryxy], id_map[exitxy]
            # Adding edge with attributes
            graph.add_edge(
                n1, n2,
                length=len(path) + 1, path=path,
                entry_vector=self.unit_vector(
                    entryxy, path[0] if len(path) > 0 else exitxy),
                exit_vector=self.unit_vector(
                    path[-1] if len(path) > 0 else entryxy, exitxy),
                net_vector=self.unit_vector(entryxy, exitxy),
                intersection=intersection)

        return graph, id_map

    def distance(self, point1, point2):
        """
        returns the distance between point1 and point2
        point1      :   (x,y) of first point
        point2      :   (x,y) of second point
        return      :   distance from point1 to point2
        """
        x1, y1 = point1
        x2, y2 = point2
        return math.sqrt((x2 - x1)**2 + (y2 - y1)**2)

    def unit_vector(self, point1, point2):
        """
        This function returns the unit vector from point1 to point2
        point1      :   (x,y) of first point
        point2      :   (x,y) of second point
        return      :   tuple containing x and y components of unit vector
                        from point1 to point2
        """
        x1, y1 = point1
        x2, y2 = point2

        vector = (x2 - x1, y2 - y1)
        vector_mag = math.sqrt(vector[0]**2 + vector[1]**2)
        vector = (vector[0] / vector_mag, vector[1] / vector_mag)

        return vector

    def dot(self, vector1, vector2):
        """
        This function returns the dot product of vector1 with vector2
        vector1      :   x, y components of first vector
        vector2      :   x, y components of second vector
        return      :   dot porduct scalar between vector1 and vector2
        """
        return vector1[0] * vector2[0] + vector1[1] * vector2[1]

    pass
