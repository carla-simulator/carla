# SPDX-FileCopyrightText: © 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#
# SPDX-License-Identifier: MIT

"""
Coordinate-convention tests for the NuRec <-> CARLA transforms in utils.py.

These pin the conversions to CARLA's *documented engine convention*, not to
whatever the installed wheel's ``Transform.get_matrix()`` happens to return.
That distinction is the whole point of this file: the previous version of these
tests round-tripped a pose through ``mat_to_carla_transform`` and back through
``get_matrix()``, so it measured the wheel against itself. When CARLA PR #9751
mirrored the pitch/roll signs of ``carla::geom`` (without touching the engine),
``mat_to_carla_transform`` was "fixed" to cancel against the mirrored matrix and
these tests stayed green -- while every pitched or rolled NuRec camera and actor
was actually mounted at minus the intended angle.

The conventions asserted here (see ``Docs/coordinate_conventions.md``):

* CARLA / Unreal world: left-handed, x forward, y RIGHT, z up,
  ``R = Rz(+yaw) * Ry(-pitch) * Rx(-roll)``; +pitch is nose UP, +roll drops the
  RIGHT side, +yaw turns right.
* NuRec / ClipGT rig: right-handed FLU, x forward, y LEFT, z up,
  ``R = Rz(yaw) * Ry(pitch) * Rx(roll)`` (scipy extrinsic "xyz").
* Between them: location ``(x, -y, z)``, rotation ``(roll, -pitch, -yaw)``,
  matrix ``S @ M @ S`` with ``S = diag(1, -1, 1)``.
"""

import os
import sys

import numpy as np
import pytest
from scipy.spatial.transform import Rotation as R

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import carla
from utils import (
    carla_transform_to_nurec,
    mat_to_carla_transform,
    mirror_pose,
    undo_carla_coordinate_transform,
    xyzquat_to_carla_transform,
    xyzeuler_to_carla_transform,
    se3_to_grpc_pose,
)

S = np.diag([1.0, -1.0, 1.0, 1.0])


def engine_matrix(pitch, yaw, roll, location=(0.0, 0.0, 0.0)):
    """
    The 4x4 pose Unreal builds from ``carla.Rotation(pitch, yaw, roll)``,
    written out from the documented convention alone -- deliberately without
    calling ``carla.Transform.get_matrix()``.

    ``R = Rz(+yaw) * Ry(-pitch) * Rx(-roll)`` == scipy extrinsic "xyz" with
    angles ``(-roll, -pitch, +yaw)``.
    """
    mat = np.eye(4)
    mat[:3, :3] = R.from_euler("xyz", [-roll, -pitch, yaw], degrees=True).as_matrix()
    mat[:3, 3] = location
    return mat


def make_pose(translation, euler_zyx_deg):
    """A NuRec-frame (right-handed FLU) pose."""
    mat = np.eye(4)
    mat[:3, :3] = R.from_euler("zyx", euler_zyx_deg, degrees=True).as_matrix()
    mat[:3, 3] = translation
    return mat


GOLDEN_POSES = [
    make_pose([0, 0, 0], [0, 0, 0]),
    make_pose([1, 2, 3], [0, 0, 0]),
    make_pose([10, -5, 2], [90, 0, 0]),
    make_pose([-3, 7, 0.5], [0, 30, 0]),
    make_pose([0, 0, 100], [0, 0, 45]),
    make_pose([12.3, -4.56, 7.89], [123, -35, 78]),
    make_pose([-100, 250, -12], [-170, 60, -120]),
]


# ---------------------------------------------------------------------------
# The wheel itself must be on the engine convention. Everything below is
# meaningless on a wheel that carries CARLA PR #9751's mirrored geom math.
# ---------------------------------------------------------------------------


def test_wheel_uses_the_engine_pitch_convention():
    """``Rotation(pitch=20)`` must give ``forward.z = +sin(20)``, not -sin(20)."""
    mat = np.array(
        carla.Transform(carla.Location(), carla.Rotation(pitch=20.0)).get_matrix()
    ).reshape(4, 4)
    assert mat[2][0] == pytest.approx(+np.sin(np.radians(20.0)), abs=1e-6)
    fwd = carla.Transform(carla.Location(), carla.Rotation(pitch=20.0)).get_forward_vector()
    assert fwd.z == pytest.approx(+np.sin(np.radians(20.0)), abs=1e-6)


def test_wheel_uses_the_engine_roll_convention():
    """+roll drops the RIGHT side: ``right.z = -cos(pitch) * sin(roll)``."""
    right = carla.Transform(
        carla.Location(), carla.Rotation(roll=25.0)
    ).get_right_vector()
    assert right.z == pytest.approx(-np.sin(np.radians(25.0)), abs=1e-6)


@pytest.mark.parametrize(
    "pitch,yaw,roll",
    [(0, 0, 0), (20, 30, 10), (-35, 78, 123), (5, 0, 0), (0, 0, 25), (-8, -170, -60)],
)
def test_get_matrix_matches_the_documented_composition(pitch, yaw, roll):
    """``get_matrix()`` == ``Rz(+yaw) Ry(-pitch) Rx(-roll)``, derived independently."""
    got = np.array(
        carla.Transform(
            carla.Location(1, 2, 3), carla.Rotation(pitch=pitch, yaw=yaw, roll=roll)
        ).get_matrix()
    ).reshape(4, 4)
    np.testing.assert_allclose(got, engine_matrix(pitch, yaw, roll, (1, 2, 3)), atol=1e-5)


# ---------------------------------------------------------------------------
# The CARLA <-> NuRec handedness boundary.
# ---------------------------------------------------------------------------


@pytest.mark.parametrize(
    "pitch,yaw,roll", [(0, 0, 0), (20, 30, 10), (-35, 78, 123), (-8, 0, 0)]
)
def test_mirror_pose_matches_libcarla_boundary(pitch, yaw, roll):
    """``S @ M @ S`` == ``carla.Transform.to_right_handed().get_matrix()``."""
    transform = carla.Transform(
        carla.Location(1, 2, 3), carla.Rotation(pitch=pitch, yaw=yaw, roll=roll)
    )
    engine = np.array(transform.get_matrix()).reshape(4, 4)
    np.testing.assert_allclose(
        mirror_pose(engine), carla_transform_to_nurec(transform), atol=1e-6
    )
    # The historic alias must stay an alias.
    np.testing.assert_allclose(
        undo_carla_coordinate_transform(engine), mirror_pose(engine), atol=0
    )


@pytest.mark.parametrize(
    "pitch,yaw,roll", [(0, 0, 0), (20, 30, 10), (-35, 78, 123), (-8, 0, 0)]
)
def test_right_handed_euler_signs(pitch, yaw, roll):
    """Pitch and yaw flip sign across the boundary; roll does not."""
    rh = carla.Transform(
        carla.Location(), carla.Rotation(pitch=pitch, yaw=yaw, roll=roll)
    ).to_right_handed()
    assert rh.rotation.roll == pytest.approx(roll, abs=1e-4)
    assert rh.rotation.pitch == pytest.approx(-pitch, abs=1e-4)
    assert rh.rotation.yaw == pytest.approx(-yaw, abs=1e-4)


# ---------------------------------------------------------------------------
# mat_to_carla_transform, checked against the engine convention.
# ---------------------------------------------------------------------------


@pytest.mark.parametrize("pose", GOLDEN_POSES)
def test_mat_to_carla_transform_lands_on_the_intended_engine_pose(pose):
    """
    The pose Unreal builds from the returned ``carla.Transform`` must be the
    Y-mirror of the NuRec pose -- computed from the convention, not from
    ``get_matrix()``.
    """
    t = mat_to_carla_transform(pose)
    built = engine_matrix(
        t.rotation.pitch,
        t.rotation.yaw,
        t.rotation.roll,
        (t.location.x, t.location.y, t.location.z),
    )
    np.testing.assert_allclose(built, S @ pose @ S, atol=1e-4)


@pytest.mark.parametrize("pose", GOLDEN_POSES)
def test_roundtrip_through_carla_transform(pose):
    """NuRec pose -> carla.Transform -> get_matrix() -> undo == original."""
    transform = mat_to_carla_transform(pose)
    carla_mat = np.array(transform.get_matrix()).reshape(4, 4)
    recovered = undo_carla_coordinate_transform(carla_mat)
    np.testing.assert_allclose(recovered, pose, atol=1e-5)


@pytest.mark.parametrize("pose", GOLDEN_POSES)
def test_roundtrip_through_the_boundary_helper(pose):
    np.testing.assert_allclose(
        carla_transform_to_nurec(mat_to_carla_transform(pose)), pose, atol=1e-5
    )


def test_known_pitch_sign():
    """
    A rig camera pitched nose-DOWN by 8 deg (right-handed pitch = +8 about the
    y-LEFT axis) must become ``carla.Rotation(pitch=-8)``, which is nose-down in
    CARLA too. Getting this sign wrong mounts the camera 16 deg away from the
    truth -- the failure that hid in the old round-trip test.
    """
    pose = make_pose([2.06, 0.07, 1.60], [0, 8, 0])  # zyx: yaw 0, pitch 8, roll 0
    t = mat_to_carla_transform(pose)
    assert t.rotation.pitch == pytest.approx(-8, abs=1e-3)
    assert t.rotation.yaw == pytest.approx(0, abs=1e-3)
    assert t.rotation.roll == pytest.approx(0, abs=1e-3)
    # ... and it really is nose down in the engine frame.
    assert t.get_forward_vector().z < 0


def test_known_roll_sign():
    """
    Right-handed roll is about the x-forward axis and does NOT flip sign: a rig
    pose rolled +15 deg (right-hand rule about forward, i.e. LEFT side down in
    a y-left frame) is ``carla.Rotation(roll=+15)``, which drops the RIGHT side
    in CARLA's mirrored frame -- the same physical side.
    """
    pose = make_pose([0, 0, 1.5], [0, 0, 15])  # zyx: yaw 0, pitch 0, roll 15
    t = mat_to_carla_transform(pose)
    assert t.rotation.roll == pytest.approx(15, abs=1e-3)
    assert t.rotation.pitch == pytest.approx(0, abs=1e-3)
    # CARLA +roll drops the right side.
    assert t.get_right_vector().z < 0


def test_known_yaw_mirror():
    """
    Anchor the convention explicitly: a NuRec pose looking +90deg (CCW,
    right-handed Z-up) must land in CARLA as yaw=-90 with Y negated.
    """
    pose = make_pose([5, 10, 1], [90, 0, 0])
    t = mat_to_carla_transform(pose)
    assert t.location.x == pytest.approx(5)
    assert t.location.y == pytest.approx(-10)
    assert t.location.z == pytest.approx(1)
    assert t.rotation.yaw == pytest.approx(-90)
    assert t.rotation.pitch == pytest.approx(0)
    assert t.rotation.roll == pytest.approx(0)


def test_nvidia_rig_calibration_is_flu():
    """
    Ground truth from NVIDIA's own ``rig_trajectories.json``: the rig frame is
    x forward / y LEFT / z up, so ``camera_cross_left_120fov`` (y = +0.95 in the
    rig) must end up on the ego's LEFT in CARLA, i.e. at negative y.
    """
    left_mount = np.eye(4)
    left_mount[:3, 3] = [2.590, 0.953, 0.978]  # camera_cross_left_120fov
    t = mat_to_carla_transform(left_mount)
    assert t.location.x == pytest.approx(2.590, abs=1e-4)
    assert t.location.y == pytest.approx(-0.953, abs=1e-4)
    assert t.location.z == pytest.approx(0.978, abs=1e-4)


# ---------------------------------------------------------------------------
# The thin wrappers must agree with mat_to_carla_transform.
# ---------------------------------------------------------------------------


@pytest.mark.parametrize("pose", GOLDEN_POSES)
def test_xyzquat_matches_mat(pose):
    quat = R.from_matrix(pose[:3, :3]).as_quat(canonical=False)
    xyzquat = np.concatenate([pose[:3, 3], quat])
    a = xyzquat_to_carla_transform(xyzquat)
    b = mat_to_carla_transform(pose)
    assert a.location.x == pytest.approx(b.location.x, abs=1e-5)
    assert a.location.y == pytest.approx(b.location.y, abs=1e-5)
    assert a.location.z == pytest.approx(b.location.z, abs=1e-5)
    # Compare rotations as matrices: euler angles may differ by gimbal aliases.
    np.testing.assert_allclose(
        engine_matrix(a.rotation.pitch, a.rotation.yaw, a.rotation.roll)[:3, :3],
        engine_matrix(b.rotation.pitch, b.rotation.yaw, b.rotation.roll)[:3, :3],
        atol=1e-5,
    )


@pytest.mark.parametrize("pose", GOLDEN_POSES)
def test_xyzeuler_matches_mat(pose):
    euler_zyx = R.from_matrix(pose[:3, :3]).as_euler("zyx", degrees=False)
    xyzeuler = np.concatenate([pose[:3, 3], euler_zyx])
    a = xyzeuler_to_carla_transform(xyzeuler)
    b = mat_to_carla_transform(pose)
    np.testing.assert_allclose(
        engine_matrix(
            a.rotation.pitch,
            a.rotation.yaw,
            a.rotation.roll,
            (a.location.x, a.location.y, a.location.z),
        ),
        engine_matrix(
            b.rotation.pitch,
            b.rotation.yaw,
            b.rotation.roll,
            (b.location.x, b.location.y, b.location.z),
        ),
        atol=1e-5,
    )


@pytest.mark.parametrize("pose", GOLDEN_POSES)
def test_se3_to_grpc_pose(pose):
    grpc_pose = se3_to_grpc_pose(pose)
    assert grpc_pose.vec.x == pytest.approx(pose[0, 3], abs=1e-6)
    assert grpc_pose.vec.y == pytest.approx(pose[1, 3], abs=1e-6)
    assert grpc_pose.vec.z == pytest.approx(pose[2, 3], abs=1e-6)
    q = np.array([grpc_pose.quat.x, grpc_pose.quat.y, grpc_pose.quat.z, grpc_pose.quat.w])
    np.testing.assert_allclose(R.from_quat(q).as_matrix(), pose[:3, :3], atol=1e-6)
