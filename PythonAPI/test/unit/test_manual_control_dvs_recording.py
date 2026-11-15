# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Exercise DVS preview recording through the real Pygame PNG encoder."""

import importlib.util
import os
from pathlib import Path
import tempfile
import types
import unittest
from unittest import mock
import weakref

try:
    import numpy as np
    import pygame
except ImportError:
    pygame = None


@unittest.skipUnless(pygame is not None, 'Pygame and NumPy are required')
class TestDVSPreviewRecording(unittest.TestCase):
    """Check recorded pixels, callback races, warmup and native writer dispatch."""

    @classmethod
    def setUpClass(cls):
        """Import the complete production example with its usual dependencies."""
        path = Path(__file__).resolve().parents[2] / 'examples' / 'manual_control.py'
        spec = importlib.util.spec_from_file_location('_manual_control_dvs_recording', path)
        cls.module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(cls.module)

    def setUp(self):
        """Own output files and initialize the callback after camera warmup."""
        # unittest owns the directory across setup, the test and cleanup.
        # pylint: disable-next=consider-using-with
        temporary = tempfile.TemporaryDirectory(prefix='carla-dvs-preview-')
        self.addCleanup(temporary.cleanup)
        previous = os.getcwd()
        os.chdir(temporary.name)
        self.addCleanup(os.chdir, previous)
        self.directory = Path(temporary.name)
        self.manager = self.module.CameraManager.__new__(self.module.CameraManager)
        self.manager.sensors = [['sensor.camera.dvs', 0]]
        self.manager.index = 0
        self.manager.recording = True
        self.manager.surface = None
        self.manager._warmup_until = 0.0  # pylint: disable=protected-access
        dtype = [('x', np.uint16), ('y', np.uint16), ('t', np.int64), ('pol', np.bool_)]
        rows = [(3, 2, 1250000000, True), (1, 0, 1250000500, False)]
        self.data = types.SimpleNamespace(
            width=4, height=3, frame=37, timestamp=1.25,
            raw_data=np.array(rows, dtype=dtype).tobytes())

    def parse(self):
        """Execute the production callback with a payload interface fixture."""
        self.module.CameraManager._parse_image(  # pylint: disable=protected-access
            weakref.ref(self.manager), self.data)

    def test_recorded_png_contains_the_frame_polarities(self):
        """Create the output directory and save the expected frame pixels."""
        self.parse()
        filename = self.directory / '_out/00000037.png'
        self.assertEqual(filename.read_bytes()[:8], b'\x89PNG\r\n\x1a\n')
        pixels = pygame.surfarray.array3d(pygame.image.load(str(filename)))
        expected = np.zeros((4, 3, 3), dtype=np.uint8)
        expected[3, 2, 2] = 255
        expected[1, 0, 0] = 255
        np.testing.assert_array_equal(pixels, expected)
        events = np.load(self.directory / '_out/00000037.npy', allow_pickle=False)
        self.assertEqual(events.tolist(), [(3, 2, 1250000000, True), (1, 0, 1250000500, False)])
        self.assertEqual(events.dtype.names, ('x', 'y', 't', 'pol'))

    def test_repeated_pixel_events_are_saved_without_loss(self):
        """Retain timestamps and polarities when events share a pixel."""
        dtype = [('x', np.uint16), ('y', np.uint16), ('t', np.int64), ('pol', np.bool_)]
        rows = [(3, 2, 100, True), (3, 2, 101, False), (3, 2, 102, True)]
        self.data.raw_data = np.array(rows, dtype=dtype).tobytes()
        self.parse()
        events = np.load('_out/00000037.npy', allow_pickle=False)
        self.assertEqual(events.tolist(), rows)
        self.assertEqual(events.dtype, np.dtype(dtype))

    def test_empty_event_arrays_keep_the_structured_schema(self):
        """Record an empty frame with its original field types."""
        self.data.raw_data = b''
        self.parse()
        events = np.load('_out/00000037.npy', allow_pickle=False)
        self.assertEqual(events.size, 0)
        self.assertEqual(events.dtype, np.dtype([
            ('x', np.uint16), ('y', np.uint16), ('t', np.int64), ('pol', np.bool_)]))

    def test_disabled_recording_and_camera_warmup_do_not_write_files(self):
        """Preserve both recording guards."""
        self.manager.recording = False
        self.parse()
        self.assertEqual(self.manager.surface.get_at((3, 2))[:3], (0, 0, 255))
        self.manager.recording = True
        self.manager._warmup_until = None  # pylint: disable=protected-access
        self.manager.surface = None
        self.parse()
        self.assertIsNone(self.manager.surface)
        self.assertFalse((self.directory / '_out').exists())

    def test_directory_io_cannot_replace_the_recorded_frame(self):
        """Keep this frame's pixels when another callback updates the preview."""
        create = os.makedirs

        def replace_preview(path, exist_ok=False):
            create(path, exist_ok=exist_ok)
            self.manager.surface = pygame.Surface((4, 3))
            self.manager.index = 1

        with mock.patch.object(self.module.os, 'makedirs', side_effect=replace_preview):
            self.parse()
        saved = pygame.image.load('_out/00000037.png')
        self.assertEqual(saved.get_at((3, 2))[:3], (0, 0, 255))
        self.assertEqual(self.manager.surface.get_at((3, 2))[:3], (0, 0, 0))

    def test_rgb_recording_keeps_the_native_writer_dispatch(self):
        """Retain save_to_disk for ordinary camera images."""
        self.manager.sensors = [['sensor.camera.rgb', 0]]
        self.data.raw_data = bytes(4 * 3 * 4)
        self.data.convert = mock.Mock()
        self.data.save_to_disk = mock.Mock()
        self.parse()
        self.data.convert.assert_called_once_with(0)
        self.data.save_to_disk.assert_called_once_with('_out/00000037')
        self.assertFalse((self.directory / '_out').exists())


if __name__ == '__main__':
    unittest.main()
