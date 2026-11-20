"""Exercise light CLI delivery and teardown against a deferred RPC boundary."""

import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[4]
SCRIPT = ROOT / "Skills/python-api/control-lights/scripts/lights.py"
FAKE_CARLA = r"""
import atexit, json, os, threading
from pathlib import Path
from types import SimpleNamespace
base = Path(os.environ['LIGHT_TEST_OUTPUT'])
atexit.register(lambda: (base / 'cleanup').touch())
class LightGroup:
    NONE, Street, Building, Vehicle, Other = range(5)
class Color:
    def __init__(self, r, g, b):
        self.r, self.g, self.b = r, g, b
class Manager:
    def __init__(self):
        self.pending = {}
    def get_all_lights(self, group):
        return [SimpleNamespace(id=1, is_on=False, intensity=100,
            color=Color(255, 255, 255), location=SimpleNamespace(x=0, y=0))]
    def turn_on(self, lights):
        self.pending['active'] = True
    def turn_off(self, lights):
        self.pending['active'] = False
    def set_color(self, lights, color):
        self.pending['color'] = [color.r, color.g, color.b]
    def set_intensity(self, lights, intensity):
        self.pending['intensity'] = intensity
class World:
    def __init__(self):
        self.manager = Manager()
    def get_settings(self):
        if (base / 'submitted.json').is_file():
            if os.environ.get('LIGHT_TEST_RPC_FAILURE'):
                raise RuntimeError('RPC connection failed')
            (base / 'received.json').write_bytes((base / 'submitted.json').read_bytes())
            (base / 'barrier').touch()
        return SimpleNamespace(synchronous_mode=False)
    def get_lightmanager(self):
        return self.manager
    def on_tick(self, callback):
        (base / 'subscribed').touch()
        if not os.environ.get('LIGHT_TEST_NO_TICK'):
            def submit():
                (base / 'submitted.json').write_text(json.dumps(self.manager.pending))
                callback(None)
            threading.Thread(target=submit).start()
        return 1
    def remove_on_tick(self, callback_id):
        assert callback_id == 1
        (base / 'removed').touch()
    def tick(self):
        raise AssertionError('The CLI must not advance the simulation')
world = World()
class Client:
    def __init__(self, host, port):
        pass
    def set_timeout(self, timeout):
        pass
    def get_world(self):
        return world
    def get_server_version(self):
        return '1.0.0'

"""


class TestLightsCLI(unittest.TestCase):
    """Execute the real CLI, including its callback acknowledgement and exit."""

    def setUp(self):
        # The directory spans setUp and the test; addCleanup owns its release.
        # pylint: disable-next=consider-using-with
        self.temporary = tempfile.TemporaryDirectory(prefix="carla lights ")
        self.addCleanup(self.temporary.cleanup)
        self.folder = Path(self.temporary.name)
        (self.folder / "carla.py").write_text(FAKE_CARLA, encoding="utf-8")

    def run_cli(self, arguments, **settings):
        """Keep the native/server boundary explicit and run a real interpreter."""
        environment = dict(os.environ, PYTHONPATH=str(self.folder),
                           LIGHT_TEST_OUTPUT=str(self.folder), CARLA_TIMEOUT="2",
                           PYTHONIOENCODING="utf-8")
        environment.update(settings)
        return subprocess.run([sys.executable, str(SCRIPT), *arguments], env=environment,
                              capture_output=True, text=True, check=False, timeout=10)

    def assert_received(self, result, expected):
        """Success requires both the submitted changes and the RPC barrier."""
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertTrue((self.folder / "received.json").is_file())
        self.assertEqual(json.loads((self.folder / "received.json").read_text()), expected)
        for name in ("cleanup", "removed", "barrier"):
            self.assertTrue((self.folder / name).is_file(), name)

    def test_turn_on_submits_all_requested_fields(self):
        """Submit the enabled state, color and intensity before success."""
        result = self.run_cli(["on", "--group", "street", "--color", "1,2,3", "--intensity", "10"])
        self.assert_received(result, {"active": True, "color": [1, 2, 3], "intensity": 10.0})

    def test_turn_off_submits_before_exit(self):
        """Submit the disabled state before normal termination."""
        self.assert_received(self.run_cli(["off", "--group", "street"]), {"active": False})

    def test_set_preserves_toggle_state(self):
        """Changing color and intensity must retain the toggle state."""
        result = self.run_cli(["set", "--color", "4,5,6", "--intensity", "20"])
        self.assert_received(result, {"color": [4, 5, 6], "intensity": 20.0})

    def test_missing_tick_fails_and_removes_callback(self):
        """A stalled clock must fail and release the callback."""
        result = self.run_cli(["on"], LIGHT_TEST_NO_TICK="1", CARLA_TIMEOUT="0.02")
        self.assertNotEqual(result.returncode, 0)
        self.assertNotIn("turned ON", result.stdout)
        self.assertTrue((self.folder / "removed").is_file())
        self.assertFalse((self.folder / "received.json").exists())

    def test_rpc_failure_is_not_reported_as_success(self):
        """A failed receipt check must not print success."""
        result = self.run_cli(["on"], LIGHT_TEST_RPC_FAILURE="1")
        self.assertNotEqual(result.returncode, 0)
        self.assertNotIn("turned ON", result.stdout)
        self.assertTrue((self.folder / "removed").is_file())

    def test_list_does_not_wait_for_write_acknowledgement(self):
        """Listing must retain its existing readonly behavior."""
        result = self.run_cli(["list", "--group", "street"])
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("1 'street' light(s)", result.stdout)
        self.assertTrue((self.folder / "cleanup").exists())
        self.assertFalse((self.folder / "subscribed").exists())

    def test_empty_set_retains_validation(self):
        """Reject an empty update before subscribing for ticks."""
        result = self.run_cli(["set"])
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("set needs --color and/or --intensity", result.stderr)
        self.assertFalse((self.folder / "subscribed").exists())


if __name__ == "__main__":
    unittest.main()
