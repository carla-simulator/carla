# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.


import carla

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

    def test_map_layer_flags_combine(self):
        print("TestWorld.test_map_layer_flags_combine")
        combined = carla.MapLayer.Ground | carla.MapLayer.Buildings
        # A combination stays a MapLayer, which load_map_layer() accepts.
        self.assertIsInstance(combined, carla.MapLayer)
        self.assertEqual(int(combined), int(carla.MapLayer.Ground) | int(carla.MapLayer.Buildings))
        self.assertIsInstance(combined & carla.MapLayer.Ground, carla.MapLayer)
        self.assertEqual(int(combined & carla.MapLayer.Ground), int(carla.MapLayer.Ground))
        self.assertIsInstance(~carla.MapLayer.ParkedVehicles, carla.MapLayer)
        self.assertEqual(int(~carla.MapLayer.All), int(carla.MapLayer.NONE))
        # A plain int on either side keeps int arithmetic.
        self.assertNotIsInstance(combined | 0x200, carla.MapLayer)
        self.assertNotIsInstance(0x200 | combined, carla.MapLayer)
        world = self.client.get_world()
        world.load_map_layer(combined)
        world.unload_map_layer(carla.MapLayer.Ground | carla.MapLayer.Buildings)
