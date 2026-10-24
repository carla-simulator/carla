# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Exercise config.py's real CLI with optional native timestep semantics."""

import contextlib
import importlib.util
import io
from pathlib import Path
import sys
import types
import unittest
from unittest import mock


class WorldSettings:
    """The native setter returns None for zero/negative fixed_delta_seconds."""

    def __init__(self, timestep):
        self.fixed_delta_seconds = timestep

    @property
    def fixed_delta_seconds(self):
        return self._fixed_delta_seconds

    @fixed_delta_seconds.setter
    def fixed_delta_seconds(self, value):
        # Mirrors the optional-value boundary in PythonAPI/carla/src/World.cpp.
        self._fixed_delta_seconds = value if value is not None and value > 0.0 else None


class TestConfigTimestep(unittest.TestCase):
    def run_config(self, arguments, initial_timestep=0.05):
        settings = WorldSettings(initial_timestep)
        world = mock.Mock()
        world.get_settings.return_value = settings
        client = mock.Mock()
        client.get_world.return_value = world
        carla = types.ModuleType('carla')
        carla.Client = mock.Mock(return_value=client)
        path = Path(__file__).resolve().parents[2] / 'util' / 'config.py'
        spec = importlib.util.spec_from_file_location('_config_timestep_test', path)
        config = importlib.util.module_from_spec(spec)
        output = io.StringIO()
        with mock.patch.dict(sys.modules, {'carla': carla}), \
                mock.patch.object(sys, 'argv', ['config.py'] + arguments), \
                contextlib.redirect_stdout(output):
            spec.loader.exec_module(config)
            config.main()
        world.apply_settings.assert_called_once_with(settings)
        return settings, output.getvalue()

    def test_zero_fps_and_delta_select_variable_time(self):
        for option in ['--fps', '--delta-seconds']:
            with self.subTest(option=option):
                settings, output = self.run_config([option, '0'])

                self.assertIsNone(settings.fixed_delta_seconds)
                self.assertIn('Set variable frame rate.', output)

    def test_nonpositive_options_normalize_to_variable_time(self):
        for option in ['--fps', '--delta-seconds']:
            with self.subTest(option=option):
                settings, output = self.run_config([option, '-1'])

                self.assertIsNone(settings.fixed_delta_seconds)
                self.assertIn('Set variable frame rate.', output)

    def test_positive_fps_sets_reciprocal_timestep(self):
        settings, output = self.run_config(['--fps', '20'], initial_timestep=None)

        self.assertAlmostEqual(settings.fixed_delta_seconds, 0.05)
        self.assertIn('50.00 milliseconds (20 FPS)', output)

    def test_positive_delta_sets_fixed_timestep(self):
        settings, output = self.run_config(['--delta-seconds', '0.1'], initial_timestep=None)

        self.assertAlmostEqual(settings.fixed_delta_seconds, 0.1)
        self.assertIn('100.00 milliseconds (10 FPS)', output)

    def test_unselected_timestep_is_preserved(self):
        for initial in [None, 0.05]:
            with self.subTest(initial=initial):
                settings, output = self.run_config(['--no-sync'], initial_timestep=initial)

                self.assertEqual(settings.fixed_delta_seconds, initial)
                self.assertNotIn('frame rate', output)

    def test_delta_seconds_retains_precedence_over_fps(self):
        settings, output = self.run_config(['--fps', '20', '--delta-seconds', '0'])

        self.assertIsNone(settings.fixed_delta_seconds)
        self.assertIn('Set variable frame rate.', output)


if __name__ == '__main__':
    unittest.main()
