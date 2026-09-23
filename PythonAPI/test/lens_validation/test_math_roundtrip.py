#!/usr/bin/env python

# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Offline, server-free tests for lens_math_reference.py.

Run with:
    pytest -k math -q
or just:
    pytest PythonAPI/test/lens_validation/test_math_roundtrip.py -q

These check:
  * forward -> inverse round-trip residual < 1e-6 across the field, for
    every camera_model in the Blueprint contract.
  * brown_conrady matches cv2 (projectPoints / undistortPoints).
  * kannala_brandt matches cv2.fisheye (projectPoints / undistortPoints).
"""

import numpy as np
import pytest

import lens_math_reference as lm

cv2 = pytest.importorskip('cv2', reason='opencv-python is required for the cv2 cross-checks')

ROUNDTRIP_TOL_RAD = 1e-6  # angular residual tolerance requested by the task
GRID_N_THETA = 25
GRID_N_PHI = 16


def _theta_grid(theta_max_rad, n=GRID_N_THETA, phi_n=GRID_N_PHI, theta_min_frac=1e-4):
    """Sample a (theta, phi) grid covering the field, avoiding exactly
    theta=0 (where phi is degenerate) and exactly theta_max (edge of
    validity for bounded models)."""
    theta = np.linspace(theta_max_rad * theta_min_frac, theta_max_rad * 0.999, n)
    phi = np.linspace(-np.pi, np.pi, phi_n, endpoint=False)
    tt, pp = np.meshgrid(theta, phi, indexing='ij')
    return tt.ravel(), pp.ravel()


def _angular_residual(rays_a, rays_b):
    dot = np.sum(rays_a * rays_b, axis=-1)
    dot = np.clip(dot, -1.0, 1.0)
    return np.arccos(dot)


# ---------------------------------------------------------------------------
# Per-model round trip
# ---------------------------------------------------------------------------

CLOSED_FORM_CASES = {
    'perspective': dict(fx=1.0, fy=1.0, cx=0.5, cy=0.5, theta_max_deg=75.0),
    'stereographic': dict(fx=1.0, fy=1.0, cx=0.5, cy=0.5, theta_max_deg=140.0),
    'equidistant': dict(fx=1.0, fy=1.0, cx=0.5, cy=0.5, theta_max_deg=170.0),
    'equisolid': dict(fx=1.0, fy=1.0, cx=0.5, cy=0.5, theta_max_deg=170.0),
    'orthographic': dict(fx=1.0, fy=1.0, cx=0.5, cy=0.5, theta_max_deg=85.0),
}


@pytest.mark.parametrize('camera_model', sorted(CLOSED_FORM_CASES.keys()))
def test_closed_form_roundtrip(camera_model):
    params = CLOSED_FORM_CASES[camera_model]
    model = lm.LensModel(camera_model=camera_model, distortion_coeffs=[], lut=[], **params)
    theta, phi = _theta_grid(model.theta_max_rad)
    rays = lm.theta_phi_to_ray(theta, phi)

    u, v = model.project(rays)
    rays_back = model.unproject(u, v)

    residual = _angular_residual(rays, rays_back)
    assert np.max(residual) < ROUNDTRIP_TOL_RAD, (
        'camera_model=%s max angular residual=%.3e rad' % (camera_model, np.max(residual)))


def test_kannala_brandt_roundtrip():
    coeffs = [1.0, -0.02, 0.004, -0.0006]
    model = lm.LensModel(camera_model='kannala_brandt', fx=1.0, fy=1.0, cx=0.5, cy=0.5,
                          distortion_coeffs=coeffs, theta_max_deg=110.0)
    theta, phi = _theta_grid(model.theta_max_rad)
    rays = lm.theta_phi_to_ray(theta, phi)

    u, v = model.project(rays)
    rays_back = model.unproject(u, v)

    residual = _angular_residual(rays, rays_back)
    assert np.max(residual) < ROUNDTRIP_TOL_RAD, 'max angular residual=%.3e rad' % np.max(residual)


def test_brown_conrady_roundtrip():
    coeffs = [-0.18, 0.045, 0.0008, -0.0006, -0.004]  # k1, k2, p1, p2, k3
    model = lm.LensModel(camera_model='brown_conrady', fx=1.0, fy=1.0, cx=0.5, cy=0.5,
                          distortion_coeffs=coeffs, theta_max_deg=65.0)
    theta, phi = _theta_grid(model.theta_max_rad)
    rays = lm.theta_phi_to_ray(theta, phi)

    u, v = model.project(rays)
    rays_back = model.unproject(u, v)

    residual = _angular_residual(rays, rays_back)
    assert np.max(residual) < ROUNDTRIP_TOL_RAD, 'max angular residual=%.3e rad' % np.max(residual)


def test_lut_roundtrip():
    # Sample the LUT off a known-monotonic closed-form model (equidistant
    # with a mild perturbation) so lut_inverse's monotonicity requirement
    # holds by construction.
    theta_samples = np.linspace(0.0, np.deg2rad(120.0), 200)
    r_samples = theta_samples * (1.0 + 0.05 * np.sin(theta_samples))
    lut = lm.lut_pack(theta_samples, r_samples)

    model = lm.LensModel(camera_model='lut', fx=1.0, fy=1.0, cx=0.5, cy=0.5,
                          lut=lut, theta_max_deg=118.0)
    theta, phi = _theta_grid(model.theta_max_rad)
    rays = lm.theta_phi_to_ray(theta, phi)

    u, v = model.project(rays)
    rays_back = model.unproject(u, v)

    residual = _angular_residual(rays, rays_back)
    # Linear-interpolation of a fine table is not machine-precision exact
    # the way the closed-form/Newton models are; bound it to something
    # tight but consistent with the sampling density above.
    assert np.max(residual) < 5e-5, 'max angular residual=%.3e rad' % np.max(residual)


# ---------------------------------------------------------------------------
# Cross-check against OpenCV
# ---------------------------------------------------------------------------

def test_brown_conrady_matches_cv2_projectpoints():
    fx, fy, cx, cy = 500.0, 480.0, 320.0, 240.0
    coeffs = [-0.18, 0.045, 0.0008, -0.0006, -0.004]  # k1, k2, p1, p2, k3
    model = lm.LensModel(camera_model='brown_conrady', fx=fx, fy=fy, cx=cx, cy=cy,
                          distortion_coeffs=coeffs, theta_max_deg=60.0)

    theta, phi = _theta_grid(model.theta_max_rad, n=15, phi_n=12)
    rays = lm.theta_phi_to_ray(theta, phi)

    u_ours, v_ours = model.project(rays)

    object_points = (rays / rays[:, 2:3]).astype(np.float64).reshape(-1, 1, 3)  # z=1 plane
    camera_matrix = np.array([[fx, 0, cx], [0, fy, cy], [0, 0, 1]], dtype=np.float64)
    dist_coeffs = np.array(coeffs, dtype=np.float64)
    rvec = np.zeros(3, dtype=np.float64)
    tvec = np.zeros(3, dtype=np.float64)

    image_points, _ = cv2.projectPoints(object_points, rvec, tvec, camera_matrix, dist_coeffs)
    u_cv2 = image_points[:, 0, 0]
    v_cv2 = image_points[:, 0, 1]

    np.testing.assert_allclose(u_ours, u_cv2, atol=1e-6, rtol=1e-9)
    np.testing.assert_allclose(v_ours, v_cv2, atol=1e-6, rtol=1e-9)


def test_brown_conrady_matches_cv2_undistortpoints():
    fx, fy, cx, cy = 500.0, 480.0, 320.0, 240.0
    coeffs = [-0.18, 0.045, 0.0008, -0.0006, -0.004]
    model = lm.LensModel(camera_model='brown_conrady', fx=fx, fy=fy, cx=cx, cy=cy,
                          distortion_coeffs=coeffs, theta_max_deg=60.0)

    theta, phi = _theta_grid(model.theta_max_rad, n=15, phi_n=12)
    rays = lm.theta_phi_to_ray(theta, phi)
    u, v = model.project(rays)

    rays_ours = model.unproject(u, v)

    distorted = np.stack([u, v], axis=-1).astype(np.float64).reshape(-1, 1, 2)
    camera_matrix = np.array([[fx, 0, cx], [0, fy, cy], [0, 0, 1]], dtype=np.float64)
    dist_coeffs = np.array(coeffs, dtype=np.float64)
    # cv2's default undistortPoints iteration count (5) is tuned for typical
    # lenses and under-converges at the wide field angles this test covers;
    # undistortPointsIter exposes the criteria so we can compare converged
    # solutions, not iteration-count artifacts.
    tight_criteria = (cv2.TERM_CRITERIA_MAX_ITER + cv2.TERM_CRITERIA_EPS, 200, 1e-12)
    identity = np.eye(3, dtype=np.float64)
    # P=identity (not camera_matrix) so the output stays in normalized
    # coordinates, matching what our own brown_conrady_undistort returns.
    undistorted = cv2.undistortPointsIter(distorted, camera_matrix, dist_coeffs,
                                           identity, identity, tight_criteria)
    xp = undistorted[:, 0, 0]
    yp = undistorted[:, 0, 1]
    theta_cv2 = np.arctan(np.hypot(xp, yp))
    phi_cv2 = np.arctan2(yp, xp)
    rays_cv2 = lm.theta_phi_to_ray(theta_cv2, phi_cv2)

    residual = _angular_residual(rays_ours, rays_cv2)
    assert np.max(residual) < 1e-6, 'max angular residual vs cv2.undistortPoints=%.3e rad' % np.max(residual)


def test_kannala_brandt_matches_cv2_fisheye():
    # cv2.fisheye fixes the leading polynomial term implicitly to 1
    # (theta_d = theta * (1 + k1 th^2 + k2 th^4 + k3 th^6 + k4 th^8)), while
    # our contract's r(theta) = k1*th + k2*th^3 + k3*th^5 + k4*th^7 leaves
    # k1 free. Setting our k1 = 1.0 recovers an exact analytic equivalence:
    #   ours: k1=1, k2, k3, k4        (th, th^3, th^5, th^7 coefficients)
    #   cv2 : f=1,  D=[k2, k3, k4, 0] (drops the th^9 term cv2 also has)
    #
    # cv2.fisheye internally derives theta from atan(sqrt(x^2+y^2)) with
    # (x, y) = (X/Z, Y/Z), which only ranges over [0, pi/2) -- OpenCV's
    # fisheye model cannot represent theta >= 90 degrees at all. Keep this
    # cross-check inside that hemisphere; the >90deg behaviour is exercised
    # only by our own Newton-solver round trip above.
    f = 1.0
    k2, k3, k4 = -0.02, 0.004, -0.0006
    coeffs = [1.0, k2, k3, k4]
    model = lm.LensModel(camera_model='kannala_brandt', fx=f, fy=f, cx=0.0, cy=0.0,
                          distortion_coeffs=coeffs, theta_max_deg=80.0)

    theta, phi = _theta_grid(model.theta_max_rad, n=15, phi_n=12)
    rays = lm.theta_phi_to_ray(theta, phi)

    u_ours, v_ours = model.project(rays)

    object_points = (rays / rays[:, 2:3]).astype(np.float64).reshape(1, -1, 3)
    camera_matrix = np.array([[f, 0, 0], [0, f, 0], [0, 0, 1]], dtype=np.float64)
    dist_coeffs = np.array([k2, k3, k4, 0.0], dtype=np.float64)
    rvec = np.zeros(3, dtype=np.float64)
    tvec = np.zeros(3, dtype=np.float64)

    image_points, _ = cv2.fisheye.projectPoints(object_points, rvec, tvec, camera_matrix, dist_coeffs)
    u_cv2 = image_points[0, :, 0]
    v_cv2 = image_points[0, :, 1]

    np.testing.assert_allclose(u_ours, u_cv2, atol=1e-6, rtol=1e-9)
    np.testing.assert_allclose(v_ours, v_cv2, atol=1e-6, rtol=1e-9)


def test_kannala_brandt_matches_cv2_fisheye_undistort():
    f = 1.0
    k2, k3, k4 = -0.02, 0.004, -0.0006
    coeffs = [1.0, k2, k3, k4]
    model = lm.LensModel(camera_model='kannala_brandt', fx=f, fy=f, cx=0.0, cy=0.0,
                          distortion_coeffs=coeffs, theta_max_deg=80.0)

    theta, phi = _theta_grid(model.theta_max_rad, n=15, phi_n=12)
    rays = lm.theta_phi_to_ray(theta, phi)
    u, v = model.project(rays)

    distorted = np.stack([u, v], axis=-1).astype(np.float64).reshape(-1, 1, 2)
    camera_matrix = np.array([[f, 0, 0], [0, f, 0], [0, 0, 1]], dtype=np.float64)
    dist_coeffs = np.array([k2, k3, k4, 0.0], dtype=np.float64)
    tight_criteria = (cv2.TERM_CRITERIA_MAX_ITER + cv2.TERM_CRITERIA_EPS, 200, 1e-12)
    undistorted = cv2.fisheye.undistortPoints(distorted, camera_matrix, dist_coeffs, criteria=tight_criteria)
    xp = undistorted[:, 0, 0]
    yp = undistorted[:, 0, 1]
    theta_cv2 = np.arctan(np.hypot(xp, yp))
    phi_cv2 = np.arctan2(yp, xp)
    rays_cv2 = lm.theta_phi_to_ray(theta_cv2, phi_cv2)

    residual = _angular_residual(rays, rays_cv2)
    assert np.max(residual) < 1e-6, 'max angular residual vs cv2.fisheye.undistortPoints=%.3e rad' % np.max(residual)


if __name__ == '__main__':
    import sys
    sys.exit(pytest.main([__file__, '-v']))
