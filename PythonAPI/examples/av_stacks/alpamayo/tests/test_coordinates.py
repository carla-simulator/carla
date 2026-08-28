import math
import unittest
from types import SimpleNamespace

import numpy as np

try:
    import carla
except ImportError:  # the wheel is optional for the pure-python tests
    carla = None

from alpamayo_coordinates import (
    alpamayo_points_to_carla_local,
    carla_rotation_matrix,
    history_to_alpamayo,
)


def transform(x, y, z=0.0, pitch=0.0, yaw=0.0, roll=0.0):
    return SimpleNamespace(
        location=SimpleNamespace(x=x, y=y, z=z),
        rotation=SimpleNamespace(pitch=pitch, yaw=yaw, roll=roll),
    )


class CoordinateTest(unittest.TestCase):
    def test_rotation_matches_expected_yaw(self):
        rotation = carla_rotation_matrix(0.0, 90.0, 0.0)
        np.testing.assert_allclose(
            rotation,
            np.array([[0.0, -1.0, 0.0], [1.0, 0.0, 0.0], [0.0, 0.0, 1.0]]),
            atol=1e-7,
        )

    def test_rotation_matches_engine_closed_form(self):
        """R = Rz(+yaw) . Ry(-pitch) . Rx(-roll), CARLA's engine convention."""
        pitch_deg, yaw_deg, roll_deg = 17.0, -43.0, 11.0
        p, y, r = (math.radians(a) for a in (-pitch_deg, yaw_deg, -roll_deg))
        rz = np.array([
            [math.cos(y), -math.sin(y), 0.0],
            [math.sin(y), math.cos(y), 0.0],
            [0.0, 0.0, 1.0],
        ])
        ry = np.array([
            [math.cos(p), 0.0, math.sin(p)],
            [0.0, 1.0, 0.0],
            [-math.sin(p), 0.0, math.cos(p)],
        ])
        rx = np.array([
            [1.0, 0.0, 0.0],
            [0.0, math.cos(r), -math.sin(r)],
            [0.0, math.sin(r), math.cos(r)],
        ])
        np.testing.assert_allclose(
            carla_rotation_matrix(pitch_deg, yaw_deg, roll_deg),
            rz @ ry @ rx,
            atol=1e-12,
        )

    def test_rotation_is_not_the_pre_2026_08_28_mirrored_matrix(self):
        """Guard against re-introducing the PR #9751 pitch/roll mirroring."""
        rotation = carla_rotation_matrix(20.0, 0.0, 0.0)
        self.assertAlmostEqual(rotation[2, 0], math.sin(math.radians(20.0)), places=12)

    @unittest.skipIf(carla is None, "the carla wheel is not installed")
    def test_rotation_matches_carla_get_matrix(self):
        pitch_deg, yaw_deg, roll_deg = 17.0, -43.0, 11.0
        transform = carla.Transform(
            carla.Location(),
            carla.Rotation(pitch=pitch_deg, yaw=yaw_deg, roll=roll_deg),
        )
        expected = np.array(transform.get_matrix(), dtype=np.float64)[:3, :3]
        np.testing.assert_allclose(
            carla_rotation_matrix(pitch_deg, yaw_deg, roll_deg),
            expected,
            atol=1e-6,
        )

    def test_history_is_relative_to_latest_and_reflects_y(self):
        history = [transform(1.0, 2.0), transform(3.0, 5.0)]
        xyz, rotations = history_to_alpamayo(history, rear_axle_x=0.0)
        np.testing.assert_allclose(xyz[0, 0, 0], [-2.0, 3.0, 0.0])
        np.testing.assert_allclose(xyz[0, 0, 1], [0.0, 0.0, 0.0])
        np.testing.assert_allclose(rotations[0, 0, 1], np.eye(3))

    def test_history_respects_latest_heading(self):
        history = [transform(0.0, 0.0), transform(0.0, 2.0, yaw=90.0)]
        xyz, _ = history_to_alpamayo(history, rear_axle_x=0.0)
        np.testing.assert_allclose(xyz[0, 0, 0], [-2.0, 0.0, 0.0], atol=1e-6)

    def test_prediction_lateral_axis_is_reflected(self):
        result = alpamayo_points_to_carla_local(
            np.array([[10.0, 2.0, 0.0]], dtype=np.float32)
        )
        np.testing.assert_array_equal(result, [[10.0, -2.0, 0.0]])


if __name__ == "__main__":
    unittest.main()
