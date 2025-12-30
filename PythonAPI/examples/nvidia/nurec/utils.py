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


def undo_carla_coordinate_transform(transform: np.ndarray) -> np.ndarray:
    """
    Undoes inverse yaw and mirror over y axis.
    Note carla applies yaw last in euler angles so we don't need to worry about the other rotations.
    This effectivley mirrors actors over the y axis, assuming they are symmetric on their y axis.
    """
    result = np.eye(4)
    rotation = R.from_matrix(transform[:3, :3])
    yaw, pitch, roll = rotation.as_euler("zyx", degrees=False)
    # yaw, pitch, roll = roll, -yaw, pitch
    # yaw, pitch, roll = -pitch, roll, yaw
    yaw = -yaw
    roll = roll
    pitch = -pitch
    rotation = R.from_euler("zyx", [yaw, pitch, roll], degrees=False)
    # carla.Rotation(pitch=euler_angles[2] * 180 / np.pi, yaw=-euler_angles[0] * 180 / np.pi, roll=euler_angles[1] * 180 / np.pi)
    result[:3, :3] = rotation.as_matrix()
    result[:3, 3] = [transform[0, 3], -transform[1, 3], transform[2, 3]]
    return result


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
    euler_angles = R.from_matrix(mat[:3, :3]).as_euler("zyx", degrees=False)
    euler_angles[1] = -euler_angles[1]
    euler_angles[2] = -euler_angles[2]
    return carla.Transform(
        carla.Location(x=mat[0, 3], y=-mat[1, 3], z=mat[2, 3]),
        carla.Rotation(
            pitch=euler_angles[2] * 180 / np.pi,
            yaw=-euler_angles[0] * 180 / np.pi,
            roll=euler_angles[1] * 180 / np.pi,
        ),
    )


def mat_to_carla_transform2(mat: np.ndarray) -> carla.Transform:
    euler_angles = R.from_matrix(mat[:3, :3]).as_euler("xyz", degrees=False)
    return carla.Transform(
        carla.Location(x=mat[0, 3], y=-mat[1, 3], z=mat[2, 3]),
        carla.Rotation(
            pitch=euler_angles[0] * 180 / np.pi,
            yaw=-euler_angles[1] * 180 / np.pi,
            roll=euler_angles[2] * 180 / np.pi,
        ),
    )


def xyzquat_to_carla_transform(xyzquat: np.ndarray) -> carla.Transform:
    euler_angles = R.from_quat(xyzquat[3:]).as_euler("zyx", degrees=False)
    euler_angles[1] = -euler_angles[1]
    euler_angles[2] = -euler_angles[2]
    return carla.Transform(
        carla.Location(x=xyzquat[0], y=-xyzquat[1], z=xyzquat[2]),
        carla.Rotation(
            pitch=euler_angles[2] * 180 / np.pi,
            yaw=-euler_angles[0] * 180 / np.pi,
            roll=euler_angles[1] * 180 / np.pi,
        ),
    )


def xyzeuler_to_carla_transform(xyzeuler: np.ndarray) -> carla.Transform:
    return carla.Transform(
        carla.Location(x=xyzeuler[0], y=-xyzeuler[1], z=xyzeuler[2]),
        carla.Rotation(
            pitch=xyzeuler[5] * 180 / np.pi,
            yaw=-xyzeuler[3] * 180 / np.pi,
            roll=xyzeuler[4] * 180 / np.pi,
        ),
    )

