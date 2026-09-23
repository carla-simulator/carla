# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Offline unit tests for the Autoware lanelet2 map generator's stop-sign
regulatory-element injection (generate_lanelet2_map.inject_stop_signs) and
its shared OpenDRIVE-id validation helper.

Runs against small hand-built .osm fixtures and fake CARLA actors -- no
simulator, no crdesigner.
"""

from __future__ import annotations

import contextlib
import io
import os
import sys
import tempfile
import unittest
import xml.etree.ElementTree as ET
from fnmatch import fnmatchcase

MAP_TOOLS_DIR = os.path.normpath(os.path.join(
    os.path.dirname(os.path.abspath(__file__)), "..", "..",
    "examples", "av_stacks", "autoware", "map_tools",
))
sys.path.insert(0, MAP_TOOLS_DIR)

import generate_lanelet2_map as gl2m  # noqa: E402


# One lanelet, boundary points at x=[0,10], y=+-1 (map frame).
BASE_OSM = """<?xml version='1.0' encoding='UTF-8'?>
<osm version="0.6">
  <MetaInfo format_version="1.0.0" map_version="1"/>
  <node id="1" action="modify" visible="true" version="1" lat="0.0" lon="0.0">
    <tag k="local_x" v="0.0000"/>
    <tag k="local_y" v="1.0000"/>
    <tag k="ele" v="0.0000"/>
  </node>
  <node id="2" action="modify" visible="true" version="1" lat="0.0" lon="0.0">
    <tag k="local_x" v="10.0000"/>
    <tag k="local_y" v="1.0000"/>
    <tag k="ele" v="0.0000"/>
  </node>
  <node id="3" action="modify" visible="true" version="1" lat="0.0" lon="0.0">
    <tag k="local_x" v="0.0000"/>
    <tag k="local_y" v="-1.0000"/>
    <tag k="ele" v="0.0000"/>
  </node>
  <node id="4" action="modify" visible="true" version="1" lat="0.0" lon="0.0">
    <tag k="local_x" v="10.0000"/>
    <tag k="local_y" v="-1.0000"/>
    <tag k="ele" v="0.0000"/>
  </node>
  <way id="10" action="modify" visible="true" version="1">
    <nd ref="1"/>
    <nd ref="2"/>
  </way>
  <way id="11" action="modify" visible="true" version="1">
    <nd ref="3"/>
    <nd ref="4"/>
  </way>
  <relation id="100" action="modify" visible="true" version="1">
    <member type="way" ref="10" role="left"/>
    <member type="way" ref="11" role="right"/>
    <tag k="type" v="lanelet"/>
    <tag k="subtype" v="road"/>
  </relation>
</osm>
"""


# A second lanelet whose nearest boundary *vertex* to the stop waypoint is
# closer than lanelet 100's, although the waypoint lies inside lanelet 100.
DECOY_LANELET_OSM = """  <node id="5" action="modify" visible="true" version="1" lat="0.0" lon="0.0">
    <tag k="local_x" v="0.0000"/>
    <tag k="local_y" v="-2.0000"/>
    <tag k="ele" v="0.0000"/>
  </node>
  <node id="6" action="modify" visible="true" version="1" lat="0.0" lon="0.0">
    <tag k="local_x" v="5.0000"/>
    <tag k="local_y" v="-2.0000"/>
    <tag k="ele" v="0.0000"/>
  </node>
  <node id="7" action="modify" visible="true" version="1" lat="0.0" lon="0.0">
    <tag k="local_x" v="0.0000"/>
    <tag k="local_y" v="-3.6000"/>
    <tag k="ele" v="0.0000"/>
  </node>
  <node id="8" action="modify" visible="true" version="1" lat="0.0" lon="0.0">
    <tag k="local_x" v="5.0000"/>
    <tag k="local_y" v="-3.6000"/>
    <tag k="ele" v="0.0000"/>
  </node>
  <way id="12" action="modify" visible="true" version="1">
    <nd ref="5"/>
    <nd ref="6"/>
  </way>
  <way id="13" action="modify" visible="true" version="1">
    <nd ref="7"/>
    <nd ref="8"/>
  </way>
  <relation id="200" action="modify" visible="true" version="1">
    <member type="way" ref="12" role="left"/>
    <member type="way" ref="13" role="right"/>
    <tag k="type" v="lanelet"/>
    <tag k="subtype" v="road"/>
  </relation>
"""

# Lanelet 100's strip with the two sides swapped: the oncoming lane.
ONCOMING_LANELET_OSM = """  <relation id="300" action="modify" visible="true" version="1">
    <member type="way" ref="11" role="left"/>
    <member type="way" ref="10" role="right"/>
    <tag k="type" v="lanelet"/>
    <tag k="subtype" v="road"/>
  </relation>
"""

# A second lanelet that shares lanelet 100's boundary geometry up to x=6 and
# then peels away to the left: the junction split a route may take instead of
# lanelet 100. It brackets the stop waypoint at x=5 exactly as lanelet 100 does.
SPLIT_LANELET_OSM = """  <node id="20" action="modify" visible="true" version="1" lat="0.0" lon="0.0">
    <tag k="local_x" v="0.0000"/>
    <tag k="local_y" v="1.0000"/>
    <tag k="ele" v="0.0000"/>
  </node>
  <node id="21" action="modify" visible="true" version="1" lat="0.0" lon="0.0">
    <tag k="local_x" v="6.0000"/>
    <tag k="local_y" v="1.0000"/>
    <tag k="ele" v="0.0000"/>
  </node>
  <node id="22" action="modify" visible="true" version="1" lat="0.0" lon="0.0">
    <tag k="local_x" v="10.0000"/>
    <tag k="local_y" v="5.0000"/>
    <tag k="ele" v="0.0000"/>
  </node>
  <node id="23" action="modify" visible="true" version="1" lat="0.0" lon="0.0">
    <tag k="local_x" v="0.0000"/>
    <tag k="local_y" v="-1.0000"/>
    <tag k="ele" v="0.0000"/>
  </node>
  <node id="24" action="modify" visible="true" version="1" lat="0.0" lon="0.0">
    <tag k="local_x" v="6.0000"/>
    <tag k="local_y" v="-1.0000"/>
    <tag k="ele" v="0.0000"/>
  </node>
  <node id="25" action="modify" visible="true" version="1" lat="0.0" lon="0.0">
    <tag k="local_x" v="12.0000"/>
    <tag k="local_y" v="3.0000"/>
    <tag k="ele" v="0.0000"/>
  </node>
  <way id="30" action="modify" visible="true" version="1">
    <nd ref="20"/>
    <nd ref="21"/>
    <nd ref="22"/>
  </way>
  <way id="31" action="modify" visible="true" version="1">
    <nd ref="23"/>
    <nd ref="24"/>
    <nd ref="25"/>
  </way>
  <relation id="400" action="modify" visible="true" version="1">
    <member type="way" ref="30" role="left"/>
    <member type="way" ref="31" role="right"/>
    <tag k="type" v="lanelet"/>
    <tag k="subtype" v="road"/>
  </relation>
"""

# A lanelet on the same strip that ends at x=4.7, before the stop waypoint at
# x=5, and whose boundary ways repeat their last node. The repeat makes the
# closing segment degenerate, which hides from an index-based test that the
# waypoint projects past the end of the boundary.
TRUNCATED_REPEATED_NODE_OSM = """  <node id="40" action="modify" visible="true" version="1" lat="0.0" lon="0.0">
    <tag k="local_x" v="0.0000"/>
    <tag k="local_y" v="1.0000"/>
    <tag k="ele" v="0.0000"/>
  </node>
  <node id="41" action="modify" visible="true" version="1" lat="0.0" lon="0.0">
    <tag k="local_x" v="4.7000"/>
    <tag k="local_y" v="1.0000"/>
    <tag k="ele" v="0.0000"/>
  </node>
  <node id="42" action="modify" visible="true" version="1" lat="0.0" lon="0.0">
    <tag k="local_x" v="0.0000"/>
    <tag k="local_y" v="-1.0000"/>
    <tag k="ele" v="0.0000"/>
  </node>
  <node id="43" action="modify" visible="true" version="1" lat="0.0" lon="0.0">
    <tag k="local_x" v="4.7000"/>
    <tag k="local_y" v="-1.0000"/>
    <tag k="ele" v="0.0000"/>
  </node>
  <way id="50" action="modify" visible="true" version="1">
    <nd ref="40"/>
    <nd ref="41"/>
    <nd ref="41"/>
  </way>
  <way id="51" action="modify" visible="true" version="1">
    <nd ref="42"/>
    <nd ref="43"/>
    <nd ref="43"/>
  </way>
  <relation id="500" action="modify" visible="true" version="1">
    <member type="way" ref="50" role="left"/>
    <member type="way" ref="51" role="right"/>
    <tag k="type" v="lanelet"/>
    <tag k="subtype" v="road"/>
  </relation>
"""

# Lanelet 100's strip again, but its first ``left`` member points at a way with
# a single usable node; the well-formed duplicate behind it has to be used.
DUPLICATE_ROLE_LANELET_OSM = """  <node id="60" action="modify" visible="true" version="1" lat="0.0" lon="0.0">
    <tag k="local_x" v="0.0000"/>
    <tag k="local_y" v="1.0000"/>
    <tag k="ele" v="0.0000"/>
  </node>
  <way id="70" action="modify" visible="true" version="1">
    <nd ref="60"/>
  </way>
  <relation id="600" action="modify" visible="true" version="1">
    <member type="way" ref="70" role="left"/>
    <member type="way" ref="10" role="left"/>
    <member type="way" ref="11" role="right"/>
    <tag k="type" v="lanelet"/>
    <tag k="subtype" v="road"/>
  </relation>
"""


# Lanelet 100's strip declared as a crosswalk: a pedestrian lanelet must never
# be handed a vehicle stop sign, however well its cross section fits.
CROSSWALK_LANELET_OSM = """  <relation id="700" action="modify" visible="true" version="1">
    <member type="way" ref="10" role="left"/>
    <member type="way" ref="11" role="right"/>
    <tag k="type" v="lanelet"/>
    <tag k="subtype" v="crosswalk"/>
  </relation>
"""


# A crosswalk far from lanelet 100 (x=[20,30] vs x=[0,10]), so a stop waypoint
# near it fits no cross section (forcing nearest_lanelet()'s vertex-distance
# fallback) and is only within the fallback's cutoff of the crosswalk, not of
# lanelet 100: the fallback must not attach the sign to the crosswalk anyway.
FAR_CROSSWALK_ONLY_OSM = """  <node id="80" action="modify" visible="true" version="1" lat="0.0" lon="0.0">
    <tag k="local_x" v="20.0000"/>
    <tag k="local_y" v="1.0000"/>
    <tag k="ele" v="0.0000"/>
  </node>
  <node id="81" action="modify" visible="true" version="1" lat="0.0" lon="0.0">
    <tag k="local_x" v="30.0000"/>
    <tag k="local_y" v="1.0000"/>
    <tag k="ele" v="0.0000"/>
  </node>
  <node id="82" action="modify" visible="true" version="1" lat="0.0" lon="0.0">
    <tag k="local_x" v="20.0000"/>
    <tag k="local_y" v="-1.0000"/>
    <tag k="ele" v="0.0000"/>
  </node>
  <node id="83" action="modify" visible="true" version="1" lat="0.0" lon="0.0">
    <tag k="local_x" v="30.0000"/>
    <tag k="local_y" v="-1.0000"/>
    <tag k="ele" v="0.0000"/>
  </node>
  <way id="90" action="modify" visible="true" version="1">
    <nd ref="80"/>
    <nd ref="81"/>
  </way>
  <way id="91" action="modify" visible="true" version="1">
    <nd ref="82"/>
    <nd ref="83"/>
  </way>
  <relation id="800" action="modify" visible="true" version="1">
    <member type="way" ref="90" role="left"/>
    <member type="way" ref="91" role="right"/>
    <tag k="type" v="lanelet"/>
    <tag k="subtype" v="crosswalk"/>
  </relation>
"""


# A wide lanelet whose "right" way is digitized in the opposite direction to
# its "left" way (left ascends local_x 2->10; right is authored 10->2, the
# same two physical endpoints as an ascending way, just backwards). This is
# what real crdesigner-converted OpenDRIVE data produces for some junction
# lanelets, and folds OsmMap's `left_pts + reversed(right_pts)` containment
# polygon into a self-crossing bowtie: a stop waypoint squarely inside
# lanelet 100 (map (5, 0)) is also, incorrectly, "contained" by this bowtie,
# so nearest_lanelet()'s old vertex-distance heuristic could steal it away
# from lanelet 100, its true approach lanelet. The polygon-agnostic fit path
# (_fit_stop_line_to_lanelet) rejects this lanelet instead, on its own drawn
# width (4 m) versus the reported lane_width (see
# InjectTrafficLightsTests.test_reversed_boundary_neighbor_is_not_mislinked).
REVERSED_RIGHT_BOUNDARY_LANELET_OSM = """  <node id="92" action="modify" visible="true" version="1" lat="0.0" lon="0.0">
    <tag k="local_x" v="2.0000"/>
    <tag k="local_y" v="1.0000"/>
    <tag k="ele" v="0.0000"/>
  </node>
  <node id="93" action="modify" visible="true" version="1" lat="0.0" lon="0.0">
    <tag k="local_x" v="10.0000"/>
    <tag k="local_y" v="1.0000"/>
    <tag k="ele" v="0.0000"/>
  </node>
  <node id="94" action="modify" visible="true" version="1" lat="0.0" lon="0.0">
    <tag k="local_x" v="10.0000"/>
    <tag k="local_y" v="-3.0000"/>
    <tag k="ele" v="0.0000"/>
  </node>
  <node id="95" action="modify" visible="true" version="1" lat="0.0" lon="0.0">
    <tag k="local_x" v="2.0000"/>
    <tag k="local_y" v="-3.0000"/>
    <tag k="ele" v="0.0000"/>
  </node>
  <way id="96" action="modify" visible="true" version="1">
    <nd ref="92"/>
    <nd ref="93"/>
  </way>
  <way id="97" action="modify" visible="true" version="1">
    <nd ref="94"/>
    <nd ref="95"/>
  </way>
  <relation id="900" action="modify" visible="true" version="1">
    <member type="way" ref="96" role="left"/>
    <member type="way" ref="97" role="right"/>
    <tag k="type" v="lanelet"/>
    <tag k="subtype" v="road"/>
  </relation>
"""


def _osm_with(*extra):
    return BASE_OSM.replace("</osm>", "".join(extra) + "</osm>")


class FakeVector3D:
    def __init__(self, x=0.0, y=0.0, z=0.0):
        self.x, self.y, self.z = x, y, z


class FakeRotation:
    def __init__(self, right=(0.0, 1.0, 0.0), forward=(1.0, 0.0, 0.0)):
        self._right = FakeVector3D(*right)
        self._forward = FakeVector3D(*forward)

    def get_right_vector(self):
        return self._right

    def get_forward_vector(self):
        return self._forward


class FakeTransform:
    def __init__(self, location, rotation=None):
        self.location = location
        self.rotation = rotation or FakeRotation()


class FakeWaypoint:
    def __init__(self, location, lane_width=1.6, rotation=None):
        self.transform = FakeTransform(location, rotation)
        self.lane_width = lane_width

    def next(self, distance):
        # No fixture here models a route continuing past this waypoint, so
        # _advance_waypoint's walk stops immediately and the waypoint it was
        # given is used as-is.
        return []


class FakeLightBox:
    def __init__(self, location, extent=(0.3, 0.9, 0.3), rotation=None):
        self.location = FakeVector3D(*location)
        self.extent = FakeVector3D(*extent)
        self.rotation = rotation or FakeRotation()


class FakeTrafficLight:
    """Exercises inject_stop_signs' actor-type filtering (bare constructor),
    and inject_traffic_lights' actor interface (remaining fields)."""

    def __init__(self, actor_id, type_id="traffic.traffic_light", opendrive_id=None,
                 stop_wps=None, affected_wps=None, light_boxes=None):
        self.id = actor_id
        self.type_id = type_id
        self._opendrive_id = opendrive_id
        self._stop_wps = stop_wps or []
        self._affected_wps = affected_wps or []
        self._light_boxes = light_boxes or []

    def get_opendrive_id(self):
        return self._opendrive_id

    def get_stop_waypoints(self):
        return self._stop_wps

    def get_affected_lane_waypoints(self):
        return self._affected_wps

    def get_light_boxes(self):
        return self._light_boxes


class FakeStopSign:
    def __init__(self, actor_id, opendrive_id, stop_wps=None, affected_wps=None,
                 type_id="traffic.stop"):
        self.id = actor_id
        self.type_id = type_id
        self._opendrive_id = opendrive_id
        self._stop_wps = stop_wps or []
        self._affected_wps = affected_wps or []

    def get_stop_waypoints(self):
        return self._stop_wps

    def get_affected_lane_waypoints(self):
        return self._affected_wps

    def get_opendrive_id(self):
        return self._opendrive_id


class FakeActors(list):
    def filter(self, pattern):
        return FakeActors(a for a in self if fnmatchcase(a.type_id, pattern))


class FakeWorld:
    def __init__(self, actors):
        self._actors = FakeActors(actors)

    def get_actors(self):
        return self._actors


def _stop_sign_waypoint():
    # CARLA (5, 0, 0) -> map (5, 0, 0): centred inside lanelet 100, whose drawn
    # width (2.0) deliberately differs from the reported lane_width (1.6).
    return FakeWaypoint(FakeVector3D(5.0, 0.0, 0.0), lane_width=1.6)


def _traffic_light_waypoint():
    # Same convention as _stop_sign_waypoint(): CARLA (5, 0, 0) -> map (5, 0),
    # centred inside lanelet 100.
    return FakeWaypoint(FakeVector3D(5.0, 0.0, 0.0), lane_width=1.6)


class InjectStopSignsTests(unittest.TestCase):
    def setUp(self):
        fd, self.osm_path = tempfile.mkstemp(suffix=".osm")
        os.close(fd)
        with open(self.osm_path, "w", encoding="utf-8") as f:
            f.write(BASE_OSM)

    def tearDown(self):
        if os.path.exists(self.osm_path):
            os.unlink(self.osm_path)

    def _write(self, osm_text):
        with open(self.osm_path, "w", encoding="utf-8") as f:
            f.write(osm_text)

    def _ref_line_points(self, root=None):
        root = root if root is not None else ET.parse(self.osm_path).getroot()
        rel, _tags = self._stop_sign_relations(root)[0]
        way_id = next(m.get("ref") for m in rel.findall("member") if m.get("role") == "ref_line")
        way = next(w for w in root.findall("way") if w.get("id") == way_id)
        pts = []
        for nd in way.findall("nd"):
            node = next(n for n in root.findall("node") if n.get("id") == nd.get("ref"))
            tags = {t.get("k"): t.get("v") for t in node.findall("tag")}
            pts.append((float(tags["local_x"]), float(tags["local_y"])))
        return sorted(pts)

    def _linked_lanelet_ids(self, root=None):
        root = root if root is not None else ET.parse(self.osm_path).getroot()
        re_ids = {rel.get("id") for rel, _t in self._stop_sign_relations(root)}
        return {rel.get("id") for rel in root.findall("relation")
                for m in rel.findall("member")
                if m.get("type") == "relation" and m.get("role") == "regulatory_element"
                and m.get("ref") in re_ids}

    def _stop_sign_relations(self, root=None):
        root = root if root is not None else ET.parse(self.osm_path).getroot()
        rels = []
        for rel in root.findall("relation"):
            tags = {t.get("k"): t.get("v") for t in rel.findall("tag")}
            if tags.get("type") == "regulatory_element" and tags.get("subtype") == "traffic_sign":
                rels.append((rel, tags))
        return rels

    def test_injects_stop_sign_regulatory_element_with_expected_schema(self):
        sign = FakeStopSign(actor_id=42, opendrive_id="7", stop_wps=[_stop_sign_waypoint()])
        world = FakeWorld([sign])

        injected = gl2m.inject_stop_signs(world, self.osm_path)
        self.assertEqual(injected, 1)

        rels = self._stop_sign_relations()
        self.assertEqual(len(rels), 1)
        rel, tags = rels[0]
        self.assertEqual(tags["subtype"], "traffic_sign")
        self.assertEqual(tags["sign_type"], "stop_sign")
        self.assertEqual(tags["carla_opendrive_id"], "7")

        members = rel.findall("member")
        self.assertEqual(len(members), 1)
        self.assertEqual(members[0].get("type"), "way")
        self.assertEqual(members[0].get("role"), "ref_line")
        self.assertIsNone(rel.find("member[@role='refers']"))

        root = ET.parse(self.osm_path).getroot()
        lanelet = next(r for r in root.findall("relation") if r.get("id") == "100")
        back_refs = [m for m in lanelet.findall("member")
                     if m.get("type") == "relation" and m.get("role") == "regulatory_element"]
        self.assertEqual(len(back_refs), 1)
        self.assertEqual(back_refs[0].get("ref"), rel.get("id"))

    def test_stop_sign_ids_derive_from_opendrive_id(self):
        sign = FakeStopSign(actor_id=42, opendrive_id="7", stop_wps=[_stop_sign_waypoint()])
        gl2m.inject_stop_signs(FakeWorld([sign]), self.osm_path)

        base = gl2m.STOP_SIGN_ID_BASE + 7 * gl2m.STOP_SIGN_ID_STRIDE
        ids = {e.get("id") for e in ET.parse(self.osm_path).getroot().iter() if e.get("id")}
        self.assertIn(str(base), ids)       # first stop-line node
        self.assertIn(str(base + 1), ids)   # second stop-line node
        self.assertIn(str(base + 2), ids)   # stop-line way
        self.assertIn(str(base + 3), ids)   # regulatory-element relation

    def test_injection_ids_are_stable_across_runs(self):
        sign = FakeStopSign(actor_id=42, opendrive_id="7", stop_wps=[_stop_sign_waypoint()])
        world = FakeWorld([sign])

        gl2m.inject_stop_signs(world, self.osm_path)
        with open(self.osm_path, "rb") as f:
            first = f.read()

        gl2m.inject_stop_signs(world, self.osm_path)
        with open(self.osm_path, "rb") as f:
            second = f.read()

        self.assertEqual(first, second)

    def test_fallback_to_affected_lane_waypoints_when_no_stop_waypoints(self):
        sign = FakeStopSign(actor_id=1, opendrive_id="3", stop_wps=[],
                             affected_wps=[_stop_sign_waypoint()])

        injected = gl2m.inject_stop_signs(FakeWorld([sign]), self.osm_path)

        self.assertEqual(injected, 1)

    def test_sign_with_no_waypoints_is_skipped(self):
        sign = FakeStopSign(actor_id=1, opendrive_id="3")

        injected = gl2m.inject_stop_signs(FakeWorld([sign]), self.osm_path)

        self.assertEqual(injected, 0)
        self.assertEqual(self._stop_sign_relations(), [])

    def test_no_live_stop_sign_actors_leaves_existing_map_unchanged(self):
        with open(self.osm_path, "rb") as f:
            before = f.read()

        world = FakeWorld([FakeTrafficLight(actor_id=9)])
        injected = gl2m.inject_stop_signs(world, self.osm_path)

        with open(self.osm_path, "rb") as f:
            after = f.read()
        self.assertEqual(injected, 0)
        self.assertEqual(before, after)

    def test_filter_ignores_non_stop_sign_actors(self):
        light = FakeTrafficLight(actor_id=1)
        sign = FakeStopSign(actor_id=2, opendrive_id="7", stop_wps=[_stop_sign_waypoint()])

        injected = gl2m.inject_stop_signs(FakeWorld([light, sign]), self.osm_path)

        self.assertEqual(injected, 1)

    def test_stale_stop_sign_elements_removed_but_shared_ones_kept(self):
        sign7 = FakeStopSign(actor_id=1, opendrive_id="7", stop_wps=[_stop_sign_waypoint()])
        gl2m.inject_stop_signs(FakeWorld([sign7]), self.osm_path)

        root = ET.parse(self.osm_path).getroot()
        rel = self._stop_sign_relations(root)[0][0]
        way_id = next(m.get("ref") for m in rel.findall("member") if m.get("type") == "way")
        way = next(w for w in root.findall("way") if w.get("id") == way_id)
        kept_node_id, dropped_node_id = [nd.get("ref") for nd in way.findall("nd")]

        # Simulate another map element (e.g. a lanelet boundary way) sharing
        # one of the stop-line's nodes -- it must survive the next cleanup.
        survivor_way = ET.SubElement(root, "way", {
            "id": "999", "action": "modify", "visible": "true", "version": "1",
        })
        ET.SubElement(survivor_way, "nd", {"ref": kept_node_id})
        tree = ET.ElementTree(root)
        ET.indent(tree, space="  ")
        tree.write(self.osm_path, encoding="UTF-8", xml_declaration=True)

        sign9 = FakeStopSign(actor_id=2, opendrive_id="9", stop_wps=[_stop_sign_waypoint()])
        injected = gl2m.inject_stop_signs(FakeWorld([sign9]), self.osm_path)
        self.assertEqual(injected, 1)

        root = ET.parse(self.osm_path).getroot()
        remaining_way_ids = {w.get("id") for w in root.findall("way")}
        remaining_node_ids = {n.get("id") for n in root.findall("node")}

        self.assertNotIn(way_id, remaining_way_ids)
        self.assertNotIn(dropped_node_id, remaining_node_ids)
        self.assertIn(kept_node_id, remaining_node_ids)
        self.assertIn("999", remaining_way_ids)

        rels = self._stop_sign_relations(root)
        self.assertEqual(len(rels), 1)
        self.assertEqual(rels[0][1]["carla_opendrive_id"], "9")

    def test_stop_line_endpoints_sit_on_the_lanelet_boundaries(self):
        # The fixture's drawn lane is 2.0 wide while the waypoint reports 1.6:
        # the stop line must follow the map, not the reported lane width.
        sign = FakeStopSign(actor_id=1, opendrive_id="7", stop_wps=[_stop_sign_waypoint()])

        gl2m.inject_stop_signs(FakeWorld([sign]), self.osm_path)

        self.assertEqual(self._ref_line_points(), [(5.0, -1.0), (5.0, 1.0)])

    def test_lanelet_containing_the_waypoint_wins_over_a_nearer_boundary_vertex(self):
        self._write(_osm_with(DECOY_LANELET_OSM))
        sign = FakeStopSign(actor_id=1, opendrive_id="7", stop_wps=[_stop_sign_waypoint()])

        gl2m.inject_stop_signs(FakeWorld([sign]), self.osm_path)

        self.assertEqual(self._linked_lanelet_ids(), {"100"})
        self.assertEqual(self._ref_line_points(), [(5.0, -1.0), (5.0, 1.0)])

    def test_oncoming_lanelet_on_the_same_strip_is_not_matched(self):
        self._write(_osm_with(ONCOMING_LANELET_OSM))
        sign = FakeStopSign(actor_id=1, opendrive_id="7", stop_wps=[_stop_sign_waypoint()])

        gl2m.inject_stop_signs(FakeWorld([sign]), self.osm_path)

        self.assertEqual(self._linked_lanelet_ids(), {"100"})

    def test_falls_back_to_lane_width_when_outside_every_lanelet(self):
        # CARLA (5, -3, 0) -> map (5, 3, 0): 2m outside lanelet 100's left
        # boundary, still inside nearest_lanelet()'s 6m vertex cutoff.
        wp = FakeWaypoint(FakeVector3D(5.0, -3.0, 0.0), lane_width=1.6)
        sign = FakeStopSign(actor_id=1, opendrive_id="7", stop_wps=[wp])

        injected = gl2m.inject_stop_signs(FakeWorld([sign]), self.osm_path)

        self.assertEqual(injected, 1)
        self.assertEqual(self._ref_line_points(), [(5.0, 2.2), (5.0, 3.8)])

    def test_every_lanelet_sharing_the_approach_gets_the_regulatory_element(self):
        # Lanelets 100 and 400 share the boundary geometry at the stop line and
        # only diverge behind it. A route through either has to see the sign, so
        # both carry a back-reference to the one regulatory element.
        self._write(_osm_with(SPLIT_LANELET_OSM))
        sign = FakeStopSign(actor_id=1, opendrive_id="7", stop_wps=[_stop_sign_waypoint()])

        injected = gl2m.inject_stop_signs(FakeWorld([sign]), self.osm_path)

        self.assertEqual(injected, 1)
        root = ET.parse(self.osm_path).getroot()
        rels = self._stop_sign_relations(root)
        self.assertEqual(len(rels), 1)
        self.assertEqual(self._linked_lanelet_ids(root), {"100", "400"})
        # One shared physical stop line, on the geometry both lanelets share.
        ref_lines = [m.get("ref") for m in rels[0][0].findall("member")
                     if m.get("role") == "ref_line"]
        self.assertEqual(len(ref_lines), 1)
        self.assertEqual(self._ref_line_points(root), [(5.0, -1.0), (5.0, 1.0)])

    def test_split_lanelet_injection_is_stable_across_runs(self):
        self._write(_osm_with(SPLIT_LANELET_OSM))
        world = FakeWorld([FakeStopSign(actor_id=1, opendrive_id="7",
                                        stop_wps=[_stop_sign_waypoint()])])

        gl2m.inject_stop_signs(world, self.osm_path)
        with open(self.osm_path, "rb") as f:
            first = f.read()
        gl2m.inject_stop_signs(world, self.osm_path)
        with open(self.osm_path, "rb") as f:
            second = f.read()

        self.assertEqual(first, second)

    def test_multi_lane_stop_line_links_every_approached_lanelet(self):
        # One sign governing two parallel lanes: CARLA (4, 0) -> map (4, 0) sits
        # in lanelet 100, CARLA (4, 2.8) -> map (4, -2.8) in lanelet 200.
        self._write(_osm_with(DECOY_LANELET_OSM))
        wps = [FakeWaypoint(FakeVector3D(4.0, 0.0, 0.0), lane_width=1.6),
               FakeWaypoint(FakeVector3D(4.0, 2.8, 0.0), lane_width=1.6)]
        sign = FakeStopSign(actor_id=1, opendrive_id="7", stop_wps=wps)

        injected = gl2m.inject_stop_signs(FakeWorld([sign]), self.osm_path)

        self.assertEqual(injected, 1)
        self.assertEqual(self._linked_lanelet_ids(), {"100", "200"})
        self.assertEqual(self._ref_line_points(), [(4.0, -3.6), (4.0, 1.0)])

    def test_lanelet_ending_before_the_waypoint_is_not_matched(self):
        # Lanelet 500 stops at x=4.7 and repeats its last boundary node, which
        # must not hide that the waypoint at x=5 lies past its end.
        self._write(_osm_with(TRUNCATED_REPEATED_NODE_OSM))
        sign = FakeStopSign(actor_id=1, opendrive_id="7", stop_wps=[_stop_sign_waypoint()])

        gl2m.inject_stop_signs(FakeWorld([sign]), self.osm_path)

        self.assertEqual(self._linked_lanelet_ids(), {"100"})
        self.assertEqual(self._ref_line_points(), [(5.0, -1.0), (5.0, 1.0)])

    def test_lanelet_with_an_unusable_duplicate_side_member_still_matches(self):
        # Lanelet 600's first "left" member is a one-node way; the well-formed
        # duplicate behind it describes the same strip as lanelet 100.
        self._write(_osm_with(DUPLICATE_ROLE_LANELET_OSM))
        sign = FakeStopSign(actor_id=1, opendrive_id="7", stop_wps=[_stop_sign_waypoint()])

        gl2m.inject_stop_signs(FakeWorld([sign]), self.osm_path)

        self.assertEqual(self._linked_lanelet_ids(), {"100", "600"})

    def test_fallback_links_a_lanelet_per_stop_waypoint(self):
        # Neither waypoint fits a cross section, so the lane-width fallback runs
        # for both lanes and still links the lanelet nearest to each of them.
        self._write(_osm_with(DECOY_LANELET_OSM))
        wps = [FakeWaypoint(FakeVector3D(5.0, -3.0, 0.0), lane_width=1.6),
               FakeWaypoint(FakeVector3D(5.0, 4.5, 0.0), lane_width=1.6)]
        sign = FakeStopSign(actor_id=1, opendrive_id="7", stop_wps=wps)

        injected = gl2m.inject_stop_signs(FakeWorld([sign]), self.osm_path)

        self.assertEqual(injected, 1)
        self.assertEqual(self._linked_lanelet_ids(), {"100", "200"})
        self.assertEqual(self._ref_line_points(), [(5.0, -5.3), (5.0, 3.8)])

    def test_pedestrian_lanelets_never_receive_the_stop_sign(self):
        self._write(_osm_with(CROSSWALK_LANELET_OSM))
        sign = FakeStopSign(actor_id=1, opendrive_id="7", stop_wps=[_stop_sign_waypoint()])

        gl2m.inject_stop_signs(FakeWorld([sign]), self.osm_path)

        self.assertEqual(self._linked_lanelet_ids(), {"100"})

    def test_fallback_never_attaches_to_a_pedestrian_lanelet(self):
        # The waypoint fits no cross section (forcing the nearest_lanelet()
        # fallback) and is only within cutoff range of a crosswalk, with no
        # vehicle lanelet nearby: the sign must be skipped, not attached to
        # the crosswalk and leave whatever road lanelet exists unregulated.
        self._write(_osm_with(FAR_CROSSWALK_ONLY_OSM))
        wp = FakeWaypoint(FakeVector3D(25.0, -1.5, 0.0), lane_width=1.6)
        sign = FakeStopSign(actor_id=1, opendrive_id="7", stop_wps=[wp])

        injected = gl2m.inject_stop_signs(FakeWorld([sign]), self.osm_path)

        self.assertEqual(injected, 0)
        self.assertEqual(self._linked_lanelet_ids(), set())

    def test_duplicate_opendrive_id_raises(self):
        s1 = FakeStopSign(actor_id=1, opendrive_id="7", stop_wps=[_stop_sign_waypoint()])
        s2 = FakeStopSign(actor_id=2, opendrive_id="7", stop_wps=[_stop_sign_waypoint()])

        with self.assertRaises(RuntimeError):
            gl2m.inject_stop_signs(FakeWorld([s1, s2]), self.osm_path)

    def test_duplicate_opendrive_id_with_leading_zeros_raises(self):
        # "007" and "7" are distinct strings but the same deterministic id
        # (int(opendrive_id) is the key), so they must collide too.
        s1 = FakeStopSign(actor_id=1, opendrive_id="007", stop_wps=[_stop_sign_waypoint()])
        s2 = FakeStopSign(actor_id=2, opendrive_id="7", stop_wps=[_stop_sign_waypoint()])

        with self.assertRaises(RuntimeError):
            gl2m.inject_stop_signs(FakeWorld([s1, s2]), self.osm_path)

    def test_non_numeric_or_empty_opendrive_id_is_skipped(self):
        for opendrive_id in ("abc", ""):
            with self.subTest(opendrive_id=opendrive_id):
                self._write(BASE_OSM)
                sign = FakeStopSign(actor_id=1, opendrive_id=opendrive_id,
                                    stop_wps=[_stop_sign_waypoint()])

                output = io.StringIO()
                with contextlib.redirect_stdout(output):
                    injected = gl2m.inject_stop_signs(FakeWorld([sign]), self.osm_path)

                self.assertEqual(injected, 0)
                self.assertEqual(self._stop_sign_relations(), [])
                self.assertEqual(self._linked_lanelet_ids(), set())
                self.assertIn("WARNING: stop sign actor 1 has non-numeric OpenDRIVE id",
                              output.getvalue())

    def test_sign_with_non_numeric_opendrive_id_does_not_block_valid_signs(self):
        bad = FakeStopSign(actor_id=1, opendrive_id="", stop_wps=[_stop_sign_waypoint()])
        good = FakeStopSign(actor_id=2, opendrive_id="7", stop_wps=[_stop_sign_waypoint()])

        with contextlib.redirect_stdout(io.StringIO()):
            injected = gl2m.inject_stop_signs(FakeWorld([bad, good]), self.osm_path)

        self.assertEqual(injected, 1)
        rels = self._stop_sign_relations()
        self.assertEqual([tags["carla_opendrive_id"] for _rel, tags in rels], ["7"])
        self.assertEqual(self._linked_lanelet_ids(), {"100"})

    def test_rerun_with_no_live_signs_still_removes_stale_regulatory_elements(self):
        sign = FakeStopSign(actor_id=1, opendrive_id="7", stop_wps=[_stop_sign_waypoint()])
        gl2m.inject_stop_signs(FakeWorld([sign]), self.osm_path)
        self.assertEqual(self._linked_lanelet_ids(), {"100"})

        injected = gl2m.inject_stop_signs(FakeWorld([]), self.osm_path)

        self.assertEqual(injected, 0)
        self.assertEqual(self._linked_lanelet_ids(), set())
        self.assertEqual(len(self._stop_sign_relations()), 0)


class InjectTrafficLightsTests(unittest.TestCase):
    def setUp(self):
        fd, self.osm_path = tempfile.mkstemp(suffix=".osm")
        os.close(fd)
        with open(self.osm_path, "w", encoding="utf-8") as f:
            f.write(BASE_OSM)

    def tearDown(self):
        if os.path.exists(self.osm_path):
            os.unlink(self.osm_path)

    def _write(self, osm_text):
        with open(self.osm_path, "w", encoding="utf-8") as f:
            f.write(osm_text)

    def _traffic_light_relations(self, root=None):
        root = root if root is not None else ET.parse(self.osm_path).getroot()
        rels = []
        for rel in root.findall("relation"):
            tags = {t.get("k"): t.get("v") for t in rel.findall("tag")}
            if tags.get("type") == "regulatory_element" and tags.get("subtype") == "traffic_light":
                rels.append((rel, tags))
        return rels

    def _linked_lanelet_ids(self, root=None):
        root = root if root is not None else ET.parse(self.osm_path).getroot()
        re_ids = {rel.get("id") for rel, _t in self._traffic_light_relations(root)}
        return {rel.get("id") for rel in root.findall("relation")
                for m in rel.findall("member")
                if m.get("type") == "relation" and m.get("role") == "regulatory_element"
                and m.get("ref") in re_ids}

    def test_injects_traffic_light_regulatory_element_with_expected_schema(self):
        light = FakeTrafficLight(actor_id=9, opendrive_id="5",
                                  stop_wps=[_traffic_light_waypoint()],
                                  light_boxes=[FakeLightBox((5.0, 0.0, 5.0))])

        injected = gl2m.inject_traffic_lights(FakeWorld([light]), self.osm_path)
        self.assertEqual(injected, 1)

        rels = self._traffic_light_relations()
        self.assertEqual(len(rels), 1)
        rel, tags = rels[0]
        self.assertEqual(tags["subtype"], "traffic_light")
        self.assertEqual(tags["carla_opendrive_id"], "5")

        members = rel.findall("member")
        self.assertEqual(len([m for m in members if m.get("role") == "refers"]), 1)
        ref_lines = [m for m in members if m.get("role") == "ref_line"]
        self.assertEqual(len(ref_lines), 1)
        self.assertEqual(ref_lines[0].get("type"), "way")

        self.assertEqual(self._linked_lanelet_ids(), {"100"})

    def test_reversed_boundary_neighbor_lanelet_is_not_mislinked(self):
        # Lanelet 900 physically overlaps lanelet 100's stop waypoint (map
        # (5, 0) sits inside 900's own bowtie fold, see
        # REVERSED_RIGHT_BOUNDARY_LANELET_OSM), so a vertex/containment-based
        # match can steal the traffic light away from the lanelet it actually
        # approaches. The current fit-based linking must not do that.
        self._write(_osm_with(REVERSED_RIGHT_BOUNDARY_LANELET_OSM))
        light = FakeTrafficLight(actor_id=9, opendrive_id="5",
                                  stop_wps=[_traffic_light_waypoint()],
                                  light_boxes=[FakeLightBox((5.0, 0.0, 5.0))])

        injected = gl2m.inject_traffic_lights(FakeWorld([light]), self.osm_path)

        self.assertEqual(injected, 1)
        self.assertEqual(self._linked_lanelet_ids(), {"100"})


class CheckUniqueOpendriveIdTests(unittest.TestCase):
    """Covers the helper directly: inject_stop_signs pre-filters invalid ids
    before calling it, so only inject_traffic_lights reaches its skip branch."""

    def test_numeric_id_is_accepted_and_recorded(self):
        seen = {}

        self.assertEqual(gl2m._check_unique_opendrive_id("traffic light", 1, "42", seen), 42)
        self.assertEqual(seen, {42: 1})

    def test_non_numeric_or_empty_id_returns_none_without_recording(self):
        for opendrive_id in ("abc", ""):
            with self.subTest(opendrive_id=opendrive_id):
                seen = {}

                with contextlib.redirect_stdout(io.StringIO()):
                    result = gl2m._check_unique_opendrive_id("traffic light", 3, opendrive_id, seen)

                self.assertIsNone(result)
                self.assertEqual(seen, {})

    def test_duplicate_numeric_id_raises(self):
        seen = {}
        gl2m._check_unique_opendrive_id("traffic light", 4, "7", seen)

        with self.assertRaises(RuntimeError):
            gl2m._check_unique_opendrive_id("traffic light", 5, "007", seen)


if __name__ == "__main__":
    unittest.main()
