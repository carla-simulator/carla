"""Occlusion filter: the z-buffer test, and the track surgery that follows from it.

Pure numpy — synthetic depth images and hand-built camera manifests, no CARLA server and no
clip on disk.  The camera is the one the exporter writes into ``calibration_estimate``
(f-theta fitted to the pinhole), so these are the same projections the real filter runs.
"""

import numpy as np
import pytest

from carla_cosmos.contracts import CameraManifest
from carla_cosmos.visibility import (ABSENT, BOX_GRID, VisibilityParams, apply_hysteresis,
                                     box_points_from_poses, box_sample_points, camera_from_manifest,
                                     camera_visibility, keep_segments, segment_ids, segments,
                                     stack_sample_points, visible_fractions)

W, H = 640, 360
FAR = 1000.0
"""Depth of "nothing there": CARLA's depth AOV saturates at 1000 m."""


def manifest(name="camera:front:wide:120fov", hfov=90.0, t=(0.0, 0.0, 0.0), rpy=(0.0, 0.0, 0.0)):
    return CameraManifest(name=name, hfov=hfov, width=W, height=H, lens="pinhole",
                          t_flu=list(t), rpy_flu=list(rpy),
                          attach_ue={"x": 0.0, "y": 0.0, "z": 0.0, "pitch": 0.0, "yaw": 0.0, "roll": 0.0},
                          shifted=False)


def depth_image(value=FAR):
    return np.full((H, W), float(value), np.float32)


def car(x=20.0, y=0.0, z=0.0, size=(4.0, 2.0, 1.5)):
    """One box straight ahead of the rig, axis aligned: ``(centre, size, quaternion)``."""
    return ([x, y, z], list(size), [0.0, 0.0, 0.0, 1.0])


def points_of(*boxes):
    return stack_sample_points(boxes)


# ----------------------------------------------------------------------------- sampling

def test_the_lattice_contains_the_corners_and_the_centre():
    pts = box_sample_points([0, 0, 0], [4, 2, 1], [0, 0, 0, 1])
    assert pts.shape == (27, 3)
    assert (pts == 0).all(axis=1).sum() == 1, "the centre is one of the samples"
    corners = pts[np.abs(pts).max(axis=1) > 0][np.all(np.abs(pts[np.abs(pts).max(axis=1) > 0])
                                                      == np.array([2.0, 1.0, 0.5]), axis=1)]
    assert len(corners) == 8


def test_the_vectorised_and_the_quaternion_sampler_agree():
    R = np.eye(3)[None]
    a = box_points_from_poses(R, np.array([[5.0, 1.0, 0.0]]), np.array([[4.0, 2.0, 1.5]]))
    b = box_sample_points([5.0, 1.0, 0.0], [4.0, 2.0, 1.5], [0.0, 0.0, 0.0, 1.0])
    assert np.allclose(a[0], b)


# ----------------------------------------------------------------------------- the z-buffer test

def test_a_box_in_front_of_the_wall_is_visible():
    cam = camera_from_manifest(manifest())
    depth = depth_image(60.0)  # a wall 60 m away; the car is at 20 m
    n_vis, n_in = camera_visibility(points_of(car()), cam.world_to_camera(np.eye(4)), cam, depth)
    assert n_in[0] == 27
    assert n_vis[0] == 27


def test_a_box_behind_the_wall_is_invisible():
    cam = camera_from_manifest(manifest())
    depth = depth_image(8.0)  # a building 8 m away, the car 20 m behind it
    n_vis, n_in = camera_visibility(points_of(car()), cam.world_to_camera(np.eye(4)), cam, depth)
    assert n_in[0] == 27
    assert n_vis[0] == 0


def test_the_tolerance_lets_the_objects_own_surface_count_as_visible():
    """The samples sit on the bounding box, the depth image sees the mesh a little in front of
    it; only a surface clearly nearer than the box is an occluder."""
    cam = camera_from_manifest(manifest())
    params = VisibilityParams()
    flat = points_of(car(x=20.0, size=(0.02, 2.0, 1.5)))  # every sample at ~20 m, one depth to reason about
    just_inside = 20.0 - params.tol_m - params.tol_rel * 20.0 + 0.05
    n_vis, _ = camera_visibility(flat, cam.world_to_camera(np.eye(4)), cam,
                                 depth_image(just_inside), params)
    assert n_vis[0] == 27
    n_vis, _ = camera_visibility(flat, cam.world_to_camera(np.eye(4)), cam,
                                 depth_image(just_inside - 0.5), params)
    assert n_vis[0] == 0


def test_a_half_covered_box_reports_the_covered_fraction():
    """The lattice has three ``y`` slices; +y is to the *left* of the image, so an occluder over
    the left half hides exactly the nine points of the ``y = +1 m`` slice."""
    cam = camera_from_manifest(manifest())
    depth = depth_image(FAR)
    depth[:, :W // 2 - 5] = 4.0
    n_vis, n_in = camera_visibility(points_of(car()), cam.world_to_camera(np.eye(4)), cam, depth)
    assert n_in[0] == 27
    assert n_vis[0] == 18
    assert n_vis[0] / n_in[0] == pytest.approx(2 / 3)


def test_points_outside_the_frame_do_not_count_as_occluded():
    """Half a car off the side of the image is judged on the half that is on it."""
    cam = camera_from_manifest(manifest())
    depth = depth_image(FAR)
    # 90 deg fov at x = 4 m: the right edge of the frame is at y = -4 m
    n_vis, n_in = camera_visibility(points_of(car(x=4.0, y=-4.0)), cam.world_to_camera(np.eye(4)),
                                    cam, depth)
    assert 0 < n_in[0] < 27
    assert n_vis[0] == n_in[0]


def test_the_range_cap_drops_far_obstacles():
    cam = camera_from_manifest(manifest())
    depth = depth_image(FAR)
    params = VisibilityParams(range_m=100.0)
    n_vis, n_in = camera_visibility(points_of(car(x=200.0, size=(4, 2, 1.5))),
                                    cam.world_to_camera(np.eye(4)), cam, depth, params)
    assert n_in[0] == 0 and n_vis[0] == 0


def test_a_box_behind_the_camera_is_never_visible():
    cam = camera_from_manifest(manifest())
    n_vis, n_in = camera_visibility(points_of(car(x=-20.0)), cam.world_to_camera(np.eye(4)),
                                    cam, depth_image())
    assert n_in[0] == 0 and n_vis[0] == 0


# ----------------------------------------------------------------------------- union over cameras

def _rear_camera():
    return manifest(name="camera:rear:tele:30fov", hfov=90.0, rpy=(0.0, 0.0, 180.0))


def test_one_decision_per_object_unions_the_cameras():
    """ClipGT tracks are global: an obstacle the front camera cannot see but the rear one can is
    exported once, as visible."""
    front, rear = camera_from_manifest(manifest()), camera_from_manifest(_rear_camera())
    behind = points_of(car(x=-20.0))
    depths = {front.name: depth_image(), rear.name: depth_image()}
    assert visible_fractions(behind, np.eye(4), [front], depths)[0] == -1.0
    assert visible_fractions(behind, np.eye(4), [front, rear], depths)[0] == 1.0


def test_out_of_frame_is_reported_as_minus_one_not_as_occluded():
    front = camera_from_manifest(manifest())
    far_left = points_of(car(x=1.0, y=60.0))
    assert visible_fractions(far_left, np.eye(4), [front], {front.name: depth_image()})[0] == -1.0


def test_the_best_camera_decides():
    front, rear = camera_from_manifest(manifest()), camera_from_manifest(_rear_camera())
    boxes = points_of(car(x=20.0))
    depths = {front.name: depth_image(3.0), rear.name: depth_image()}  # front blocked, rear clear
    assert visible_fractions(boxes, np.eye(4), [front], depths)[0] == 0.0
    # the rear camera looks the other way, so it does not see it either: still 0, not -1
    assert visible_fractions(boxes, np.eye(4), [front, rear], depths)[0] == 0.0


def test_a_camera_without_a_depth_image_is_skipped():
    front = camera_from_manifest(manifest())
    assert visible_fractions(points_of(car()), np.eye(4), [front], {})[0] == -1.0


def test_no_boxes_is_not_an_error():
    front = camera_from_manifest(manifest())
    assert visible_fractions(np.zeros((0, 27, 3)), np.eye(4), [front],
                             {front.name: depth_image()}).shape == (0,)


# ----------------------------------------------------------------------------- decisions over time

def test_hysteresis_bridges_a_short_dropout():
    states = np.array([1, 1, 1, 0, 1, 1, 1], np.int8)
    assert apply_hysteresis(states, 3).tolist() == [1, 1, 1, 1, 1, 1, 1]


def test_hysteresis_leaves_a_long_dropout_alone():
    states = np.array([1, 1, 0, 0, 0, 1, 1], np.int8)
    assert apply_hysteresis(states, 3).tolist() == [1, 1, 0, 0, 0, 1, 1]


def test_hysteresis_does_not_invent_visibility_at_the_ends():
    """There is nothing on the other side of a leading or trailing gap to interpolate from."""
    states = np.array([0, 1, 1, 1, 0], np.int8)
    assert apply_hysteresis(states, 3).tolist() == [0, 1, 1, 1, 0]


def test_hysteresis_never_bridges_an_absent_actor():
    states = np.array([1, 1, ABSENT, 1, 1], np.int8)
    assert apply_hysteresis(states, 3).tolist() == [1, 1, ABSENT, 1, 1]


def test_hysteresis_is_off_at_zero_and_one():
    states = np.array([1, 0, 1], np.int8)
    assert apply_hysteresis(states, 0).tolist() == [1, 0, 1]
    assert apply_hysteresis(states, 1).tolist() == [1, 0, 1]


def test_segments_are_the_visible_runs():
    assert segments(np.array([0, 1, 1, 0, 0, 1, 1, 1], np.int8)) == [(1, 3), (5, 8)]
    assert segments(np.array([ABSENT, 1, ABSENT], np.int8)) == [(1, 2)]


def test_short_segments_are_dropped_because_the_loader_skips_them():
    """NVIDIA's ``_load_dynamic_objects`` skips any track with fewer than two observations."""
    assert keep_segments(np.array([1, 0, 1, 1], np.int8)) == [(2, 4)]


def test_an_uninterrupted_track_keeps_its_plain_id():
    assert segment_ids("42", 1) == ["42"]


def test_a_split_track_gets_one_id_per_segment():
    """Without this the loader interpolates the box straight across the hole, because it fills
    any timestamp between a track's first and last observation."""
    assert segment_ids("42", 3) == ["42#0", "42#1", "42#2"]


# ----------------------------------------------------------------------------- parameters

def test_the_parameters_round_trip_through_the_manifest():
    p = VisibilityParams(mode="depth", min_visible_fraction=0.2, range_m=80.0, hysteresis_frames=5)
    assert VisibilityParams.from_dict(p.as_dict()) == p


def test_bad_parameters_are_rejected():
    for kwargs in ({"mode": "lidar"}, {"min_visible_fraction": 1.5}, {"range_m": 0.0},
                   {"hysteresis_frames": -1}, {"grid": 1}):
        with pytest.raises(ValueError):
            VisibilityParams(**kwargs)


def test_none_disables_the_filter():
    assert not VisibilityParams(mode="none").enabled
    assert VisibilityParams().enabled


def test_a_finer_lattice_is_still_a_lattice():
    p = VisibilityParams(grid=4)
    assert p.lattice().shape == (64, 3)
    assert VisibilityParams(grid=3).lattice() is BOX_GRID


# ----------------------------------------------------------------------------- capture wiring

def test_a_capture_filters_by_default():
    from carla_cosmos import Capture, Rig

    cap = Capture(None, None, Rig.single(), frames=4, fps=30)
    assert cap.visibility.mode == "depth"
    assert cap.visibility.as_dict()["min_visible_fraction"] == 0.05


def test_a_capture_without_the_depth_aov_turns_the_filter_off():
    """The test has nothing to test against, so it says so instead of silently passing everything."""
    from carla_cosmos import Capture, Rig

    cap = Capture(None, None, Rig.single(), frames=4, fps=30, aovs=("rgb", "semantic"))
    assert cap.visibility.mode == "none"


def test_capture_parameters_reach_the_filter():
    from carla_cosmos import Capture, Rig

    cap = Capture(None, None, Rig.single(), frames=4, fps=30, visibility="depth",
                  min_visible_fraction=0.2, range_m=60.0, hysteresis_frames=5)
    assert (cap.visibility.min_visible_fraction, cap.visibility.range_m,
            cap.visibility.hysteresis_frames) == (0.2, 60.0, 5)
