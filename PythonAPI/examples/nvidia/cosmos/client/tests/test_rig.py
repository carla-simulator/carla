"""Rig presets, YAML round trip and the roofline mounting rule (pure parts)."""

import numpy as np
import pytest

from carla_cosmos.contracts import AV_CAMERAS
from carla_cosmos.rig import Camera, Rig, _apply_mount_rule


def test_presets():
    av7 = Rig.nvidia_av7()
    assert [c.name for c in av7.cameras] == list(AV_CAMERAS)
    assert {c.hfov for c in av7.cameras} == {120.0, 70.0, 30.0}
    single = Rig.single()
    assert [c.name for c in single.cameras] == ["camera:front:wide:120fov"]
    assert single.cameras[0].hfov == 90.0


def test_yaml_round_trip(tmp_path):
    rig = Rig.nvidia_av7()
    rig.save(tmp_path / "rig.yaml")
    loaded = Rig.load(tmp_path / "rig.yaml")
    assert loaded.name == rig.name and loaded.mount == rig.mount
    for a, b in zip(loaded.cameras, rig.cameras):
        assert a.name == b.name and a.hfov == b.hfov
        np.testing.assert_allclose(a.t, b.t)
        np.testing.assert_allclose(a.rpy, b.rpy)


def test_duplicate_camera_names_rejected():
    cam = Camera("camera:front:wide:120fov", 90, (1, 0, 1))
    with pytest.raises(ValueError, match="duplicate"):
        Rig(name="bad", cameras=[cam, cam])


def test_mount_rule_lifts_inside_camera_to_roofline():
    centre = np.array([0.0, 0.0, 0.75])
    extent = np.array([2.4, 0.9, 0.75])  # sedan-ish half sizes; roof at z=1.5
    inside = np.array([0.3, 0.0, 1.4])   # inside the cabin
    pos, shifted = _apply_mount_rule("roofline", inside, centre, extent, margin=0.05)
    assert shifted
    assert pos[2] == pytest.approx(1.55)  # roof + margin
    assert abs(pos[0]) <= 2.4 and abs(pos[1]) <= 0.9


def test_mount_rule_keeps_outside_camera():
    centre = np.array([0.0, 0.0, 0.75])
    extent = np.array([2.4, 0.9, 0.75])
    outside = np.array([0.0, 0.0, 1.7])  # already above the roof
    pos, shifted = _apply_mount_rule("roofline", outside, centre, extent, margin=0.05)
    assert not shifted
    np.testing.assert_allclose(pos, outside)


def test_mount_rule_exact_never_moves():
    centre = np.zeros(3)
    extent = np.ones(3)
    pos, shifted = _apply_mount_rule("exact", np.array([0.0, 0.0, 0.0]), centre, extent, 0.05)
    assert not shifted and (pos == 0).all()
