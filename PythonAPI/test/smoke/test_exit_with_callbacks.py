# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Smoke test: a script that exits with callbacks registered must not crash.

CARLA's client threads keep delivering world ticks after the script ends,
while Python is being finalized. Before the module stopped them from calling
into Python at exit, about a third of these runs crashed on Python 3.14
(segmentation fault or "Fatal Python error: gilstate_tss_set").
"""

import subprocess
import sys
import textwrap

from . import SmokeTest, TESTING_ADDRESS

# Ends while several on_tick callbacks are still firing on every server frame.
CHILD_SCRIPT = textwrap.dedent('''
    import sys
    import time
    import carla

    class Hud:
        def __init__(self):
            self.history = []

        def on_world_tick(self, snapshot):
            self.history.append(snapshot.timestamp.elapsed_seconds)
            if len(self.history) > 100:
                self.history = self.history[-50:]

    client = carla.Client(sys.argv[1], int(sys.argv[2]))
    client.set_timeout(30.0)
    world = client.get_world()
    huds = [Hud() for _ in range(6)]
    for hud in huds:
        world.on_tick(lambda snapshot, hud=hud: hud.on_world_tick(snapshot))
    time.sleep(0.5)
''')

RUNS = 20


class TestExitWithCallbacks(SmokeTest):
    def test_exit_while_on_tick_callbacks_fire(self):
        # Callbacks only fire while the world ticks on its own.
        settings = self.world.get_settings()
        self.assertFalse(settings.synchronous_mode)
        host, port = TESTING_ADDRESS
        for run in range(RUNS):
            result = subprocess.run(
                [sys.executable, '-c', CHILD_SCRIPT, host, str(port)],
                capture_output=True, text=True, timeout=120, check=False)
            self.assertEqual(
                result.returncode, 0,
                f'run {run + 1} of {RUNS} exited with {result.returncode}:\n'
                f'{result.stderr[-2000:]}')
