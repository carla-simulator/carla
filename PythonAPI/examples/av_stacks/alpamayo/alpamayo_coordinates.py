"""CARLA left-handed to Alpamayo right-handed coordinate conversions."""

from __future__ import annotations

import math
from collections.abc import Sequence

import numpy as np


Y_REFLECTION = np.diag([1.0, -1.0, 1.0])


def carla_rotation_matrix(pitch_deg: float, yaw_deg: float, roll_deg: float) -> np.ndarray:
    """Match the rotation block returned by ``carla.Transform.get_matrix``."""
    pitch = -math.radians(pitch_deg)
    yaw = math.radians(yaw_deg)
    roll = -math.radians(roll_deg)
    cp, sp = math.cos(pitch), math.sin(pitch)
    cy, sy = math.cos(yaw), math.sin(yaw)
    cr, sr = math.cos(roll), math.sin(roll)
    return np.array(
        [
            [cp * cy, cy * sp * sr - sy * cr, -cy * sp * cr - sy * sr],
            [cp * sy, sy * sp * sr + cy * cr, -sy * sp * cr + cy * sr],
            [sp, -cp * sr, cp * cr],
        ],
        dtype=np.float64,
    )


def transform_components(transform: object) -> tuple[np.ndarray, np.ndarray]:
    location = transform.location
    rotation = transform.rotation
    xyz = np.array([location.x, location.y, location.z], dtype=np.float64)
    matrix = carla_rotation_matrix(rotation.pitch, rotation.yaw, rotation.roll)
    return xyz, matrix


def rear_axle_world_pose(transform: object, rear_axle_x: float) -> tuple[np.ndarray, np.ndarray]:
    actor_xyz, actor_rotation = transform_components(transform)
    rear_axle_xyz = actor_xyz + actor_rotation @ np.array([rear_axle_x, 0.0, 0.0])
    return rear_axle_xyz, actor_rotation


def history_to_alpamayo(
    transforms: Sequence[object], rear_axle_x: float
) -> tuple[np.ndarray, np.ndarray]:
    if not transforms:
        raise ValueError("at least one transform is required")
    poses = [rear_axle_world_pose(transform, rear_axle_x) for transform in transforms]
    t0_xyz, t0_rotation = poses[-1]
    positions = []
    rotations = []
    for xyz, rotation in poses:
        relative_xyz = t0_rotation.T @ (xyz - t0_xyz)
        relative_rotation = t0_rotation.T @ rotation
        positions.append(Y_REFLECTION @ relative_xyz)
        rotations.append(Y_REFLECTION @ relative_rotation @ Y_REFLECTION)
    return (
        np.asarray(positions, dtype=np.float32)[None, None],
        np.asarray(rotations, dtype=np.float32)[None, None],
    )


def alpamayo_points_to_carla_local(points: np.ndarray) -> np.ndarray:
    points = np.asarray(points)
    if points.shape[-1] != 3:
        raise ValueError("points must end in xyz")
    result = points.copy()
    result[..., 1] *= -1.0
    return result
