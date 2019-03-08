# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import carla

import unittest


from . import TESTING_ADDRESS


class TestClient(unittest.TestCase):
    def test_version(self):
        c = carla.Client(*TESTING_ADDRESS)
        self.assertEqual(c.get_client_version(), c.get_server_version())
