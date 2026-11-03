# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Unit tests for agents.tools.misc."""

import os
import sys
import unittest

import carla

AGENTS_PARENT_DIR = os.path.normpath(os.path.join(
    os.path.dirname(os.path.abspath(__file__)), "..", "..", "carla",
))
sys.path.insert(0, AGENTS_PARENT_DIR)

from agents.tools.misc import is_within_distance  # pylint: disable=import-error,wrong-import-position


def _transform(x, y, yaw=0.0):
    return carla.Transform(carla.Location(x=x, y=y), carla.Rotation(yaw=yaw))


class TestIsWithinDistance(unittest.TestCase):
    """The ego looks along +x, so a target on the x axis is at exactly 0 or
    180 degrees, the ends of the angle intervals the navigation agents pass.
    """

    def setUp(self):
        self.ego = _transform(0.0, 0.0)

    def test_target_straight_ahead_is_in_interval_starting_at_0(self):
        """A target dead ahead is inside [0, 90]."""
        target = _transform(10.0, 0.0)
        self.assertTrue(is_within_distance(target, self.ego, 20.0, [0, 90]))

    def test_target_straight_behind_is_in_interval_ending_at_180(self):
        """A target dead behind is inside [160, 180]."""
        target = _transform(-10.0, 0.0)
        self.assertTrue(is_within_distance(target, self.ego, 20.0, [160, 180]))

    def test_target_outside_interval_is_rejected(self):
        """Angles outside the interval are still rejected."""
        self.assertFalse(is_within_distance(_transform(-10.0, 0.0), self.ego, 20.0, [0, 90]))
        self.assertFalse(is_within_distance(_transform(10.0, 0.0), self.ego, 20.0, [160, 180]))
        self.assertFalse(is_within_distance(_transform(10.0, 10.0), self.ego, 20.0, [0, 30]))

    def test_target_beyond_max_distance_is_rejected(self):
        """The distance check still applies."""
        target = _transform(30.0, 0.0)
        self.assertFalse(is_within_distance(target, self.ego, 20.0, [0, 90]))


if __name__ == '__main__':
    unittest.main()
