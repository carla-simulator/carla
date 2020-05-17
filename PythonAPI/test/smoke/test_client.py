# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.


from . import SmokeTest


class TestClient(SmokeTest):
    def test_version(self):
        client_v = self.client.get_client_version().replace("-dbg", "")
        server_v = self.client.get_server_version().replace("-dbg", "")
        self.assertEqual(client_v, server_v)
