"""Coordinate conversions between CARLA (Unreal) and NVIDIA ClipGT (FLU) frames.

Conventions
-----------
CARLA / Unreal: left-handed, x forward, y right, z up, metres, degrees
(``carla.Rotation(pitch, yaw, roll)``).  NVIDIA world scenario: right-handed
FLU, x forward, y **left**, z up, metres, quaternions ``(x, y, z, w)``.

With ``M`` the 4x4 matrix of a ``carla.Transform`` (:func:`ue_matrix`; not ``get_matrix()``, see there) and
``S = diag(1, -1, 1, 1)`` the conversion is ``M_flu = S @ M @ S``.  This maps
points ``(x, y, z) -> (x, -y, z)`` and turns the left-handed rotation into a
proper right-handed one.  The recipe was verified geometrically in the Phase 0
spike (reprojected obstacles land on the right vehicles).  Do not hand-map
Euler angles between the two frames; go through matrices.
"""

from __future__ import annotations

import math
from typing import Sequence

import numpy as np
from scipy.spatial.transform import Rotation

import carla

S_FLIP_Y = np.diag([1.0, -1.0, 1.0, 1.0])
"""Similarity transform between the UE and FLU frames (its own inverse)."""


# ----------------------------------------------------------------------------- UE <-> FLU

def ue_rotation_matrix(rot: carla.Rotation) -> np.ndarray:
    """3x3 rotation of a ``carla.Rotation`` as the engine applies it (left-handed UE frame).

    ``R = Rz(yaw) * Ry(pitch) * Rx(roll)`` with UE signs: ``+pitch`` tilts the forward axis **up**
    (``R[2,0] = +sin(pitch)``), ``+roll`` tilts the right axis **down** (``R[2,1] = -cos(pitch)*sin(roll)``).
    This is CARLA 0.9.x's ``Transform::GetMatrix`` and matches what the engine does with the same
    ``Rotation`` — verified against real camera images (a camera at ``pitch=+20`` fills the frame with
    sky, at ``roll=+25`` the right side sees more ground).

    Not ``tf.get_matrix()``: on the ue58 branch LibCarla commit 4d853ed98 (PR #9751, "pitch/roll fix")
    flipped the pitch and roll signs in ``Rotation::RotateVector`` / ``Transform::GetMatrix`` /
    ``GetInverseMatrix`` / ``get_forward/right/up_vector``, so those describe a camera pitched and
    rolled the other way than the engine mounts it.  Building the matrix here keeps this module correct
    with either wheel; see the hand-off notes ("CARLA geom regression").
    """
    cy, sy = math.cos(math.radians(rot.yaw)), math.sin(math.radians(rot.yaw))
    cp, sp = math.cos(math.radians(rot.pitch)), math.sin(math.radians(rot.pitch))
    cr, sr = math.cos(math.radians(rot.roll)), math.sin(math.radians(rot.roll))
    return np.array([
        [cp * cy, cy * sp * sr - sy * cr, -cy * sp * cr - sy * sr],
        [cp * sy, sy * sp * sr + cy * cr, -sy * sp * cr + cy * sr],
        [sp, -cp * sr, cp * cr],
    ], dtype=np.float64)


def ue_matrix(tf: carla.Transform) -> np.ndarray:
    """4x4 float64 pose of a ``carla.Transform`` (metres), engine convention (see :func:`ue_rotation_matrix`)."""
    m = np.eye(4)
    m[:3, :3] = ue_rotation_matrix(tf.rotation)
    m[:3, 3] = [tf.location.x, tf.location.y, tf.location.z]
    return m


def ue_forward_right_up(rot: carla.Rotation) -> tuple[np.ndarray, np.ndarray, np.ndarray]:
    """Forward / right / up axes of a ``carla.Rotation`` in the engine convention (replaces ``get_*_vector``)."""
    r = ue_rotation_matrix(rot)
    return r[:, 0].copy(), r[:, 1].copy(), r[:, 2].copy()


def ue_to_flu(m_ue: np.ndarray) -> np.ndarray:
    """Convert a 4x4 UE pose (or point-carrying matrix) to FLU."""
    return S_FLIP_Y @ m_ue @ S_FLIP_Y


def flu_to_ue(m_flu: np.ndarray) -> np.ndarray:
    """Convert a 4x4 FLU pose to UE (inverse of :func:`ue_to_flu`)."""
    return S_FLIP_Y @ m_flu @ S_FLIP_Y


def ue_point_to_flu(p: Sequence[float]) -> np.ndarray:
    """Convert a UE point ``(x, y, z)`` to FLU."""
    return np.array([p[0], -p[1], p[2]], dtype=np.float64)


def ue_transform_from_matrix(m: np.ndarray, tol: float = 1e-3) -> carla.Transform:
    """Inverse of :func:`ue_matrix` (engine convention).

    With ``R = Rz(yaw) * Ry(pitch) * Rx(roll)`` as in :func:`ue_rotation_matrix`:
    ``pitch = asin(m[2,0])``, ``yaw = atan2(m[1,0], m[0,0])``, ``roll = atan2(-m[2,1], m[2,2])``.
    The result is verified by round trip; a mismatch larger than ``tol`` (non-rigid input) raises
    ``ValueError``.
    """
    r = m[:3, :3]
    sp = float(np.clip(r[2, 0], -1.0, 1.0))
    pitch = math.degrees(math.asin(sp))
    yaw = math.degrees(math.atan2(r[1, 0], r[0, 0]))
    roll = math.degrees(math.atan2(-r[2, 1], r[2, 2]))
    tf = carla.Transform(
        carla.Location(x=float(m[0, 3]), y=float(m[1, 3]), z=float(m[2, 3])),
        carla.Rotation(pitch=pitch, yaw=yaw, roll=roll),
    )
    err = float(np.abs(ue_matrix(tf) - m).max())
    if err > tol:
        raise ValueError(f"matrix is not a rigid UE transform (round-trip error {err:.3e})")
    return tf


# ----------------------------------------------------------------------------- FLU helpers

def flu_pose_matrix(t: Sequence[float], rpy_deg: Sequence[float]) -> np.ndarray:
    """FLU pose from translation (m) and roll-pitch-yaw (deg, intrinsic ``xyz``).

    This is exactly how NVIDIA's ClipGT loader reads ``nominalSensor2Rig_FLU``
    (``Rotation.from_euler("xyz", radians(rpy))``).
    """
    m = np.eye(4)
    m[:3, :3] = Rotation.from_euler("xyz", np.radians(rpy_deg)).as_matrix()
    m[:3, 3] = t
    return m


def flu_rpy_deg(m_flu: np.ndarray) -> list[float]:
    """Roll-pitch-yaw (deg, ``xyz``) of an FLU pose, for ``rig_json``."""
    return [float(v) for v in Rotation.from_matrix(m_flu[:3, :3]).as_euler("xyz", degrees=True)]


def quat_xyzw(m: np.ndarray) -> dict[str, float]:
    """Rotation of a 4x4 pose as ``{"x","y","z","w"}`` (scipy order)."""
    q = Rotation.from_matrix(m[:3, :3]).as_quat()
    return {"x": float(q[0]), "y": float(q[1]), "z": float(q[2]), "w": float(q[3])}


def xyz(v: Sequence[float]) -> dict[str, float]:
    """Vector as ``{"x","y","z"}``."""
    return {"x": float(v[0]), "y": float(v[1]), "z": float(v[2])}


def bbox_matrix_ue(bb: carla.BoundingBox) -> np.ndarray:
    """World-space ``carla.BoundingBox`` (as returned by ``get_level_bbs``) as a UE pose."""
    m = ue_matrix(carla.Transform(carla.Location(), bb.rotation))
    m[:3, 3] = [bb.location.x, bb.location.y, bb.location.z]
    return m


# ----------------------------------------------------------------------------- camera model

def pinhole_focal_px(width: int, hfov_deg: float) -> float:
    """Pinhole focal length in pixels for a horizontal FOV."""
    return (width / 2.0) / math.tan(math.radians(hfov_deg) / 2.0)


def pinhole_ftheta_poly(width: int, height: int, hfov_deg: float, order: int = 5,
                        samples: int = 400) -> tuple[list[float], float]:
    """Fit NVIDIA's ``pixeldistance-to-angle`` polynomial to a pinhole camera.

    theta(r) = sum_{i=1..order} k_i r^i (k_0 = 0) is least-squares fitted to
    ``atan(r / f)`` on ``r in [0, r_corner]``.  Returns ``(poly[6], max_resid_rad)``.
    A pinhole is not a polynomial, so the residual grows with FOV; for the 120
    degree views it stays below 1e-3 rad at 1280x720 (checked in tests).
    """
    f = pinhole_focal_px(width, hfov_deg)
    r_max = math.hypot(width / 2.0, height / 2.0)
    r = np.linspace(0.0, r_max, samples)
    theta = np.arctan(r / f)
    design = np.stack([r ** i for i in range(1, order + 1)], axis=1)
    k, *_ = np.linalg.lstsq(design, theta, rcond=None)
    poly = [0.0] + [float(v) for v in k]
    poly += [0.0] * (6 - len(poly))
    resid = float(np.abs(design @ k - theta).max())
    return poly, resid
