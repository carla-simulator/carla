# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import time

import carla

from . import SmokeTest

# The packaged build only ships Town10HD_Opt and Mine_01; the default
# SmokeTest.tearDown loads Town03, which does not exist on the package.
TEST_TOWN = "Town10HD_Opt"


class TestDebugClear(SmokeTest):

    def setUp(self):
        super().setUp()
        self.load_world(TEST_TOWN)
        self.world = self.client.get_world()
        self.debug = self.world.debug

    def tearDown(self):
        self.load_world(TEST_TOWN)
        self.world = None
        self.client = None

    def test_clear_debug_shapes_and_strings(self):
        print("TestDebugClear.test_clear_debug_shapes_and_strings")

        # Clearing with nothing drawn must be a no-op, not a crash: the
        # handlers null-guard the line batcher and the HUD.
        self.debug.clear_debug_shape()
        self.debug.clear_debug_string()
        self.assertIsNotNone(self.world.wait_for_tick())

        for i in range(10):
            self.debug.draw_line(
                carla.Location(x=float(i), y=0.0, z=1.0),
                carla.Location(x=float(i), y=10.0, z=1.0),
                thickness=0.1,
                color=carla.Color(255, 0, 0),
                life_time=-1.0,
                persistent_lines=True,
            )
            self.debug.draw_point(
                carla.Location(x=float(i), y=5.0, z=2.0),
                size=0.2,
                color=carla.Color(0, 255, 0),
                life_time=-1.0,
                persistent_lines=True,
            )
            self.debug.draw_string(
                carla.Location(x=float(i), y=0.0, z=2.5),
                f"label_{i}",
                draw_shadow=False,
                color=carla.Color(255, 255, 255),
                life_time=-1.0,
                persistent_lines=True,
            )

        # Give the server one tick to register the shapes.
        before = self.world.wait_for_tick()
        self.assertIsNotNone(before)

        self.debug.clear_debug_shape()
        self.debug.clear_debug_string()

        # The world must still be responsive after the clear: two ticks
        # without a timeout pin "no exception, no RPC stall".
        after = self.world.wait_for_tick()
        self.assertGreater(after.frame, before.frame)
        self.assertGreater(self.world.wait_for_tick().frame, after.frame)
