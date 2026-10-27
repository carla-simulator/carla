# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Every native fork branch remains reachable when turn labels coincide."""

# Exercise the complete queue builder with actual native lane geometry.
# pylint: disable=protected-access

import os
from pathlib import Path
import random
import sys
from types import SimpleNamespace
import unittest

import carla

ROOT = Path(os.environ.get('CARLA_BRANCH_TEST_ROOT', Path(__file__).resolve().parents[3]))
sys.path.insert(0, str(ROOT / 'PythonAPI/carla'))
from agents.navigation.local_planner import (  # pylint: disable=wrong-import-position
    LocalPlanner, RoadOption, _retrieve_options)


def native_fork(rule, curvatures=(.01, -.01)):
    """Create two real outgoing junction branches with independently varied labels."""
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
            length, x, junction = 10, 10, 9
            geometry = f'<arc curvature="{curvatures[road_id - 2]}"/>'
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
    return carla.Map('native-random-fork', xml + '</junction></OpenDRIVE>'), lane_id


class TestLocalPlannerBranchSelection(unittest.TestCase):
    """Random waypoint selection must not collapse equal-valued turn labels."""

    @staticmethod
    def planner(native_map, waypoint):
        """Only the remote vehicle state boundary is inert; planner/controller are real."""
        world = SimpleNamespace(get_map=lambda: native_map)
        vehicle = SimpleNamespace(get_world=lambda: world,
                                  get_control=carla.VehicleControl,
                                  get_location=lambda: waypoint.transform.location)
        return LocalPlanner(vehicle, map_inst=native_map)

    def choices(self, rule, curvatures):
        """Run deterministic real random seeds without leaving global RNG state behind."""
        native_map, lane_id = native_fork(rule, curvatures)
        current = native_map.get_waypoint_xodr(1, lane_id, 9.)
        branches = current.next(2.)
        self.assertEqual({waypoint.road_id for waypoint in branches}, {2, 3})
        options = _retrieve_options(branches, current)
        planner = self.planner(native_map, current)
        selected = set()
        state = random.getstate()
        try:
            for seed in range(64):
                random.seed(seed)
                planner.set_global_plan([(current, RoadOption.LANEFOLLOW)], stop_waypoint_creation=False)
                planner._compute_next_waypoints(1)
                self.assertEqual(len(planner.get_plan()), 2)
                waypoint, option = planner.get_plan()[-1]
                selected.add(waypoint.road_id)
                expected = options[[candidate.road_id for candidate in branches].index(waypoint.road_id)]
                self.assertEqual(option, expected)
        finally:
            random.setstate(state)
        return options, selected

    def test_duplicate_turn_labels_do_not_hide_either_native_branch(self):
        """Matching straight, left or right labels retain two valid native choices."""
        for rule in ('RHT', 'LHT'):
            for curvatures, expected in (((.01, -.01), RoadOption.STRAIGHT),
                                         ((.2, .22), RoadOption.LEFT),
                                         ((-.2, -.22), RoadOption.RIGHT)):
                with self.subTest(rule=rule, curvatures=curvatures):
                    options, selected = self.choices(rule, curvatures)
                    self.assertEqual(options, [expected, expected])
                    self.assertEqual(selected, {2, 3})

    def test_distinct_turn_labels_preserve_both_choices_control(self):
        """The original left/right fork behavior remains available."""
        for rule in ('RHT', 'LHT'):
            with self.subTest(rule=rule):
                options, selected = self.choices(rule, (.2, -.2))
                self.assertEqual(set(options), {RoadOption.LEFT, RoadOption.RIGHT})
                self.assertEqual(selected, {2, 3})

    def test_single_branch_control_remains_lane_follow(self):
        """Sampling before the fork keeps the ordinary one-successor path."""
        native_map, lane_id = native_fork('RHT')
        current = native_map.get_waypoint_xodr(1, lane_id, 1.)
        planner = self.planner(native_map, current)
        planner._compute_next_waypoints(1)
        waypoint, option = planner.get_plan()[-1]
        self.assertEqual(waypoint.road_id, 1)
        self.assertEqual(option, RoadOption.LANEFOLLOW)


if __name__ == '__main__':
    unittest.main()
