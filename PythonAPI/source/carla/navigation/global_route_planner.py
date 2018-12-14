# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
This module provides GlobalRoutePlanner implementation.
"""

import math
import networkx as nx
import carla


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
        self.dao = dao

    def setup(self):
        """
        Perform initial server data lookup for detailed topology
        and builds graph representation of the world map.
        """
        self.topology = self.dao.get_topology()
        # Creating graph of the world map and also a map from
        # node co-ordinates to node id
        self.graph, self.id_map = self.build_graph()

    def plan_route(self, origin, destination):
        """
        The following function generates the route plan based on
        origin      : tuple containing x, y of the route's start position
        destination : tuple containing x, y of the route's end position

        return      : list of turn by turn navigation decision
        possible values (for now) are START, GO_STRAIGHT, LEFT, RIGHT,
        FOLLOW_LANE, STOP
        """

        threshold = 0.0523599   # 5 degrees
        route = self.path_search(origin, destination)
        plan = []
        plan.append('START')
        # Compare current edge and next edge to decide on action
        for i in range(len(route) - 2):
            current_edge = self.graph.edges[route[i], route[i + 1]]
            next_edge = self.graph.edges[route[i + 1], route[i + 2]]
            if not current_edge['intersection']:
                cv = current_edge['exit_vector']
                nv = None
                if next_edge['intersection']:
                    nv = next_edge['net_vector']
                    ca = round(math.atan2(*cv[::-1]) * 180 / math.pi)
                    na = round(math.atan2(*nv[::-1]) * 180 / math.pi)
                    angle_list = [ca, na]
                    for i in range(len(angle_list)):
                        if angle_list[i] > 0:
                            angle_list[i] = 180 - angle_list[i]
                        elif angle_list[i] < 0:
                            angle_list[i] = -1 * (180 + angle_list[i])
                    ca, na = angle_list
                    if abs(na - ca) < threshold:
                        action = 'GO_STRAIGHT'
                    elif na - ca > 0:
                        action = 'LEFT'
                    else:
                        action = 'RIGHT'
                else:
                    action = 'FOLLOW_LANE'
                plan.append(action)
        plan.append('STOP')
        return plan

    def path_search(self, origin, destination):
        """
        This function finds the shortest path connecting origin and destination
        using A* search with distance heuristic
        """
        xo, yo = origin
        xd, yd = destination
        start = self.localise(xo, yo)
        end = self.localise(xd, yd)

        def dist(n1, n2):
            (x1, y1) = self.graph.nodes[n1]['vertex']
            (x2, y2) = self.graph.nodes[n2]['vertex']
            return ((x1 - x2) ** 2 + (y1 - y2) ** 2) ** 0.5
        route = nx.astar_path(
            self.graph, source=self.id_map[start['entry']],
            target=self.id_map[end['exit']],
            heuristic=dist,
            weight='length')

        return route

    def localise(self, x, y):
        """
        This function finds the road segment closest to (x, y)
        """
        distance = float('inf')
        nearest = (distance, dict())

        # Measuring distances from segment waypoints and (x, y)
        for segment in self.topology:
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
        This function builds a graph representation of topology
        """
        graph = nx.DiGraph()
        # Map with structure {(x,y): id, ... }
        id_map = dict()

        for segment in self.topology:

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
        """
        x1, y1 = point1
        x2, y2 = point2
        return math.sqrt((x2 - x1)**2 + (y2 - y1)**2)

    def unit_vector(self, point1, point2):
        """
        This function returns the unit vector from point1 to point2
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
        """
        return vector1[0] * vector2[0] + vector1[1] * vector2[1]

    pass
