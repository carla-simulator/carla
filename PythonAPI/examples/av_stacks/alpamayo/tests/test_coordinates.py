import unittest
from types import SimpleNamespace

import numpy as np

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
