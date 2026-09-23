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


# ---------------------------------------------------------------------------
# CARLA <-> NuRec frame conventions
# ---------------------------------------------------------------------------
#
# CARLA / Unreal world frame: left-handed, x forward, y RIGHT, z up. Its euler
# angles compose as ``R = Rz(+yaw) * Ry(-pitch) * Rx(-roll)`` with the standard
# (right-handed) elementary matrices: +pitch tilts the nose UP, +roll drops the
# RIGHT side, +yaw turns the nose right. That is what the engine builds from a
# ``carla.Rotation`` and what ``carla.Transform.get_matrix()`` returns; see
# ``Docs/coordinate_conventions.md``.
#
# NuRec / ClipGT rig frame: right-handed FLU, x forward, y LEFT, z up (verified
# against the shipped ``rig_trajectories.json`` calibrations: the ego advances
# along +x, ``camera_cross_left_120fov`` sits at y = +0.95 and
# ``camera_cross_right_120fov`` at y = -0.98, both ~+1 m in z). Rig poses
# compose as ``R = Rz(yaw) * Ry(pitch) * Rx(roll)``, i.e. scipy's *extrinsic*
# ``"xyz"`` euler order. This is exactly CARLA's ``RightHandedTransform``.
#
# Mapping between them (with S = diag(1, -1, 1)): location ``(x, -y, z)``,
# rotation ``(roll, -pitch, -yaw)``, matrix ``M' = S @ M @ S``. PITCH AND YAW
# FLIP SIGN, ROLL DOES NOT. The matrix form is exact for any pose and is its
# own inverse; the euler form is what ``carla.Transform.to_right_handed()`` /
# ``carla.Transform.from_right_handed()`` implement in LibCarla.
#
# HISTORY — do not re-fit this to the wheel: CARLA PR #9751 mirrored the pitch
# and roll signs of ``carla::geom`` while leaving the engine alone, so on wheels
# built between 4d853ed98 and the fix ``get_matrix()`` returns the mirror image
# of the pose Unreal actually builds. ``mat_to_carla_transform`` below had been
# fitted to that mirrored ``get_matrix()`` and therefore placed every pitched or
# rolled camera/actor at MINUS the intended angle (2x the angle wrong); the old
# round-trip tests could not see it because they measured the wheel against
# itself. Never derive the euler signs from ``get_matrix()`` — derive them from
# the engine convention above, and check with
# ``carla.Transform(carla.Location(), carla.Rotation(pitch=20)).get_matrix()[2][0]``
# which must be **+0.342**.
_Y_MIRROR = np.diag([1.0, -1.0, 1.0, 1.0])

# ``carla.Transform.to_right_handed`` / ``from_right_handed`` and the
# ``RightHanded*`` types landed with the geom fix. When they are present we use
# them, so the handedness change lives in exactly one place (LibCarla).
_HAS_RH_BOUNDARY = hasattr(carla.Transform, "from_right_handed")


def _require_fixed_wheel() -> None:
    """
    Refuse to build CARLA poses on a wheel whose geom math is still mirrored.

    Failing loudly beats mounting every pitched sensor at minus the intended
    angle for two months, which is what happened the last time this went
    unnoticed.
    """
    if _HAS_RH_BOUNDARY:
        return
    probe = np.array(
        carla.Transform(carla.Location(), carla.Rotation(pitch=20.0)).get_matrix()
    ).reshape(4, 4)[2][0]
    raise RuntimeError(
        "This carla wheel predates the geom pitch/roll fix "
        "(carla.Transform.from_right_handed is missing; "
        f"Rotation(pitch=20).get_matrix()[2][0] = {probe:+.3f}, expected +0.342). "
        "NuRec camera and actor poses would be mirrored in pitch and roll. "
        "Rebuild the PythonAPI wheel from a branch that carries "
        "'fix(geom): restore the engine pitch/roll sign convention in LibCarla'."
    )


def mirror_pose(transform: np.ndarray) -> np.ndarray:
    """
    Map a 4x4 pose matrix between the NuRec (RH, y-left) and CARLA (LH, y-right)
    frames: ``S @ M @ S`` with ``S = diag(1, -1, 1, 1)``. Self-inverse.

    Identical, element for element, to
    ``carla.Transform.to_right_handed().get_matrix()`` on a fixed wheel (pinned
    by ``tests/test_coordinates.py::test_mirror_pose_matches_libcarla_boundary``).
    """
    return _Y_MIRROR @ transform @ _Y_MIRROR


def undo_carla_coordinate_transform(transform: np.ndarray) -> np.ndarray:
    """
    Convert a CARLA pose matrix (from ``Transform.get_matrix()``) into the NuRec
    right-handed frame. Kept under its historic name; it is mirror_pose().
    """
    return mirror_pose(transform)


def carla_transform_to_nurec(transform: carla.Transform) -> np.ndarray:
    """
    CARLA pose -> 4x4 NuRec (right-handed FLU) pose matrix, through LibCarla's
    explicit handedness boundary. Preferred over
    ``undo_carla_coordinate_transform(t.get_matrix())`` whenever the
    ``carla.Transform`` itself is in hand.
    """
    if _HAS_RH_BOUNDARY:
        return np.array(transform.to_right_handed().get_matrix()).reshape(4, 4)
    _require_fixed_wheel()  # raises
    raise AssertionError("unreachable")


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
    Convert a NuRec 4x4 pose (right-handed FLU) to a ``carla.Transform``.

    The NuRec rig frame composes as ``Rz(yaw) @ Ry(pitch) @ Rx(roll)`` with the
    standard right-handed elementary matrices -- scipy's *extrinsic* ``"xyz"``
    order -- which is precisely ``carla.RightHandedTransform``. Handing that to
    ``carla.Transform.from_right_handed()`` performs the (x, -y, z) /
    (roll, -pitch, -yaw) mapping inside LibCarla, so the handedness change is
    not duplicated here.

    Note the sign flip on pitch and yaw: a rig pose that is nose-down by 8 deg
    (right-handed pitch = +8, y pointing left) becomes ``carla.Rotation`` with
    pitch = -8. The pre-2026-08-28 implementation decomposed the *mirrored*
    matrix and passed the angles straight through, which cancelled against the
    then-mirrored ``get_matrix()`` in the tests but placed the real actor or
    sensor at +8 -- twice the intended angle away from the truth.
    """
    _require_fixed_wheel()
    roll, pitch, yaw = R.from_matrix(np.asarray(mat)[:3, :3]).as_euler(
        "xyz", degrees=True
    )
    return carla.Transform.from_right_handed(
        carla.RightHandedTransform(
            carla.RightHandedVector3D(
                x=float(mat[0, 3]), y=float(mat[1, 3]), z=float(mat[2, 3])
            ),
            carla.RightHandedRotation(
                roll=float(roll), pitch=float(pitch), yaw=float(yaw)
            ),
        )
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

