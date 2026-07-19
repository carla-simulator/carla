#!/usr/bin/env python

# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""CARLA client script: spawns a checkerboard/ChArUco calibration target and
captures the sensor.camera.rt_lens camera at N known poses around it.

SERVER-DEPENDENT. This needs a running CARLA server (the rt_lens migration
build). It is import-clean and argument-parsed today; the `import carla`
dependency is loaded lazily (see `_require_carla`) so the pattern-generation
helpers below can still be unit-tested without carla installed.

Asset requirement
------------------
This script spawns a *flat static target actor* and expects to find a
blueprint for it in the CARLA blueprint library, tried in this order:
    static.prop.calibrationboard
    static.prop.checkerboard
    static.prop.chessboard
None of these ship with CARLA today. To use this script:
  1. Generate the pattern PNG with `generate_checkerboard_texture()` or
     `generate_charuco_texture()` below (both run offline, no CARLA needed).
  2. Import it into UE5 as an unlit, non-metallic Material and apply it to a
     simple flat plane Static Mesh (e.g. a 2m x 1.5m quad with the pattern
     covering the full UV rect, matching BOARD_ROWS/BOARD_COLS/SQUARE_SIZE_M
     below so image-space corners map to known physical spacing).
  3. Register that plane as a Blueprint under one of the ids above (or add
     your id to CALIBRATION_TARGET_BLUEPRINT_CANDIDATES).
  4. Place the plane's origin at its geometric center so the pose math here
     (which treats the target's `carla.Transform` as the board center) is
     correct.

Output layout
-------------
For each pose N, writes:
    <out_dir>/images/view_%03d.png
and a single manifest:
    <out_dir>/manifest.json
containing, per view: image path, the camera's configured lens params, and
each detected-corner's ground-truth 3D position *in camera space* (computed
from CARLA's exact ground-truth transforms, not from PnP) so
test_calibration_refit.py can fit intrinsics without also having to solve
for extrinsics.
"""

import argparse
import json
import math
import os
import sys

import numpy as np

try:
    import carla  # noqa: F401  (only touched inside functions that need it)
except ImportError:
    carla = None


def _require_carla():
    if carla is None:
        raise RuntimeError(
            'the `carla` PythonAPI module is not importable in this '
            'environment. Build/install it (see PythonAPI/carla) and make '
            'sure it is on PYTHONPATH, then re-run this script against a '
            'running CARLA server.')


CALIBRATION_TARGET_BLUEPRINT_CANDIDATES = [
    'static.prop.calibrationboard',
    'static.prop.checkerboard',
    'static.prop.chessboard',
]

RT_LENS_SENSOR_ID = 'sensor.camera.rt_lens'

# Physical board spec the offline pattern generators and the pose math agree on.
BOARD_ROWS = 7          # inner corners, short side
BOARD_COLS = 10         # inner corners, long side
SQUARE_SIZE_M = 0.08    # physical square edge length in meters


# ---------------------------------------------------------------------------
# Offline pattern generation (no CARLA needed) -- produces the texture PNG
# the asset step (step 2 above) needs.
# ---------------------------------------------------------------------------

def generate_checkerboard_texture(out_path, rows=BOARD_ROWS, cols=BOARD_COLS,
                                   square_px=120, margin_squares=1):
    """rows/cols are INNER corner counts (cv2.findChessboardCorners
    convention); the drawn board therefore has (rows+1) x (cols+1) squares
    plus a margin so all inner corners are unambiguous."""
    import cv2
    n_rows_sq = rows + 1
    n_cols_sq = cols + 1
    h = (n_rows_sq + 2 * margin_squares) * square_px
    w = (n_cols_sq + 2 * margin_squares) * square_px
    img = np.full((h, w), 255, dtype=np.uint8)
    for r in range(n_rows_sq):
        for c in range(n_cols_sq):
            if (r + c) % 2 == 0:
                y0 = (r + margin_squares) * square_px
                x0 = (c + margin_squares) * square_px
                img[y0:y0 + square_px, x0:x0 + square_px] = 0
    cv2.imwrite(out_path, img)
    return out_path


def generate_charuco_texture(out_path, rows=BOARD_ROWS + 1, cols=BOARD_COLS + 1,
                              square_px=120, marker_ratio=0.7,
                              dictionary_id=None):
    """ChArUco alternative (more robust partial-view detection). Requires
    cv2.aruco (opencv-contrib)."""
    import cv2
    if not hasattr(cv2, 'aruco'):
        raise RuntimeError('cv2.aruco is not available (need opencv-contrib-python)')
    if dictionary_id is None:
        dictionary_id = cv2.aruco.DICT_5X5_1000
    aruco_dict = cv2.aruco.getPredefinedDictionary(dictionary_id)
    board = cv2.aruco.CharucoBoard(
        (cols, rows), SQUARE_SIZE_M, SQUARE_SIZE_M * marker_ratio, aruco_dict)
    img = board.generateImage((cols * square_px, rows * square_px))
    cv2.imwrite(out_path, img)
    return out_path


# ---------------------------------------------------------------------------
# Pose sampling
# ---------------------------------------------------------------------------

def generate_poses(n, radius_m, target_location, elevation_range_deg=(10.0, 70.0),
                    azimuth_range_deg=(0.0, 360.0), seed=0xC0FFEE):
    """Returns a list of (location_xyz, rotation_pyr_deg) camera poses on a
    sphere of `radius_m` around target_location, each oriented to look at
    the target center. rotation_pyr_deg = (pitch, yaw, roll) matching
    carla.Rotation field order.

    Deterministic (seeded) jittered sampling: gives good coverage of the
    calibration target from varied angles/distances without requiring a
    fixed grid, which is what a real calibration data collection wants
    (per OpenCV calibration best practice: vary distance and tilt).
    """
    rng = np.random.default_rng(seed)
    az = np.deg2rad(rng.uniform(azimuth_range_deg[0], azimuth_range_deg[1], size=n))
    el = np.deg2rad(rng.uniform(elevation_range_deg[0], elevation_range_deg[1], size=n))
    r = rng.uniform(0.6, 1.0, size=n) * radius_m

    tx, ty, tz = target_location
    poses = []
    for i in range(n):
        x = tx + r[i] * np.cos(el[i]) * np.cos(az[i])
        y = ty + r[i] * np.cos(el[i]) * np.sin(az[i])
        z = tz + r[i] * np.sin(el[i])
        dx, dy, dz = tx - x, ty - y, tz - z
        yaw = math.degrees(math.atan2(dy, dx))
        pitch = math.degrees(math.atan2(dz, math.hypot(dx, dy)))
        roll = 0.0
        poses.append(((x, y, z), (pitch, yaw, roll)))
    return poses


# ---------------------------------------------------------------------------
# CARLA scene interaction (needs a live server)
# ---------------------------------------------------------------------------

class CalibrationScene(object):
    def __init__(self, host='127.0.0.1', port=2000, timeout_s=10.0):
        _require_carla()
        self.client = carla.Client(host, port)
        self.client.set_timeout(timeout_s)
        self.world = self.client.get_world()
        self.target_actor = None
        self.camera_actor = None

    def find_target_blueprint(self):
        bp_lib = self.world.get_blueprint_library()
        for candidate in CALIBRATION_TARGET_BLUEPRINT_CANDIDATES:
            found = bp_lib.filter(candidate)
            if len(found) > 0:
                return found[0]
        raise RuntimeError(
            'no calibration-target blueprint found (tried %s). See the '
            'module docstring for the asset this script expects.' %
            CALIBRATION_TARGET_BLUEPRINT_CANDIDATES)

    def spawn_target(self, location_xyz):
        bp = self.find_target_blueprint()
        transform = carla.Transform(
            carla.Location(*location_xyz), carla.Rotation(0.0, 0.0, 0.0))
        self.target_actor = self.world.spawn_actor(bp, transform)
        return self.target_actor

    def spawn_camera(self, attributes):
        bp_lib = self.world.get_blueprint_library()
        found = bp_lib.filter(RT_LENS_SENSOR_ID)
        if len(found) == 0:
            raise RuntimeError(
                '%s blueprint not found -- server does not have the rt_lens '
                'migration build loaded yet.' % RT_LENS_SENSOR_ID)
        bp = found[0]
        for key, value in attributes.items():
            if bp.has_attribute(key):
                bp.set_attribute(key, str(value))
            else:
                raise RuntimeError('%s has no attribute %r' % (RT_LENS_SENSOR_ID, key))
        # spawn detached; pose is set per-view via set_transform below
        self.camera_actor = self.world.spawn_actor(bp, carla.Transform())
        return self.camera_actor

    def capture_view(self, location_xyz, rotation_pyr_deg, out_path, sync_ticks=3):
        transform = carla.Transform(
            carla.Location(*location_xyz),
            carla.Rotation(pitch=rotation_pyr_deg[0], yaw=rotation_pyr_deg[1],
                            roll=rotation_pyr_deg[2]))
        self.camera_actor.set_transform(transform)

        captured = {}

        def _on_image(image):
            image.save_to_disk(out_path)
            captured['frame'] = image.frame

        listen_handle = self.camera_actor.listen(_on_image)
        for _ in range(sync_ticks):
            self.world.tick() if self.world.get_settings().synchronous_mode else self.world.wait_for_tick()
        self.camera_actor.stop()
        return transform, captured.get('frame')

    def board_corners_camera_space(self, camera_transform):
        """Ground-truth 3D position of every inner checkerboard corner,
        expressed in the camera's local space, using CARLA's exact
        transforms (no PnP / no pixel measurement involved). Returns an
        array of shape (BOARD_ROWS*BOARD_COLS, 3)."""
        board_to_world = self.target_actor.get_transform()
        world_to_camera = camera_transform.get_inverse_matrix()

        pts = []
        # Inner corners are offset by one square from the board's outer
        # edge; board-local origin is the plane's center (see docstring).
        half_w = (BOARD_COLS) * SQUARE_SIZE_M / 2.0
        half_h = (BOARD_ROWS) * SQUARE_SIZE_M / 2.0
        for r in range(BOARD_ROWS):
            for c in range(BOARD_COLS):
                local_x = -half_w + (c + 0.5) * SQUARE_SIZE_M
                local_y = -half_h + (r + 0.5) * SQUARE_SIZE_M
                local_point = carla.Location(x=local_x, y=local_y, z=0.0)
                world_point = board_to_world.transform(local_point)
                m = np.array(world_to_camera)
                wp = np.array([world_point.x, world_point.y, world_point.z, 1.0])
                cam_point = m @ wp
                pts.append(cam_point[:3])
        return np.array(pts)

    def destroy(self):
        for actor in (self.camera_actor, self.target_actor):
            if actor is not None:
                actor.destroy()


def run_capture(args):
    scene = CalibrationScene(host=args.host, port=args.port)
    scene.spawn_target(args.target_location)

    attributes = {
        'camera_model': args.camera_model,
        'fx': args.fx, 'fy': args.fy, 'cx': args.cx, 'cy': args.cy,
        'image_size_x': args.image_size_x, 'image_size_y': args.image_size_y,
        'sensor_tick': 0.0,
    }
    if args.distortion_coeffs:
        attributes['distortion_coeffs'] = args.distortion_coeffs
    scene.spawn_camera(attributes)

    poses = generate_poses(args.num_poses, args.radius_m, args.target_location)

    images_dir = os.path.join(args.out_dir, 'images')
    os.makedirs(images_dir, exist_ok=True)
    manifest = {
        'camera_model': args.camera_model,
        'attributes': attributes,
        'board_rows': BOARD_ROWS, 'board_cols': BOARD_COLS,
        'square_size_m': SQUARE_SIZE_M,
        'views': [],
    }

    for i, (loc, rot) in enumerate(poses):
        image_path = os.path.join(images_dir, 'view_%03d.png' % i)
        transform, frame = scene.capture_view(loc, rot, image_path)
        corners_cam = scene.board_corners_camera_space(transform)
        manifest['views'].append({
            'image': os.path.relpath(image_path, args.out_dir),
            'frame': frame,
            'camera_location': loc,
            'camera_rotation_pyr_deg': rot,
            'corners_camera_space': corners_cam.tolist(),
        })

    with open(os.path.join(args.out_dir, 'manifest.json'), 'w') as f:
        json.dump(manifest, f, indent=2)

    scene.destroy()
    print('wrote %d views to %s' % (len(poses), args.out_dir))


def build_arg_parser():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--host', default='127.0.0.1')
    parser.add_argument('-p', '--port', type=int, default=2000)
    parser.add_argument('--out-dir', required=True,
                         help='directory to write images/ and manifest.json into')
    parser.add_argument('--camera-model', default='brown_conrady',
                         choices=['perspective', 'stereographic', 'equidistant',
                                  'equisolid', 'orthographic', 'kannala_brandt',
                                  'brown_conrady', 'lut'])
    parser.add_argument('--distortion-coeffs', default='',
                         help='csv floats, model-dependent length')
    parser.add_argument('--fx', type=float, default=1.0)
    parser.add_argument('--fy', type=float, default=1.0)
    parser.add_argument('--cx', type=float, default=0.5)
    parser.add_argument('--cy', type=float, default=0.5)
    parser.add_argument('--image-size-x', type=int, default=1920)
    parser.add_argument('--image-size-y', type=int, default=1080)
    parser.add_argument('--num-poses', type=int, default=20)
    parser.add_argument('--radius-m', type=float, default=3.0)
    parser.add_argument('--target-location', type=float, nargs=3,
                         default=[0.0, 0.0, 1.0], metavar=('X', 'Y', 'Z'))
    return parser


def main(argv=None):
    parser = build_arg_parser()
    args = parser.parse_args(argv)
    run_capture(args)


if __name__ == '__main__':
    sys.exit(main())
