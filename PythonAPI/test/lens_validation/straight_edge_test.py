#!/usr/bin/env python

# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""World-space straight-line test: a real straight edge, imaged through a
distorting lens model, must trace out the exact analytic curve predicted by
lens_math_reference -- not an approximation. This is specifically the test a
vertex-displacement (mesh-warp / UV-shader) approximation of lens distortion
would fail: a vertex-displacement approach only bends geometry at mesh
vertices, so a straight edge crossing a low-tessellation region renders as a
sequence of straight facets, each individually straight, deviating from the
true continuously-curved locus between vertices. Per-pixel analytic
projection (what rt_lens does) does not have that failure mode.

Two ways to run:

1. SYNTHETIC (offline, runs now, no CARLA/server needed):
       pytest straight_edge_test.py -k synthetic
   Builds a straight line directly in camera space, projects it through
   lens_math_reference.LensModel, and checks:
     a) zero-distortion models (perspective, brown_conrady with all-zero
        coeffs) image a straight line to an exactly straight line.
     b) models with real distortion image it to a measurably CURVED locus
        (nonzero max deviation from the best-fit straight line) -- i.e.
        this file's own curve-generation math is doing something a
        vertex-displacement approximation could not reproduce exactly.

2. REAL (server-dependent, skipped until a server + straight-edge asset
   exist):
       python straight_edge_test.py --host ... --p0 X Y Z --p1 X Y Z ...
   Spawns a straight-edge prop between two known world points, captures it
   with the configured rt_lens camera, extracts the imaged edge pixels
   (color threshold), and compares them to the analytic curve computed by
   transforming the same two world points through the camera's exact
   `carla.Transform` and lens_math_reference.LensModel.project.

Asset requirement (for mode 2): a thin, brightly (unlit, single-color)
textured static mesh blueprint that can be scaled/rotated to span two given
world points, tried in this order:
    static.prop.straightedge
    static.prop.streetbarrier
Add your id to STRAIGHT_EDGE_BLUEPRINT_CANDIDATES if you use something else.
"""

import argparse
import socket

import numpy as np
import pytest

import lens_math_reference as lm

try:
    import carla
except ImportError:
    carla = None


STRAIGHT_EDGE_BLUEPRINT_CANDIDATES = [
    'static.prop.straightedge',
    'static.prop.streetbarrier',
]

RT_LENS_SENSOR_ID = 'sensor.camera.rt_lens'

DEFAULT_CURVE_TOLERANCE_PX = 0.5


# ---------------------------------------------------------------------------
# Shared curve math (used by both the synthetic self-test and the real path)
# ---------------------------------------------------------------------------

def analytic_image_curve(model, points_camera_space):
    """points_camera_space: (N, 3) points already in camera-local space
    (+Z forward). Returns (u, v) arrays, the exact per-point projection --
    this is the ground-truth curve any renderer (vertex-displacement or
    per-pixel-analytic) is being checked against."""
    return model.project(points_camera_space)


def max_deviation_from_best_fit_line(u, v):
    """Fits a straight line to (u, v) via total least squares (first
    principal component) and returns the max perpendicular deviation of
    any point from that line -- i.e. how far from "straight" the curve is.
    Used to (a) confirm zero-distortion curves ARE straight, and (b)
    confirm distorted curves are NOT."""
    pts = np.stack([u, v], axis=-1)
    centroid = pts.mean(axis=0)
    centered = pts - centroid
    _, _, vt = np.linalg.svd(centered, full_matrices=False)
    direction = vt[0]              # unit vector along the best-fit line
    normal = np.array([-direction[1], direction[0]])
    perpendicular_offsets = centered @ normal
    return float(np.max(np.abs(perpendicular_offsets)))


def curve_deviation_px(detected_uv, analytic_uv):
    """Nearest-point (per detected sample, closest analytic sample)
    deviation in the same units as u/v -- caller multiplies by image size
    to convert to pixels if u/v are normalized."""
    analytic = np.stack(analytic_uv, axis=-1)
    detected = np.asarray(detected_uv)
    deviations = np.empty(len(detected))
    for i, p in enumerate(detected):
        deviations[i] = np.min(np.hypot(analytic[:, 0] - p[0], analytic[:, 1] - p[1]))
    return deviations


# ---------------------------------------------------------------------------
# 1. SYNTHETIC self-test (offline, runs now)
# ---------------------------------------------------------------------------

def _camera_space_line(z_depth=5.0, x_range=(-3.0, 3.0), y_offset=0.4, n=400):
    """A straight, off-axis 3D line at fixed depth -- off-axis so it isn't
    trivially radially symmetric (which would make curvature harder to
    distinguish from a pure scale change)."""
    x = np.linspace(x_range[0], x_range[1], n)
    y = np.full_like(x, y_offset)
    z = np.full_like(x, z_depth)
    return np.stack([x, y, z], axis=-1)


def test_synthetic_zero_distortion_is_straight():
    line = _camera_space_line()
    for camera_model, coeffs in [
        ('perspective', []),
        ('brown_conrady', [0.0, 0.0, 0.0, 0.0, 0.0]),
    ]:
        model = lm.LensModel(camera_model=camera_model, fx=0.9, fy=0.85, cx=0.5, cy=0.5,
                              distortion_coeffs=coeffs, theta_max_deg=80.0)
        u, v = analytic_image_curve(model, line)
        deviation = max_deviation_from_best_fit_line(u, v)
        assert deviation < 1e-9, (
            'camera_model=%s should image a straight line to a straight line, '
            'got max deviation %.3e' % (camera_model, deviation))


def test_synthetic_distorted_is_measurably_curved():
    line = _camera_space_line()
    cases = [
        ('equidistant', []),
        ('stereographic', []),
        ('kannala_brandt', [1.0, -0.05, 0.01, -0.002]),
        ('brown_conrady', [-0.18, 0.045, 0.0008, -0.0006, -0.004]),
    ]
    for camera_model, coeffs in cases:
        model = lm.LensModel(camera_model=camera_model, fx=0.9, fy=0.85, cx=0.5, cy=0.5,
                              distortion_coeffs=coeffs, theta_max_deg=100.0)
        u, v = analytic_image_curve(model, line)
        deviation = max_deviation_from_best_fit_line(u, v)
        # A vertex-displacement approximation with a modest tessellation
        # (say, mesh edges every ~10% of the line's angular extent) would
        # linearly interpolate between correctly-placed vertices and could
        # therefore end up within a fraction of a pixel of "straight" over
        # any short segment even though the true curve is not; the
        # threshold here is deliberately well above typical sub-pixel
        # tessellation error so this test is checking for gross curvature,
        # not micro-faceting -- straight_edge_test's REAL-capture path
        # (mode 2) is what catches the fine-grained tessellation failure
        # against actual rendered pixels.
        assert deviation > 1e-4, (
            'camera_model=%s expected measurable curvature (distortion should bend a '
            'straight line), got max deviation %.3e (looks straight)' % (camera_model, deviation))


def test_curve_deviation_px_helper_self_consistent():
    """Sanity check on curve_deviation_px itself: sampling the SAME
    analytic curve at a much finer rate than the "detected" points and
    comparing should give ~0 deviation (this is what a "perfect" real
    capture would look like -- detected pixels landing exactly on the
    analytic curve). A curved locus sampled at finite density has an
    inherent nearest-neighbor gap, so the tolerance here reflects that
    sampling density, not an expectation of exact coincidence."""
    model = lm.LensModel(camera_model='equidistant', fx=0.9, fy=0.85, cx=0.5, cy=0.5,
                          distortion_coeffs=[], theta_max_deg=100.0)
    line = _camera_space_line(n=20000)
    u_fine, v_fine = analytic_image_curve(model, line)

    coarse_line = _camera_space_line(n=17)
    u_coarse, v_coarse = analytic_image_curve(model, coarse_line)
    detected = np.stack([u_coarse, v_coarse], axis=-1)

    deviations = curve_deviation_px(detected, (u_fine, v_fine))
    assert np.max(deviations) < 5e-5


# ---------------------------------------------------------------------------
# 2. REAL capture-driven check (server-dependent; skips if unreachable)
# ---------------------------------------------------------------------------

def _server_reachable(host, port, timeout_s=1.5):
    try:
        with socket.create_connection((host, port), timeout=timeout_s):
            return True
    except OSError:
        return False


def extract_edge_pixels_by_color(image_bgr, target_bgr, tolerance=30):
    """Per-row centroid of pixels matching target_bgr within `tolerance`
    (per channel). Returns an (M, 2) array of (u, v) pixel centers, one per
    row that had a match. Simple and robust for a single-color unlit prop
    against a contrasting background; swap for a Hough-line fit if the
    real asset needs it."""
    import cv2
    diff = np.abs(image_bgr.astype(np.int16) - np.array(target_bgr, dtype=np.int16))
    mask = np.all(diff <= tolerance, axis=-1)
    ys, xs = np.nonzero(mask)
    if len(xs) == 0:
        return np.empty((0, 2))
    points = []
    for row in np.unique(ys):
        row_xs = xs[ys == row]
        points.append((float(np.mean(row_xs)), float(row)))
    return np.array(points)


@pytest.fixture
def carla_world_for_edge(request):
    host = request.config.getoption('--host', default='127.0.0.1')
    port = request.config.getoption('--port', default=2000)
    if carla is None:
        pytest.skip('carla PythonAPI module is not importable in this environment')
    if not _server_reachable(host, port):
        pytest.skip('no CARLA server reachable at %s:%s' % (host, port))
    client = carla.Client(host, port)
    client.set_timeout(10.0)
    world = client.get_world()
    if len(world.get_blueprint_library().filter(RT_LENS_SENSOR_ID)) == 0:
        pytest.skip('%s not present -- rt_lens migration build not loaded yet' % RT_LENS_SENSOR_ID)
    return world


def test_straight_edge_matches_analytic_curve(carla_world_for_edge, tmp_path, request):
    world = carla_world_for_edge
    bp_lib = world.get_blueprint_library()
    edge_bp = None
    for candidate in STRAIGHT_EDGE_BLUEPRINT_CANDIDATES:
        found = bp_lib.filter(candidate)
        if len(found) > 0:
            edge_bp = found[0]
            break
    if edge_bp is None:
        pytest.skip('no straight-edge blueprint available (see module docstring)')

    p0 = carla.Location(x=6.0, y=-2.0, z=1.5)
    p1 = carla.Location(x=6.0, y=2.0, z=1.5)
    camera_transform = carla.Transform(carla.Location(x=0.0, y=0.0, z=1.5))

    edge_center = carla.Location(
        x=(p0.x + p1.x) / 2.0, y=(p0.y + p1.y) / 2.0, z=(p0.z + p1.z) / 2.0)
    edge_actor = world.spawn_actor(edge_bp, carla.Transform(edge_center))

    lens_params = dict(camera_model='equidistant', fx=0.9, fy=0.85, cx=0.5, cy=0.5,
                        image_size_x=1920, image_size_y=1080)
    camera_bp = bp_lib.filter(RT_LENS_SENSOR_ID)[0]
    for key, value in lens_params.items():
        if camera_bp.has_attribute(key):
            camera_bp.set_attribute(key, str(value))
    camera = world.spawn_actor(camera_bp, camera_transform)

    try:
        out_path = str(tmp_path / 'straight_edge.png')
        captured = {}

        def _on_image(image):
            image.save_to_disk(out_path)
            captured['done'] = True

        camera.listen(_on_image)
        settings = world.get_settings()
        for _ in range(3):
            world.tick() if settings.synchronous_mode else world.wait_for_tick()
        camera.stop()

        import cv2
        image_bgr = cv2.imread(out_path, cv2.IMREAD_COLOR)
        assert image_bgr is not None

        detected = extract_edge_pixels_by_color(image_bgr, target_bgr=(0, 0, 255))
        assert len(detected) > 10, 'straight-edge prop not detected in captured image'

        world_to_camera = np.array(camera_transform.get_inverse_matrix())
        n_samples = 400
        t = np.linspace(0.0, 1.0, n_samples)
        world_points = np.stack([
            p0.x + t * (p1.x - p0.x),
            p0.y + t * (p1.y - p0.y),
            p0.z + t * (p1.z - p0.z),
            np.ones(n_samples),
        ], axis=-1)
        camera_points = (world_to_camera @ world_points.T).T[:, :3]

        model = lm.LensModel(**{k: v for k, v in lens_params.items() if k != 'image_size_x' and k != 'image_size_y'},
                              distortion_coeffs=[])
        u, v = analytic_image_curve(model, camera_points)
        u_px = u * lens_params['image_size_x']
        v_px = v * lens_params['image_size_y']

        deviations = curve_deviation_px(detected, (u_px, v_px))
        tol = request.config.getoption('--curve-tolerance-px', default=DEFAULT_CURVE_TOLERANCE_PX)
        assert np.max(deviations) < tol, (
            'max detected-vs-analytic curve deviation = %.2fpx (tol=%.2fpx); a '
            'vertex-displacement lens approximation would show large deviations here '
            'even though it might pass simpler tests' % (np.max(deviations), tol))
    finally:
        camera.destroy()
        edge_actor.destroy()


def build_arg_parser():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--host', default='127.0.0.1')
    parser.add_argument('-p', '--port', type=int, default=2000)
    parser.add_argument('--curve-tolerance-px', type=float, default=DEFAULT_CURVE_TOLERANCE_PX)
    return parser


if __name__ == '__main__':
    args = build_arg_parser().parse_args()
    if not _server_reachable(args.host, args.port):
        raise SystemExit('no CARLA server reachable at %s:%s' % (args.host, args.port))
    print('server reachable; run this file under pytest for the actual check:\n'
          '  pytest straight_edge_test.py --host %s --port %d' % (args.host, args.port))
