# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Native junction sections retain their turn decision for any topology order."""

from pathlib import Path
import sys
import unittest

import carla

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / 'PythonAPI/carla'))
from agents.navigation.global_route_planner import GlobalRoutePlanner  # pylint: disable=wrong-import-position
from agents.navigation.local_planner import RoadOption  # pylint: disable=wrong-import-position


def native_junction(junction=True):
    """Use one valid connecting road with two consecutive driving lane sections."""
    xml = ('<OpenDRIVE><header revMajor="1" revMinor="4"><geoReference>'
           '+proj=tmerc +lat_0=0 +lon_0=0 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m'
           '</geoReference></header>')
    for role, length, x in ((1, 100, 0), (2, 200, 100), (3, 100.1, 300)):
        in_junction = role == 2 and junction
        if role == 1:
            link = ('<successor elementType="junction" elementId="10"/>' if junction else
                    '<successor elementType="road" elementId="2" contactPoint="start"/>')
        elif role == 2:
            link = ('<predecessor elementType="road" elementId="1" contactPoint="end"/>'
                    '<successor elementType="road" elementId="3" contactPoint="start"/>')
        else:
            link = ('<predecessor elementType="junction" elementId="10"/>' if junction else
                    '<predecessor elementType="road" elementId="2" contactPoint="end"/>')
        xml += (f'<road id="{role}" length="{length}" junction="{10 if in_junction else -1}" rule="RHT">'
                f'<link>{link}</link><planView><geometry s="0" x="{x}" y="0" hdg="0" length="{length}">'
                '<line/></geometry></planView><lanes>')
        for station in ((0, 100) if role == 2 else (0,)):
            xml += (f'<laneSection s="{station}"><center><lane id="0" type="none"/></center><right>'
                    '<lane id="-1" type="driving"><link><predecessor id="-1"/><successor id="-1"/></link>'
                    '<width sOffset="0" a="3.5" b="0" c="0" d="0"/></lane></right></laneSection>')
        xml += '</lanes></road>'
    if junction:
        xml += ('<junction id="10"><connection id="0" incomingRoad="1" connectingRoad="2" '
                'contactPoint="start"><laneLink from="-1" to="-1"/></connection></junction>')
    return carla.Map('two-section-turn', xml + '</OpenDRIVE>')


class TopologyOrder:
    """Change only the unspecified list order; all geometry and queries stay native."""

    def __init__(self, native_map, first_road):
        self.native_map = native_map
        self.first_road = first_road

    def __getattr__(self, name):
        return getattr(self.native_map, name)

    def get_topology(self):
        """Return the same native pairs in another valid API order."""
        return sorted(self.native_map.get_topology(), key=lambda pair: pair[0].road_id != self.first_road)


class TestIntersectionZeroNode(unittest.TestCase):
    """Graph node zero belongs to a real junction exit, distinct from sentinel -1."""

    def assert_route(self, zero_end, junction=True):
        """Derive the graph and public route from actual native lane sections."""
        native_map = native_junction(junction)
        self.assertEqual(len(native_map.get_topology()), 4)
        wrapped = TopologyOrder(native_map, 3 if zero_end else 1)
        planner = GlobalRoutePlanner(wrapped, 2.)
        origin = native_map.get_waypoint_xodr(1, -1, 90).transform.location
        destination = native_map.get_waypoint_xodr(2, -1, 105).transform.location
        # Verify the fixture reaches a real zero exit without injecting state.
        # pylint: disable-next=protected-access
        nodes = planner._path_search(origin, destination)
        self.assertEqual(len(nodes), 4)
        self.assertEqual(nodes[-1] == 0, zero_end)
        route = planner.trace_route(origin, destination)
        self.assertEqual(route[0][0].road_id, 1)
        self.assertEqual(route[-1][0].road_id, 2)
        self.assertLess(route[-1][0].transform.location.distance(destination), 4)
        interior = [option for waypoint, option in route if waypoint.road_id == 2 and waypoint.s > 100.000001]
        self.assertTrue(interior)
        expected = RoadOption.STRAIGHT if junction else RoadOption.LANEFOLLOW
        self.assertTrue(all(option == expected for option in interior))

    def test_zero_exit_keeps_the_turn_through_both_native_junction_sections(self):
        """A real zero exit retains the decision in the second section."""
        self.assert_route(True)

    def test_nonzero_exit_control_keeps_its_existing_turn(self):
        """Nonzero exits preserve the existing turn labels."""
        self.assert_route(False)

    def test_nonjunction_zero_node_control_stays_lane_follow(self):
        """A zero node alone does not turn a nonjunction into an intersection."""
        self.assert_route(True, junction=False)


if __name__ == '__main__':
    unittest.main()
