# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import random

from . import SmokeTest


class TestClient(SmokeTest):
    def test_version(self):
        self.assertEqual(self.client.get_client_version(), self.client.get_server_version())

    def test_reload_world(self):
        map_name = self.client.get_world().get_map().name
        world = self.client.reload_world()
        self.assertEqual(map_name, world.get_map().name)

    def test_load_all_maps(self):
        map_names = list(self.client.get_available_maps())
        random.shuffle(map_names)
        for map_name in map_names:
            world = self.client.load_world(map_name)
            self.assertEqual(map_name.split('/')[-1], world.get_map().name)
