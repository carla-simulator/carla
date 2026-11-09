# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import subprocess
import sys
import textwrap

from . import SmokeTest

# The script changes a light, which the light manager sends to the server on the
# next tick. Synchronous mode means no tick arrives, so the change is still
# pending when the client goes away and the light manager is destroyed last.
# The destructor used to throw from there, which aborts the process, so the
# script runs in its own process instead of taking the test runner down.
SCRIPT = textwrap.dedent("""
    import gc, sys
    import carla
    client = carla.Client(sys.argv[1], int(sys.argv[2]))
    client.set_timeout(60.0)
    world = client.get_world()
    settings = world.get_settings()
    settings.synchronous_mode = True
    settings.fixed_delta_seconds = 0.05
    world.apply_settings(settings)
    light_manager = world.get_lightmanager()
    lights = light_manager.get_all_lights()
    if not lights:
        print("NO_LIGHTS", flush=True)
        sys.exit(0)
    light_manager.turn_off(lights[:1])
    del lights, world, settings, client
    gc.collect()
    del light_manager
    gc.collect()
    print("SURVIVED", flush=True)
""")


class TestLightManager(SmokeTest):

    def setUp(self):
        super().setUp()
        self.original_settings = self.world.get_settings()

    def tearDown(self):
        self.client.get_world().apply_settings(self.original_settings)
        super().tearDown()

    def test_pending_changes_outliving_the_client(self):
        result = subprocess.run(
            [sys.executable, "-c", SCRIPT, *map(str, self.testing_address)],
            capture_output=True, text=True, timeout=300)
        if "NO_LIGHTS" in result.stdout:
            self.skipTest("the current map has no lights")
        self.assertEqual(
            result.returncode, 0,
            "the process died when the light manager was destroyed:\n" + result.stderr)
        self.assertIn("SURVIVED", result.stdout)
