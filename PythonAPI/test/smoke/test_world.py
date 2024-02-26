# Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.


from . import SmokeTest


class TestWorld(SmokeTest):
    def test_fixed_delta_seconds(self):
        print("TestWorld.test_fixed_delta_seconds")
        world = self.client.get_world()
        settings = world.get_settings()
        self.assertFalse(settings.synchronous_mode)
        for expected_delta_seconds in [0.1, 0.066667, 0.05, 0.033333, 0.016667, 0.011112]:
            settings.fixed_delta_seconds = expected_delta_seconds
            world.apply_settings(settings)
            for _ in range(0, 20):
                delta_seconds = world.wait_for_tick().timestamp.delta_seconds
                self.assertAlmostEqual(expected_delta_seconds, delta_seconds)
        settings.fixed_delta_seconds = None
        world.apply_settings(settings)
