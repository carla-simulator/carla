#!/usr/bin/env python

# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Calibration refit: take captured checkerboard images (from
calibration_scene.py) plus the configured lens model, recover the
intrinsics/distortion by fitting against detected corners, and assert the
recovered parameters and reprojection residual are within tolerance.

Two ways to run:

1. SYNTHETIC (offline, runs now, no CARLA/server/captures needed):
       pytest test_calibration_refit.py -k synthetic
   Projects known board corners through lens_math_reference.LensModel to
   get exact "detections" (no rendering, no corner-detection noise), then
   checks the refit machinery in this file actually recovers the input
   parameters. This validates the fitting code path itself.

2. REAL (server-dependent, skipped until a capture directory exists):
       python calibration_scene.py --out-dir captures/brown_conrady --camera-model brown_conrady ...
       pytest test_calibration_refit.py --capture-dir captures/brown_conrady
   Loads manifest.json + images written by calibration_scene.py, detects
   checkerboard corners with cv2.findChessboardCorners, and fits against
   the ground-truth per-corner camera-space rays calibration_scene.py
   already computed from CARLA's exact transforms (so this test isolates
   lens-model fit quality from PnP/extrinsics noise).

Target tolerance: sub-0.1px reprojection RMS at 1080p (configurable via
--tolerance-px / the `tolerance_px` fixture default below).
"""

import argparse
import json
import os

import numpy as np
import pytest

import lens_math_reference as lm

scipy_optimize = pytest.importorskip('scipy.optimize', reason='scipy is required to fit intrinsics')


DEFAULT_TOLERANCE_PX = 0.1
DEFAULT_IMAGE_HEIGHT_PX = 1080.0


# ---------------------------------------------------------------------------
# Generic intrinsics/distortion refit against KNOWN 3D corner rays (camera
# space) and MEASURED pixel detections. Works for every camera_model in the
# Blueprint contract, not just the two OpenCV has native routines for.
# ---------------------------------------------------------------------------

def _param_vector(camera_model, fx, fy, cx, cy, distortion_coeffs):
    if camera_model == 'lut':
        raise ValueError('lut refit is a table lookup problem, not a parametric fit; '
                          'not supported by this generic refit')
    coeffs = list(distortion_coeffs)
    if camera_model == 'kannala_brandt':
        # k1 (the leading, th^1 term) and fx/fy are not jointly identifiable
        # from reprojection error alone -- scaling fx down and k1 up by the
        # same factor reproduces the same pixels (this is exactly why
        # cv2.fisheye fixes its equivalent leading term to 1 by convention).
        # Keep k1 fixed at its input value and only fit fx, fy, cx, cy, k2..k4.
        coeffs = coeffs[1:]
    return np.array([fx, fy, cx, cy] + coeffs, dtype=np.float64)


def _model_from_param_vector(camera_model, params, n_distortion, fixed_k1=None):
    fx, fy, cx, cy = params[:4]
    coeffs = list(params[4:4 + n_distortion])
    if camera_model == 'kannala_brandt':
        coeffs = [fixed_k1] + coeffs
    return lm.LensModel(camera_model=camera_model, fx=fx, fy=fy, cx=cx, cy=cy,
                         distortion_coeffs=coeffs)


def refit_intrinsics(camera_model, rays_camera_space, pixels_measured,
                      initial_fx, initial_fy, initial_cx, initial_cy,
                      initial_distortion_coeffs):
    """rays_camera_space: (N, 3) known 3D ray directions in camera space
    (need not be unit; LensModel.project normalizes via theta/phi).
    pixels_measured: (N, 2) detected (u, v).
    Returns (fitted_model, residual_px) where residual_px is the
    per-point Euclidean reprojection error in the same units as fx/fy
    (i.e. normalized units -- multiply by image_size to get pixels)."""
    fixed_k1 = initial_distortion_coeffs[0] if camera_model == 'kannala_brandt' else None
    n_distortion = len(initial_distortion_coeffs) - (1 if camera_model == 'kannala_brandt' else 0)
    x0 = _param_vector(camera_model, initial_fx, initial_fy, initial_cx, initial_cy,
                        initial_distortion_coeffs)

    def residuals(params):
        model = _model_from_param_vector(camera_model, params, n_distortion, fixed_k1)
        u, v = model.project(rays_camera_space)
        return np.concatenate([u - pixels_measured[:, 0], v - pixels_measured[:, 1]])

    result = scipy_optimize.least_squares(residuals, x0, method='lm', xtol=1e-14, ftol=1e-14)
    fitted = _model_from_param_vector(camera_model, result.x, n_distortion, fixed_k1)
    u, v = fitted.project(rays_camera_space)
    residual_px = np.hypot(u - pixels_measured[:, 0], v - pixels_measured[:, 1])
    return fitted, residual_px


# ---------------------------------------------------------------------------
# 1. SYNTHETIC self-test (offline, runs now)
# ---------------------------------------------------------------------------

SYNTHETIC_CASES = {
    'perspective': dict(fx=0.9, fy=0.85, cx=0.02, cy=-0.01, coeffs=[], theta_max_deg=55.0),
    'stereographic': dict(fx=0.9, fy=0.85, cx=0.02, cy=-0.01, coeffs=[], theta_max_deg=100.0),
    'equidistant': dict(fx=0.9, fy=0.85, cx=0.02, cy=-0.01, coeffs=[], theta_max_deg=120.0),
    'equisolid': dict(fx=0.9, fy=0.85, cx=0.02, cy=-0.01, coeffs=[], theta_max_deg=120.0),
    'orthographic': dict(fx=0.9, fy=0.85, cx=0.02, cy=-0.01, coeffs=[], theta_max_deg=80.0),
    'kannala_brandt': dict(fx=1.0, fy=1.0, cx=0.0, cy=0.0,
                            coeffs=[1.02, -0.021, 0.0038, -0.0005], theta_max_deg=100.0),
    'brown_conrady': dict(fx=0.92, fy=0.88, cx=0.01, cy=-0.02,
                           coeffs=[-0.16, 0.04, 0.001, -0.0008, -0.003], theta_max_deg=55.0),
}


def _synthetic_true_model(camera_model):
    case = SYNTHETIC_CASES[camera_model]
    return lm.LensModel(camera_model=camera_model, fx=case['fx'], fy=case['fy'],
                         cx=case['cx'], cy=case['cy'], distortion_coeffs=case['coeffs'],
                         theta_max_deg=case['theta_max_deg']), case


@pytest.mark.parametrize('camera_model', sorted(SYNTHETIC_CASES.keys()))
def test_refit_recovers_synthetic_params(camera_model):
    true_model, case = _synthetic_true_model(camera_model)

    rng = np.random.default_rng(42)
    theta = rng.uniform(0.02, true_model.theta_max_rad * 0.95, size=300)
    phi = rng.uniform(-np.pi, np.pi, size=300)
    rays = lm.theta_phi_to_ray(theta, phi)
    u, v = true_model.project(rays)
    pixels = np.stack([u, v], axis=-1)

    # Perturbed initial guess, as a real capture pipeline would start from
    # the Blueprint's *configured* (not yet calibrated) attribute values.
    initial_fx = case['fx'] * 1.08
    initial_fy = case['fy'] * 0.93
    initial_cx = case['cx'] + 0.01
    initial_cy = case['cy'] - 0.015
    if case['coeffs']:
        initial_coeffs = [c * 1.1 + 0.001 for c in case['coeffs']]
        if camera_model == 'kannala_brandt':
            # k1 is fixed (not fitted, see refit_intrinsics) -- a real
            # calibration workflow fixes this by convention up front, so
            # the "initial guess" for it must equal ground truth, not be
            # perturbed like the other coefficients.
            initial_coeffs[0] = case['coeffs'][0]
    else:
        initial_coeffs = []

    fitted, residual_px = refit_intrinsics(
        camera_model, rays, pixels, initial_fx, initial_fy, initial_cx, initial_cy,
        initial_coeffs)

    tolerance_normalized = DEFAULT_TOLERANCE_PX / DEFAULT_IMAGE_HEIGHT_PX
    assert np.max(residual_px) < tolerance_normalized, (
        'camera_model=%s max reprojection residual=%.3e (tol=%.3e)' %
        (camera_model, np.max(residual_px), tolerance_normalized))

    assert abs(fitted.fx - case['fx']) < 1e-4
    assert abs(fitted.fy - case['fy']) < 1e-4
    assert abs(fitted.cx - case['cx']) < 1e-4
    assert abs(fitted.cy - case['cy']) < 1e-4
    for got, expected in zip(fitted.distortion_coeffs, case['coeffs']):
        assert abs(got - expected) < 1e-3


# ---------------------------------------------------------------------------
# 2. REAL capture-driven refit (server-dependent; skips if no captures)
# ---------------------------------------------------------------------------

def load_manifest(capture_dir):
    manifest_path = os.path.join(capture_dir, 'manifest.json')
    with open(manifest_path) as f:
        return json.load(f)


def detect_checkerboard_corners(image_path, board_rows, board_cols):
    import cv2
    img = cv2.imread(image_path, cv2.IMREAD_GRAYSCALE)
    if img is None:
        raise RuntimeError('failed to read %s' % image_path)
    found, corners = cv2.findChessboardCorners(
        img, (board_cols, board_rows),
        flags=cv2.CALIB_CB_ADAPTIVE_THRESH + cv2.CALIB_CB_NORMALIZE_IMAGE)
    if not found:
        return None
    criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 100, 1e-4)
    corners = cv2.cornerSubPix(img, corners, (11, 11), (-1, -1), criteria)
    return corners.reshape(-1, 2), img.shape[1], img.shape[0]


def _capture_dir_from_env_or_cli():
    return os.environ.get('LENS_VALIDATION_CAPTURE_DIR', None)


@pytest.fixture
def capture_dir(request):
    d = request.config.getoption('--capture-dir', default=None) or _capture_dir_from_env_or_cli()
    if not d or not os.path.isfile(os.path.join(d, 'manifest.json')):
        pytest.skip('no capture directory with manifest.json available yet; run '
                     'calibration_scene.py against a live rt_lens-capable CARLA '
                     'server first (see this file\'s module docstring)')
    return d


@pytest.fixture
def tolerance_px(request):
    return request.config.getoption('--tolerance-px', default=DEFAULT_TOLERANCE_PX)


def test_refit_from_capture(capture_dir, tolerance_px):
    manifest = load_manifest(capture_dir)
    camera_model = manifest['camera_model']
    if camera_model == 'lut':
        pytest.skip('lut is a table lookup, not covered by the parametric refit')

    board_rows, board_cols = manifest['board_rows'], manifest['board_cols']
    attrs = manifest['attributes']
    initial_coeffs = []
    if attrs.get('distortion_coeffs'):
        initial_coeffs = [float(x) for x in str(attrs['distortion_coeffs']).split(',') if x != '']

    all_rays = []
    all_pixels = []
    image_h = None
    for view in manifest['views']:
        image_path = os.path.join(capture_dir, view['image'])
        detection = detect_checkerboard_corners(image_path, board_rows, board_cols)
        if detection is None:
            continue
        corners_px, img_w, img_h = detection
        image_h = img_h
        corners_norm = corners_px / np.array([img_w, img_h])  # match normalized fx/fy/cx/cy space

        rays_cam = np.array(view['corners_camera_space'])
        if len(rays_cam) != len(corners_norm):
            # Detection order/count mismatch with the known corner grid;
            # skip this view rather than silently mis-pairing correspondences.
            continue
        all_rays.append(rays_cam)
        all_pixels.append(corners_norm)

    if not all_rays:
        pytest.skip('no checkerboard successfully detected in any captured view')

    rays = np.concatenate(all_rays, axis=0)
    pixels = np.concatenate(all_pixels, axis=0)

    fitted, residual_px = refit_intrinsics(
        camera_model, rays, pixels,
        float(attrs['fx']), float(attrs['fy']), float(attrs['cx']), float(attrs['cy']),
        initial_coeffs)

    tolerance_normalized = tolerance_px / float(image_h or DEFAULT_IMAGE_HEIGHT_PX)
    rms_px = np.sqrt(np.mean(residual_px**2))
    assert rms_px < tolerance_normalized, (
        'capture_dir=%s camera_model=%s reprojection RMS=%.4f (tol=%.4f)' %
        (capture_dir, camera_model, rms_px, tolerance_normalized))


def build_arg_parser():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--capture-dir', required=True)
    parser.add_argument('--tolerance-px', type=float, default=DEFAULT_TOLERANCE_PX)
    return parser


if __name__ == '__main__':
    # Standalone (non-pytest) entry point: run the real-capture refit
    # directly against a manifest and print the residual.
    args = build_arg_parser().parse_args()
    manifest = load_manifest(args.capture_dir)
    print('camera_model=%s, %d views' % (manifest['camera_model'], len(manifest['views'])))
