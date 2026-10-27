# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Valid terminal fork lanes do not require a three-metre successor."""

# Exercise the complete queue builder with actual native lane geometry.
# pylint: disable=protected-access

import os
from pathlib import Path
import sys
from types import SimpleNamespace
import unittest

import carla

ROOT = Path(os.environ.get('CARLA_TERMINAL_PROBE_ROOT', Path(__file__).resolve().parents[3]))
sys.path.insert(0, str(ROOT / 'PythonAPI/carla'))
from agents.navigation.local_planner import (  # pylint: disable=wrong-import-position
    LocalPlanner, RoadOption, _retrieve_options)


def native_terminal_fork(rule, lengths):
    """Create real outgoing lanes with authored finite terminal lengths."""
    lane_id = -1 if rule == 'RHT' else 1
    side = 'right' if lane_id < 0 else 'left'
    xml = ('<OpenDRIVE><header revMajor="1" revMinor="4"><geoReference>'
           '+proj=tmerc +lat_0=0 +lon_0=0 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m'
           '</geoReference></header>')
    for road_id in (1, 2, 3):
        if road_id == 1:
            length, x, junction, geometry = 10, 0, -1, '<line/>'
            road_link = '<successor elementType="junction" elementId="9"/>'
            lane_link = f'<successor id="{lane_id}"/>'
        else:
            length, x, junction = lengths[road_id - 2], 10, 9
            geometry = f'<arc curvature="{.01 if road_id == 2 else -.01}"/>'
            road_link = '<predecessor elementType="road" elementId="1" contactPoint="end"/>'
            lane_link = f'<predecessor id="{lane_id}"/>'
        xml += (f'<road id="{road_id}" length="{length}" junction="{junction}" rule="{rule}">'
                f'<link>{road_link}</link><planView><geometry s="0" x="{x}" y="0" hdg="0" '
                f'length="{length}">{geometry}</geometry></planView><lanes><laneSection s="0">'
                '<center><lane id="0" type="none"/></center>'
                f'<{side}><lane id="{lane_id}" type="driving"><link>{lane_link}</link>'
                '<width sOffset="0" a="3.5" b="0" c="0" d="0"/>'
                f'</lane></{side}></laneSection></lanes></road>')
    xml += '<junction id="9">'
    for index, connecting in enumerate((2, 3)):
        xml += (f'<connection id="{index}" incomingRoad="1" connectingRoad="{connecting}" '
                f'contactPoint="start"><laneLink from="{lane_id}" to="{lane_id}"/></connection>')
    return carla.Map('native-terminal-fork', xml + '</junction></OpenDRIVE>'), lane_id


class TestLocalTerminalProbe(unittest.TestCase):
    """A valid candidate can terminate before the turn-classification lookahead."""

    def candidates(self, rule, lengths):
        """Keep every lane and waypoint query native, including terminal GetNext."""
        native_map, lane_id = native_terminal_fork(rule, lengths)
        current = native_map.get_waypoint_xodr(1, lane_id, 9.)
        branches = current.next(2.)
        self.assertEqual({waypoint.road_id for waypoint in branches}, {2, 3})
        return native_map, current, branches

    @staticmethod
    def planner(native_map, current):
        """Keep vehicle/world reads inert while constructing the real controller."""
        world = SimpleNamespace(get_map=lambda: native_map)
        vehicle = SimpleNamespace(get_world=lambda: world,
                                  get_control=carla.VehicleControl,
                                  get_location=lambda: current.transform.location)
        return LocalPlanner(vehicle, map_inst=native_map)

    def test_short_terminal_candidates_still_receive_turn_labels(self):
        """One or both finite dead ends remain valid candidates for classification."""
        for rule in ('RHT', 'LHT'):
            for lengths in ((1.5, 10.), (10., 1.5), (1.5, 1.5)):
                with self.subTest(rule=rule, lengths=lengths):
                    _, current, branches = self.candidates(rule, lengths)
                    self.assertTrue(any(not waypoint.next(3.) for waypoint in branches))
                    self.assertEqual(_retrieve_options(branches, current), [RoadOption.STRAIGHT] * 2)

    def test_complete_queue_builder_accepts_a_native_short_terminal_branch(self):
        """Turn lookahead must not crash the actual local planner's queue update."""
        for rule in ('RHT', 'LHT'):
            with self.subTest(rule=rule):
                native_map, current, _ = self.candidates(rule, (1.5, 10.))
                planner = self.planner(native_map, current)
                planner.set_global_plan([(current, RoadOption.LANEFOLLOW)], stop_waypoint_creation=False)
                planner._compute_next_waypoints(1)
                self.assertEqual(len(planner.get_plan()), 2)
                waypoint, option = planner.get_plan()[-1]
                self.assertIn(waypoint.road_id, (2, 3))
                self.assertEqual(option, RoadOption.STRAIGHT)

    def test_long_successor_lookahead_control_keeps_both_labels(self):
        """Ordinary branches still classify their actual three-metre successors."""
        for rule in ('RHT', 'LHT'):
            with self.subTest(rule=rule):
                _, current, branches = self.candidates(rule, (10., 10.))
                self.assertTrue(all(waypoint.next(3.) for waypoint in branches))
                self.assertEqual(_retrieve_options(branches, current), [RoadOption.STRAIGHT] * 2)


if __name__ == '__main__':
    unittest.main()
