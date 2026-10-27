# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Search actual native lane networks using an independent weighted-path oracle."""

# Exercise the complete existing graph builder and its actual route-search entry.
# pylint: disable=protected-access

import math
import os
from pathlib import Path
import sys
import unittest

import carla
import networkx as nx

ROOT = Path(os.environ.get('CARLA_ROUTE_COST_TEST_ROOT', Path(__file__).resolve().parents[3]))
sys.path.insert(0, str(ROOT / 'PythonAPI/carla'))
from agents.navigation.global_route_planner import GlobalRoutePlanner  # pylint: disable=wrong-import-position


def road_xml(road, lane_id, rule):
    """Serialize authored native road geometry and explicit directed lane links."""
    road_id, length, junction, x, y, heading, geometry, links = road
    side = 'right' if lane_id < 0 else 'left'
    return (f'<road id="{road_id}" length="{length}" junction="{junction}" rule="{rule}">'
            f'<link>{links}</link><planView><geometry s="0" x="{x}" y="{y}" '
            f'hdg="{heading}" length="{length}">{geometry}</geometry></planView>'
            '<lanes><laneSection s="0"><center><lane id="0" type="none"/></center>'
            f'<{side}><lane id="{lane_id}" type="driving"><link>'
            f'<predecessor id="{lane_id}"/><successor id="{lane_id}"/></link>'
            '<width sOffset="0" a="3.5" b="0" c="0" d="0"/>'
            f'</lane></{side}></laneSection></lanes></road>')


def diamond(rule='RHT'):
    """Build a true native fork with a longer arc and two shorter straight roads."""
    lane_id = -1 if rule == 'RHT' else 1
    offset = lane_id * 1.75
    # Correct the reference-line radius for the lane-centre lateral offset.
    # All alternative lane-centre endpoints coincide at x=10 and x=100.
    radius = 90 / math.sqrt(2) + offset
    length = radius * math.pi / 2
    roads = [
        (1, 10, -1, 0, 0, 0, '<line/>',
         '<successor elementType="junction" elementId="9"/>'),
        (2, length, 9, 10 + math.sin(-math.pi / 4) * offset,
         offset * (1 - math.cos(math.pi / 4)), -math.pi / 4,
         f'<arc curvature="{1 / radius}"/>',
         '<predecessor elementType="road" elementId="1" contactPoint="end"/>'
         '<successor elementType="road" elementId="5" contactPoint="start"/>'),
        (3, 45, 9, 10, 0, 0, '<line/>',
         '<predecessor elementType="road" elementId="1" contactPoint="end"/>'
         '<successor elementType="road" elementId="4" contactPoint="start"/>'),
        (4, 45, 9, 55, 0, 0, '<line/>',
         '<predecessor elementType="road" elementId="3" contactPoint="end"/>'
         '<successor elementType="road" elementId="5" contactPoint="start"/>'),
        (5, 10, -1, 100, 0, 0, '<line/>',
         '<predecessor elementType="junction" elementId="9"/>'),
        (6, 10, -1, 300, 0, 0, '<line/>',
         '<successor elementType="road" elementId="7" contactPoint="start"/>'),
        (7, 10, -1, 310, 0, 0, '<line/>',
         '<predecessor elementType="road" elementId="6" contactPoint="end"/>'),
    ]
    xml = ('<OpenDRIVE><header revMajor="1" revMinor="4"><geoReference>'
           '+proj=tmerc +lat_0=0 +lon_0=0 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m'
           '</geoReference></header>')
    xml += ''.join(road_xml(road, lane_id, rule) for road in roads)
    xml += '<junction id="9">'
    for identifier, road_id in enumerate((2, 3)):
        xml += (f'<connection id="{identifier}" incomingRoad="1" connectingRoad="{road_id}" '
                f'contactPoint="start"><laneLink from="{lane_id}" to="{lane_id}"/></connection>')
    return carla.Map('native-diamond', xml + '</junction></OpenDRIVE>'), lane_id


class TestRouteSearchCost(unittest.TestCase):
    """Valid native alternatives must minimize the graph's existing edge costs."""

    @staticmethod
    def paths(rule, resolution):
        """Run the actual search and an independent Dijkstra oracle on its graph."""
        native_map, lane_id = diamond(rule)
        origin = native_map.get_waypoint_xodr(1, lane_id, 5).transform.location
        destination = native_map.get_waypoint_xodr(5, lane_id, 5).transform.location
        planner = GlobalRoutePlanner(native_map, resolution)
        start, end = planner._localize(origin), planner._localize(destination)
        actual = planner._path_search(origin, destination)
        expected = nx.dijkstra_path(planner._graph, start[0], end[0], weight='length') + [end[1]]
        return native_map, lane_id, planner, actual, expected

    def test_sparse_sampling_returns_the_minimum_existing_cost(self):
        """A metre-valued heuristic overestimates costs measured in samples."""
        for rule in ('RHT', 'LHT'):
            for resolution in (2., 4.):
                with self.subTest(rule=rule, sampling_resolution=resolution):
                    native_map, lane_id, planner, actual, expected = self.paths(rule, resolution)
                    self.assertEqual({point.road_id for point in
                                      native_map.get_waypoint_xodr(1, lane_id, 9).next(2)}, {2, 3})
                    self.assertEqual(nx.path_weight(planner._graph, actual, 'length'),
                                     nx.path_weight(planner._graph, expected, 'length'))

    def test_dense_sampling_keeps_the_existing_straight_route_control(self):
        """A case where the original estimate was admissible retains its route."""
        for rule in ('RHT', 'LHT'):
            for resolution in (.5, 1.):
                with self.subTest(rule=rule, sampling_resolution=resolution):
                    _, _, planner, actual, expected = self.paths(rule, resolution)
                    self.assertEqual(nx.path_weight(planner._graph, actual, 'length'),
                                     nx.path_weight(planner._graph, expected, 'length'))
                    self.assertEqual([planner._graph.edges[first, second]['entry_waypoint'].road_id
                                      for first, second in zip(actual, actual[1:])], [1, 3, 4, 5])

    def test_localized_destination_edge_is_still_appended_control(self):
        """Searching at the destination retains the endpoint-edge contract."""
        native_map, lane_id = diamond()
        planner = GlobalRoutePlanner(native_map, 2.)
        location = native_map.get_waypoint_xodr(5, lane_id, 5).transform.location
        edge = planner._localize(location)
        self.assertEqual(planner._path_search(location, location), list(edge))


    def test_unreachable_destination_keeps_no_path_control(self):
        """A disconnected native road remains unreachable without invented links."""
        native_map, lane_id = diamond()
        planner = GlobalRoutePlanner(native_map, 2.)
        origin = native_map.get_waypoint_xodr(1, lane_id, 5).transform.location
        destination = native_map.get_waypoint_xodr(6, lane_id, 5).transform.location
        with self.assertRaises(nx.NetworkXNoPath):
            planner._path_search(origin, destination)

    def test_zero_cost_lane_change_edges_remain_supported_control(self):
        """Native parallel-lane links retain their existing zero edge cost."""
        for rule, lane_id, side in (('RHT', -1, 'right'), ('LHT', 1, 'left')):
            with self.subTest(rule=rule):
                xml = road_xml((1, 100, -1, 0, 0, 0, '<line/>',
                                '<successor elementType="road" elementId="2" contactPoint="start"/>'),
                               lane_id, rule)
                xml += road_xml((2, 10, -1, 100, 0, 0, '<line/>',
                                 '<predecessor elementType="road" elementId="1" contactPoint="end"/>'),
                                lane_id, rule)
                extra = (f'<lane id="{2 * lane_id}" type="driving"><link>'
                         f'<predecessor id="{2 * lane_id}"/><successor id="{2 * lane_id}"/></link>'
                         '<width sOffset="0" a="3.5" b="0" c="0" d="0"/></lane>')
                xml = xml.replace(f'</{side}>', extra + f'</{side}>')
                xml = xml.replace('</lane>',
                                  '<roadMark sOffset="0" type="broken" width=".15" '
                                  'laneChange="both"/></lane>')
                native_map = carla.Map('parallel-lane-control',
                                       '<OpenDRIVE><header revMajor="1" revMinor="4"/>' + xml + '</OpenDRIVE>')
                planner = GlobalRoutePlanner(native_map, 2.)
                origin = native_map.get_waypoint_xodr(1, lane_id, 10).transform.location
                destination = native_map.get_waypoint_xodr(1, 2 * lane_id, 80).transform.location
                start, end = planner._localize(origin), planner._localize(destination)
                expected = nx.dijkstra_path(planner._graph, start[0], end[0], weight='length') + [end[1]]
                actual = planner._path_search(origin, destination)
                self.assertTrue(any(edge['length'] == 0 for _, _, edge in planner._graph.edges(data=True)))
                self.assertEqual(nx.path_weight(planner._graph, actual, 'length'),
                                 nx.path_weight(planner._graph, expected, 'length'))


if __name__ == '__main__':
    unittest.main()
