import unittest

import numpy as np

from alpamayo_coordinates import carla_rotation_matrix
from alpamayo_rig import (
    BODY_CLEARANCE_CAMERA_INDICES,
    CAMERA_RING,
    CARLA_THIRD_PERSON_CAMERA,
    body_clearance_z,
)


OFFICIAL_FRONT_WIDE_ROTATION = np.array(
    [
        [0.9999336303, -0.0085594911, 0.0077116868],
        [0.0085383114, 0.9999596970, 0.0027751869],
        [-0.0077351302, -0.0027091579, 0.9999664135],
    ]
)


class RigCalibrationTest(unittest.TestCase):
    def test_front_wide_translation_matches_official_calibration(self):
        spec = CAMERA_RING[1]
        x, y, z, _pitch, _yaw, _roll = spec.carla_pose(-1.3535001278)
        self.assertAlmostEqual(x, 0.3434038722, places=6)
        self.assertAlmostEqual(y, 0.010188, places=6)
        self.assertAlmostEqual(z, 1.435701, places=6)

    @unittest.skip(
        "Calibration provenance is unresolved after the 2026-08-28 geom fix. "
        "OFFICIAL_FRONT_WIDE_ROTATION below reproduces carla_rotation_matrix "
        "exactly (4.7e-9) only under the pre-#9751 MIRRORED pitch/roll "
        "convention, which LibCarla no longer uses. Either (a) the matrix is "
        "the true extrinsic and CameraSpec.carla_pose must return "
        "(+pitch_up_deg, -yaw_left_deg, -roll_carla_deg), which would re-aim "
        "the physical cameras by ~0.9 deg, or (b) carla_pose is right and the "
        "matrix was fitted through the buggy wheel. The stored yaw_left_deg "
        "is also the negative of the yaw this matrix carries, which no "
        "pitch/roll mirroring can explain -- so the field names cannot be "
        "trusted to settle it. Needs the upstream Alpamayo calibration."
    )
    def test_front_wide_full_extrinsic_matches_official_rotation(self):
        spec = CAMERA_RING[1]
        _x, _y, _z, pitch, yaw, roll = spec.carla_pose(-1.3535001278)
        np.testing.assert_allclose(
            carla_rotation_matrix(pitch, yaw, roll),
            OFFICIAL_FRONT_WIDE_ROTATION,
            atol=1e-6,
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
