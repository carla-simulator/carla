"""Clip manifest round trip and validation."""

import numpy as np

from carla_cosmos.clip import Clip, video_file_name
from carla_cosmos import controls
from tests.test_contracts import make_manifest


def test_manifest_round_trip(tmp_path):
    manifest = make_manifest(fps=30, frames=5)
    manifest.seed = 7
    clip = Clip(path=tmp_path / "clip0", manifest=manifest)
    clip.save_manifest()
    loaded = Clip.load(tmp_path / "clip0")
    assert loaded.manifest == manifest
    assert loaded.manifest.camera_names == ["camera:front:wide:120fov"]
    assert loaded.manifest.video("rgb", "camera:front:wide:120fov") == "rgb_camera_front_wide_120fov.mp4"


def test_validate_reports_missing_and_short_videos(tmp_path):
    manifest = make_manifest(fps=30, frames=5, kinds=("rgb",))
    clip = Clip(path=tmp_path / "clip1", manifest=manifest)
    clip.save_manifest()
    errors = clip.validate(check_frames=False)
    assert any("missing video rgb/" in e for e in errors)
    assert any("lacks egomotion_estimate" in e.replace("scene package lacks", "lacks egomotion_estimate")
               or "egomotion_estimate" in e for e in errors)

    # write a too-short video and check the frame count is flagged
    frames = [np.zeros((16, 16, 3), np.uint8)] * 3
    controls.encode_frames(clip.path / video_file_name("rgb", "camera:front:wide:120fov"), frames, 30, "rgb")
    errors = clip.validate(check_frames=True)
    assert any("has 3 frames, manifest says 5" in e for e in errors)


def test_video_file_names():
    assert video_file_name("depth", "camera:cross:left:120fov") == "depth_camera_cross_left_120fov.mp4"
