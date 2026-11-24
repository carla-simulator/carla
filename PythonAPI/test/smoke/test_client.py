# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import os
import sys

from . import SmokeTest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
from expected_version import expected_version_pattern  # noqa: E402


class TestClient(SmokeTest):
    def test_version(self):
        print("TestClient.test_version")
        self.assertEqual(self.client.get_client_version(), self.client.get_server_version())

    def test_version_matches_checkout(self):
        print("TestClient.test_version_matches_checkout")
        pattern = expected_version_pattern()
        if pattern is None:
            self.skipTest("not a git checkout: the built commit is unknown")
        self.assertRegex(self.client.get_client_version(), r"\A{}\Z".format(pattern))
