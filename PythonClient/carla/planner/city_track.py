# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

from carla.planner.graph import sldist

from carla.planner.astar import AStar
from carla.planner.map import CarlaMap


class CityTrack(object):

    def __init__(self, city_name):

        # These values are fixed for every city.
        self._node_density = 50.0
        self._pixel_density = 0.1643

        self._map = CarlaMap(city_name, self._pixel_density, self._node_density)

        self._astar = AStar()

        # Refers to the start position of the previous route computation
        self._previous_node = []

        # The current computed route
        self._route = None

    def project_node(self, position):
        """
            Projecting the graph node into the city road
        """

        node = self._map.convert_to_node(position)

        # To change the orientation with respect to the map standards

        node = tuple([int(x) for x in node])

        # Set to zero if it is less than zero.

        node = (max(0, node[0]), max(0, node[1]))
        node = (min(self._map.get_graph_resolution()[0] - 1, node[0]),
                min(self._map.get_graph_resolution()[1] - 1, node[1]))

        node = self._map.search_on_grid(node)

        return node

    def get_intersection_nodes(self):
        return self._map.get_intersection_nodes()

    def get_pixel_density(self):
        return self._pixel_density

    def get_node_density(self):
        return self._node_density

    def is_at_goal(self, source, target):
        return source == target

    def is_at_new_node(self, current_node):
        return current_node != self._previous_node

    def is_away_from_intersection(self, current_node):
        return self._closest_intersection_position(current_node) > 1

    def is_far_away_from_route_intersection(self, current_node):
        # CHECK FOR THE EMPTY CASE
        if self._route is None:
            raise RuntimeError('Impossible to find route'
                               + ' Current planner is limited'
                               + ' Try to select start points away from intersections')

        return self._closest_intersection_route_position(current_node,
                                                         self._route) > 4

    def compute_route(self, node_source, source_ori, node_target, target_ori):

        self._previous_node = node_source

        a_star = AStar()
        a_star.init_grid(self._map.get_graph_resolution()[0],
                         self._map.get_graph_resolution()[1],
                         self._map.get_walls_directed(node_source, source_ori,
                                                      node_target, target_ori), node_source,
                         node_target)

        route = a_star.solve()

        # JuSt a Corner Case
        # Clean this to avoid having to use this function
        if route is None:
            a_star = AStar()
            a_star.init_grid(self._map.get_graph_resolution()[0],
                             self._map.get_graph_resolution()[1], self._map.get_walls(),
                             node_source, node_target)

            route = a_star.solve()

        self._route = route

        return route

    def get_distance_closest_node_route(self, pos, route):
        distance = []

        for node_iter in route:

            if node_iter in self._map.get_intersection_nodes():
                distance.append(sldist(node_iter, pos))

        if not distance:
            return sldist(route[-1], pos)
        return sorted(distance)[0]


    def _closest_intersection_position(self, current_node):

        distance_vector = []
        for node_iterator in self._map.get_intersection_nodes():
            distance_vector.append(sldist(node_iterator, current_node))

        return sorted(distance_vector)[0]


    def _closest_intersection_route_position(self, current_node, route):

        distance_vector = []
        for _ in route:
            for node_iterator in self._map.get_intersection_nodes():
                distance_vector.append(sldist(node_iterator, current_node))

        return sorted(distance_vector)[0]

