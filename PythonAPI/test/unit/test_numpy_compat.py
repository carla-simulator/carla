# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Tests that validate the NumPy 2 compatibility refactoring.

The refactoring replaces two deprecated patterns across CARLA example
scripts so the codebase compiles and runs on both NumPy 1.x and NumPy 2.x:

  - ``np.bool``   -> ``np.bool_``  (``np.bool`` was removed in NumPy 1.24)
  - ``np.matrix`` -> ``np.array``  (``np.matrix`` is deprecated)

These tests act as a regression guard (by scanning the refactored files
for the deprecated patterns) AND as behavioural tests (by exercising the
replacement patterns exactly the way the production code uses them).
"""

import os
import re
import unittest

import numpy as np


REPO_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), '..', '..', '..'))

# Files modified by the NumPy 2 refactoring. Each must stay free of the
# deprecated alias listed alongside it.
DVS_BOOL_FILES = (
    'PythonAPI/examples/V2XDemo.py',
    'PythonAPI/examples/manual_control_chrono.py',
    'PythonAPI/examples/manual_control_carsim.py',
)
MATRIX_FILES = (
    'PythonAPI/examples/client_bounding_boxes.py',
)

# Matches "np.bool" NOT followed by an underscore or alphanumeric char.
# Catches "np.bool)" and "np.bool," but not "np.bool_".
NP_BOOL_RE = re.compile(r'\bnp\.bool(?![_\w])')
# Matches "np.matrix" as a constructor call. The class itself (np.matrix)
# as a type reference is also deprecated and should be flagged.
NP_MATRIX_RE = re.compile(r'\bnp\.matrix\b')


def _read(relpath):
    with open(os.path.join(REPO_ROOT, relpath), 'r', encoding='utf-8') as fh:
        return fh.read()


class TestNumpy2RegressionGuard(unittest.TestCase):
    """Static checks: the deprecated patterns must not reappear in the
    files that the NumPy 2 refactoring touched."""

    def test_dvs_files_have_no_bare_np_bool(self):
        for relpath in DVS_BOOL_FILES:
            with self.subTest(file=relpath):
                source = _read(relpath)
                matches = NP_BOOL_RE.findall(source)
                self.assertEqual(
                    matches, [],
                    f"Deprecated 'np.bool' found in {relpath}; "
                    f"use 'np.bool_' instead (removed in NumPy 1.24).")

    def test_bounding_box_file_has_no_np_matrix(self):
        for relpath in MATRIX_FILES:
            with self.subTest(file=relpath):
                source = _read(relpath)
                matches = NP_MATRIX_RE.findall(source)
                self.assertEqual(
                    matches, [],
                    f"Deprecated 'np.matrix' found in {relpath}; "
                    f"use 'np.array' with '@' or 'np.dot' instead.")

    def test_dvs_files_use_np_bool_underscore(self):
        """Positive check: the DVS dtype must use np.bool_."""
        for relpath in DVS_BOOL_FILES:
            with self.subTest(file=relpath):
                source = _read(relpath)
                self.assertIn(
                    "('pol', np.bool_)", source,
                    f"{relpath} should declare the DVS 'pol' field as "
                    f"np.bool_.")


class TestDvsEventPipeline(unittest.TestCase):
    """Behavioural check: reproduce the exact DVS processing pipeline
    used by the example scripts (V2XDemo.py, manual_control_*), end to
    end, and verify it works on the installed NumPy."""

    DVS_DTYPE = np.dtype([
        ('x', np.uint16), ('y', np.uint16),
        ('t', np.int64), ('pol', np.bool_)])

    def _build_raw_buffer(self):
        events = np.array(
            [(2, 1, 100, True),
             (0, 3, 200, False),
             (4, 2, 300, True)],
            dtype=self.DVS_DTYPE)
        return events.tobytes(), events

    def test_dtype_field_layout(self):
        """DVS dtype must match the production layout exactly."""
        self.assertEqual(self.DVS_DTYPE.names, ('x', 'y', 't', 'pol'))
        self.assertEqual(self.DVS_DTYPE['pol'], np.dtype(np.bool_))

    def test_frombuffer_roundtrip(self):
        """np.frombuffer on DVS raw bytes must recover the original events."""
        raw, original = self._build_raw_buffer()
        recovered = np.frombuffer(raw, dtype=self.DVS_DTYPE)
        np.testing.assert_array_equal(original, recovered)

    def test_image_indexing_pattern(self):
        """The production pipeline indexes into an image via:
            dvs_img[dvs_events[:]['y'],
                    dvs_events[:]['x'],
                    dvs_events[:]['pol'] * 2] = 255
        The bool->int coercion (pol * 2) must produce valid channel indices."""
        raw, _ = self._build_raw_buffer()
        dvs_events = np.frombuffer(raw, dtype=self.DVS_DTYPE)
        dvs_img = np.zeros((4, 5, 3), dtype=np.uint8)
        dvs_img[dvs_events[:]['y'],
                dvs_events[:]['x'],
                dvs_events[:]['pol'] * 2] = 255
        # Event 0: (x=2, y=1, pol=True  -> channel 2) => (1, 2, 2) = 255
        # Event 1: (x=0, y=3, pol=False -> channel 0) => (3, 0, 0) = 255
        # Event 2: (x=4, y=2, pol=True  -> channel 2) => (2, 4, 2) = 255
        self.assertEqual(dvs_img[1, 2, 2], 255)
        self.assertEqual(dvs_img[3, 0, 0], 255)
        self.assertEqual(dvs_img[2, 4, 2], 255)
        # Bool channel must map to indices 0 or 2 only, never 1.
        self.assertEqual(dvs_img[:, :, 1].sum(), 0)


class TestBoundingBoxMatrix(unittest.TestCase):
    """Behavioural check: replicate the get_matrix() pattern from
    PythonAPI/examples/client_bounding_boxes.py so np.array can be
    exercised the same way np.matrix was."""

    @staticmethod
    def _get_matrix(location, yaw_deg, pitch_deg, roll_deg):
        """Mirror of ClientSideBoundingBoxes.get_matrix() after the
        np.matrix -> np.array refactoring."""
        c_y = np.cos(np.radians(yaw_deg))
        s_y = np.sin(np.radians(yaw_deg))
        c_r = np.cos(np.radians(roll_deg))
        s_r = np.sin(np.radians(roll_deg))
        c_p = np.cos(np.radians(pitch_deg))
        s_p = np.sin(np.radians(pitch_deg))
        matrix = np.array(np.identity(4))
        matrix[0, 3] = location[0]
        matrix[1, 3] = location[1]
        matrix[2, 3] = location[2]
        matrix[0, 0] = c_p * c_y
        matrix[0, 1] = c_y * s_p * s_r - s_y * c_r
        matrix[0, 2] = -c_y * s_p * c_r - s_y * s_r
        matrix[1, 0] = s_y * c_p
        matrix[1, 1] = s_y * s_p * s_r + c_y * c_r
        matrix[1, 2] = -s_y * s_p * c_r + c_y * s_r
        matrix[2, 0] = s_p
        matrix[2, 1] = -c_p * s_r
        matrix[2, 2] = c_p * c_r
        return matrix

    def test_identity_transform_is_ndarray(self):
        """The refactored get_matrix must return a plain ndarray,
        never a deprecated np.matrix."""
        m = self._get_matrix((0.0, 0.0, 0.0), 0.0, 0.0, 0.0)
        self.assertIsInstance(m, np.ndarray)
        self.assertNotIsInstance(m, np.matrix)
        self.assertEqual(m.shape, (4, 4))
        np.testing.assert_array_equal(m, np.eye(4))

    def test_translation_only(self):
        m = self._get_matrix((5.0, 10.0, -3.0), 0.0, 0.0, 0.0)
        self.assertAlmostEqual(m[0, 3], 5.0)
        self.assertAlmostEqual(m[1, 3], 10.0)
        self.assertAlmostEqual(m[2, 3], -3.0)

    def test_matrix_composition_via_np_dot(self):
        """The production code composes transforms with np.dot(). That
        must behave the same for np.array as it did for np.matrix."""
        translation = self._get_matrix((5.0, 0.0, 0.0), 0.0, 0.0, 0.0)
        rotation = self._get_matrix((0.0, 0.0, 0.0), 90.0, 0.0, 0.0)
        composed = np.dot(translation, rotation)
        self.assertIsInstance(composed, np.ndarray)
        self.assertEqual(composed.shape, (4, 4))
        self.assertAlmostEqual(composed[0, 3], 5.0)

    def test_world_point_transform(self):
        """Transform a point through the matrix, mirroring the
        np.dot(bb_world_matrix, np.transpose(cords)) pattern."""
        m = self._get_matrix((1.0, 2.0, 3.0), 0.0, 0.0, 0.0)
        point = np.array([0.0, 0.0, 0.0, 1.0])
        world = np.dot(m, np.transpose(point))
        self.assertAlmostEqual(world[0], 1.0)
        self.assertAlmostEqual(world[1], 2.0)
        self.assertAlmostEqual(world[2], 3.0)


class TestNumpyEnvironment(unittest.TestCase):
    """Sanity check that identifies which NumPy ABI we are running
    against so test failures are interpretable in CI logs."""

    def test_numpy_major_version_is_supported(self):
        major = int(np.__version__.split('.')[0])
        self.assertIn(major, (1, 2),
                      f"Unsupported NumPy major version: {np.__version__}")

    def test_bool_underscore_is_available(self):
        """np.bool_ must exist on every NumPy version CARLA supports."""
        self.assertTrue(hasattr(np, 'bool_'))


# ==============================================================================
# -- TestCarlaCExtensionLoad ---------------------------------------------------
# ==============================================================================


class TestCarlaCExtensionLoad(unittest.TestCase):
    """
    Validates that the Boost.Python-backed carla module imports and exposes
    its core types.  On NumPy 1.x this guards against regression; on NumPy 2.x
    this is the end-to-end proof that the Boost.Python dtype.cpp patch worked.

    These tests are skipped (not failed) when the carla wheel is not installed,
    so the test suite can run in a plain Python environment without a full CARLA
    build.
    """

    @classmethod
    def setUpClass(cls):
        try:
            import carla  # noqa: F401
            cls.carla = carla
            cls.skip_reason = None
        except ImportError as exc:
            cls.carla = None
            cls.skip_reason = str(exc)

    def _skip_if_no_carla(self):
        if self.carla is None:
            self.skipTest(f"carla wheel not installed: {self.skip_reason}")

    def test_import_carla(self):
        self._skip_if_no_carla()
        self.assertTrue(hasattr(self.carla, 'Client'))

    def test_sensor_data_types_exist(self):
        self._skip_if_no_carla()
        for attr in ('Image', 'LidarMeasurement', 'RadarMeasurement',
                     'DVSEventArray', 'GnssMeasurement'):
            self.assertTrue(
                hasattr(self.carla, attr),
                f"carla.{attr} is missing, Boost.Python C extension load failed?",
            )

    def test_numpy_version_logged(self):
        """Print the NumPy ABI so CI logs identify which path was exercised."""
        self._skip_if_no_carla()
        abi = ('NumPy 2.x (NEP 52, opaque PyArray_Descr)'
               if np.__version__.startswith('2.')
               else 'NumPy 1.x (direct PyArray_Descr access)')
        print(f"\n[test_numpy_compat] carla C extension OK with NumPy "
              f"{np.__version__}, {abi}")
