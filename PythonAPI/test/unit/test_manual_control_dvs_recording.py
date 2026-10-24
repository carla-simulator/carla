# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Exercise the actual manual_control callback without pygame or native CARLA.

Only native data and the display/file-writing boundary are replaced. Imports are
isolated so discovery alongside tests using real CARLA/pygame is unaffected.
"""

import importlib.util
from pathlib import Path
import sys
import types
import unittest
from unittest import mock
import weakref

import numpy as np


class Surface:
    def __init__(self, pixels):
        self.pixels = pixels.copy()


def load_manual_control():
    carla = types.ModuleType('carla')
    carla.ColorConverter = types.SimpleNamespace(Raw=0)
    pygame = types.ModuleType('pygame')
    pygame.surfarray = types.SimpleNamespace(make_surface=Surface)
    pygame.image = types.SimpleNamespace(save=mock.Mock())
    constants = types.ModuleType('pygame.locals')

    def get_constant(name):
        if name.startswith(('K_', 'KMOD_')):
            return 0
        raise AttributeError(name)

    constants.__getattr__ = get_constant
    path = Path(__file__).resolve().parents[2] / 'examples' / 'manual_control.py'
    spec = importlib.util.spec_from_file_location('_manual_control_dvs_test', path)
    module = importlib.util.module_from_spec(spec)
    with mock.patch.dict(sys.modules, {
            'carla': carla, 'pygame': pygame, 'pygame.locals': constants}):
        spec.loader.exec_module(module)
    return module


class DVSEventArray:
    """The bound DVS data interface has raw_data/frame but no save_to_disk."""

    def __init__(self, events=(), frame=37):
        dtype = [('x', np.uint16), ('y', np.uint16), ('t', np.int64), ('pol', np.bool_)]
        self.raw_data = np.array(list(events), dtype=dtype).tobytes()
        self.width = 4
        self.height = 3
        self.frame = frame


class NativeImage:
    def __init__(self):
        self.width = 4
        self.height = 3
        self.frame = 38
        self.raw_data = np.arange(4 * 3 * 4, dtype=np.uint8).tobytes()
        self.convert = mock.Mock()
        self.save_to_disk = mock.Mock()


class TestManualControlDVSRecording(unittest.TestCase):
    def setUp(self):
        self.module = load_manual_control()
        self.manager = self.module.CameraManager.__new__(self.module.CameraManager)
        self.manager.sensors = [['sensor.camera.dvs', 0]]
        self.manager.index = 0
        self.manager.recording = True
        self.manager.surface = None
        self.created = False
        self.mkdir_calls = []
        self.saved = []

        def mkdir(path, exist_ok=False):
            self.assertEqual((path, exist_ok), ('_out', True))
            self.created = True
            self.mkdir_calls.append(path)

        def save(surface, path):
            # Enforce directory creation before the actual write boundary.
            self.assertTrue(self.created)
            self.saved.append((surface, path))

        self.module.os = types.SimpleNamespace(makedirs=mkdir)
        self.module.pygame.image.save.side_effect = save

    def parse(self, image):
        self.module.CameraManager._parse_image(weakref.ref(self.manager), image)

    def test_dvs_recording_saves_rendered_polarity_image_as_png(self):
        data = DVSEventArray([(3, 2, 100, True), (1, 0, 101, False)])
        self.parse(data)

        self.assertEqual(self.mkdir_calls, ['_out'])
        self.assertEqual(len(self.saved), 1)
        surface, path = self.saved[0]
        self.assertIs(surface, self.manager.surface)
        self.assertEqual(path, '_out/00000037.png')
        expected = np.zeros((4, 3, 3), dtype=np.uint8)
        expected[3, 2, 2] = 255
        expected[1, 0, 0] = 255
        np.testing.assert_array_equal(surface.pixels, expected)

    def test_empty_dvs_frame_records_black_image(self):
        self.parse(DVSEventArray(frame=0))

        self.assertEqual(self.saved[0][1], '_out/00000000.png')
        np.testing.assert_array_equal(self.saved[0][0].pixels, np.zeros((4, 3, 3), dtype=np.uint8))

    def test_multiple_dvs_frames_reuse_directory_and_keep_frame_names(self):
        self.parse(DVSEventArray(frame=5))
        self.parse(DVSEventArray(frame=9))

        self.assertEqual(self.mkdir_calls, ['_out', '_out'])
        self.assertEqual([path for _, path in self.saved], ['_out/00000005.png', '_out/00000009.png'])

    def test_dvs_preview_does_not_create_or_record_files(self):
        self.manager.recording = False
        self.parse(DVSEventArray([(3, 2, 100, True)]))

        self.assertEqual(self.manager.surface.pixels[3, 2, 2], 255)
        self.assertFalse(self.mkdir_calls)
        self.assertFalse(self.saved)

    def test_rgb_recording_keeps_native_image_writer(self):
        self.manager.sensors = [['sensor.camera.rgb', 0]]
        image = NativeImage()
        self.parse(image)

        image.convert.assert_called_once_with(0)
        image.save_to_disk.assert_called_once_with('_out/00000038')
        self.assertFalse(self.mkdir_calls)
        self.assertFalse(self.saved)

    def test_lidar_recording_keeps_native_point_cloud_writer(self):
        self.manager.sensors = [['sensor.lidar.ray_cast', 0]]
        self.manager.hud = types.SimpleNamespace(dim=(4, 3))
        self.manager.lidar_range = 50
        data = types.SimpleNamespace(raw_data=np.array([[0, 0, 0, 1]], dtype=np.float32).tobytes(),
                                     frame=39, save_to_disk=mock.Mock())
        self.parse(data)

        data.save_to_disk.assert_called_once_with('_out/00000039')
        self.assertFalse(self.mkdir_calls)
        self.assertFalse(self.saved)

    def test_optical_flow_recording_keeps_color_coded_native_writer(self):
        self.manager.sensors = [['sensor.camera.optical_flow', 0]]
        colored = NativeImage()
        data = types.SimpleNamespace(get_color_coded_flow=mock.Mock(return_value=colored))
        self.parse(data)

        data.get_color_coded_flow.assert_called_once_with()
        colored.save_to_disk.assert_called_once_with('_out/00000038')
        self.assertFalse(self.mkdir_calls)
        self.assertFalse(self.saved)

    def test_expired_manager_callback_does_nothing(self):
        self.module.CameraManager._parse_image(lambda: None, DVSEventArray())

        self.assertIsNone(self.manager.surface)
        self.assertFalse(self.mkdir_calls)
        self.assertFalse(self.saved)

    def test_import_stubs_do_not_escape_module_loading(self):
        before = {name: sys.modules.get(name) for name in ('carla', 'pygame', 'pygame.locals')}
        load_manual_control()
        self.assertEqual(before, {name: sys.modules.get(name) for name in before})


if __name__ == '__main__':
    unittest.main()
