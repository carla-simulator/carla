# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Run the actual map-file CLI with inert world/converter boundaries."""

import contextlib
import importlib.util
import io
from pathlib import Path
import sys
import tempfile
import types
import unittest
from unittest import mock


class TestConfigMapFiles(unittest.TestCase):
    """File errors must stop the CLI before world operations."""

    def setUp(self):
        self.world = mock.Mock()
        self.world.get_settings.return_value = types.SimpleNamespace(
            fixed_delta_seconds=0.05)
        self.client = mock.Mock()
        for method in ('get_world', 'load_world', 'reload_world',
                       'generate_opendrive_world'):
            getattr(self.client, method).return_value = self.world
        self.client_factory = mock.Mock(return_value=self.client)
        self.convert = mock.Mock(return_value='<converted-opendrive/>')
        self.stdout = io.StringIO()
        self.stderr = io.StringIO()

    def run_cli(self, arguments, read_error=False):
        """Load the complete production CLI and invoke its real main()."""
        carla_stub = types.SimpleNamespace(
            Client=self.client_factory,
            OpendriveGenerationParameters=types.SimpleNamespace,
            Osm2Odr=types.SimpleNamespace(convert=self.convert))
        source = Path(__file__).resolve().parents[2] / 'util' / 'config.py'
        spec = importlib.util.spec_from_file_location('_config_map_files', source)
        module = importlib.util.module_from_spec(spec)
        with contextlib.ExitStack() as stack:
            stack.enter_context(mock.patch.dict(sys.modules, {'carla': carla_stub}))
            stack.enter_context(mock.patch.object(sys, 'argv', ['config.py'] + arguments))
            stack.enter_context(contextlib.redirect_stdout(self.stdout))
            stack.enter_context(contextlib.redirect_stderr(self.stderr))
            spec.loader.exec_module(module)
            if read_error:
                opener = mock.mock_open()
                opener.return_value.read.side_effect = OSError('read blocked')
                stack.enter_context(mock.patch.object(module, 'open', opener, create=True))
            module.main()

    def assert_no_world_operations(self):
        """Argument failures must not load a world, convert data or apply settings."""
        for method in ('get_world', 'load_world', 'reload_world',
                       'generate_opendrive_world'):
            getattr(self.client, method).assert_not_called()
        self.world.get_settings.assert_not_called()
        self.world.apply_settings.assert_not_called()
        self.convert.assert_not_called()

    def assert_file_error(self, option, path, read_error=False):
        """Require argparse's nonzero status and a useful option-specific error."""
        with self.assertRaises(SystemExit) as raised:
            self.run_cli([option, str(path), '--no-sync'], read_error=read_error)
        self.assertEqual(raised.exception.code, 2)
        self.assertIn(f'{option}: cannot read', self.stderr.getvalue())
        self.assert_no_world_operations()

    def test_missing_xodr_path(self):
        """A nonexistent OpenDRIVE file is an argument error."""
        with tempfile.TemporaryDirectory(prefix='carla-map-input-') as directory:
            self.assert_file_error('--xodr-path', Path(directory) / 'missing.xodr')

    def test_missing_osm_path(self):
        """A nonexistent OSM file stops before the optional converter."""
        with tempfile.TemporaryDirectory(prefix='carla-map-input-') as directory:
            self.assert_file_error('--osm-path', Path(directory) / 'missing.osm')

    def test_directory_xodr_path(self):
        """A directory cannot be read as an OpenDRIVE file."""
        with tempfile.TemporaryDirectory(prefix='carla-map-input-') as directory:
            self.assert_file_error('--xodr-path', directory)

    def test_directory_osm_path(self):
        """A directory cannot be read as an OSM file."""
        with tempfile.TemporaryDirectory(prefix='carla-map-input-') as directory:
            self.assert_file_error('--osm-path', directory)

    def test_empty_xodr_path_argument(self):
        """An explicitly empty path is rejected rather than leaving world unbound."""
        self.assert_file_error('--xodr-path', '')

    def test_empty_osm_path_argument(self):
        """An explicitly empty OSM path is rejected."""
        self.assert_file_error('--osm-path', '')

    def test_undecodable_file(self):
        """Invalid UTF-8 gets the same clear file-error treatment."""
        with tempfile.TemporaryDirectory(prefix='carla-map-input-') as directory:
            path = Path(directory) / 'invalid.xodr'
            path.write_bytes(b'\xff')
            self.assert_file_error('--xodr-path', path)

    def test_read_failure_is_nonzero(self):
        """An error during read must not be reported as successful process exit."""
        with tempfile.TemporaryDirectory(prefix='carla-map-input-') as directory:
            path = Path(directory) / 'map.xodr'
            path.write_text('<OpenDRIVE/>', encoding='utf-8')
            self.assert_file_error('--xodr-path', path, read_error=True)

    def test_valid_xodr_input_preserves_data_and_parameters(self):
        """A readable path with spaces reaches the existing generation boundary."""
        with tempfile.TemporaryDirectory(prefix='carla-map-input-') as directory:
            path = Path(directory) / 'map with spaces.xodr'
            data = '<OpenDRIVE><!-- road \u00e9 --></OpenDRIVE>'
            path.write_text(data, encoding='utf-8')
            self.run_cli(['--xodr-path', str(path)])
        generated_data, parameters = self.client.generate_opendrive_world.call_args.args
        self.assertEqual(generated_data, data)
        self.assertEqual(parameters.vertex_distance, 2.0)
        self.assertEqual(parameters.max_road_length, 500.0)
        self.assertEqual(parameters.wall_height, 1.0)
        self.assertEqual(parameters.additional_width, 0.6)
        self.assertTrue(parameters.smooth_junctions)
        self.assertTrue(parameters.enable_mesh_visibility)
        self.convert.assert_not_called()
        self.world.apply_settings.assert_called_once_with(self.world.get_settings.return_value)

    def test_readable_osm_dispatches_to_recording_converter(self):
        """Check dispatch only; this boundary does not perform OSM conversion."""
        with tempfile.TemporaryDirectory(prefix='carla-map-input-') as directory:
            path = Path(directory) / 'map.osm'
            path.write_text('<osm/>', encoding='utf-8')
            self.run_cli(['--osm-path', str(path)])
        self.convert.assert_called_once_with('<osm/>')
        self.assertEqual(self.client.generate_opendrive_world.call_args.args[0],
                         '<converted-opendrive/>')
        self.world.apply_settings.assert_called_once()

    def test_empty_file_content_is_still_forwarded(self):
        """File validation does not add an OpenDRIVE content/schema validator."""
        with tempfile.TemporaryDirectory(prefix='carla-map-input-') as directory:
            path = Path(directory) / 'empty.xodr'
            path.write_text('', encoding='utf-8')
            self.run_cli(['--xodr-path', str(path)])
        self.assertEqual(self.client.generate_opendrive_world.call_args.args[0], '')

    def test_named_map_keeps_precedence_over_invalid_file_options(self):
        """Unused lower-priority file options must not be opened or validated."""
        self.run_cli(['--map', 'Town01', '--reload-map', '--xodr-path', '', '--osm-path', ''])
        self.client.load_world.assert_called_once_with('Town01')
        self.client.reload_world.assert_not_called()
        self.client.generate_opendrive_world.assert_not_called()

    def test_reload_keeps_precedence_over_invalid_file_options(self):
        """Reload still wins over both file options."""
        self.run_cli(['--reload-map', '--xodr-path', '', '--osm-path', ''])
        self.client.reload_world.assert_called_once()
        self.client.generate_opendrive_world.assert_not_called()

    def test_xodr_keeps_precedence_over_invalid_osm_option(self):
        """Selected OpenDRIVE input does not validate the unused OSM path."""
        with tempfile.TemporaryDirectory(prefix='carla-map-input-') as directory:
            path = Path(directory) / 'map.xodr'
            path.write_text('<OpenDRIVE/>', encoding='utf-8')
            self.run_cli(['--xodr-path', str(path), '--osm-path', ''])
        self.client.generate_opendrive_world.assert_called_once()
        self.convert.assert_not_called()


if __name__ == '__main__':
    unittest.main()
