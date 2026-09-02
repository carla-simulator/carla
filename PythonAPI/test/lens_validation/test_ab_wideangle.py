#!/usr/bin/env python

# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""A/B check: sensor.camera.wide_angle (cubemap resample, exact-distortion
reference) vs sensor.camera.rt_lens (path-traced), configured with matching
lens parameters, viewing the same static scene from the same transform.

Two assertions:
  1. Distortion FIELD match: project the calibration checkerboard's corners
     through both cameras and compare detected pixel positions -- these
     should agree within a small sampling tolerance (cubemap resample vs
     path-traced rasterization of the same analytic distortion function).
     We compare FEATURE POINT POSITIONS, not raw pixels, since the two
     rendering paths produce different pixel content even where the
     geometric mapping agrees.
  2. Depth-of-field / chromatic-aberration ONLY in rt_lens: with a small
     aperture_fstop and a scene containing targets at multiple distances,
     wide_angle must stay uniformly sharp everywhere (it has no DoF model);
     rt_lens must show a sharpness falloff away from focus_distance_m.

SERVER-DEPENDENT -- needs both sensor.camera.wide_angle and the rt_lens
migration build. `carla` is imported lazily so this file is import-clean
and its argparse CLI usable without carla installed; pytest tests here
skip (rather than error) when no server is reachable.

ASSUMPTION (unverified until the engine side lands): sensor.camera.wide_angle
exposes fx, fy, cx, cy, distortion_coeffs attributes compatible with the
rt_lens contract for the closed-form/kannala_brandt/brown_conrady models,
so the same parameters can be applied to both blueprints for the A/B. If
wide_angle's actual attribute names differ, update WIDE_ANGLE_ATTR_MAP
below -- everything else in this file is otherwise unaffected.
"""

import argparse
import os
import socket

import numpy as np
import pytest

import lens_math_reference as lm
from calibration_scene import (
    BOARD_ROWS, BOARD_COLS, SQUARE_SIZE_M,
    CALIBRATION_TARGET_BLUEPRINT_CANDIDATES,
)
from test_calibration_refit import detect_checkerboard_corners

try:
    import carla
except ImportError:
    carla = None


WIDE_ANGLE_SENSOR_ID = 'sensor.camera.wide_angle'
RT_LENS_SENSOR_ID = 'sensor.camera.rt_lens'

# See module docstring ASSUMPTION note.
WIDE_ANGLE_ATTR_MAP = {
    'camera_model': 'camera_model',
    'fx': 'fx', 'fy': 'fy', 'cx': 'cx', 'cy': 'cy',
    'distortion_coeffs': 'distortion_coeffs',
    'image_size_x': 'image_size_x', 'image_size_y': 'image_size_y',
}

DEFAULT_FEATURE_TOLERANCE_PX = 1.5  # cubemap-resample vs path-traced sampling slop
DEFAULT_NEAR_M, DEFAULT_MID_M, DEFAULT_FAR_M = 2.0, 6.0, 18.0


def _server_reachable(host, port, timeout_s=1.5):
    try:
        with socket.create_connection((host, port), timeout=timeout_s):
            return True
    except OSError:
        return False


@pytest.fixture
def carla_world(request):
    host = request.config.getoption('--host', default='127.0.0.1')
    port = request.config.getoption('--port', default=2000)
    if carla is None:
        pytest.skip('carla PythonAPI module is not importable in this environment')
    if not _server_reachable(host, port):
        pytest.skip('no CARLA server reachable at %s:%s' % (host, port))
    client = carla.Client(host, port)
    client.set_timeout(10.0)
    world = client.get_world()
    bp_lib = world.get_blueprint_library()
    if len(bp_lib.filter(RT_LENS_SENSOR_ID)) == 0:
        pytest.skip('%s not present -- rt_lens migration build not loaded yet' % RT_LENS_SENSOR_ID)
    if len(bp_lib.filter(WIDE_ANGLE_SENSOR_ID)) == 0:
        pytest.skip('%s not present on this server' % WIDE_ANGLE_SENSOR_ID)
    return world


def _spawn_camera(world, sensor_id, attributes, transform):
    bp = world.get_blueprint_library().filter(sensor_id)[0]
    for key, value in attributes.items():
        if bp.has_attribute(key):
            bp.set_attribute(key, str(value))
    return world.spawn_actor(bp, transform)


def _capture_sync(world, camera, out_path):
    captured = {}

    def _on_image(image):
        image.save_to_disk(out_path)
        captured['frame'] = image.frame

    handle = camera.listen(_on_image)  # noqa: F841 (handle kept alive by camera)
    settings = world.get_settings()
    for _ in range(3):
        if settings.synchronous_mode:
            world.tick()
        else:
            world.wait_for_tick()
    camera.stop()
    return captured.get('frame')


def _sharpness_map(image_gray, n_regions=5):
    """Variance-of-Laplacian sharpness per horizontal band, coarse but
    sufficient to see a DoF falloff vs. a uniformly-sharp reference."""
    import cv2
    h = image_gray.shape[0]
    band_h = h // n_regions
    scores = []
    for i in range(n_regions):
        band = image_gray[i * band_h:(i + 1) * band_h, :]
        lap = cv2.Laplacian(band, cv2.CV_64F)
        scores.append(float(lap.var()))
    return scores


# ---------------------------------------------------------------------------
# 1. Distortion field match
# ---------------------------------------------------------------------------

def test_distortion_field_matches(carla_world, tmp_path, request):
    world = carla_world
    lens_params = dict(
        camera_model='brown_conrady', fx=0.9, fy=0.85, cx=0.5, cy=0.5,
        distortion_coeffs='-0.16,0.04,0.001,-0.0008,-0.003',
        image_size_x=1920, image_size_y=1080)

    bp_lib = world.get_blueprint_library()
    target_candidates = None
    for candidate in CALIBRATION_TARGET_BLUEPRINT_CANDIDATES:
        found = bp_lib.filter(candidate)
        if len(found) > 0:
            target_candidates = found[0]
            break
    if target_candidates is None:
        pytest.skip('no calibration-target blueprint available (see calibration_scene.py docstring)')

    target_transform = carla.Transform(carla.Location(x=6.0, y=0.0, z=1.0))
    target = world.spawn_actor(target_candidates, target_transform)
    camera_transform = carla.Transform(carla.Location(x=0.0, y=0.0, z=1.0))

    try:
        rt_lens_cam = _spawn_camera(world, RT_LENS_SENSOR_ID, lens_params, camera_transform)
        wide_attrs = {WIDE_ANGLE_ATTR_MAP[k]: v for k, v in lens_params.items()
                      if k in WIDE_ANGLE_ATTR_MAP}
        wide_cam = _spawn_camera(world, WIDE_ANGLE_SENSOR_ID, wide_attrs, camera_transform)

        rt_lens_path = str(tmp_path / 'rt_lens.png')
        wide_path = str(tmp_path / 'wide_angle.png')
        _capture_sync(world, rt_lens_cam, rt_lens_path)
        _capture_sync(world, wide_cam, wide_path)

        rt_corners = detect_checkerboard_corners(rt_lens_path, BOARD_ROWS, BOARD_COLS)
        wide_corners = detect_checkerboard_corners(wide_path, BOARD_ROWS, BOARD_COLS)
        assert rt_corners is not None, 'checkerboard not detected in rt_lens capture'
        assert wide_corners is not None, 'checkerboard not detected in wide_angle capture'

        rt_px, _, _ = rt_corners
        wide_px, _, _ = wide_corners
        assert rt_px.shape == wide_px.shape, 'corner count mismatch between the two cameras'

        deviations = np.hypot(*(rt_px - wide_px).T)
        tol = request.config.getoption('--feature-tolerance-px', default=DEFAULT_FEATURE_TOLERANCE_PX)
        assert np.max(deviations) < tol, (
            'max feature-point deviation between rt_lens and wide_angle = %.2fpx (tol=%.2fpx)' %
            (np.max(deviations), tol))
    finally:
        for actor in (locals().get('rt_lens_cam'), locals().get('wide_cam'), target):
            if actor is not None:
                actor.destroy()


# ---------------------------------------------------------------------------
# 2. DoF / CA present only in rt_lens
# ---------------------------------------------------------------------------

def test_dof_only_in_rt_lens(carla_world, tmp_path):
    world = carla_world
    lens_params = dict(
        camera_model='perspective', fx=1.0, fy=1.0, cx=0.5, cy=0.5,
        image_size_x=1280, image_size_y=720,
        aperture_fstop=1.4, focus_distance_m=DEFAULT_MID_M,
        samples_per_pixel=64)

    bp_lib = world.get_blueprint_library()
    target_candidates = None
    for candidate in CALIBRATION_TARGET_BLUEPRINT_CANDIDATES:
        found = bp_lib.filter(candidate)
        if len(found) > 0:
            target_candidates = found[0]
            break
    if target_candidates is None:
        pytest.skip('no calibration-target blueprint available (see calibration_scene.py docstring)')

    camera_transform = carla.Transform(carla.Location(x=0.0, y=0.0, z=1.0))
    targets = []
    try:
        for depth in (DEFAULT_NEAR_M, DEFAULT_MID_M, DEFAULT_FAR_M):
            t = world.spawn_actor(target_candidates, carla.Transform(
                carla.Location(x=depth, y=(depth - DEFAULT_MID_M) * 0.02, z=1.0)))
            targets.append(t)

        rt_lens_cam = _spawn_camera(world, RT_LENS_SENSOR_ID, lens_params, camera_transform)
        wide_attrs = {WIDE_ANGLE_ATTR_MAP[k]: v for k, v in lens_params.items()
                      if k in WIDE_ANGLE_ATTR_MAP}
        wide_cam = _spawn_camera(world, WIDE_ANGLE_SENSOR_ID, wide_attrs, camera_transform)

        rt_lens_path = str(tmp_path / 'rt_lens_dof.png')
        wide_path = str(tmp_path / 'wide_angle_dof.png')
        _capture_sync(world, rt_lens_cam, rt_lens_path)
        _capture_sync(world, wide_cam, wide_path)

        import cv2
        rt_gray = cv2.imread(rt_lens_path, cv2.IMREAD_GRAYSCALE)
        wide_gray = cv2.imread(wide_path, cv2.IMREAD_GRAYSCALE)
        assert rt_gray is not None and wide_gray is not None

        rt_scores = _sharpness_map(rt_gray)
        wide_scores = _sharpness_map(wide_gray)

        wide_spread = (max(wide_scores) - min(wide_scores)) / (max(wide_scores) + 1e-9)
        assert wide_spread < 0.15, (
            'wide_angle should be uniformly in-focus everywhere, got band scores %r '
            '(relative spread %.3f)' % (wide_scores, wide_spread))

        rt_spread = (max(rt_scores) - min(rt_scores)) / (max(rt_scores) + 1e-9)
        assert rt_spread > wide_spread, (
            'rt_lens should show a measurable sharpness falloff away from '
            'focus_distance_m=%.1f that wide_angle does not; rt band scores %r, '
            'wide band scores %r' % (lens_params['focus_distance_m'], rt_scores, wide_scores))
    finally:
        for actor in list(locals().get('targets', [])) + [locals().get('rt_lens_cam'), locals().get('wide_cam')]:
            if actor is not None:
                actor.destroy()


def build_arg_parser():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--host', default='127.0.0.1')
    parser.add_argument('-p', '--port', type=int, default=2000)
    parser.add_argument('--feature-tolerance-px', type=float, default=DEFAULT_FEATURE_TOLERANCE_PX)
    return parser


if __name__ == '__main__':
    args = build_arg_parser().parse_args()
    if not _server_reachable(args.host, args.port):
        raise SystemExit('no CARLA server reachable at %s:%s' % (args.host, args.port))
    print('server reachable; run this file under pytest for the actual A/B checks:\n'
          '  pytest test_ab_wideangle.py --host %s --port %d' % (args.host, args.port))
