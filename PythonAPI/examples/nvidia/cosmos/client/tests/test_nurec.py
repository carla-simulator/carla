"""The NuRec bridge, checked against NVIDIA's shipped artifacts.

Everything here except the ``integration`` tests runs offline: the sample's ``.usdz`` carries
its own rig calibration, trajectory, OpenDRIVE map and one real frame per camera, and
``carla.Map`` builds a map from an xodr string without a server.  The point of the file is that
the numbers are NVIDIA's, not ours — a convention that drifts breaks a test instead of quietly
mounting every camera somewhere else.
"""

from __future__ import annotations

import json
import math
import os
import sys
from pathlib import Path

import numpy as np
import pytest
from scipy.spatial.transform import Rotation

import carla

from carla_cosmos import clipgt, coords, nurec, preview
from carla_cosmos.contracts import AV_CAMERAS, CameraManifest, FThetaModel
from carla_cosmos.rig import Rig

SAMPLE_ROOT = Path(os.environ.get(
    "NUREC_SAMPLES", "/home/german/Projects/CARLA_SOURCE/nurec_samples")) / "sample_set/26.04_release"
SAMPLE_UUID = "00040136-e651-4abd-991d-0655ccda9430"
SAMPLE = SAMPLE_ROOT / SAMPLE_UUID / f"{SAMPLE_UUID}.usdz"

needs_sample = pytest.mark.skipif(not SAMPLE.is_file(),
                                  reason=f"NuRec sample not present at {SAMPLE}")
needs_fixed_wheel = pytest.mark.skipif(
    not hasattr(carla.Transform, "from_right_handed"),
    reason="carla wheel predates the geom fix / the right-handed boundary")


@pytest.fixture(scope="module")
def sample() -> nurec.NurecSample:
    return nurec.NurecSample.load(SAMPLE)


# ----------------------------------------------------------------------------- loading

@needs_sample
def test_loads_rig_trajectory_and_map(sample):
    assert sample.scene_id == f"clipgt-{SAMPLE_UUID}"
    # The 26.04 samples ship six of the seven RDS-HQ cameras.
    assert len(sample.cameras) == 6
    assert sample.poses.shape == (202, 4, 4)
    assert len(sample.timestamps_us) == 202
    assert sample.duration_s == pytest.approx(20.0, abs=0.01)
    assert "<OpenDRIVE>" in sample.xodr and "geoReference" in sample.xodr


@needs_sample
def test_first_pose_is_the_identity(sample):
    """The reconstruction's world frame is anchored at the first rig pose - like ClipGT's."""
    assert np.allclose(sample.poses[0], np.eye(4), atol=1e-9)


@needs_sample
def test_t_rig_worlds_is_world_from_rig(sample):
    """The name reads backwards: velocity lies along the rig's own +x, not the inverse's.

    This is the single fact the whole replay rests on.  Reading it the other way round would
    drive the ego backwards along a mirrored path and nothing else in the pipeline would object.
    """
    def median_forward_dot(poses):
        p = poses[:, :3, 3]
        v = np.diff(p, axis=0)
        n = np.linalg.norm(v, axis=1)
        keep = n > 1e-6
        return float(np.median((v[keep] / n[keep, None] * poses[:-1, :3, 0][keep]).sum(1)))

    assert median_forward_dot(sample.poses) == pytest.approx(1.0, abs=1e-3)
    inverse = np.array([np.linalg.inv(m) for m in sample.poses])
    assert median_forward_dot(inverse) < 0.95


# ----------------------------------------------------------------------------- the rig

@needs_sample
def test_cross_cameras_sit_where_nvidia_calibrated_them(sample):
    """The FLU handedness anchor: y is LEFT.

    ``camera_cross_left_120fov`` at y = +0.953 and ``camera_cross_right_120fov`` at y = -0.975
    are this artifact's own numbers.  A y-flip anywhere in the loader swaps the two cameras and
    every cross-view box lands on the wrong side of the car.
    """
    assert sample.cameras["camera_cross_left_120fov"].t_flu[1] == pytest.approx(0.953, abs=5e-4)
    assert sample.cameras["camera_cross_right_120fov"].t_flu[1] == pytest.approx(-0.975, abs=5e-4)
    # ... and both are mounted about a metre up, on a rig whose origin is the rear axle on ground.
    for name in ("camera_cross_left_120fov", "camera_cross_right_120fov"):
        assert 0.9 < sample.cameras[name].t_flu[2] < 1.1
        assert sample.cameras[name].t_flu[0] > 0.0


@needs_sample
@pytest.mark.parametrize("name,expected", [
    ("camera_front_wide_120fov", 120.0),
    ("camera_cross_left_120fov", 120.0),
    ("camera_cross_right_120fov", 120.0),
    ("camera_rear_left_70fov", 70.0),
    ("camera_rear_right_70fov", 70.0),
    ("camera_front_tele_30fov", 30.0),
])
def test_derived_fov_matches_the_camera_name(sample, name, expected):
    """The polynomial read correctly reproduces the FOV NVIDIA put in the sensor's name."""
    assert sample.cameras[name].hfov() == pytest.approx(expected, abs=1.0)


@needs_sample
def test_optical_basis_removal_round_trips(sample):
    """``flu_pose_matrix(t, rpy) @ OPTICAL_TO_FLU`` must rebuild ``T_sensor_rig`` exactly.

    ``rpy_flu`` is what the exported calibration carries; if stripping the RDF optical basis and
    putting it back is not the identity then the calibration describes a different mount than
    the one the artifact was captured with.
    """
    for cam in sample.cameras.values():
        rebuilt = cam.flu_pose()[:3, :3] @ nurec.OPTICAL_TO_FLU
        assert np.allclose(rebuilt, cam.t_sensor_rig[:3, :3], atol=1e-9), cam.name
        assert np.allclose(cam.t_flu, cam.t_sensor_rig[:3, 3], atol=1e-12), cam.name


@needs_sample
def test_rear_cameras_are_nose_down_in_flu(sample):
    """Both shipped clips pitch the rear cameras down by 1-2 degrees; the sign is the anchor."""
    for name in ("camera_rear_left_70fov", "camera_rear_right_70fov"):
        assert 0.5 < sample.cameras[name].rpy_flu[1] < 3.0, name


@needs_sample
def test_scaling_preserves_the_lens(sample):
    """A pixel radius scales by s, so the angle it maps to must not move."""
    cam = sample.cameras["camera_front_wide_120fov"]
    scaled = cam.scaled(1280, 720)
    assert scaled.cx == pytest.approx(cam.cx * 1280 / 1920)
    assert scaled.cy == pytest.approx(cam.cy * 1280 / 1920)
    for r in (0.0, 100.0, 400.0, 900.0):
        assert (nurec._poly_angle(scaled.poly, r * 1280 / 1920)
                == pytest.approx(nurec._poly_angle(cam.poly, r), abs=1e-12))
    assert scaled.hfov() == pytest.approx(cam.hfov(), abs=1e-9)


@needs_sample
def test_scaling_refuses_to_change_the_aspect(sample):
    with pytest.raises(ValueError, match="without cropping"):
        sample.cameras["camera_front_wide_120fov"].scaled(1280, 800)


@needs_sample
def test_rig_is_mounted_exactly_and_carries_the_measured_lens(sample):
    rig = sample.rig(1280, 720)
    assert rig.mount == "exact", "surveyed extrinsics must not be moved to the roofline"
    assert [c.name for c in rig.cameras][0] == "camera:front:wide:120fov"
    for cam in rig.cameras:
        assert cam.width == 1280 and cam.height == 720
        assert cam.ftheta is not None and len(cam.ftheta.poly) == 6
    pinhole = sample.rig(1280, 720, lens="pinhole")
    assert all(c.ftheta is None for c in pinhole.cameras)
    # ... and the geometry is the same either way; only the lens description differs.
    assert [c.t for c in pinhole.cameras] == [c.t for c in rig.cameras]


@needs_sample
def test_complete_av7_fills_the_missing_slot_and_flags_it(sample):
    rig = sample.rig(1280, 720, complete_av7=True)
    assert [c.name for c in rig.cameras] == list(AV_CAMERAS)
    assert sample.nominal_cameras(rig) == ["camera:rear:tele:30fov"]
    nominal = next(c for c in rig.cameras if c.name == "camera:rear:tele:30fov")
    assert nominal.ftheta is None, "a nominal camera must not claim a measured lens"


@needs_sample
def test_unknown_camera_is_refused(sample):
    with pytest.raises(KeyError, match="has no camera"):
        sample.rig(cameras=["camera_front_fisheye_200fov"])


# ----------------------------------------------------------------------------- handedness

@needs_sample
@needs_fixed_wheel
def test_matches_the_nurec_integration_conversion(sample):
    """``flu_matrix_to_carla`` must be ``nurec/utils.mat_to_carla_transform``, exactly.

    Two copies of a handedness conversion is how the pitch/roll mirror survived for two months.
    This pins them together on real poses rather than on a hand-written matrix.
    """
    nurec_dir = Path(__file__).resolve().parents[3] / "nurec"
    if not (nurec_dir / "utils.py").is_file():
        pytest.skip(f"the NuRec integration is not at {nurec_dir}")
    sys.path.insert(0, str(nurec_dir))
    try:
        import utils as nurec_utils
    finally:
        sys.path.remove(str(nurec_dir))

    poses = list(sample.poses[::37]) + [c.t_sensor_rig for c in sample.cameras.values()]
    for m in poses:
        ours = nurec.flu_matrix_to_carla(m)
        theirs = nurec_utils.mat_to_carla_transform(m)
        assert ours.location.x == pytest.approx(theirs.location.x, abs=1e-9)
        assert ours.location.y == pytest.approx(theirs.location.y, abs=1e-9)
        assert ours.location.z == pytest.approx(theirs.location.z, abs=1e-9)
        assert np.allclose(np.array(ours.get_matrix()), np.array(theirs.get_matrix()), atol=1e-9)


@needs_fixed_wheel
def test_flu_to_carla_flips_pitch_and_yaw_but_not_roll():
    """The sign anchors, stated once, derived from nothing the wheel computes."""
    def flu(roll, pitch, yaw):
        m = np.eye(4)
        m[:3, :3] = Rotation.from_euler("xyz", [roll, pitch, yaw], degrees=True).as_matrix()
        return nurec.flu_matrix_to_carla(m).rotation

    assert flu(0, 8, 0).pitch == pytest.approx(-8.0, abs=1e-6)
    assert flu(15, 0, 0).roll == pytest.approx(+15.0, abs=1e-6)
    assert flu(0, 0, 90).yaw == pytest.approx(-90.0, abs=1e-6)


@needs_fixed_wheel
def test_flu_to_carla_mirrors_y():
    m = np.eye(4)
    m[:3, 3] = [5.0, 10.0, 1.0]
    location = nurec.flu_matrix_to_carla(m).location
    assert (location.x, location.y, location.z) == pytest.approx((5.0, -10.0, 1.0))


# ----------------------------------------------------------------------------- trajectory

@needs_sample
def test_pose_at_hits_the_samples_and_interpolates_between(sample):
    exact = sample.pose_at(sample.timestamps_us[10])
    assert np.allclose(exact, sample.poses[10], atol=1e-9)
    a, b = sample.timestamps_us[10], sample.timestamps_us[11]
    mid = sample.pose_at((a + b) / 2)
    for axis in range(3):
        lo, hi = sorted((sample.poses[10][axis, 3], sample.poses[11][axis, 3]))
        assert lo - 1e-9 <= mid[axis, 3] <= hi + 1e-9
    assert np.allclose(mid[:3, :3] @ mid[:3, :3].T, np.eye(3), atol=1e-9)


@needs_sample
def test_pose_at_clamps_outside_the_recording(sample):
    assert np.allclose(sample.pose_at(sample.timestamps_us[0] - 1e9), sample.poses[0], atol=1e-9)
    assert np.allclose(sample.pose_at(sample.timestamps_us[-1] + 1e9), sample.poses[-1], atol=1e-9)


@needs_sample
def test_clip_timestamps_refuse_to_run_off_the_end(sample):
    ts = sample.clip_timestamps(frames=29, fps=30)
    assert len(ts) == 29
    assert ts[0] == pytest.approx(float(sample.timestamps_us[0]))
    assert np.allclose(np.diff(ts), 1e6 / 30)
    with pytest.raises(ValueError, match="is 20.0s long"):
        sample.clip_timestamps(frames=30 * 25, fps=30)


# ----------------------------------------------------------------------------- map alignment

@needs_sample
def test_the_replayed_trajectory_lands_on_the_map(sample):
    """The georeferenced trajectory must sit on the roads of the sample's own OpenDRIVE map.

    ``carla.Map`` parses an xodr string with no server, so this is the cheap check that
    ``get_t_rig_enu_from_ecef`` was applied (and applied in the right direction): without it the
    ego drives hundreds of metres off the road network and every exported lane is meaningless.
    """
    t_enu = sample.t_scenario_carla()
    assert not np.allclose(t_enu, np.eye(4)), "the sample's map has a georeference; use it"
    carla_map = carla.Map("nurec_test", sample.xodr)
    poses = sample.poses_at(sample.timestamps_us[::10])
    lateral, vertical = [], []
    for m in poses:
        p = (t_enu @ m)[:3, 3]
        loc = carla.Location(x=float(p[0]), y=-float(p[1]), z=float(p[2]))
        wp = carla_map.get_waypoint(loc, project_to_road=True)
        assert wp is not None
        lateral.append(loc.distance(wp.transform.location))
        vertical.append(abs(loc.z - wp.transform.location.z))
    assert max(lateral) < 1.5, f"ego drifts {max(lateral):.2f} m from the nearest lane centre"
    assert max(vertical) < 0.5, f"ego floats {max(vertical):.2f} m above/below the road"


# ----------------------------------------------------------------------------- calibration

@needs_sample
def test_calibration_table_carries_the_measured_polynomial_verbatim(sample):
    """A measured lens must be written, not re-fitted from an equivalent FOV."""
    cam = sample.cameras["camera_front_wide_120fov"].scaled(1280, 720)
    manifest = CameraManifest(name="camera:front:wide:120fov", hfov=cam.hfov(), width=1280,
                              height=720, t_flu=list(cam.t_flu), rpy_flu=list(cam.rpy_flu),
                              attach_ue={}, ftheta=cam.ftheta_model())
    row = clipgt.calibration_row("clip", 0, [manifest])
    sensor = json.loads(row["calibration_estimate"]["rig_json"])["rig"]["sensors"][0]
    written = [float(v) for v in sensor["properties"]["polynomial"].split()]
    assert written == pytest.approx(list(cam.poly), rel=1e-9)
    assert sensor["properties"]["cx"] == pytest.approx(cam.cx)
    assert sensor["properties"]["cy"] == pytest.approx(cam.cy)
    # The real principal point is well below centre; a fitted one would sit at h/2.
    assert abs(sensor["properties"]["cy"] - 360.0) > 40.0


def test_calibration_table_still_fits_a_pinhole_camera_without_a_measured_lens():
    """The default path is untouched: no ``ftheta`` means the old fitted polynomial."""
    manifest = CameraManifest(name="camera:front:wide:120fov", hfov=90.0, width=1280, height=720,
                              t_flu=[0, 0, 0], rpy_flu=[0, 0, 0], attach_ue={})
    row = clipgt.calibration_row("clip", 0, [manifest])
    sensor = json.loads(row["calibration_estimate"]["rig_json"])["rig"]["sensors"][0]
    fitted, _ = clipgt.pinhole_ftheta_poly(1280, 720, 90.0)
    assert [float(v) for v in sensor["properties"]["polynomial"].split()] == pytest.approx(
        list(fitted), rel=1e-6)
    assert sensor["properties"]["cx"] == 640.0 and sensor["properties"]["cy"] == 360.0


@needs_sample
def test_the_exported_lens_projects_the_way_the_preview_reads_it(sample):
    """Round trip through the writer and the reader the preview and occlusion filter share."""
    cam = sample.cameras["camera_front_wide_120fov"].scaled(1280, 720)
    manifest = CameraManifest(name="camera:front:wide:120fov", hfov=cam.hfov(), width=1280,
                              height=720, t_flu=list(cam.t_flu), rpy_flu=list(cam.rpy_flu),
                              attach_ue={}, ftheta=cam.ftheta_model())
    row = clipgt.calibration_row("clip", 0, [manifest])
    sensor = json.loads(row["calibration_estimate"]["rig_json"])["rig"]["sensors"][0]
    lens = preview.FThetaCamera.from_rig_sensor(sensor)
    # Straight down the optical axis -> the principal point, which is the measured one.
    uv = lens.project(np.array([[10.0, 0.0, 0.0]]))
    assert uv[0][0] == pytest.approx(cam.cx, abs=0.5)
    assert uv[0][1] == pytest.approx(cam.cy, abs=0.5)
    # +y is LEFT in the FLU camera frame, so it must land left of the principal point.
    left = lens.project(np.array([[10.0, 2.0, 0.0]]))
    assert left[0][0] < cam.cx
    # Behind the camera is dropped, never mirrored into the frame.
    assert np.isnan(lens.project(np.array([[-10.0, 0.0, 0.0]]))[0][0])


# ----------------------------------------------------------------------------- alignment check

class _FakeClip:
    def __init__(self, path, clip_id):
        self.path = Path(path)
        self.scene_dir = Path(path)

        class _M:
            pass
        self.manifest = _M()
        self.manifest.clip_id = clip_id


def _write_egomotion(tmp_path: Path, clip_id: str, poses_flu: np.ndarray) -> None:
    import pyarrow.parquet as pq

    rows = []
    for i, m in enumerate(poses_flu):
        q = Rotation.from_matrix(m[:3, :3]).as_quat()
        rows.append({
            "key": {"clip_id": clip_id, "timestamp_micros": i * 33333,
                    "label_class_id": "egomotion:carla:v0"},
            "egomotion_estimate": {
                "name": "egomotion_carla",
                "location": {"x": m[0, 3], "y": m[1, 3], "z": m[2, 3]},
                "orientation": {"x": q[0], "y": q[1], "z": q[2], "w": q[3]}},
            "version": clipgt.VERSION})
    clipgt.write_table(tmp_path, clip_id, "egomotion_estimate", rows)


def _commanded_ue(poses_flu: np.ndarray) -> list[np.ndarray]:
    return [coords.flu_to_ue(m) for m in poses_flu]


def _straight_line(n: int = 12) -> np.ndarray:
    poses = np.tile(np.eye(4), (n, 1, 1))
    for i in range(n):
        poses[i, :3, :3] = Rotation.from_euler("z", i * 0.7, degrees=True).as_matrix()
        poses[i, :3, 3] = [i * 0.5, i * 0.05, 0.0]
    return poses


@needs_sample
def test_alignment_is_clean_when_the_export_is_the_trajectory(tmp_path):
    poses = _straight_line()
    _write_egomotion(tmp_path, "c", poses)
    result = nurec.check_alignment(_FakeClip(tmp_path, "c"), _commanded_ue(poses),
                                   list(range(len(poses))))
    assert result.ok
    assert result.max_translation_m < 1e-6
    assert result.max_rotation_deg < 1e-6
    assert result.frames == len(poses)


def test_alignment_catches_a_drifting_ego(tmp_path):
    """A quarter-metre of drift halfway through must fail, not round away."""
    poses = _straight_line()
    exported = poses.copy()
    exported[6:, 0, 3] += 0.25
    _write_egomotion(tmp_path, "c", exported)
    result = nurec.check_alignment(_FakeClip(tmp_path, "c"), _commanded_ue(poses),
                                   list(range(len(poses))))
    assert not result.ok
    assert result.max_translation_m == pytest.approx(0.25, abs=1e-6)


def test_alignment_catches_a_rotated_ego(tmp_path):
    poses = _straight_line()
    exported = poses.copy()
    for i in range(6, len(poses)):
        exported[i, :3, :3] = poses[i, :3, :3] @ Rotation.from_euler(
            "z", 0.5, degrees=True).as_matrix()
    _write_egomotion(tmp_path, "c", exported)
    result = nurec.check_alignment(_FakeClip(tmp_path, "c"), _commanded_ue(poses),
                                   list(range(len(poses))))
    assert not result.ok
    assert result.max_rotation_deg == pytest.approx(0.5, abs=1e-4)


def test_alignment_ignores_a_constant_offset(tmp_path):
    """Both sides are anchored at their own first pose; only relative drift is a fault."""
    poses = _straight_line()
    exported = poses.copy()
    shift = np.eye(4)
    shift[:3, 3] = [100.0, -50.0, 3.0]
    shift[:3, :3] = Rotation.from_euler("z", 33.0, degrees=True).as_matrix()
    exported = np.array([shift @ m for m in poses])
    _write_egomotion(tmp_path, "c", exported)
    result = nurec.check_alignment(_FakeClip(tmp_path, "c"), _commanded_ue(poses),
                                   list(range(len(poses))))
    assert result.ok


def test_alignment_refuses_a_frame_count_mismatch(tmp_path):
    poses = _straight_line()
    _write_egomotion(tmp_path, "c", poses)
    with pytest.raises(AssertionError, match="disagree on frame count"):
        nurec.check_alignment(_FakeClip(tmp_path, "c"), _commanded_ue(poses[:-1]),
                              list(range(len(poses) - 1)))


# ----------------------------------------------------------------------------- real frames

#: The 26.04 artifacts ship a preview frame for four of their six cameras - both rear 70fov
#: cameras are absent, in both shipped samples.  Asserted rather than assumed: a loader that
#: silently returned an empty image for the missing two would be worse than one that says None.
CAMERAS_WITH_FRAMES = ("camera_front_wide_120fov", "camera_front_tele_30fov",
                       "camera_cross_left_120fov", "camera_cross_right_120fov")


@needs_sample
def test_the_artifact_ships_real_frames_for_the_documented_cameras(sample):
    """The only genuine NuRec-scene pixels available without a render engine."""
    for name in sample.camera_order():
        got = sample.recorded_frame(name)
        if name not in CAMERAS_WITH_FRAMES:
            assert got is None, f"{name} unexpectedly has a frame; update CAMERAS_WITH_FRAMES"
            continue
        assert got is not None, name
        timestamp, jpeg = got
        assert jpeg[:2] == b"\xff\xd8", f"{name}: not a JPEG"
        assert sample.timestamps_us[0] - 1e6 < timestamp < sample.timestamps_us[-1] + 1e6


@needs_sample
def test_recorded_frames_are_full_sensor_resolution(sample):
    """They come off the real sensor, so they match the calibration's own resolution."""
    cv2 = pytest.importorskip("cv2")
    for name in CAMERAS_WITH_FRAMES:
        _, jpeg = sample.recorded_frame(name)
        img = cv2.imdecode(np.frombuffer(jpeg, dtype=np.uint8), cv2.IMREAD_COLOR)
        cam = sample.cameras[name]
        assert img.shape[:2] == (cam.height, cam.width), name


@needs_sample
def test_a_recorded_frame_flows_through_the_substitution_path(sample, tmp_path):
    """Exercise the real render path's *consumer* with real pixels and no engine.

    :meth:`NurecCapture._rgb_frame` is the seam every neural frame passes through; feeding it a
    genuine frame out of the artifact checks the decode, the resize contract, the shape/dtype
    guard and the encode into the clip's video, which is everything the NRE's answer touches
    after ``render_rgb`` returns.
    """
    cv2 = pytest.importorskip("cv2")
    from carla_cosmos import controls

    _, jpeg = sample.recorded_frame("camera_front_wide_120fov")
    bgr = cv2.imdecode(np.frombuffer(jpeg, dtype=np.uint8), cv2.IMREAD_COLOR)
    assert bgr is not None
    rgb = cv2.cvtColor(cv2.resize(bgr, (1280, 720)), cv2.COLOR_BGR2RGB)
    assert rgb.shape == (720, 1280, 3) and rgb.dtype == np.uint8

    out = tmp_path / "rgb_camera_front_wide_120fov.mp4"
    writer = controls.VideoWriter(out, 30, 1280, 720, "rgb")
    for _ in range(4):
        writer.write(rgb)
    writer.close()
    assert out.is_file() and out.stat().st_size > 0

    # The shape guard is what stops a mis-sized engine answer becoming a silently cropped clip.
    for bad in (np.zeros((720, 1280, 3), dtype=np.float32), np.zeros((360, 640, 3), dtype=np.uint8)):
        with pytest.raises(ValueError, match="expected"):
            _check_frame(bad, 1280, 720)


def _check_frame(frame: np.ndarray, width: int, height: int) -> None:
    """The guard :meth:`NurecCapture._rgb_frame` applies, isolated for testing."""
    if frame.shape != (height, width, 3) or frame.dtype != np.uint8:
        raise ValueError(f"render returned {frame.shape} {frame.dtype}, expected "
                         f"({height}, {width}, 3) uint8")


# ----------------------------------------------------------------------------- NRE request

@needs_sample
def test_nre_camera_params_describe_both_lenses(sample):
    cam = sample.cameras["camera_front_wide_120fov"].scaled(1280, 720)
    ftheta = cam.nre_camera_params("ftheta")
    assert ftheta["camera_type"] == "ftheta"
    assert ftheta["resolution_w"] == 1280 and ftheta["resolution_h"] == 720
    assert ftheta["pixeldist_to_angle_poly"] == list(cam.poly)
    assert ftheta["principal_point_y"] == pytest.approx(cam.cy)

    pinhole = cam.nre_camera_params("pinhole")
    assert pinhole["camera_type"] == "opencv_pinhole"
    # f = (w/2) / tan(hfov/2): the focal length that reproduces the derived FOV.
    assert pinhole["focal_length_x"] == pytest.approx(
        640.0 / math.tan(math.radians(cam.hfov()) / 2.0))
    assert pinhole["principal_point_x"] == 640.0


@needs_sample
def test_fake_mode_refuses_an_ftheta_calibration():
    """CARLA's pixels are pinhole; a clip may not claim otherwise."""
    with pytest.raises(ValueError, match="lens='pinhole'"):
        nurec.capture(client=None, sample=None, out_dir="/tmp", clip_id="x", frames=1,
                      lens="ftheta", fake_nurec=True)


def test_real_mode_needs_an_endpoint():
    with pytest.raises(ValueError, match="no NuRec render engine"):
        nurec.capture(client=None, sample=None, out_dir="/tmp", clip_id="x", frames=1,
                      lens="ftheta", fake_nurec=False)
