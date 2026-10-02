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
    # yaw-only transforms always agreed, even on the regressed wheels
    g = np.array(carla.Transform(carla.Location(), carla.Rotation(yaw=37.0)).get_matrix())
    assert np.allclose(g, coords.ue_matrix(carla.Transform(carla.Location(), carla.Rotation(yaw=37.0))), atol=1e-6)


def test_get_matrix_agrees_with_ue_matrix_on_fixed_wheels():
    """On a wheel built after the LibCarla geom fix, ``get_matrix()`` IS ``ue_matrix()``.

    ``fix/geom-engine-convention`` restores the 0.9.x / engine pitch and roll signs that
    LibCarla ``4d853ed98`` (PR #9751) had mirrored, so the :mod:`carla_cosmos.coords`
    bypass becomes redundant -- it is kept only so this package still works against
    wheels that predate the fix.  Skipped, not failed, on such an older wheel: the
    bypass is what makes that case safe.
    """
    import carla
    from carla_cosmos import coords

    probe = carla.Transform(carla.Location(), carla.Rotation(pitch=20.0))
    if np.array(probe.get_matrix())[2][0] < 0.0:
        pytest.skip("wheel predates the LibCarla geom fix; coords.ue_matrix bypass is in use")

    cases = [
        (0.0, 0.0, 0.0), (20.0, 0.0, 0.0), (-20.0, 0.0, 0.0),
        (0.0, 0.0, 25.0), (0.0, 0.0, -25.0), (0.0, 37.0, 0.0),
        (13.0, 47.0, -31.0), (-62.0, -155.0, 88.0),
    ]
    for pitch, yaw, roll in cases:
        tf = carla.Transform(
            carla.Location(1.0, -2.0, 3.0), carla.Rotation(pitch=pitch, yaw=yaw, roll=roll))
        assert np.allclose(np.array(tf.get_matrix()), coords.ue_matrix(tf), atol=1e-5), (
            f"get_matrix() != ue_matrix() at pitch={pitch} yaw={yaw} roll={roll}")
        assert np.allclose(
            np.array(tf.get_inverse_matrix()) @ np.array(tf.get_matrix()), np.eye(4), atol=1e-4)
        fwd, right, up = coords.ue_forward_right_up(tf.rotation)
        for got, want in ((tf.get_forward_vector(), fwd),
                          (tf.get_right_vector(), right),
                          (tf.get_up_vector(), up)):
            assert np.allclose([got.x, got.y, got.z], want, atol=1e-5)


# --------------------------------------------------------------------------- carla.Transform.to_right_handed()

_HAS_TO_RIGHT_HANDED = hasattr(carla.Transform, "to_right_handed")

_needs_boundary_api = pytest.mark.skipif(
    not _HAS_TO_RIGHT_HANDED,
    reason="wheel predates carla.Transform.to_right_handed(); this module's own "
           "UE->FLU helpers are the conversion path there",
)

_BOUNDARY_CASES = [
    # (location, (pitch, yaw, roll)) in CARLA's frame
    ((0.0, 0.0, 0.0), (0.0, 0.0, 0.0)),
    ((1.0, 2.0, 3.0), (20.0, 30.0, 10.0)),
    ((-4.5, 6.25, 0.5), (-15.0, 120.0, 40.0)),
    ((100.0, -50.0, 2.0), (0.0, -90.0, 0.0)),
    ((0.0, 7.0, -1.0), (35.0, 0.0, 0.0)),
    ((3.0, 3.0, 3.0), (0.0, 0.0, -25.0)),
    ((-9.0, 0.25, 11.0), (13.0, 47.0, -31.0)),
    ((-62.0, 8.0, 1.5), (-62.0, -155.0, 88.0)),
]


def _boundary_transforms():
    for (x, y, z), (pitch, yaw, roll) in _BOUNDARY_CASES:
        yield carla.Transform(
            carla.Location(x=x, y=y, z=z),
            carla.Rotation(pitch=pitch, yaw=yaw, roll=roll))


@_needs_boundary_api
def test_to_right_handed_matrix_agrees_with_ue_to_flu():
    """``carla.Transform.to_right_handed().get_matrix()`` IS ``ue_to_flu(ue_matrix(tf))``.

    LibCarla now carries the UE -> FLU mapping itself (``RightHandedTransform``,
    ``Docs/coordinate_conventions.md``).  This pins the C++ adapter against this
    package's independent numpy implementation, which was written first and was
    validated geometrically in the Phase 0 spike.
    """
    for tf in _boundary_transforms():
        got = np.array(tf.to_right_handed().get_matrix(), dtype=np.float64)
        want = coords.ue_to_flu(coords.ue_matrix(tf))
        np.testing.assert_allclose(got, want, atol=1e-5, err_msg=str(tf))


@_needs_boundary_api
def test_to_right_handed_rpy_agrees_with_flu_rpy_deg():
    """``.rotation`` (roll, pitch, yaw) is what ``coords.flu_rpy_deg`` reports."""
    for tf in _boundary_transforms():
        rh = tf.to_right_handed().rotation
        want = coords.flu_rpy_deg(coords.ue_to_flu(coords.ue_matrix(tf)))
        # Euler angles are not unique; compare through the rotation matrix to
        # stay robust, then check the direct triple where it is unambiguous.
        np.testing.assert_allclose(
            coords.flu_pose_matrix([0, 0, 0], [rh.roll, rh.pitch, rh.yaw])[:3, :3],
            coords.flu_pose_matrix([0, 0, 0], want)[:3, :3],
            atol=1e-5, err_msg=str(tf))
        if abs(rh.pitch) < 89.0:
            np.testing.assert_allclose(
                [rh.roll, rh.pitch, rh.yaw], want, atol=1e-3, err_msg=str(tf))


@_needs_boundary_api
def test_to_right_handed_quaternion_agrees_with_quat_xyzw():
    """``get_quaternion()`` is ``coords.quat_xyzw`` of the same FLU pose (up to sign)."""
    for tf in _boundary_transforms():
        q = tf.to_right_handed().get_quaternion()
        want = coords.quat_xyzw(coords.ue_to_flu(coords.ue_matrix(tf)))
        got = np.array([q.x, q.y, q.z, q.w])
        ref = np.array([want["x"], want["y"], want["z"], want["w"]])
        # q and -q are the same rotation.
        assert abs(float(got @ ref)) == pytest.approx(1.0, abs=1e-5), str(tf)


@_needs_boundary_api
def test_to_right_handed_location_agrees_with_ue_point_to_flu():
    for tf in _boundary_transforms():
        rh = tf.to_right_handed().location
        np.testing.assert_allclose(
            [rh.x, rh.y, rh.z],
            coords.ue_point_to_flu([tf.location.x, tf.location.y, tf.location.z]),
            atol=1e-6, err_msg=str(tf))


@_needs_boundary_api
def test_from_right_handed_round_trips():
    for tf in _boundary_transforms():
        back = carla.Transform.from_right_handed(tf.to_right_handed())
        np.testing.assert_allclose(
            coords.ue_matrix(back), coords.ue_matrix(tf), atol=1e-5, err_msg=str(tf))


@_needs_boundary_api
def test_reference_case_10_20_30():
    """CARLA (roll, pitch, yaw) = (10, 20, 30) is FLU (10, -20, -30)."""
    rh = carla.Rotation(pitch=20.0, yaw=30.0, roll=10.0).to_right_handed()
    assert (rh.roll, rh.pitch, rh.yaw) == pytest.approx((10.0, -20.0, -30.0), abs=1e-4)
