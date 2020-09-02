# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.


from . import SmokeTest


class TestClient(SmokeTest):
    def test_version(self):
        print("TestClient.test_version")
        self.assertEqual(self.client.get_client_version(), self.client.get_server_version())
