"""Coordinate-conversion tests, pinned against the installed carla wheel."""

import math

import numpy as np
import pytest
from scipy.spatial.transform import Rotation

import carla
from carla_cosmos import coords


def _random_transforms(n: int, seed: int = 42):
    rng = np.random.default_rng(seed)
    for _ in range(n):
        yield carla.Transform(
            carla.Location(*(rng.uniform(-500, 500, 3))),
            carla.Rotation(pitch=float(rng.uniform(-89, 89)),
                           yaw=float(rng.uniform(-180, 180)),
                           roll=float(rng.uniform(-180, 180))),
        )


def test_matrix_decomposition_round_trip_500():
    for tf in _random_transforms(500):
        m = coords.ue_matrix(tf)
        back = coords.ue_transform_from_matrix(m)
        np.testing.assert_allclose(coords.ue_matrix(back), m, atol=1e-5)


def test_ue_flu_is_involution():
    for tf in _random_transforms(50, seed=7):
        m = coords.ue_matrix(tf)
        np.testing.assert_allclose(coords.flu_to_ue(coords.ue_to_flu(m)), m, atol=1e-12)


def test_flu_rotation_is_right_handed():
    for tf in _random_transforms(50, seed=3):
        r = coords.ue_to_flu(coords.ue_matrix(tf))[:3, :3]
        assert np.linalg.det(r) == pytest.approx(1.0, abs=1e-6)
        np.testing.assert_allclose(r @ r.T, np.eye(3), atol=1e-6)


def test_flu_yaw_sign():
    """UE yaw +30 deg (towards +y = right) must become FLU yaw -30 deg."""
    tf = carla.Transform(carla.Location(), carla.Rotation(yaw=30.0))
    m_flu = coords.ue_to_flu(coords.ue_matrix(tf))
    rpy = coords.flu_rpy_deg(m_flu)
    assert rpy[2] == pytest.approx(-30.0, abs=1e-6)
    assert rpy[0] == pytest.approx(0.0, abs=1e-6)
    assert rpy[1] == pytest.approx(0.0, abs=1e-6)


def test_point_conversion_flips_y():
    p = coords.ue_point_to_flu((1.0, 2.0, 3.0))
    np.testing.assert_allclose(p, [1.0, -2.0, 3.0])


def test_flu_pose_matrix_matches_loader_convention():
    """flu_pose_matrix must equal scipy from_euler('xyz'), which is what the ClipGT loader does."""
    t, rpy = (1.7, -0.01, 1.44), (0.13, 0.47, -0.71)
    m = coords.flu_pose_matrix(t, rpy)
    expected = Rotation.from_euler("xyz", np.radians(rpy)).as_matrix()
    np.testing.assert_allclose(m[:3, :3], expected, atol=1e-12)
    np.testing.assert_allclose(coords.flu_rpy_deg(m), rpy, atol=1e-9)


@pytest.mark.parametrize("hfov", [30.0, 70.0, 90.0, 120.0])
def test_ftheta_fit_residual(hfov):
    width, height = 1280, 720
    poly, resid = coords.pinhole_ftheta_poly(width, height, hfov)
    assert len(poly) == 6
    assert poly[0] == 0.0
    # 1.5e-3 rad = 0.086 deg at the extreme image corner; the fit residual for a
    # 120 deg pinhole at 1280x720 measures 1.005e-3 (a pinhole is not a polynomial).
    assert resid < 1.5e-3, f"f-theta fit residual {resid} too high for hfov={hfov}"
    # evaluate the polynomial against the exact pinhole angle at half radius
    f = coords.pinhole_focal_px(width, hfov)
    r = math.hypot(width / 2, height / 2) / 2
    theta_poly = sum(k * r ** i for i, k in enumerate(poly))
    assert theta_poly == pytest.approx(math.atan(r / f), abs=2e-3)


def test_ue_matrix_matches_engine_not_get_matrix():
    """Engine convention (verified with real camera images): +pitch looks up, +roll drops the right side.
    LibCarla 4d853ed98 (ue58 branch) flipped both signs in get_matrix(); we must not follow it."""
    import carla
    from carla_cosmos import coords

    fwd, right, up = coords.ue_forward_right_up(carla.Rotation(pitch=20.0))
    assert fwd[2] > 0.3 and abs(up[0] + fwd[2]) < 1e-9
    fwd, right, up = coords.ue_forward_right_up(carla.Rotation(roll=25.0))
    assert right[2] < -0.4 and up[1] > 0.4
    m = coords.ue_matrix(carla.Transform(carla.Location(1, 2, 3), carla.Rotation(pitch=3.26, yaw=-151.4, roll=-0.63)))
    tf = coords.ue_transform_from_matrix(m)
    assert abs(tf.rotation.pitch - 3.26) < 1e-4 and abs(tf.rotation.yaw + 151.4) < 1e-4 and abs(tf.rotation.roll + 0.63) < 1e-4  # float32 fields
    # the wheel's get_matrix() disagrees on pitch/roll signs (the regression); yaw-only transforms agree
    g = np.array(carla.Transform(carla.Location(), carla.Rotation(yaw=37.0)).get_matrix())
    assert np.allclose(g, coords.ue_matrix(carla.Transform(carla.Location(), carla.Rotation(yaw=37.0))), atol=1e-6)
