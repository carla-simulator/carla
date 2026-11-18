# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Exercise the agents supplied by an installed CARLA wheel."""

import unittest

import carla

from agents.navigation.basic_agent import BasicAgent
from agents.navigation.behavior_agent import BehaviorAgent
from agents.navigation.constant_velocity_agent import ConstantVelocityAgent
from agents.navigation.global_route_planner import GlobalRoutePlanner
from agents.navigation.local_planner import RoadOption


def native_route_map():
    """Reuse three linked native roads with consecutive driving sections."""
    xml = ('<OpenDRIVE><header revMajor="1" revMinor="4"><geoReference>'
           '+proj=tmerc +lat_0=0 +lon_0=0 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m'
           '</geoReference></header>')
    for role, length, x in ((1, 100, 0), (2, 200, 100), (3, 100.1, 300)):
        if role == 1:
            link = '<successor elementType="road" elementId="2" contactPoint="start"/>'
        elif role == 2:
            link = ('<predecessor elementType="road" elementId="1" contactPoint="end"/>'
                    '<successor elementType="road" elementId="3" contactPoint="start"/>')
        else:
            link = '<predecessor elementType="road" elementId="2" contactPoint="end"/>'
        xml += (f'<road id="{role}" length="{length}" junction="-1" rule="RHT">'
                f'<link>{link}</link><planView><geometry s="0" x="{x}" y="0" hdg="0" length="{length}">'
                '<line/></geometry></planView><lanes>')
        for station in ((0, 100) if role == 2 else (0,)):
            xml += (f'<laneSection s="{station}"><center><lane id="0" type="none"/></center><right>'
                    '<lane id="-1" type="driving"><link><predecessor id="-1"/><successor id="-1"/></link>'
                    '<width sOffset="0" a="3.5" b="0" c="0" d="0"/></lane></right></laneSection>')
        xml += '</lanes></road>'
    return carla.Map('installed-agent-route', xml + '</OpenDRIVE>')


class TestAgentPackage(unittest.TestCase):
    """Keep installed legacy agents usable with the native client."""

    def test_installed_agents_plan_a_native_route(self):
        """Plan between native lane points with the distributed agent code."""
        self.assertTrue(issubclass(BehaviorAgent, BasicAgent))
        self.assertTrue(issubclass(ConstantVelocityAgent, BasicAgent))
        native_map = native_route_map()
        origin = native_map.get_waypoint_xodr(1, -1, 90).transform.location
        destination = native_map.get_waypoint_xodr(2, -1, 105).transform.location
        route = GlobalRoutePlanner(native_map, 2.0).trace_route(origin, destination)
        self.assertGreater(len(route), 2)
        self.assertLess(route[0][0].transform.location.distance(origin), 2.1)
        self.assertLess(route[-1][0].transform.location.distance(destination), 4.1)
        self.assertTrue(all(isinstance(waypoint, carla.Waypoint) for waypoint, _ in route))
        self.assertTrue(all(waypoint.road_id in (1, 2) and waypoint.lane_id == -1
                            for waypoint, _ in route))
        self.assertTrue(all(option == RoadOption.LANEFOLLOW for _, option in route))

if __name__ == '__main__':
    unittest.main()
