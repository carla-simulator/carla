# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Keep native OpenDRIVE route samples inside their lane intervals."""

import math
from pathlib import Path
import sys
import unittest

import carla

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'carla'))
# The client wheel does not contain the agents package.
# pylint: disable-next=wrong-import-position
from agents.navigation.global_route_planner import GlobalRoutePlanner


def native_map(lengths=(20.0, 1.0, 20.0), lane_id=-1, rule='RHT', curve=False, section_end=False):
    """Join three native roads with consistent geometry and lane links."""
    roads = []
    pose = {'x': 0.0, 'y': 0.0, 'heading': 0.0}
    for identifier, length in enumerate(lengths, 1):
        curvature = 0.1 if curve and identifier == 2 else 0.0
        links, lane_links = '', ''
        if identifier > 1:
            links += f'<predecessor elementType="road" elementId="{identifier - 1}" contactPoint="end"/>'
            lane_links += f'<predecessor id="{lane_id}"/>'
        if identifier < 3:
            links += f'<successor elementType="road" elementId="{identifier + 1}" contactPoint="start"/>'
            lane_links += f'<successor id="{lane_id}"/>'
        section = f'''<laneSection s="0"><center><lane id="0" type="none"/></center>
          <{'right' if lane_id < 0 else 'left'}><lane id="{lane_id}" type="driving" level="false">
            <link>{lane_links}</link><width sOffset="0" a="3.5" b="0" c="0" d="0"/>
          </lane></{'right' if lane_id < 0 else 'left'}></laneSection>'''
        if section_end and identifier == 3:
            section += '<laneSection s="0.5"><center><lane id="0" type="none"/></center></laneSection>'
        shape = f'<arc curvature="{curvature}"/>' if curvature else '<line/>'
        roads.append(f'''<road id="{identifier}" length="{length}" junction="-1" rule="{rule}">
          <link>{links}</link><planView><geometry s="0" x="{pose['x']}" y="{pose['y']}"
            hdg="{pose['heading']}" length="{length}">{shape}</geometry></planView>
          <lanes>{section}</lanes></road>''')
        if curvature:
            pose['x'] += (math.sin(pose['heading'] + length * curvature) - math.sin(pose['heading'])) / curvature
            pose['y'] += (math.cos(pose['heading']) - math.cos(pose['heading'] + length * curvature)) / curvature
        else:
            pose['x'] += length * math.cos(pose['heading'])
            pose['y'] += length * math.sin(pose['heading'])
        pose['heading'] += length * curvature
    return carla.Map('intervals', '''<OpenDRIVE><header revMajor="1" revMinor="4">
      <geoReference>+proj=tmerc +lat_0=0 +lon_0=0 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m</geoReference>
      </header>''' + ''.join(roads) + '</OpenDRIVE>')


class TestRouteTopologySampling(unittest.TestCase):
    """Exercise the production constructor and public route query."""

    def assert_forward_route(self, wmap, planner, forward=True, terminal_length=20.0):
        """Keep road progression and every spatial step in travel order."""
        first, last = (1, 3) if forward else (3, 1)
        lane_id = wmap.get_topology()[0][0].lane_id
        origin = wmap.get_waypoint_xodr(first, lane_id, 10).transform.location
        target = wmap.get_waypoint_xodr(last, lane_id, terminal_length / 2).transform.location
        route = planner.trace_route(origin, target)
        self.assertTrue(route)
        self.assertEqual(route[-1][0].road_id, last)
        for (previous, _), (waypoint, _) in zip(route, route[1:]):
            self.assertGreaterEqual((1 if forward else -1) * (
                waypoint.transform.location.x - previous.transform.location.x), -0.0001)

    def test_short_connector_stays_inside_its_lane(self):
        """A fixed step must not borrow a waypoint from the following road."""
        for lane_id, rule in ((-1, 'RHT'), (1, 'RHT'), (1, 'LHT'), (-1, 'LHT')):
            with self.subTest(lane_id=lane_id, rule=rule):
                wmap = native_map(lane_id=lane_id, rule=rule)
                planner = GlobalRoutePlanner(wmap, 2)
                # Inspect actual constructor output without replacing its graph.
                # pylint: disable-next=protected-access
                segment = next(item for item in planner._topology if item['entry'].road_id == 2)
                self.assertTrue(all((point.road_id, point.section_id, point.lane_id) == (2, 0, lane_id)
                                    for point in segment['path']))

    def test_short_connector_does_not_reverse_the_route(self):
        """The public route must not step past the connector and backtrack."""
        for lane_id, rule in ((-1, 'RHT'), (1, 'RHT'), (1, 'LHT'), (-1, 'LHT')):
            with self.subTest(lane_id=lane_id, rule=rule):
                forward = (lane_id < 0) == (rule == 'RHT')
                wmap = native_map(lane_id=lane_id, rule=rule)
                self.assert_forward_route(wmap, GlobalRoutePlanner(wmap, 2), forward)

    def test_short_terminal_lane_retains_a_routable_edge(self):
        """A terminal remainder smaller than one sample still has an edge."""
        wmap = native_map(lengths=(20, 1, 0.5))
        self.assert_forward_route(wmap, GlobalRoutePlanner(wmap, 2), terminal_length=0.5)

    def test_lane_section_boundary_limits_the_terminal_remainder(self):
        """Sampling ends at the lane section rather than the whole road end."""
        wmap = native_map(section_end=True)
        planner = GlobalRoutePlanner(wmap, 2)
        # pylint: disable-next=protected-access
        edge = planner._road_id_to_edge[3][0][-1]
        # pylint: disable-next=protected-access
        self.assertTrue(planner._graph.has_edge(*edge))
        # pylint: disable-next=protected-access
        self.assertAlmostEqual(planner._graph.edges[edge]['exit_waypoint'].s, 0.5, delta=0.0001)

    def test_curved_lane_is_sampled_by_longitudinal_distance(self):
        """Nearby endpoints cannot truncate the interior of a long arc."""
        wmap = native_map(lengths=(20, 20 * math.pi - 1.5, 20), curve=True)
        planner = GlobalRoutePlanner(wmap, 2)
        # pylint: disable-next=protected-access
        segment = next(item for item in planner._topology if item['entry'].road_id == 2)
        self.assertGreater(len(segment['path']), 20)
        self.assertGreater(max(point.transform.location.distance(segment['entry'].transform.location)
                               for point in segment['path']), 10)
        self.assertTrue(all(point.road_id == 2 for point in segment['path']))

    def test_ordinary_segment_control(self):
        """Ordinary lane interiors retain ordered samples and a complete route."""
        wmap = native_map(lengths=(20, 12, 20))
        planner = GlobalRoutePlanner(wmap, 2)
        # pylint: disable-next=protected-access
        segment = next(item for item in planner._topology if item['entry'].road_id == 2)
        samples = [point.s for point in segment['path']]
        self.assertGreaterEqual(len(samples), 4)
        self.assertEqual(samples, sorted(samples))
        self.assertTrue(all(point.road_id == 2 for point in segment['path']))
        self.assert_forward_route(wmap, planner)


if __name__ == '__main__':
    unittest.main()
