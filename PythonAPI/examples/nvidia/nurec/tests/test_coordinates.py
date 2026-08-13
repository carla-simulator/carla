# SPDX-FileCopyrightText: © 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#
# SPDX-License-Identifier: MIT

"""
Coordinate-convention tests for the NuRec <-> CARLA transforms in utils.py.

These run against the installed carla wheel's own Transform.get_matrix(), so
they empirically pin the handedness convention (left-handed, Y-mirrored,
yaw-negated) to the CARLA version in use. If a CARLA release ever changes its
coordinate conventions, these tests fail loudly instead of silently mirroring
every actor and camera.
"""

import os
import sys

import numpy as np
import pytest
from scipy.spatial.transform import Rotation as R

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import carla
from utils import (
    mat_to_carla_transform,
    undo_carla_coordinate_transform,
    xyzquat_to_carla_transform,
    xyzeuler_to_carla_transform,
    se3_to_grpc_pose,
)


def make_pose(translation, euler_zyx_deg):
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


@pytest.mark.parametrize("pose", GOLDEN_POSES)
def test_roundtrip_through_carla_transform(pose):
    """NuRec pose -> carla.Transform -> get_matrix() -> undo == original."""
    transform = mat_to_carla_transform(pose)
    carla_mat = np.array(transform.get_matrix()).reshape(4, 4)
    recovered = undo_carla_coordinate_transform(carla_mat)
    np.testing.assert_allclose(recovered, pose, atol=1e-5)


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
    ma = np.array(a.get_matrix()).reshape(4, 4)[:3, :3]
    mb = np.array(b.get_matrix()).reshape(4, 4)[:3, :3]
    np.testing.assert_allclose(ma, mb, atol=1e-5)


@pytest.mark.parametrize("pose", GOLDEN_POSES)
def test_xyzeuler_matches_mat(pose):
    euler_zyx = R.from_matrix(pose[:3, :3]).as_euler("zyx", degrees=False)
    xyzeuler = np.concatenate([pose[:3, 3], euler_zyx])
    a = xyzeuler_to_carla_transform(xyzeuler)
    b = mat_to_carla_transform(pose)
    ma = np.array(a.get_matrix()).reshape(4, 4)
    mb = np.array(b.get_matrix()).reshape(4, 4)
    np.testing.assert_allclose(ma, mb, atol=1e-5)


@pytest.mark.parametrize("pose", GOLDEN_POSES)
def test_se3_to_grpc_pose(pose):
    grpc_pose = se3_to_grpc_pose(pose)
    assert grpc_pose.vec.x == pytest.approx(pose[0, 3], abs=1e-6)
    assert grpc_pose.vec.y == pytest.approx(pose[1, 3], abs=1e-6)
    assert grpc_pose.vec.z == pytest.approx(pose[2, 3], abs=1e-6)
    q = np.array([grpc_pose.quat.x, grpc_pose.quat.y, grpc_pose.quat.z, grpc_pose.quat.w])
    np.testing.assert_allclose(R.from_quat(q).as_matrix(), pose[:3, :3], atol=1e-6)


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
