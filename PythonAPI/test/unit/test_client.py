# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import os
import sys
import unittest

import carla

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
from expected_version import expected_version_pattern  # noqa: E402


class TestClient(unittest.TestCase):
    def test_client_version(self):
        pattern = expected_version_pattern()
        if pattern is None:
            self.skipTest("not a git checkout: the built commit is unknown")
        c = carla.Client('localhost', 8080)
        self.assertRegex(c.get_client_version(), r"\A{}\Z".format(pattern))
