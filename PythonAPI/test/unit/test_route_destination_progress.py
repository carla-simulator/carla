# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB). Licensed under the MIT license.
"""Real native OpenDrive routes stop at the requested projected destination."""

import os
from pathlib import Path
import sys
import unittest

import carla
import networkx as nx

ROOT = Path(os.environ.get("CARLA_ROUTE_TEST_ROOT", Path(__file__).resolve().parents[3]))
sys.path.insert(0, str(ROOT / "PythonAPI/carla"))
from agents.navigation.global_route_planner import GlobalRoutePlanner
from agents.navigation.local_planner import RoadOption


def _road(road_id=1, x=0.0, positive=False, elevation=0.0, link="", lane_link=""):
    side, lane_id = ("left", 1) if positive else ("right", -1)
    return (
        '<road id="%d" length="100" junction="-1">%s'
        '<planView><geometry s="0" x="%s" y="0" hdg="0" length="100"><line/>'
        '</geometry></planView><elevationProfile><elevation s="0" a="%s" b="0" c="0" d="0"/>'
        '</elevationProfile><lanes><laneSection s="0"><center><lane id="0" type="none"/>'
        '</center><%s><lane id="%d" type="driving" level="false">%s'
        '<width sOffset="0" a="3.5" b="0" c="0" d="0"/></lane></%s>'
        '</laneSection></lanes></road>'
    ) % (road_id, link, x, elevation, side, lane_id, lane_link, side)


def _map(roads):
    return carla.Map("route-destination", '<OpenDRIVE><header revMajor="1" revMinor="4">'
                     '<geoReference>+proj=tmerc +lat_0=0 +lon_0=0 +k=1 +x_0=0 +y_0=0 '
                     '+datum=WGS84 +units=m</geoReference></header>' + roads + '</OpenDRIVE>')


def _line_map(positive=False, elevation=0.0):
    lane_id = 1 if positive else -1
    first = _road(positive=positive, elevation=elevation,
                  link='<link><successor elementType="road" elementId="2" contactPoint="start"/></link>',
                  lane_link='<link><successor id="%d"/></link>' % lane_id)
    second = _road(2, 100.0, positive=positive, elevation=elevation,
                   link='<link><predecessor elementType="road" elementId="1" contactPoint="end"/></link>',
                   lane_link='<link><predecessor id="%d"/></link>' % lane_id)
    return _map(first + second)


def _junction_map(two_connectors=False, disconnected=False):
    first = _road(link='<link><successor elementType="junction" elementId="10"/></link>',
                  lane_link='<link><successor id="-1"/></link>')
    connector = _road(2, 100.0,
        link='<link><predecessor elementType="road" elementId="1" contactPoint="end"/>'
             '<successor elementType="road" elementId="3" contactPoint="start"/></link>',
        lane_link='<link><predecessor id="-1"/><successor id="-1"/></link>')
    connector = connector.replace('junction="-1"', 'junction="10"')
    if two_connectors:
        second = _road(3, 200.0,
            link='<link><predecessor elementType="road" elementId="2" contactPoint="end"/>'
                 '<successor elementType="road" elementId="4" contactPoint="start"/></link>',
            lane_link='<link><predecessor id="-1"/><successor id="-1"/></link>')
        second = second.replace('junction="-1"', 'junction="10"')
        second += _road(4, 300.0,
            link='<link><predecessor elementType="junction" elementId="10"/></link>',
            lane_link='<link><predecessor id="-1"/></link>')
    else:
        second = _road(3, 200.0,
            link='<link><predecessor elementType="junction" elementId="10"/></link>',
            lane_link='<link><predecessor id="-1"/></link>')
    junction = ('<junction id="10" name="route-state"><connection id="0" incomingRoad="1" '
                'connectingRoad="2" contactPoint="start"><laneLink from="-1" to="-1"/>'
                '</connection></junction>')
    other = ""
    if disconnected:
        other = _road(99, 1000.0,
            link='<link><successor elementType="road" elementId="100" contactPoint="start"/></link>',
            lane_link='<link><successor id="-1"/></link>')
        other += _road(100, 1100.0,
            link='<link><predecessor elementType="road" elementId="99" contactPoint="end"/></link>',
            lane_link='<link><predecessor id="-1"/></link>')
    return _map(first + connector + second + junction + other)


class TestRouteDestinationProgress(unittest.TestCase):
    def route(self, wmap, start_s, end_s, resolution=0.1, lane_id=-1,
              start_road=1, end_road=1, height=2.0):
        origin = wmap.get_waypoint_xodr(start_road, lane_id, start_s).transform.location
        destination = wmap.get_waypoint_xodr(end_road, lane_id, end_s).transform.location
        origin.z += height
        destination.z += height
        result = GlobalRoutePlanner(wmap, resolution).trace_route(origin, destination)
        self.assertTrue(result)
        return result

    def test_elevated_destination_stops_near_its_projected_waypoint(self):
        result = self.route(_line_map(), 10.0, 30.0)
        self.assertEqual(result[-1][0].road_id, 1)
        self.assertAlmostEqual(result[-1][0].s, 30.0, delta=0.11)

    def test_destination_below_the_road_stops_at_the_projected_waypoint(self):
        result = self.route(_line_map(elevation=5.0), 10.0, 30.0, height=-5.0)
        self.assertEqual(result[-1][0].road_id, 1)
        self.assertAlmostEqual(result[-1][0].s, 30.0, delta=0.11)

    def test_destination_between_samples_stops_at_the_nearest_sample(self):
        result = self.route(_line_map(), 10.0, 33.0, resolution=2.0, height=10.0)
        self.assertEqual(result[-1][0].road_id, 1)
        self.assertAlmostEqual(result[-1][0].s, 33.0, delta=1.01)

    def test_positive_lane_stops_before_passing_the_destination(self):
        result = self.route(_line_map(positive=True), 80.0, 30.0, lane_id=1)
        self.assertAlmostEqual(result[-1][0].s, 30.0, delta=0.11)

    def test_only_the_final_road_is_truncated(self):
        first = _road(link='<link><successor elementType="road" elementId="2" contactPoint="start"/></link>',
                      lane_link='<link><successor id="-1"/></link>')
        second = _road(2, 100.0,
                       link='<link><predecessor elementType="road" elementId="1" contactPoint="end"/></link>',
                       lane_link='<link><predecessor id="-1"/></link>')
        result = self.route(_map(first + second), 10.0, 40.0, resolution=1.0, end_road=2, height=5.0)
        self.assertIn(1, {waypoint.road_id for waypoint, _ in result})
        self.assertEqual(result[-1][0].road_id, 2)
        self.assertAlmostEqual(result[-1][0].s, 40.0, delta=1.01)

    def test_road_level_destination_preserves_existing_distance_tolerance(self):
        result = self.route(_line_map(), 10.0, 30.0, resolution=2.0, height=0.0)
        self.assertLessEqual(abs(result[-1][0].s - 30.0), 4.01)

    def test_origin_already_past_the_destination_does_not_drive_to_the_road_end(self):
        result = self.route(_line_map(), 60.0, 30.0)
        self.assertEqual(len(result), 1)
        self.assertAlmostEqual(result[0][0].s, 60.0, delta=0.11)


class TestRouteDecisionState(unittest.TestCase):
    @staticmethod
    def location(wmap, road, s):
        return wmap.get_waypoint_xodr(road, -1, s).transform.location

    @staticmethod
    def signature(route):
        return [(waypoint.id, option) for waypoint, option in route]

    def assert_fresh_equivalent(self, planner, wmap, origin, destination):
        actual = planner.trace_route(origin, destination)
        expected = GlobalRoutePlanner(wmap, 2.0).trace_route(origin, destination)
        self.assertEqual(self.signature(actual), self.signature(expected))

    def test_repeated_junction_query_matches_a_fresh_planner(self):
        wmap = _junction_map()
        origin, destination = self.location(wmap, 1, 90.0), self.location(wmap, 2, 5.0)
        planner = GlobalRoutePlanner(wmap, 2.0)
        first = planner.trace_route(origin, destination)
        self.assertIn(RoadOption.STRAIGHT, [option for _, option in first])
        self.assert_fresh_equivalent(planner, wmap, origin, destination)

    def test_previous_destination_does_not_change_a_new_route_decision(self):
        wmap = _junction_map()
        origin = self.location(wmap, 1, 90.0)
        planner = GlobalRoutePlanner(wmap, 2.0)
        planner.trace_route(origin, self.location(wmap, 2, 5.0))
        self.assert_fresh_equivalent(planner, wmap, origin, self.location(wmap, 2, 12.0))

    def test_failed_query_does_not_change_the_next_route_decisions(self):
        wmap = _junction_map(disconnected=True)
        origin, destination = self.location(wmap, 1, 90.0), self.location(wmap, 2, 5.0)
        planner = GlobalRoutePlanner(wmap, 2.0)
        planner.trace_route(origin, destination)
        with self.assertRaises(nx.NetworkXNoPath):
            planner.trace_route(origin, self.location(wmap, 99, 10.0))
        self.assert_fresh_equivalent(planner, wmap, origin, destination)

    def test_consecutive_intersection_edges_share_the_current_turn(self):
        wmap = _junction_map(two_connectors=True)
        planner = GlobalRoutePlanner(wmap, 2.0)
        route = planner._path_search(self.location(wmap, 1, 90.0), self.location(wmap, 3, 5.0))
        decisions = [planner._turn_decision(index, route) for index in range(len(route) - 1)]
        self.assertEqual(decisions, [RoadOption.LANEFOLLOW, RoadOption.STRAIGHT, RoadOption.STRAIGHT])

    def test_repeated_nonjunction_destination_control(self):
        wmap = _junction_map()
        planner = GlobalRoutePlanner(wmap, 2.0)
        origin, destination = self.location(wmap, 1, 90.0), self.location(wmap, 3, 30.0)
        planner.trace_route(origin, destination)
        self.assert_fresh_equivalent(planner, wmap, origin, destination)


if __name__ == "__main__":
    unittest.main()
