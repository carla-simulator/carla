# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
This module provides GlobalRoutePlanner implementation.
"""

import math

import numpy as np
import networkx as nx

import carla
from agents.navigation.local_planner import RoadOption
from agents.tools.misc import vector


class GlobalRoutePlanner(object):
    """
    This class provides a very high level route plan.
    Instantiate the class by passing a reference to
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
        self._road_id_to_edge = None

    def setup(self):
        """
        Perform initial server data lookup for detailed topology
        and builds graph representation of the world map.
        """
        self._topology = self._dao.get_topology()
        # Creating graph of the world map and also a maping from
        # node co-ordinates to node id along with a map from road id to edge
        self._graph, self._id_map, self._road_id_to_edge = self._build_graph()
        self._lane_change_link()

    def _build_graph(self):
        """
        This function builds a networkx  graph representation of topology.
        The topology is read from self._topology.
        graph node properties:
            vertex   -   carla.Location object of node's position in world map
        graph edge properties:
            entry_vector    -   unit vector along tangent at entry point
            exit_vector     -   unit vector along tangent at exit point
            net_vector      -   unit vector of the chord from entry to exit
            intersection    -   boolean indicating if the edge belongs to an
                                intersection
        return      :   graph -> networkx graph representing the world map,
                        id_map-> mapping from (x,y,z) to node id
                        road_id_to_edge-> map from road id to edge in the graph
        """
        graph = nx.DiGraph()
        id_map = dict() # Map with structure {(x,y,z): id, ... }
        road_id_to_edge = dict() # Map with structure {road_id: {lane_id: edge, ... }, ... }

        for segment in self._topology:

            entry_wp = segment['entry']
            exit_wp = segment['exit']
            path = segment['path']
            intersection = entry_wp.is_intersection
            road_id, lane_id = entry_wp.road_id, entry_wp.lane_id

            for vertex_wp in entry_wp, exit_wp:
                # Adding unique nodes and populating id_map
                location = vertex_wp.transform.location
                if (location.x, location.y, location.z) not in id_map:
                    new_id = len(id_map)
                    id_map[location.x, location.y, location.z] = new_id
                    graph.add_node(new_id, vertex=location)
            n1 = id_map[entry_wp.transform.location.x, entry_wp.transform.location.y, entry_wp.transform.location.z]
            n2 = id_map[exit_wp.transform.location.x, exit_wp.transform.location.y, exit_wp.transform.location.z]
            if road_id not in road_id_to_edge:
                road_id_to_edge[road_id] = dict()
            road_id_to_edge[road_id][lane_id] = (n1, n2)

            # Adding edge with attributes
            graph.add_edge(
                n1, n2,
                length=len(path) + 1, path=path,
                entry_vector=vector(
                    entry_wp.transform.location,
                    path[0].transform.location if len(path) > 0 else exit_wp.transform.location),
                exit_vector=vector(
                    path[-1].transform.location if len(path) > 0 else entry_wp.transform.location,
                    exit_wp.transform.location),
                net_vector=vector(entry_wp.transform.location, exit_wp.transform.location),
                intersection=intersection, type=RoadOption.LANEFOLLOW)

        return graph, id_map, road_id_to_edge

    def _localise(self, location):
        """
        This function finds the road segment closest to given waypoint
        x, y        :   co-rodinates to be localized in the graph
        return      :   pair node ids representing an edge in the graph
        """
        waypoint = self._dao.get_waypoint(location)
        return self._road_id_to_edge[waypoint.road_id][waypoint.lane_id]

    def _lane_change_link(self):
        """
        This method places zero cost links in the topology graph
        representing availability of lane changes.
        """

        for segment in self._topology:
            left_found, right_found = False, False
            for waypoint in segment['path']:

                next_waypoint = None
                next_road_option = None

                if bool(waypoint.lane_change & carla.LaneChange.Right) and not right_found:
                    next_waypoint = waypoint.get_right_lane()
                    next_road_option = RoadOption.CHANGELANERIGHT
                    next_segment = self._localise(next_waypoint.transform.location)
                    next_edge = self._graph.edges[next_segment[0], next_segment[1]]
                    sloc = segment['entry'].transform.location
                    self._graph.add_edge(
                        self._id_map[sloc.x, sloc.y, sloc.z], next_segment[1],
                        length=next_edge['length'], type=next_road_option, change_waypoint = waypoint)
                    right_found = True

                if bool(waypoint.lane_change & carla.LaneChange.Left) and not left_found:
                    next_waypoint = waypoint.get_left_lane()
                    next_road_option = RoadOption.CHANGELANELEFT
                    next_segment = self._localise(next_waypoint.transform.location)
                    next_edge = self._graph.edges[next_segment[0], next_segment[1]]
                    sloc = segment['entry'].transform.location
                    self._graph.add_edge(
                        self._id_map[sloc.x, sloc.y, sloc.z], next_segment[1],
                        length=next_edge['length'], type=next_road_option, change_waypoint = waypoint)
                    left_found = True

                if left_found and right_found: break

    def _distance_heuristic(self, n1, n2):
        """
        Distance heuristic calculator for path searching
        in self._graph
        """
        l1 = self._graph.nodes[n1]['vertex']
        l2 = self._graph.nodes[n2]['vertex']
        return l1.distance(l2)

    def _path_search(self, origin, destination):
        """
        This function finds the shortest path connecting origin and destination
        using A* search with distance heuristic.
        origin      :   carla.Location object of start position
        desitnation :   carla.Location object of of end position
        return      :   path as list of node ids (as int) of the graph self._graph
        connecting origin and destination
        """

        start, end = self._localise(origin), self._localise(destination)

        route = nx.astar_path(
            self._graph, source=start[0], target=end[1],
            heuristic=self._distance_heuristic, weight='length')

        return route

    def plan_route(self, origin, destination):
        """
        The following function generates the route plan based on
        origin      : carla.Location object of the route's start position
        destination : carla.Location object of the route's end position
        return      : list of turn by turn navigation decisions as
        agents.navigation.local_planner.RoadOption elements
        Possible values are STRAIGHT, LEFT, RIGHT, LANEFOLLOW, VOID
        CHANGELANELEFT, CHANGELANERIGHT
        """

        threshold = math.radians(5.0)
        route = self._path_search(origin, destination)
        plan = []

        if len(route) == 2 and self._graph.edges[route[0], route[1]]['type'] != RoadOption.LANEFOLLOW:
            plan.append(self._graph.edges[route[0], route[1]]['type'])

        # Compare current edge and next edge to decide on action
        for i in range(len(route) - 2):
            current_edge = self._graph.edges[route[i], route[i + 1]]
            next_edge = self._graph.edges[route[i + 1], route[i + 2]]
            if next_edge['type'] != RoadOption.LANEFOLLOW:
                plan.append(next_edge['type'])
            elif current_edge['type'] == RoadOption.LANEFOLLOW:
                cv, nv = current_edge['exit_vector'], next_edge['net_vector']
                entry_edges, exit_edges = 0, 0
                cross_list = []
                # Accumulating cross products of all other paths
                for neighbor in self._graph.successors(route[i+1]):
                    select_edge = self._graph.edges[route[i+1], neighbor]
                    if select_edge['type'] == RoadOption.LANEFOLLOW:
                        entry_edges += 1
                        if neighbor != route[i + 2]:
                            sv = select_edge['net_vector']
                            cross_list.append(np.cross(cv, sv)[2])
                for neighbor in self._graph.predecessors(route[i+2]): exit_edges += 1
                if entry_edges == 1 and exit_edges > 1: cross_list.append(0)
                # Calculating turn decision
                elif next_edge['intersection'] and (entry_edges > 1 or exit_edges > 1):
                    next_cross = np.cross(cv, nv)[2]
                    deviation = math.acos(np.dot(cv, nv) /\
                        (np.linalg.norm(cv)*np.linalg.norm(nv)))
                    if deviation < threshold: action = RoadOption.STRAIGHT
                    elif next_cross < min(cross_list):
                        action = RoadOption.LEFT
                    elif next_cross > max(cross_list):
                        action = RoadOption.RIGHT
                    plan.append(action)

        return plan

    def verify_intersection(self, waypoint):
        """
        This function recieves a waypoint and returns true
        if the given waypoint is part of a true intersection
        else returns false
        """

        is_intersection = False
        if waypoint.is_intersection :
            entry_node_id, exit_node_id = self._localise(waypoint.transform.location)
            segment = self._graph.edges[entry_node_id, exit_node_id]
            entry_edges, exit_edges = 0, 0
            for _ in self._graph.successors(entry_node_id): 
                if segment['type'] == RoadOption.LANEFOLLOW: entry_edges += 1
            for _ in self._graph.predecessors(exit_node_id):
                if segment['type'] == RoadOption.LANEFOLLOW: exit_edges += 1
            if entry_edges > 1 or exit_edges > 1: is_intersection = True

        return is_intersection

    pass
