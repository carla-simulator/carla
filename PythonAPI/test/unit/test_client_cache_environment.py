# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Native client initialization tolerates an omitted user-home environment."""

import importlib.util
import os
from pathlib import Path
import subprocess
import sys
import unittest


class TestClientCacheEnvironment(unittest.TestCase):
    """Exercise startup in new processes before the native cache initializer runs."""

    def setUp(self):
        spec = importlib.util.find_spec('carla')
        if spec is None or spec.origin is None:
            self.skipTest('An installed native CARLA module is required')
        origin = Path(spec.origin).resolve()
        self.module_root = origin.parent.parent if spec.submodule_search_locations else origin.parent
        self.working_directory = Path(__file__).resolve().parent
        self.home_variable = 'USERPROFILE' if os.name == 'nt' else 'HOME'

    def assert_native_import(self, home_value=None, cache_override=None):
        """Modify only child environment values; import performs no file request."""
        environment = os.environ.copy()
        environment.pop(self.home_variable, None)
        environment.pop('CARLA_CACHE_DIR', None)
        if home_value is not None:
            environment[self.home_variable] = home_value
        if cache_override is not None:
            environment['CARLA_CACHE_DIR'] = cache_override
        program = '''
import os
import sys
if os.name == 'nt':
    import ctypes
    ctypes.windll.kernel32.SetErrorMode(0x8003)
sys.path.insert(0, sys.argv[1])
import carla
assert carla.Location(1.0, 2.0, 3.0).x == 1.0
print('native-cache-import-ok')
'''
        result = subprocess.run(
            [sys.executable, '-I', '-B', '-X', 'faulthandler', '-c', program, str(self.module_root)],
            cwd=self.working_directory, env=environment, capture_output=True, text=True,
            encoding='utf-8', errors='replace', timeout=30, check=False)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn('native-cache-import-ok', result.stdout)

    def test_native_import_without_home_environment(self):
        """A home variable is optional for importing and using native value types."""
        self.assert_native_import()

    def test_empty_home_environment_control(self):
        """Retain the existing empty-home working-directory fallback."""
        self.assert_native_import(home_value='')

    def test_present_home_environment_control(self):
        """Ordinary home-based initialization continues to work."""
        self.assert_native_import(home_value=str(self.working_directory))

    def test_cache_override_without_home_control(self):
        """An explicit cache path continues to take precedence over the home value."""
        self.assert_native_import(cache_override=str(self.working_directory / 'cache-override'))


if __name__ == '__main__':
    unittest.main()
