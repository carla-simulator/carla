import unittest

import numpy as np

from alpamayo_rig import (
    CAMERA_INDICES,
    CAMERA_NAMES,
    DASHBOARD_CAMERA_INDICES,
    DISPLAY_CAMERA_INDICES,
    TRAJECTORY_CAMERA_INDICES,
)
from alpamayo_schema import validate_request


def valid_request():
    metadata = {"camera_names": list(CAMERA_NAMES)}
    arrays = {
        "image_frames": np.zeros((7, 4, 3, 8, 12), dtype=np.uint8),
        "camera_indices": np.asarray(CAMERA_INDICES, dtype=np.int64),
        "ego_history_xyz": np.zeros((1, 1, 16, 3), dtype=np.float32),
        "ego_history_rot": np.zeros((1, 1, 16, 3, 3), dtype=np.float32),
        "relative_timestamps": np.zeros((7, 4), dtype=np.float32),
        "absolute_timestamps": np.zeros((7, 4), dtype=np.int64),
        "ego_t0": np.zeros(1, dtype=np.int64),
        "ego_t0_relative": np.zeros(1, dtype=np.float32),
        "ego_t0_frame_idx": np.full(1, 3, dtype=np.int64),
    }
    return metadata, arrays


class SchemaTest(unittest.TestCase):
    def test_canonical_request(self):
        metadata, arrays = valid_request()
        validate_request(metadata, arrays)
        self.assertEqual(TRAJECTORY_CAMERA_INDICES, (0, 1, 2, 3, 5, 6))

    def test_dashboard_adds_one_display_only_camera(self):
        self.assertEqual(CAMERA_INDICES, tuple(range(7)))
        self.assertEqual(DISPLAY_CAMERA_INDICES, tuple(range(8)))
        self.assertEqual(DASHBOARD_CAMERA_INDICES, (0, 1, 2, 6, 3, 4, 5, 7))

    def test_rejects_wrong_dtype(self):
        metadata, arrays = valid_request()
        arrays["image_frames"] = arrays["image_frames"].astype(np.float32)
        with self.assertRaisesRegex(ValueError, "image_frames must have dtype"):
            validate_request(metadata, arrays)

    def test_rejects_wrong_camera_order(self):
        metadata, arrays = valid_request()
        arrays["camera_indices"] = arrays["camera_indices"][::-1].copy()
        with self.assertRaisesRegex(ValueError, "camera_indices"):
            validate_request(metadata, arrays)


if __name__ == "__main__":
    unittest.main()
