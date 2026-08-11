# SPDX-FileCopyrightText: © 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#
# SPDX-License-Identifier: MIT

"""
Utility Functions Module

This module provides utility functions for coordinate transformations and conversions
between different representation formats used in NUREC and CARLA integration.

Key Functions:
- se3_to_grpc_pose: Convert SE3 transformation matrices to gRPC Pose format
- undo_carla_coordinate_transform: Handle coordinate system differences between CARLA and NUREC
- actor_to_grpc_pose: Convert CARLA actor poses to gRPC format with blueprint offsets
- mat_to_carla_transform: Convert 4x4 matrices to CARLA Transform objects
- xyzquat_to_carla_transform: Convert xyz+quaternion to CARLA Transform
- xyzeuler_to_carla_transform: Convert xyz+euler angles to CARLA Transform
- handle_exception: Handle timeout exceptions and write stack traces to temp files

The module handles the coordinate system differences between CARLA (left-handed, Z-up)
and NUREC (right-handed coordinate systems), applying necessary transformations for
proper alignment. It also manages vehicle blueprint offsets between bounding box
centers and reference points (typically rear axle center).
"""

import numpy as np
from nre.grpc.protos import common_pb2 as grpc_types
import carla
from scipy.spatial.transform import Rotation as R
import logging
from typing import Optional, Dict, Any
from blueprint_library import BlueprintLibrary
import tempfile
import traceback
import os

logger = logging.getLogger(__name__)

def handle_exception(exception: Exception) -> None:
    """
    Handle exceptions and check for timeout conditions.
    
    Args:
        exception: The exception that was caught
    """
    # Check if the exception contains "timeout", "time-out" in its message
    exception_str = str(exception).lower()
    is_timeout = "timeout" in exception_str or "time-out" in exception_str
    
    if is_timeout:
        logger.error("Received a timeout exception. The CARLA simulator has exited.")
        logger.error(f"Timeout details: {exception}")
    else:
        logger.error(f"{exception}")
    
    # Write stack trace to a temp file
    try:
        with tempfile.NamedTemporaryFile(mode='w', suffix='.txt', delete=False, prefix='carla_exception_') as temp_file:
            temp_file.write(f"Exception: {type(exception).__name__}\n")
            temp_file.write(f"Message: {exception}\n")
            temp_file.write("\nStack trace:\n")
            temp_file.write(traceback.format_exc())
            temp_file_path = temp_file.name
        
        logger.info(f"Stack trace written to: {temp_file_path}")
    except Exception as write_error:
        logger.error(f"Failed to write stack trace to temp file: {write_error}")


def se3_to_grpc_pose(se3: np.ndarray) -> grpc_types.Pose:
    """
    Converts a single SE3 4x4 matrix into a Alpasim gRPC Pose

    Args:
        single SE3 matrix [np.ndarray]

    Returns:
        single Alpasim gRPC Pose [grpc_types.Pose]
    """
    quat = R.from_matrix(se3[..., :3, :3]).as_quat(canonical=False)
    vec3 = se3[..., :3, 3]

    return grpc_types.Pose(
        vec=grpc_types.Vec3(
            x=vec3[0],
            y=vec3[1],
            z=vec3[2],
        ),
        quat=grpc_types.Quat(
            x=quat[0],
            y=quat[1],
            z=quat[2],
            w=quat[3],
        ),
    )


# CARLA is left-handed (Y mirrored) relative to NuRec's right-handed frame.
# Both directions of the conversion are the SAME operation: conjugation by the
# Y-mirror, M' = S @ M @ S. This is exact for any pose — no euler-angle
# order/sign juggling — and is its own inverse. The previous hand-rolled euler
# implementation swapped pitch and roll (verified against
# carla.Transform.get_matrix() on the 0.10 wheel; see tests/test_coordinates.py).
_Y_MIRROR = np.diag([1.0, -1.0, 1.0, 1.0])


def mirror_pose(transform: np.ndarray) -> np.ndarray:
    """Map a 4x4 pose between NuRec (RH) and CARLA (LH) conventions (self-inverse)."""
    return _Y_MIRROR @ transform @ _Y_MIRROR


def undo_carla_coordinate_transform(transform: np.ndarray) -> np.ndarray:
    """
    Convert a CARLA pose matrix (from Transform.get_matrix()) into the NuRec
    right-handed frame. Kept under its historic name; it is mirror_pose().
    """
    return mirror_pose(transform)


def actor_to_grpc_pose(
    actor: carla.ActorSnapshot,
    t_carla_nurec: np.ndarray,
    blueprint_library: Optional[BlueprintLibrary] = None,
    actor_blueprints: Optional[Dict[int, str]] = None,
) -> grpc_types.Pose:
    transform_matrix = np.array(actor.get_transform().get_matrix()).reshape(4, 4)

    # Apply inverse offset if this is a vehicle and we have its blueprint ID
    if (
        blueprint_library is not None
        and actor_blueprints is not None
        and actor.id in actor_blueprints
    ):
        blueprint_id = actor_blueprints[actor.id]
        # Apply inverse offset (rear axle center to bounding box center)
        transform_matrix_new = blueprint_library.apply_offset_to_pose(
            transform_matrix, blueprint_id, inverse=False
        )
        # print(f"Applied inverse offset to actor {actor.id}")
        # print(f"Transform matrix: {transform_matrix_new[0, 3]} {transform_matrix_new[1, 3]} {transform_matrix_new[2, 3]}")
        # print(f"Was {transform_matrix[0, 3]} {transform_matrix[1, 3]} {transform_matrix[2, 3]}")
        # print(f"Blueprint ID: {blueprint_id}")
        transform_matrix = transform_matrix_new
    else:
        logger.warning(f"Actor {actor.id} not found in actor_mapping")

    transform_matrix = t_carla_nurec @ undo_carla_coordinate_transform(transform_matrix)
    vec = grpc_types.Vec3(
        x=transform_matrix[0, 3],
        y=transform_matrix[1, 3],
        z=transform_matrix[2, 3],
    )
    quat = R.from_matrix(transform_matrix[:3, :3]).as_quat(canonical=False)
    quat = grpc_types.Quat(
        x=quat[0],
        y=quat[1],
        z=quat[2],
        w=quat[3],
    )
    return grpc_types.Pose(vec=vec, quat=quat)

def mat_to_carla_transform(mat: np.ndarray) -> carla.Transform:
    """
    Convert a NuRec 4x4 pose to a carla.Transform.

    The carla wheel composes rotations as Rx(roll) @ Ry(pitch) @ Rz(yaw)
    (verified numerically against Transform.get_matrix(); pinned by
    tests/test_coordinates.py), i.e. scipy's intrinsic 'xyz' order.
    """
    mirrored = mirror_pose(mat)
    roll, pitch, yaw = R.from_matrix(mirrored[:3, :3]).as_euler("xyz", degrees=True)
    return carla.Transform(
        carla.Location(x=mirrored[0, 3], y=mirrored[1, 3], z=mirrored[2, 3]),
        carla.Rotation(pitch=pitch, yaw=yaw, roll=roll),
    )


def xyzquat_to_carla_transform(xyzquat: np.ndarray) -> carla.Transform:
    """xyzquat: [x, y, z, qx, qy, qz, qw] in the NuRec frame."""
    mat = np.eye(4)
    mat[:3, :3] = R.from_quat(xyzquat[3:]).as_matrix()
    mat[:3, 3] = xyzquat[:3]
    return mat_to_carla_transform(mat)


def xyzeuler_to_carla_transform(xyzeuler: np.ndarray) -> carla.Transform:
    """xyzeuler: [x, y, z, yaw_z, pitch_y, roll_x] (radians) in the NuRec frame."""
    mat = np.eye(4)
    mat[:3, :3] = R.from_euler("zyx", xyzeuler[3:], degrees=False).as_matrix()
    mat[:3, 3] = xyzeuler[:3]
    return mat_to_carla_transform(mat)

