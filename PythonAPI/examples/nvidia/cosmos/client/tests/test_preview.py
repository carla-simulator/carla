"""Ground-truth preview maths.  No CARLA, no clip on disk: a synthetic rig and a handful of points."""

import numpy as np
import pytest

from carla_cosmos.preview import (
    BOX_EDGES,
    LAYERS,
    Box,
    FThetaCamera,
    PreviewCamera,
    SceneGT,
    box_corners,
    cameras_from_rig_json,
    densify,
    draw_scene,
    euler_to_matrix,
    label_patch,
    parse_frames,
    polyline_segments,
    pose_matrix,
    split_tracks,
)

# The f-theta polynomial the AV rig exports (pixeldistance-to-angle, 120 fov).
POLYNOMIAL = "0 0.002727027851 -2.278943261e-07 -6.955316787e-09 1.040047606e-11 -4.899275118e-15"


def make_sensor(name="camera:front:wide:120fov", rpy=(0.0, 0.0, 0.0), t=(1.7, 0.0, 1.5)):
    return {
        "name": name,
        "properties": {"Model": "ftheta", "cx": 640.0, "cy": 360.0, "width": 1280, "height": 720,
                       "polynomial": POLYNOMIAL, "polynomial-type": "pixeldistance-to-angle"},
        "nominalSensor2Rig_FLU": {"t": list(t), "roll-pitch-yaw": list(rpy)},
    }


@pytest.fixture
def lens():
    return FThetaCamera.from_rig_sensor(make_sensor())


# ----------------------------------------------------------------------------- projection

def test_optical_axis_projects_to_the_principal_point(lens):
    uv = lens.project(np.array([[10.0, 0.0, 0.0]]))
    assert uv[0] == pytest.approx([lens.cx, lens.cy], abs=1e-6)


def test_left_is_left_and_up_is_up(lens):
    """Camera frame is FLU: +y is the left of the image (u < cx), +z is up (v < cy)."""
    left, up = lens.project(np.array([[10.0, 2.0, 0.0], [10.0, 0.0, 2.0]]))
    assert left[0] < lens.cx and left[1] == pytest.approx(lens.cy)
    assert up[1] < lens.cy and up[0] == pytest.approx(lens.cx)


def test_points_behind_the_camera_are_nan(lens):
    uv = lens.project(np.array([[-5.0, 1.0, 0.0], [0.0, 1.0, 0.0]]))
    assert np.isnan(uv).all()


def test_points_past_the_lens_range_are_nan(lens):
    assert lens.theta_max <= np.radians(89.0)
    assert np.isnan(lens.project(np.array([[0.01, 10.0, 0.0]]))).all()


def test_radius_grows_with_the_angle(lens):
    """The polynomial is inverted only over its monotonic part, so pixel radius rises with theta."""
    angles = np.radians([1, 5, 10, 20, 30, 40, 50])
    pts = np.stack([np.cos(angles), np.sin(angles), np.zeros_like(angles)], axis=1) * 10.0
    radii = lens.cx - lens.project(pts)[:, 0]
    assert np.all(np.diff(radii) > 0)


# ----------------------------------------------------------------------------- polylines

def test_densify_caps_the_segment_length():
    P = densify(np.array([[0.0, 0.0, 0.0], [10.0, 0.0, 0.0]]), step=0.5)
    assert len(P) == 21
    assert np.max(np.linalg.norm(np.diff(P, axis=0), axis=1)) <= 0.5 + 1e-9


def test_polyline_crossing_behind_the_camera_is_clipped_not_folded(lens):
    """A line running from in front of the camera to behind it must lose its tail, not wrap around."""
    coarse = np.array([[6.0, 1.0, 0.0], [-6.0, 1.0, 0.0]])
    assert list(polyline_segments(lens.project(coarse))) == []  # both ends: one visible, one NaN

    uv = lens.project(densify(coarse, 0.5))
    segments = list(polyline_segments(uv))
    assert 5 < len(segments) < len(uv) - 1  # part of the line drawn, part dropped
    finite = uv[~np.isnan(uv).any(axis=1)]
    assert np.all(finite[:, 0] < lens.cx)  # y = +1 is left of centre for every visible sample
    # walking towards the camera the samples march steadily left; a fold would send them back right
    assert np.all(np.diff(finite[:, 0]) < 0)


# ----------------------------------------------------------------------------- rig and poses

def test_euler_matrix_is_the_clipgt_xyz_convention():
    R = euler_to_matrix([0.0, 0.0, 90.0])
    assert R @ np.array([1.0, 0.0, 0.0]) == pytest.approx([0.0, 1.0, 0.0], abs=1e-9)


def test_world_to_camera_uses_the_rig_mounting(lens):
    cam = PreviewCamera.from_rig_sensor(make_sensor(t=(1.0, 0.0, 1.5)))
    w2c = cam.world_to_camera(np.eye(4))
    uv = cam.project_world(np.array([[11.0, 0.0, 1.5]]), w2c)
    assert uv[0] == pytest.approx([lens.cx, lens.cy], abs=1e-6)


def test_ego_pose_moves_the_world_with_the_car(lens):
    cam = PreviewCamera.from_rig_sensor(make_sensor(t=(0.0, 0.0, 0.0)))
    ego = pose_matrix(euler_to_matrix([0.0, 0.0, 90.0]), [5.0, 5.0, 0.0])  # car at (5,5) facing +y
    uv = cam.project_world(np.array([[5.0, 15.0, 0.0]]), cam.world_to_camera(ego))
    assert uv[0] == pytest.approx([cam.lens.cx, cam.lens.cy], abs=1e-6)


def test_cameras_from_rig_json():
    rig = {"rig": {"sensors": [make_sensor(), make_sensor("camera:cross:left:120fov", rpy=(0, 0, 66.4))]}}
    cams = cameras_from_rig_json(rig)
    assert set(cams) == {"camera:front:wide:120fov", "camera:cross:left:120fov"}
    assert cams["camera:cross:left:120fov"].sensor_to_rig.shape == (4, 4)


# ----------------------------------------------------------------------------- boxes and tracks

def test_box_corners_span_the_size():
    corners = box_corners([1.0, 2.0, 3.0], [2.0, 4.0, 6.0], [0.0, 0.0, 0.0, 1.0])
    assert corners.shape == (8, 3)
    assert corners.min(axis=0) == pytest.approx([0.0, 0.0, 0.0])
    assert corners.max(axis=0) == pytest.approx([2.0, 4.0, 6.0])
    assert len(BOX_EDGES) == 12


def _obstacle_row(track: str, ts: int):
    return {"key": {"timestamp_micros": ts},
            "obstacle": {"trackline_id": track, "center": {"x": 1.0, "y": 0.0, "z": 0.0},
                         "size": {"x": 4.0, "y": 2.0, "z": 1.5},
                         "orientation": {"x": 0.0, "y": 0.0, "z": 0.0, "w": 1.0}, "category": "car"}}


def test_two_row_tracks_are_parked_and_held_across_frames():
    rows = ([_obstacle_row("parked", 10), _obstacle_row("parked", 90)]
            + [_obstacle_row("moving", ts) for ts in (10, 50, 90)])
    static, dynamic = split_tracks(rows)
    assert [b.track_id for b in static] == ["parked"]
    assert sorted(dynamic) == [10, 50, 90]
    assert all(len(v) == 1 and v[0].track_id == "moving" for v in dynamic.values())


def test_boxes_at_merges_parked_and_moving():
    scene = SceneGT(clip_id="c", cameras={}, timestamps=[10], ego_poses=[np.eye(4)],
                    static_boxes={"obstacle": [Box(np.zeros(3), np.ones(3), np.array([0, 0, 0, 1.0]))]},
                    dynamic_boxes={"obstacle": {10: [Box(np.ones(3), np.ones(3), np.array([0, 0, 0, 1.0]))]}})
    assert len(scene.boxes_at("obstacle", 10)) == 2
    assert len(scene.boxes_at("obstacle", 99)) == 1  # the parked one is always there


# ----------------------------------------------------------------------------- drawing

def test_draw_scene_paints_the_layer_colours():
    cam = PreviewCamera.from_rig_sensor(make_sensor(t=(0.0, 0.0, 0.0)))
    lane = np.array([[5.0, 1.0, 0.0], [30.0, 1.0, 0.0]])
    scene = SceneGT(clip_id="c", cameras={cam.name: cam}, timestamps=[0], ego_poses=[np.eye(4)],
                    polylines={"lane_line": [lane]},
                    static_boxes={"obstacle": [Box(np.array([10.0, 0.0, 0.0]), np.array([4.0, 2.0, 1.5]),
                                                   np.array([0.0, 0.0, 0.0, 1.0]), category="car")]})
    frame = np.zeros((720, 1280, 3), np.uint8)
    out = draw_scene(frame, cam, cam.world_to_camera(np.eye(4)), scene, 0,
                     layers=("lane_line", "obstacle"))
    painted = out.reshape(-1, 3)
    assert (painted == LAYERS["lane_line"].color).all(axis=1).sum() > 50
    assert (painted == LAYERS["obstacle"].color).all(axis=1).sum() > 50


def test_draw_scene_dims_the_rgb():
    cam = PreviewCamera.from_rig_sensor(make_sensor())
    scene = SceneGT(clip_id="c", cameras={cam.name: cam}, timestamps=[0], ego_poses=[np.eye(4)])
    frame = np.full((32, 32, 3), 100, np.uint8)
    assert draw_scene(frame, cam, np.eye(4), scene, 0, layers=(), dim=0.5).max() == 50


def test_label_patch_draws_text():
    patch = label_patch("front_wide_120fov · scene GT", height=22)
    assert patch.shape[0] == 22 and patch.shape[2] == 3
    assert patch.max() > 100  # something was actually drawn


# ----------------------------------------------------------------------------- misc

def test_parse_frames():
    assert parse_frames(None, 255) == (0, 255)
    assert parse_frames("10:60", 255) == (10, 60)
    assert parse_frames("100:", 255) == (100, 255)
    assert parse_frames(":30", 255) == (0, 30)
    assert parse_frames("200:900", 255) == (200, 255)
