import unittest

import numpy as np

from alpamayo_coordinates import carla_rotation_matrix
from alpamayo_rig import (
    BODY_CLEARANCE_CAMERA_INDICES,
    CAMERA_RING,
    CARLA_THIRD_PERSON_CAMERA,
    body_clearance_z,
)


class RigCalibrationTest(unittest.TestCase):
    def test_front_wide_full_extrinsic_matches_official_rotation(self):
        spec = CAMERA_RING[1]
        x, y, z, pitch, yaw, roll = spec.carla_pose(-1.3535001278)
        self.assertAlmostEqual(x, 0.3434038722, places=6)
        self.assertAlmostEqual(y, 0.010188, places=6)
        self.assertAlmostEqual(z, 1.435701, places=6)
        expected = np.array(
            [
                [0.9999336303, -0.0085594911, 0.0077116868],
                [0.0085383114, 0.9999596970, 0.0027751869],
                [-0.0077351302, -0.0027091579, 0.9999664135],
            ]
        )
        np.testing.assert_allclose(
            carla_rotation_matrix(pitch, yaw, roll), expected, atol=1e-6
        )

    def test_model_cameras_have_fitted_fisheye_intrinsics(self):
        for spec in CAMERA_RING:
            self.assertIsNotNone(spec.focal_length_px)
            self.assertEqual(len(spec.kannala_brandt), 4)
            self.assertIsNotNone(spec.principal_point_px)
        self.assertIsNone(CARLA_THIRD_PERSON_CAMERA.kannala_brandt)
        self.assertIsNone(CARLA_THIRD_PERSON_CAMERA.principal_point_px)

    def test_forward_center_cameras_clear_mkz_roof(self):
        self.assertEqual(BODY_CLEARANCE_CAMERA_INDICES, (1, 6))
        self.assertAlmostEqual(body_clearance_z(1.44, 0.763, 0.762, 0.08), 1.605)
        self.assertAlmostEqual(body_clearance_z(1.80, 0.763, 0.762, 0.08), 1.80)


if __name__ == "__main__":
    unittest.main()
