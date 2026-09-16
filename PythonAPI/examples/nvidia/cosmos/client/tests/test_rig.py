"""Rig presets, YAML round trip and the roofline mounting rule (pure parts)."""

import numpy as np
import pytest

import carla

from carla_cosmos import coords
from carla_cosmos.contracts import AV_CAMERAS
from carla_cosmos.rig import Camera, Rig, _apply_mount_rule, rear_axle_xy_from_bones


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


# ----------------------------------------------------------------------------- rear axle

_WHEELS = {"Wheel_Front_Left": (1.47, -0.80), "Wheel_Front_Right": (1.47, 0.80),
           "Wheel_Rear_Left": (-1.40, -0.80), "Wheel_Rear_Right": (-1.40, 0.80)}


def _bones_at(x_world: float):
    """The wheel bones of a vehicle whose actor origin is at ``(x_world, 0)``, yaw 0."""
    names = list(_WHEELS)
    tfs = [carla.Transform(carla.Location(x=x_world + x, y=y, z=0.34)) for x, y in _WHEELS.values()]
    return names, tfs


def _pose_at(x_world: float):
    return coords.ue_matrix(carla.Transform(carla.Location(x=x_world)))


def test_rear_axle_from_bones_is_the_mean_rear_wheel():
    xy = rear_axle_xy_from_bones(*_bones_at(0.0), _pose_at(0.0))
    assert xy == pytest.approx([-1.40, 0.0])


def test_stale_bones_bias_the_axle_by_one_tick_of_travel():
    """``get_vehicle_bone_world_transforms()`` lags ``get_transform()`` by one tick.

    Referencing the bones against the *current* pose drags the axle backwards by the distance the
    car covered in that tick (0.27 m at 8 m/s, 30 fps) - which is how the exported rig origin ended
    up near the rear bumper.  The pose of the previous tick cancels it exactly."""
    travel = 8.0 / 30.0
    stale_bones = _bones_at(0.0)          # skeleton as of the previous tick
    biased = rear_axle_xy_from_bones(*stale_bones, _pose_at(travel))
    assert biased[0] == pytest.approx(-1.40 - travel)
    fixed = rear_axle_xy_from_bones(*stale_bones, _pose_at(0.0))
    assert fixed[0] == pytest.approx(-1.40)


def test_rear_axle_needs_rear_wheel_bones():
    with pytest.raises(RuntimeError, match="no rear wheel bones"):
        rear_axle_xy_from_bones(["Wheel_Front_Left"], [carla.Transform()], np.eye(4))
