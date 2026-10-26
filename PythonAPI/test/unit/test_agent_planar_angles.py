# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Planar angular checks on real graded OpenDrive lanes."""
import math
import os
from pathlib import Path
import sys
import unittest

import carla

ROOT = Path(os.environ.get('CARLA_PLANAR_ANGLE_TEST_ROOT', Path(__file__).resolve().parents[3]))
sys.path.insert(0, str(ROOT / 'PythonAPI/carla'))
from agents.tools.misc import is_within_distance  # pylint: disable=wrong-import-position


def native_lane(grade, heading, rule, lane_id):
    """Create real surface heights/orientations and a slightly offset forward target."""
    xml = ('<OpenDRIVE><header revMajor="1" revMinor="4"><geoReference>'
           '+proj=tmerc +lat_0=0 +lon_0=0 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m'
           f'</geoReference></header><road id="1" name="grade" length="100" junction="-1" rule="{rule}">'
           f'<planView><geometry s="0" x="0" y="0" hdg="{heading}" length="100"><line/></geometry></planView>'
           f'<elevationProfile><elevation s="0" a="0" b="{grade}" c="0" d="0"/></elevationProfile>'
           '<lanes><laneSection s="0"><left><lane id="1" type="driving" level="false">'
           '<width sOffset="0" a="3.5" b="0" c="0" d="0"/></lane></left>'
           '<center><lane id="0" type="none"/></center><right><lane id="-1" type="driving" level="false">'
           '<width sOffset="0" a="3.5" b="0" c="0" d="0"/></lane></right>'
           '</laneSection></lanes></road></OpenDRIVE>')
    native_map = carla.Map('graded-planar-angle', xml)
    reference = native_map.get_waypoint_xodr(1, lane_id, 40).transform
    positive = (rule == 'RHT') == (lane_id < 0)
    target = native_map.get_waypoint_xodr(1, lane_id, 48 if positive else 32).transform
    target.location += carla.Location(target.get_right_vector() * 0.1)
    return reference, target


class TestPlanarAgentAngles(unittest.TestCase):
    """Separate slope normalization from the exact-interval-boundary defect."""

    def test_real_sloped_lane_target_stays_inside_narrow_planar_cone(self):
        """A nonzero bearing avoids reliance on either exact interval endpoint."""
        for grade in (-0.2, 0.2):
            for heading in (0.0, 0.7):
                for rule in ('RHT', 'LHT'):
                    for lane_id in (-1, 1):
                        with self.subTest(grade=grade, heading=heading, rule=rule, lane=lane_id):
                            reference, target = native_lane(grade, heading, rule, lane_id)
                            delta = target.location - reference.location
                            bearing = math.degrees(math.atan2(delta.y, delta.x))
                            angle = abs((bearing - reference.rotation.yaw + 180) % 360 - 180)
                            self.assertGreater(angle, 0.1)
                            self.assertLess(angle, 2.0)
                            self.assertGreater(abs(delta.z), 1.0)
                            self.assertTrue(is_within_distance(target, reference, 15, [0, 5]))

    def test_flat_lane_nonzero_bearing_control(self):
        """The same horizontal geometry already works when pitch is zero."""
        for rule in ('RHT', 'LHT'):
            for lane in (-1, 1):
                reference, target = native_lane(0.0, 0.7, rule, lane)
                self.assertTrue(is_within_distance(target, reference, 15, [0, 5]))

    def test_sloped_distance_and_outside_cone_controls(self):
        """Normalizing orientation does not alter range or widen the requested cone."""
        reference, target = native_lane(0.2, 0.0, 'RHT', -1)
        self.assertFalse(is_within_distance(target, reference, 5, [0, 5]))
        target.location += carla.Location(target.get_right_vector() * 5.0)
        self.assertFalse(is_within_distance(target, reference, 15, [0, 5]))


if __name__ == '__main__':
    unittest.main()
