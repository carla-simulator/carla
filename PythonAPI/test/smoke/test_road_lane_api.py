# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Smoke tests for the carla.Road / carla.LaneSection / carla.Lane API.

These cover the load-bearing behaviour rather than every accessor: that the new
objects agree with the waypoints they were derived from, that the lane graph is
consistent in both directions, that identity works well enough for dicts and
sets, and that the junction accessors describe the junction they came from.
"""

import math

import carla

from . import SmokeTest


class TestRoadLaneAPI(SmokeTest):

    def setUp(self):
        super(TestRoadLaneAPI, self).setUp()
        self.map = self.world.get_map()

    def _a_normal_road(self, min_length=0.0):
        """A non-junction road with driving lanes, at least min_length long."""
        for road in self.map.get_roads():
            if road.is_junction or road.length < min_length:
                continue
            if road.get_lanes(0.0):
                return road
        self.fail("no non-junction road >= %.1f m with driving lanes on %s"
                  % (min_length, self.map.name))

    # -- carla.Map entry points ---------------------------------------------

    def test_map_roads_are_enumerable_and_addressable(self):
        print("TestRoadLaneAPI.test_map_roads_are_enumerable_and_addressable")
        roads = self.map.get_roads()
        self.assertGreater(len(roads), 0, "map reports no roads")

        ids = [r.id for r in roads]
        self.assertEqual(len(ids), len(set(ids)), "duplicate road ids")

        # Every enumerated road is retrievable by its own id, and comes back equal.
        for road in roads[:20]:
            fetched = self.map.get_road(road.id)
            self.assertIsNotNone(fetched, "get_road(%d) returned None" % road.id)
            self.assertEqual(road, fetched)
            self.assertEqual(hash(road), hash(fetched))

        # An id that cannot exist yields None rather than raising.
        self.assertIsNone(self.map.get_road(max(ids) + 10000))

    def test_map_junctions_are_enumerable(self):
        print("TestRoadLaneAPI.test_map_junctions_are_enumerable")
        junctions = self.map.get_junctions()
        self.assertGreater(len(junctions), 0, "map reports no junctions")
        for junction in junctions[:5]:
            self.assertEqual(junction.id, self.map.get_junction_by_id(junction.id).id)

    # -- waypoint <-> lane <-> road agreement -------------------------------

    def test_waypoint_lane_and_road_match_the_waypoint(self):
        print("TestRoadLaneAPI.test_waypoint_lane_and_road_match_the_waypoint")
        checked = 0
        for waypoint in self.map.generate_waypoints(20.0)[:60]:
            lane = waypoint.get_lane()
            road = waypoint.get_road()
            self.assertIsNotNone(lane)
            self.assertIsNotNone(road)
            # The ids the waypoint already exposed must name the same elements.
            self.assertEqual(lane.id, waypoint.lane_id)
            self.assertEqual(lane.section_id, waypoint.section_id)
            self.assertEqual(lane.road_id, waypoint.road_id)
            self.assertEqual(road.id, waypoint.road_id)
            self.assertEqual(lane.type, waypoint.lane_type)
            self.assertEqual(lane.get_road(), road)
            checked += 1
        self.assertGreater(checked, 0, "no waypoints generated")

    def test_lane_waypoint_round_trip(self):
        print("TestRoadLaneAPI.test_lane_waypoint_round_trip")
        road = self._a_normal_road()
        lane = road.get_lanes(0.0)[0]
        # A waypoint taken from a lane must resolve back to that same lane.
        waypoint = lane.get_waypoint(lane.s_start + min(1.0, lane.length / 2.0))
        self.assertIsNotNone(waypoint)
        self.assertEqual(waypoint.get_lane(), lane)
        # And its width must agree with the lane's own profile at that s.
        self.assertAlmostEqual(
            waypoint.lane_width, lane.get_width(waypoint.s), delta=1e-3)

    def test_lane_get_waypoint_outside_the_lane_returns_none(self):
        print("TestRoadLaneAPI.test_lane_get_waypoint_outside_the_lane_returns_none")
        road = self._a_normal_road()
        lane = road.get_lanes(0.0)[0]
        self.assertIsNone(lane.get_waypoint(lane.s_start - 10.0))
        self.assertIsNone(lane.get_waypoint(lane.s_start + lane.length + 10.0))

    # -- lateral neighbours --------------------------------------------------

    def test_lane_neighbours_agree_with_waypoint_neighbours(self):
        print("TestRoadLaneAPI.test_lane_neighbours_agree_with_waypoint_neighbours")
        compared = 0
        for waypoint in self.map.generate_waypoints(20.0)[:60]:
            lane = waypoint.get_lane()
            for lane_side, wp_side in (
                    (lane.get_right_lane(), waypoint.get_right_waypoint()),
                    (lane.get_left_lane(), waypoint.get_left_waypoint())):
                if wp_side is None:
                    continue
                # Where the waypoint finds a neighbour, the lane must find the
                # same one. The reverse need not hold: the waypoint accessor
                # also honours lane-change permission.
                self.assertIsNotNone(lane_side)
                self.assertEqual(lane_side.id, wp_side.lane_id)
                self.assertEqual(lane_side.road_id, wp_side.road_id)
                compared += 1
        self.assertGreater(compared, 0, "no lateral neighbours found to compare")

    def test_deprecated_waypoint_lane_accessors_still_work(self):
        print("TestRoadLaneAPI.test_deprecated_waypoint_lane_accessors_still_work")
        for waypoint in self.map.generate_waypoints(20.0)[:20]:
            for old, new in ((waypoint.get_right_lane(), waypoint.get_right_waypoint()),
                             (waypoint.get_left_lane(), waypoint.get_left_waypoint())):
                if old is None:
                    self.assertIsNone(new)
                else:
                    self.assertEqual(old.id, new.id)

    # -- the lane graph ------------------------------------------------------

    def test_lane_graph_is_consistent_in_both_directions(self):
        print("TestRoadLaneAPI.test_lane_graph_is_consistent_in_both_directions")
        checked = 0
        for waypoint in self.map.generate_waypoints(20.0)[:40]:
            lane = waypoint.get_lane()
            for following in lane.get_next_lane():
                # If B follows A, then A must precede B.
                self.assertIn(lane, following.get_previous_lane(),
                              "%s lists %s as next, but not the reverse" % (lane, following))
                checked += 1
            if checked > 30:
                break
        self.assertGreater(checked, 0, "lane graph produced no edges")

    # -- road structure ------------------------------------------------------

    def test_road_sections_cover_the_road(self):
        print("TestRoadLaneAPI.test_road_sections_cover_the_road")
        road = self._a_normal_road()
        sections = road.get_sections()
        self.assertGreater(len(sections), 0)
        for section in sections:
            self.assertEqual(section.road_id, road.id)
            self.assertEqual(section.get_road(), road)
            self.assertEqual(road.get_section(section.id), section)
            self.assertGreaterEqual(section.s_start, 0.0)
            self.assertGreater(section.length, 0.0)
            for lane in section.get_lanes():
                self.assertEqual(lane.section_id, section.id)
                self.assertTrue(section.contains_lane(lane.id))
                self.assertNotEqual(lane.id, 0, "the centre lane must never be returned")
        # The section covering s=0 is the one that starts there.
        first = road.get_section_at(0.0)
        self.assertIsNotNone(first)
        self.assertAlmostEqual(first.s_start, 0.0, delta=1e-6)

    def test_road_cross_section_matches_its_lanes(self):
        print("TestRoadLaneAPI.test_road_cross_section_matches_its_lanes")
        road = self._a_normal_road()
        s = min(1.0, road.length / 2.0)
        lanes = road.get_lanes(s)
        waypoints = road.get_waypoints_at(s)
        self.assertGreater(len(lanes), 0)
        # One waypoint per lane, each on its own lane, all at the same s.
        self.assertEqual(len(waypoints), len(lanes))
        self.assertEqual(
            sorted(w.lane_id for w in waypoints), sorted(l.id for l in lanes))
        for waypoint in waypoints:
            self.assertAlmostEqual(waypoint.s, s, delta=1e-3)

    def test_road_waypoints_sample_along_the_road(self):
        print("TestRoadLaneAPI.test_road_waypoints_sample_along_the_road")
        # Long enough that several stations fit, so "along" is distinguishable
        # from the single-station cross-section.
        road = self._a_normal_road(min_length=30.0)
        step = 2.0
        waypoints = road.get_waypoints(step)
        self.assertGreater(len(waypoints), 0)
        for waypoint in waypoints:
            self.assertEqual(waypoint.road_id, road.id)

        # One station every `step` metres from 0 while still on the road.
        expected_stations = int(math.ceil(road.length / step))
        stations = {round(w.s, 3) for w in waypoints}
        self.assertEqual(len(stations), expected_stations)
        self.assertGreater(len(stations), 1)
        # Which is what separates this from get_waypoints_at(), one station.
        self.assertEqual(len({round(w.s, 3) for w in road.get_waypoints_at(0.0)}), 1)

    def test_road_connectivity(self):
        print("TestRoadLaneAPI.test_road_connectivity")
        road = self._a_normal_road()
        for neighbour in road.get_next_road() + road.get_previous_road():
            self.assertIsNotNone(self.map.get_road(neighbour.id))
        self.assertIsInstance(road.successor_id, int)
        self.assertIsInstance(road.predecessor_id, int)
        # A non-junction road reports no junction.
        self.assertFalse(road.is_junction)
        self.assertIsNone(road.get_junction())

    # -- junctions -----------------------------------------------------------

    def test_junction_roads_and_waypoints(self):
        print("TestRoadLaneAPI.test_junction_roads_and_waypoints")
        junction = self.map.get_junctions()[0]

        connecting = junction.get_connecting_roads()
        adjacent = junction.get_adjacent_roads()
        self.assertGreater(len(connecting), 0)
        self.assertGreater(len(adjacent), 0)

        # Roads inside the junction belong to it; roads outside do not. The two
        # sets must not overlap.
        for road in connecting:
            self.assertTrue(road.is_junction)
            self.assertEqual(road.get_junction().id, junction.id)
        connecting_ids = {r.id for r in connecting}
        self.assertTrue(connecting_ids.isdisjoint({r.id for r in adjacent}))

        # get_waypoints() describes the inside: its waypoints are on the
        # connecting roads and report themselves as being in a junction.
        pairs = junction.get_waypoints(carla.LaneType.Driving)
        self.assertGreater(len(pairs), 0)
        for start, end in pairs:
            self.assertIn(start.road_id, connecting_ids)
            self.assertIn(end.road_id, connecting_ids)
            self.assertTrue(start.is_junction)

        # Entry/exit lanes and waypoints all describe the outside: one waypoint
        # per lane, on the adjacent road where it meets the junction.
        adjacent_ids = {r.id for r in adjacent}
        for lanes, waypoints in (
                (junction.get_entry_lanes(), junction.get_entry_waypoints()),
                (junction.get_exit_lanes(), junction.get_exit_waypoints())):
            self.assertGreater(len(lanes), 0)
            self.assertEqual(len(waypoints), len(lanes))
            for lane in lanes:
                self.assertIn(lane.road_id, adjacent_ids)
                self.assertNotIn(lane.road_id, connecting_ids)
            # Each waypoint sits on its own entry/exit lane, outside.
            self.assertEqual(
                sorted((w.road_id, w.section_id, w.lane_id) for w in waypoints),
                sorted((l.road_id, l.section_id, l.id) for l in lanes))
            for waypoint in waypoints:
                self.assertFalse(waypoint.is_junction,
                                 "entry/exit waypoints must be outside the junction")

    # -- identity ------------------------------------------------------------

    def test_identity_is_usable_in_sets_and_dicts(self):
        print("TestRoadLaneAPI.test_identity_is_usable_in_sets_and_dicts")
        road = self._a_normal_road()
        lane = road.get_lanes(0.0)[0]
        section = lane.get_section()

        # Two independently fetched handles to the same element are one entry.
        self.assertEqual(len({road, self.map.get_road(road.id)}), 1)
        self.assertEqual(
            len({lane, self.map.get_lane(lane.road_id, lane.section_id, lane.id)}), 1)
        self.assertEqual(
            len({section, self.map.get_lane_section(section.road_id, section.id)}), 1)

        # Different elements stay distinct.
        other = next((l for l in road.get_lanes(0.0) if l.id != lane.id), None)
        if other is not None:
            self.assertNotEqual(lane, other)
            self.assertEqual(len({lane, other}), 2)

        # __str__ should name the element rather than the default repr.
        self.assertIn("Road", str(road))
        self.assertIn("Lane", str(lane))
        self.assertIn("LaneSection", str(section))
