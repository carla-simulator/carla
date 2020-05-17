# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import carla
import time

from . import SmokeTest


class TestSensor(SmokeTest):
    def test_all_sensors(self):
        def listen(data):
            assert data is not None

        world = self.client.get_world()
        bp_library = world.get_blueprint_library()
        for bp in bp_library.filter('sensor.*'):
            sensor = world.spawn_actor(bp, carla.Transform())
            sensor.listen(listen)
            time.sleep(2)
            sensor.destroy()
