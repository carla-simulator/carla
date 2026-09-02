import math
import unittest

import numpy as np

from alpamayo_coordinates import carla_rotation_matrix
from alpamayo_rig import (
    BODY_CLEARANCE_CAMERA_INDICES,
    CAMERA_RING,
    CARLA_THIRD_PERSON_CAMERA,
    body_clearance_z,
)


# Rotation block of ``carla.Transform.get_matrix()`` for the front-wide camera,
# from the public Alpamayo 2 validation calibration.  See
# ``RigCalibrationTest.test_front_wide_full_extrinsic_matches_official_rotation``
# for the provenance and the live measurements that pinned its frame down.
OFFICIAL_FRONT_WIDE_ROTATION = np.array(
    [
        [0.9999336303, -0.0085594911, 0.0077116868],
        [0.0085383114, 0.9999596970, 0.0027751869],
        [-0.0077351302, -0.0027091579, 0.9999664135],
    ]
)

# NVIDIA's own seven-camera RDS-HQ rig as carried by the Cosmos client
# (``carla_cosmos/rig.py::_NVIDIA_AV7``, the ``multiview_example1``
# calibration): FLU roll/pitch/yaw in degrees, read the DriveWorks way from
# ``nominalSensor2Rig_FLU`` with ``Rotation.from_euler("xyz", radians(rpy))``.
# Same rig, a different clip's calibration, so the poses agree only to a few
# tenths of a degree -- but every sign must match.
NVIDIA_AV7_FLU_RPY_DEG = {
    "camera_front_wide_120fov": (0.13, 0.47, -0.71),
    "camera_cross_left_120fov": (-0.17, 0.44, 66.41),
    "camera_cross_right_120fov": (-0.19, -1.38, -66.81),
    "camera_rear_left_70fov": (0.63, 3.26, 151.40),
    "camera_rear_right_70fov": (-0.28, 0.87, -154.69),
    "camera_rear_tele_30fov": (-0.25, 0.58, 179.19),
    "camera_front_tele_30fov": (-0.33, -0.72, -0.73),
}
# rear_right and front_tele also differ in translation between the two
# calibrations, so their angles drift further than the rest.
_AV7_ANGLE_TOLERANCE_DEG = {"camera_rear_right_70fov": 0.9, "camera_front_tele_30fov": 0.4}

Y_FLIP = np.diag([1.0, -1.0, 1.0])


def flu_rotation_matrix(roll_deg: float, pitch_deg: float, yaw_deg: float) -> np.ndarray:
    """Right-handed FLU rotation ``Rz(yaw) . Ry(pitch) . Rx(roll)``."""
    r, p, y = (math.radians(v) for v in (roll_deg, pitch_deg, yaw_deg))
    rz = np.array([[math.cos(y), -math.sin(y), 0.0], [math.sin(y), math.cos(y), 0.0], [0.0, 0.0, 1.0]])
    ry = np.array([[math.cos(p), 0.0, math.sin(p)], [0.0, 1.0, 0.0], [-math.sin(p), 0.0, math.cos(p)]])
    rx = np.array([[1.0, 0.0, 0.0], [0.0, math.cos(r), -math.sin(r)], [0.0, math.sin(r), math.cos(r)]])
    return rz @ ry @ rx


def flu_rpy_deg(matrix: np.ndarray) -> tuple[float, float, float]:
    """Inverse of :func:`flu_rotation_matrix`."""
    pitch = math.degrees(math.asin(max(-1.0, min(1.0, -matrix[2, 0]))))
    yaw = math.degrees(math.atan2(matrix[1, 0], matrix[0, 0]))
    roll = math.degrees(math.atan2(matrix[2, 1], matrix[2, 2]))
    return roll, pitch, yaw


class RigCalibrationTest(unittest.TestCase):
    def test_front_wide_translation_matches_official_calibration(self):
        spec = CAMERA_RING[1]
        x, y, z, _pitch, _yaw, _roll = spec.carla_pose(-1.3535001278)
        self.assertAlmostEqual(x, 0.3434038722, places=6)
        self.assertAlmostEqual(y, 0.010188, places=6)
        self.assertAlmostEqual(z, 1.435701, places=6)

    def test_front_wide_full_extrinsic_matches_official_rotation(self):
        """``carla_pose`` must reproduce the official extrinsic exactly.

        Provenance (settled 2026-08-28, after the LibCarla geom fix).
        ``OFFICIAL_FRONT_WIDE_ROTATION`` is a **CARLA** rotation matrix (engine
        convention, ``R = Rz(+yaw) . Ry(-pitch) . Rx(-roll)``).  Decoding it
        that way gives ``pitch=-0.443195, yaw=+0.489230, roll=+0.155228``;
        conjugating it into FLU (``S . R . S`` with ``S = diag(1, -1, 1)``)
        gives ``roll=+0.155228, pitch=+0.443195, yaw=-0.489230``, which is
        exactly the shape of an NVIDIA ``nominalSensor2Rig_FLU`` entry and
        matches the same rig's front-wide camera in the Cosmos client's
        ``_NVIDIA_AV7`` table -- ``(0.13, 0.47, -0.71)`` -- on all three signs.
        The genuine NuRec clip calibrations shipped in
        ``PhysicalAI-Autonomous-Vehicles-NuRec`` (``rig_trajectories.json``,
        ``T_sensor_rig``) have the same structure once the optical axes are
        rotated into FLU.

        Until 2026-08-28 this assertion held only under the pre-#9751
        *mirrored* pitch/roll convention, because ``carla_pose`` returned
        ``(-pitch_up_deg, -yaw_left_deg, +roll_right_up_deg)``: the two sign
        errors cancelled inside the buggy ``get_matrix`` but the cameras were
        physically mounted 2x pitch and 2x roll away from the calibration.

        Measured on the live Town10HD_Opt server (synchronous, one ticking
        client, 1920x1080 pinhole, 60 deg FOV, f = 1662.8 px, camera pose
        measured by fitting the ground plane in the depth AOV; the local road
        camber contributes a constant -0.42 deg pitch / +0.30 deg roll to every
        reading, so the numbers below are relative to a pitch=roll=0 reference
        camera at the same location):

        ==========================================  ==========  =========  ============
        mounting                                    pitch       roll       horizon row
        ==========================================  ==========  =========  ============
        old ``carla_pose`` (+0.443, -0.155)         +0.431      -0.123     540.45 px
        fixed ``carla_pose`` (-0.443, +0.155)       -0.448      +0.160     514.92 px
        Cosmos ``_NVIDIA_AV7`` front wide           -0.479      +0.145     514.02 px
        reference pitch=+2 / pitch=-2               +1.994 / -1.956        585.80 / 471.12 px
        reference roll=+2 / roll=-2                 +1.971 / -1.975
        ==========================================  ==========  =========  ============

        The fixed mounting lands 0.90 px (0.031 deg) from the independent
        NVIDIA rig; the old one is 26.4 px (0.91 deg) away and aims the
        front-wide camera *above* the horizon instead of the 0.44 deg down an
        RDS-HQ front camera is calibrated for.
        """
        spec = CAMERA_RING[1]
        _x, _y, _z, pitch, yaw, roll = spec.carla_pose(-1.3535001278)
        self.assertAlmostEqual(pitch, -0.443195, places=6)
        self.assertAlmostEqual(yaw, 0.489230, places=6)
        self.assertAlmostEqual(roll, 0.155228, places=6)
        np.testing.assert_allclose(
            carla_rotation_matrix(pitch, yaw, roll),
            OFFICIAL_FRONT_WIDE_ROTATION,
            atol=1e-6,
        )

    def test_official_rotation_is_an_nvidia_flu_sensor_to_rig_entry(self):
        """The FLU form of the official matrix matches NVIDIA's own AV7 rig."""
        flu = Y_FLIP @ OFFICIAL_FRONT_WIDE_ROTATION @ Y_FLIP
        roll, pitch, yaw = flu_rpy_deg(flu)
        self.assertAlmostEqual(roll, 0.155228, places=5)
        self.assertAlmostEqual(pitch, 0.443195, places=5)
        self.assertAlmostEqual(yaw, -0.489230, places=5)
        expected = NVIDIA_AV7_FLU_RPY_DEG["camera_front_wide_120fov"]
        np.testing.assert_allclose((roll, pitch, yaw), expected, atol=0.25)

    def test_camera_ring_matches_the_nvidia_av7_flu_rig(self):
        """Every stored angle is the NVIDIA FLU value with its named sign.

        ``pitch_up_deg = -pitch_flu``, ``yaw_left_deg = +yaw_flu``,
        ``roll_right_up_deg = -roll_flu``.
        """
        for spec in CAMERA_RING:
            with self.subTest(spec.name):
                roll_flu, pitch_flu, yaw_flu = NVIDIA_AV7_FLU_RPY_DEG[spec.name]
                atol = _AV7_ANGLE_TOLERANCE_DEG.get(spec.name, 0.5)
                self.assertAlmostEqual(spec.roll_right_up_deg, -roll_flu, delta=atol)
                self.assertAlmostEqual(spec.pitch_up_deg, -pitch_flu, delta=atol)
                self.assertAlmostEqual(spec.yaw_left_deg, yaw_flu, delta=8.0)

    def test_carla_pose_round_trips_through_the_flu_rig(self):
        """``carla_pose`` is exactly the FLU->CARLA mapping of the stored rig."""
        for spec in CAMERA_RING:
            with self.subTest(spec.name):
                _x, _y, _z, pitch, yaw, roll = spec.carla_pose(0.0)
                flu = flu_rotation_matrix(
                    -spec.roll_right_up_deg, -spec.pitch_up_deg, spec.yaw_left_deg
                )
                np.testing.assert_allclose(
                    carla_rotation_matrix(pitch, yaw, roll),
                    Y_FLIP @ flu @ Y_FLIP,
                    atol=1e-9,
                )

    def test_carla_pitch_sign_matches_the_nvidia_rig(self):
        """Each camera tilts the way NVIDIA's rig says, not the mirror of it.

        CARLA pitch is positive up and FLU pitch is positive down, so the CARLA
        pitch must carry the opposite sign to the AV7 FLU pitch.  Five of the
        seven cameras (front wide, cross left, both rears, rear tele) are
        calibrated nose-down; cross right and front tele are genuinely a little
        nose-up in both rigs.
        """
        for spec in CAMERA_RING:
            with self.subTest(spec.name):
                _x, _y, _z, pitch, _yaw, _roll = spec.carla_pose(0.0)
                _roll_flu, pitch_flu, _yaw_flu = NVIDIA_AV7_FLU_RPY_DEG[spec.name]
                self.assertEqual(math.copysign(1.0, pitch), math.copysign(1.0, -pitch_flu))
        front_wide_pitch = CAMERA_RING[1].carla_pose(0.0)[3]
        self.assertLess(front_wide_pitch, 0.0)  # measured live: 0.44 deg below level

    def test_chase_camera_looks_down_at_the_hero(self):
        _x, _y, _z, pitch, _yaw, _roll = CARLA_THIRD_PERSON_CAMERA.carla_pose(0.0)
        self.assertAlmostEqual(pitch, -32.0)

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
