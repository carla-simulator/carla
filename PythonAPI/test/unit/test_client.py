# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import unittest
from subprocess import check_output

import carla


class TestClient(unittest.TestCase):
    def test_client_version(self):
        c = carla.Client('localhost', 8080)
        v = c.get_client_version()
        out = check_output(
            [
                'git',
                'describe',
                '--tags',
                '--dirty',
                '--always',
            ]
        )
        out = out.decode('utf8')
        self.assertEqual(str(v), str(out.strip()))
