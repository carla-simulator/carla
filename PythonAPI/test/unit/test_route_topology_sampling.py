# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB). Licensed under the MIT license.
"""Sample real OpenDrive lane intervals without stepping beyond topology edges."""

import math
import os
from pathlib import Path
import sys
import unittest

import carla

ROOT = Path(os.environ.get("CARLA_SAMPLING_TEST_ROOT", Path(__file__).resolve().parents[3]))
sys.path.insert(0, str(ROOT / "PythonAPI/carla"))
from agents.navigation.global_route_planner import GlobalRoutePlanner


def _road(road_id, x, length, lane_id, rule, previous=None, following=None,
          curve=False, terminate_at=None):
    link = '<link>'
    lane_link = '<link>'
    if previous is not None:
        link += '<predecessor elementType="road" elementId="%s" contactPoint="end"/>' % previous
        lane_link += '<predecessor id="%s"/>' % lane_id
    if following is not None:
        link += '<successor elementType="road" elementId="%s" contactPoint="start"/>' % following
        lane_link += '<successor id="%s"/>' % lane_id
    link += '</link>'
    lane_link += '</link>'
    side = "right" if lane_id < 0 else "left"
    lane = ('<laneSection s="0"><center><lane id="0" type="none"/></center><%s>'
            '<lane id="%s" type="driving" level="false">%s'
            '<width sOffset="0" a="3.5" b="0" c="0" d="0"/></lane></%s></laneSection>'
            % (side, lane_id, lane_link, side))
    if terminate_at is not None:
        lane += '<laneSection s="%s"><center><lane id="0" type="none"/></center></laneSection>' % terminate_at
    geometry = '<arc curvature="0.1"/>' if curve else '<line/>'
    return ('<road id="%s" length="%s" junction="-1" rule="%s">%s'
            '<planView><geometry s="0" x="%s" y="0" hdg="0" length="%s">%s'
            '</geometry></planView><lanes>%s</lanes></road>'
            % (road_id, length, rule, link, x, length, geometry, lane))


def _map(lane_id=-1, rule="RHT", terminal_length=100.0, middle_length=1.0,
         curve=False, terminate_in_section=False):
    forward = (lane_id < 0) == (rule == "RHT")
    first_length = 100.0 if forward else terminal_length
    last_length = terminal_length if forward else 100.0
    first = _road(1, 0.0, first_length, lane_id, rule, following=2)
    middle = _road(2, first_length, middle_length, lane_id, rule, previous=1, following=3, curve=curve)
    last_x = first_length if curve else first_length + middle_length
    last = _road(3, last_x, last_length, lane_id, rule, previous=2,
                 terminate_at=0.5 if terminate_in_section else None)
    return carla.Map('bounded-sampling', '<OpenDRIVE><header revMajor="1" revMinor="4">'
        '<geoReference>+proj=tmerc +lat_0=0 +lon_0=0 +k=1 +x_0=0 +y_0=0 '
        '+datum=WGS84 +units=m</geoReference></header>' + first + middle + last + '</OpenDRIVE>')


class TestRouteTopologySampling(unittest.TestCase):
    def trace(self, lane_id=-1, rule="RHT", terminal_length=100.0):
        wmap = _map(lane_id, rule, terminal_length)
        forward = (lane_id < 0) == (rule == "RHT")
        self.assertEqual(wmap.get_waypoint_xodr(2, lane_id, 0.5).get_lane().is_positive_direction, forward)
        origin = wmap.get_waypoint_xodr(1 if forward else 3, lane_id, 90.0).transform.location
        target_road = 3 if forward else 1
        target_s = 10.0 if terminal_length == 100.0 else terminal_length / 2
        destination = wmap.get_waypoint_xodr(target_road, lane_id, target_s).transform.location
        planner = GlobalRoutePlanner(wmap, 2.0)
        result = planner.trace_route(origin, destination)
        self.assertTrue(result)
        self.assertEqual(result[-1][0].road_id, target_road)
        direction = 1.0 if forward else -1.0
        for (previous, _), (waypoint, _) in zip(result, result[1:]):
            delta = waypoint.transform.location.x - previous.transform.location.x
            self.assertGreaterEqual(direction * delta, -0.0001)
        return planner

    def test_short_connector_samples_stay_inside_their_lane(self):
        planner = GlobalRoutePlanner(_map(), 2.0)
        segment = next(segment for segment in planner._topology if segment['entry'].road_id == 2)
        self.assertTrue(all(point.road_id == 2 for point in segment['path']))

    def test_short_connector_does_not_create_a_backward_route_step(self):
        self.trace()

    def test_short_terminal_lane_retains_a_routable_edge(self):
        self.trace(terminal_length=0.5)

    def test_positive_rht_lane_does_not_backtrack(self):
        self.trace(lane_id=1)

    def test_positive_rht_short_terminal_lane_remains_routable(self):
        self.trace(lane_id=1, terminal_length=0.5)

    def test_positive_lht_lane_does_not_backtrack(self):
        self.trace(lane_id=1, rule="LHT")

    def test_negative_lht_short_terminal_lane_remains_routable(self):
        self.trace(lane_id=-1, rule="LHT", terminal_length=0.5)

    def test_lane_ending_before_the_road_uses_its_section_boundary(self):
        wmap = _map(terminate_in_section=True)
        planner = GlobalRoutePlanner(wmap, 2.0)
        self.assertAlmostEqual(wmap.get_waypoint_xodr(3, -1, 0.25).get_lane().length, 0.5)
        edge = planner._road_id_to_edge[3][0][-1]
        self.assertTrue(planner._graph.has_edge(*edge))
        self.assertAlmostEqual(planner._graph.edges[edge]['exit_waypoint'].s, 0.5, delta=0.0001)

    def test_closed_arc_is_sampled_by_lane_length_instead_of_endpoint_distance(self):
        length = 20.0 * math.pi
        planner = GlobalRoutePlanner(_map(middle_length=length, curve=True), 2.0)
        segment = next(segment for segment in planner._topology if segment['entry'].road_id == 2)
        self.assertGreater(len(segment['path']), 20)
        self.assertGreater(max(point.transform.location.distance(segment['entry'].transform.location)
                               for point in segment['path']), 10.0)

    def test_ordinary_long_segment_sampling_control(self):
        planner = GlobalRoutePlanner(_map(middle_length=100.0), 2.0)
        segment = next(segment for segment in planner._topology if segment['entry'].road_id == 2)
        self.assertGreater(len(segment['path']), 40)
        self.assertTrue(all(point.road_id == 2 and 0.0 < point.s < 100.0 for point in segment['path']))


if __name__ == "__main__":
    unittest.main()
