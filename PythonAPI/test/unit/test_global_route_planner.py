# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Plan native OpenDRIVE routes beside an unrelated loose-end lane."""

import math
from pathlib import Path
import sys
import unittest

import carla

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'carla'))
# The agents package ships in the source tree rather than the client wheel.
# pylint: disable-next=wrong-import-position
from agents.navigation.global_route_planner import GlobalRoutePlanner


def road_xml(identifier, geometry, connection, kind='driving'):
    """Describe one lane and its OpenDRIVE connectivity."""
    previous, following = connection
    x, y, heading, curvature = geometry
    shape = f'<arc curvature="{curvature}"/>' if curvature else '<line/>'
    links, lane_links = '', ''
    if previous is not None:
        links += f'<predecessor elementType="road" elementId="{previous}" contactPoint="end"/>'
        lane_links += '<predecessor id="-1"/>'
    if following is not None:
        if following == 'junction':
            links += '<successor elementType="junction" elementId="10"/>'
        else:
            links += f'<successor elementType="road" elementId="{following}" contactPoint="start"/>'
        lane_links += '<successor id="-1"/>'
    junction = 10 if identifier in (2, 4, 99) else -1
    return f'''<road name="r{identifier}" length="10" id="{identifier}" junction="{junction}">
      <link>{links}</link>
      <planView><geometry s="0" x="{x}" y="{y}" hdg="{heading}" length="10">
        {shape}</geometry></planView>
      <lanes><laneSection s="0"><center><lane id="0" type="none" level="false"/></center>
        <right><lane id="-1" type="{kind}" level="false"><link>{lane_links}</link>
          <width sOffset="0" a="3.5" b="0" c="0" d="0"/>
          <roadMark sOffset="0" type="none" color="white" laneChange="none"/>
        </lane></right></laneSection></lanes></road>'''


def junction_map(angle, loose_end, alternative):
    """Let native topology generation create the planner's loose-end edge."""
    roads = road_xml(1, (0, 0, 0, 0), (None, 'junction'))
    connections = ''
    for identifier, degrees in [(2, angle)] + ([(4, -150)] if alternative else []):
        heading = math.radians(degrees)
        curvature = heading / 10
        # Every branch shares the approach tangent, then curves to its exit.
        end = (10 + math.sin(heading) / curvature, (1 - math.cos(heading)) / curvature) if curvature else (20, 0)
        roads += road_xml(identifier, (10, 0, 0, curvature), (1, identifier + 1))
        roads += road_xml(identifier + 1, (*end, heading, 0), (identifier, None))
        connections += f'''<connection id="{identifier}" incomingRoad="1"
          connectingRoad="{identifier}" contactPoint="start"><laneLink from="-1" to="-1"/></connection>'''
    if loose_end:
        # Sidewalk lanes are omitted as topology entries, but can be successors.
        roads += road_xml(99, (10, 0, 0, 0), (1, None), kind='sidewalk')
        connections += '''<connection id="99" incomingRoad="1" connectingRoad="99" contactPoint="start">
          <laneLink from="-1" to="-1"/></connection>'''
    xml = f'''<OpenDRIVE><header revMajor="1" revMinor="4" name="loose" version="1"
        north="100" south="-100" east="100" west="-100">
        <geoReference>+proj=tmerc +lat_0=0 +lon_0=0 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m</geoReference>
      </header>{roads}<junction id="10" name="junction">{connections}</junction></OpenDRIVE>'''
    return carla.Map('loose', xml)


class TestGlobalRoutePlanner(unittest.TestCase):
    """Exercise production topology construction and complete route planning."""

    def assert_routes(self, loose_end):
        """Preserve route endpoints and absolute or relative turn decisions."""
        cases = [(0, False, 'STRAIGHT'), (90, False, 'LEFT'),
                 (-90, False, 'RIGHT'), (-60, True, 'LEFT')] if loose_end else [(0, False, 'STRAIGHT')]
        for angle, alternative, expected in cases:
            with self.subTest(angle=angle, alternative=alternative, loose_end=loose_end):
                wmap = junction_map(angle, loose_end, alternative)
                planner = GlobalRoutePlanner(wmap, 2)
                # Inspect constructor output without injecting planner state.
                # pylint: disable-next=protected-access
                loose = [data for _, _, data in planner._graph.edges(data=True)
                         if data['entry_waypoint'].road_id == 99]
                self.assertEqual(len(loose), int(loose_end))
                if loose_end:
                    self.assertIsNone(loose[0]['net_vector'])
                    self.assertTrue(loose[0]['path'])
                start = wmap.get_waypoint_xodr(1, -1, 1).transform.location
                destination = wmap.get_waypoint_xodr(2, -1, 9).transform.location
                route = planner.trace_route(start, destination)
                self.assertEqual(route[0][0].road_id, 1)
                self.assertEqual(route[-1][0].road_id, 2)
                self.assertLess(route[-1][0].transform.location.distance(destination), 4)
                selected = [option.name for waypoint, option in route if waypoint.road_id == 2 and waypoint.s > 0.5]
                self.assertGreater(len(selected), 1)
                self.assertEqual(set(selected), {expected})

    def test_routes_beside_loose_end(self):
        """An unrelated edge without a direction must not abort route planning."""
        self.assert_routes(loose_end=True)

    def test_routes_without_loose_end(self):
        """An ordinary route must retain its endpoints and straight decision."""
        self.assert_routes(loose_end=False)


if __name__ == '__main__':
    unittest.main()
