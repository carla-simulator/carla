# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Exercise actual LiDAR callbacks with real NumPy and isolated display stubs."""

import importlib.util
from pathlib import Path
import sys
import types
import unittest
from unittest import mock
import warnings
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
    constants = types.ModuleType('pygame.locals')

    def get_constant(name):
        if name.startswith(('K_', 'KMOD_')):
            return 0
        raise AttributeError(name)

    constants.__getattr__ = get_constant
    path = Path(__file__).resolve().parents[2] / 'examples/manual_control.py'
    spec = importlib.util.spec_from_file_location('_manual_control_lidar_test', path)
    module = importlib.util.module_from_spec(spec)
    with mock.patch.dict(sys.modules, {
            'carla': carla, 'pygame': pygame, 'pygame.locals': constants}):
        spec.loader.exec_module(module)
    return module


class LidarData:
    def __init__(self, points, semantic=False):
        self.frame = 42
        self.save_to_disk = mock.Mock()
        self.accessed = []
        self.tags = [point[2] for point in points] if semantic else []
        if semantic:
            dtype = [('x', 'f4'), ('y', 'f4'), ('z', 'f4'),
                     ('cos_angle', 'f4'), ('object_idx', 'u4'), ('object_tag', 'u4')]
            rows = [(x, y, 0, 1, 0, tag) for x, y, tag in points]
            self.raw_data = np.array(rows, dtype=dtype).tobytes()
        else:
            self.raw_data = np.array([(x, y, 0, 1) for x, y in points],
                                     dtype=np.float32).tobytes()

    def __len__(self):
        return len(self.tags)

    def __getitem__(self, index):
        self.accessed.append(index)
        return types.SimpleNamespace(object_tag=self.tags[index])


class TestManualControlLidarPreview(unittest.TestCase):
    def setUp(self):
        self.module = load_manual_control()
        self.manager = self.module.CameraManager.__new__(self.module.CameraManager)
        self.manager.index = 0
        self.manager.recording = False
        self.manager.surface = None
        self.manager.lidar_range = 50
        self.manager.hud = types.SimpleNamespace(dim=(4, 4))

    def parse(self, points, semantic=False, dim=(4, 4)):
        sensor = 'sensor.lidar.ray_cast_semantic' if semantic else 'sensor.lidar.ray_cast'
        self.manager.sensors = [[sensor, 0]]
        self.manager.hud.dim = dim
        data = LidarData(points, semantic)
        self.module.CameraManager._parse_image(weakref.ref(self.manager), data)
        return data, self.manager.surface.pixels

    def test_regular_positive_range_boundaries_do_not_index_past_image(self):
        _, pixels = self.parse([(0, 50), (50, 0), (0, 0)])
        expected = np.zeros((4, 4, 3), dtype=np.uint8)
        expected[2, 2] = 255
        np.testing.assert_array_equal(pixels, expected)

    def test_semantic_positive_range_boundaries_do_not_index_past_image(self):
        data, pixels = self.parse([(0, 50, 1), (50, 0, 2), (0, 0, 10)], True)
        expected = np.zeros((4, 4, 3), dtype=np.uint8)
        expected[2, 2] = self.module.OBJECT_TO_COLOR[10]
        np.testing.assert_array_equal(pixels, expected)
        self.assertEqual(data.accessed, [2])

    def test_regular_outside_points_are_not_reflected_or_truncated_into_image(self):
        _, pixels = self.parse([(-55, 0), (0, -55), (100, 0), (0, 100), (0, 0)])
        expected = np.zeros((4, 4, 3), dtype=np.uint8)
        expected[2, 2] = 255
        np.testing.assert_array_equal(pixels, expected)

    def test_semantic_filter_keeps_original_point_color_indices(self):
        data, pixels = self.parse([(-55, 0, 5), (0, 0, 10), (0, 55, 7)], True)
        expected = np.zeros((4, 4, 3), dtype=np.uint8)
        expected[2, 2] = self.module.OBJECT_TO_COLOR[10]
        np.testing.assert_array_equal(pixels, expected)
        self.assertEqual(data.accessed, [1])

    def test_regular_interior_projection_and_non_square_orientation_unchanged(self):
        _, pixels = self.parse([(25, 0), (-25, 25), (-50, 0), (0, -50)], dim=(8, 4))
        expected = np.zeros((8, 4, 3), dtype=np.uint8)
        for point in [(5, 2), (3, 3), (2, 2), (4, 0)]:
            expected[point] = 255
        np.testing.assert_array_equal(pixels, expected)

    def test_semantic_interior_projection_and_colors_unchanged(self):
        _, pixels = self.parse([(25, 0, 10), (-25, 25, 4), (0, -50, 1)], True, (8, 4))
        expected = np.zeros((8, 4, 3), dtype=np.uint8)
        for point, tag in [((5, 2), 10), ((3, 3), 4), ((4, 0), 1)]:
            expected[point] = self.module.OBJECT_TO_COLOR[tag]
        np.testing.assert_array_equal(pixels, expected)

    def test_empty_frames_still_render_black(self):
        for semantic in (False, True):
            with self.subTest(semantic=semantic):
                _, pixels = self.parse([], semantic)
                np.testing.assert_array_equal(pixels, np.zeros((4, 4, 3), dtype=np.uint8))

    def test_preview_filter_does_not_modify_recorded_point_cloud(self):
        self.manager.recording = True
        for semantic in (False, True):
            with self.subTest(semantic=semantic):
                points = [(0, 50, 5), (0, 0, 10)] if semantic else [(0, 50), (0, 0)]
                expected_raw = LidarData(points, semantic).raw_data
                data, _ = self.parse(points, semantic)
                self.assertEqual(data.raw_data, expected_raw)
                data.save_to_disk.assert_called_once_with('_out/00000042')

    def test_nonfinite_points_do_not_reach_integer_conversion(self):
        for semantic in (False, True):
            with self.subTest(semantic=semantic), warnings.catch_warnings():
                warnings.simplefilter('error', RuntimeWarning)
                points = [(float('nan'), 0), (0, float('inf')), (0, 0)]
                if semantic:
                    points = [(x, y, 10) for x, y in points]
                _, pixels = self.parse(points, semantic)
                expected = np.zeros((4, 4, 3), dtype=np.uint8)
                expected[2, 2] = self.module.OBJECT_TO_COLOR[10] if semantic else 255
                np.testing.assert_array_equal(pixels, expected)

    def test_import_stubs_are_restored(self):
        before = {name: sys.modules.get(name) for name in ('carla', 'pygame', 'pygame.locals')}
        load_manual_control()
        self.assertEqual(before, {name: sys.modules.get(name) for name in before})


if __name__ == '__main__':
    unittest.main()
