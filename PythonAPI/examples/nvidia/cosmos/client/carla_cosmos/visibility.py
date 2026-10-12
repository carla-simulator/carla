"""Occlusion-aware obstacle visibility: a z-buffer test against the captured depth AOVs.

Why
---
NVIDIA's RDS-HQ obstacle tracks come from lidar auto-labelling, so a car parked behind a
building simply has no label, while a car half-hidden behind another car usually still has
one.  The exporter's ground truth is the simulator's, which knows every actor whether a
camera can see it or not, so an unfiltered export paints boxes over buildings — out of
distribution for the Cosmos AV renderer, which then hallucinates a vehicle there.

The test
--------
Every obstacle is sampled on a 3x3x3 lattice of its oriented box (:data:`BOX_GRID`; the
27 points contain the 8 corners and the centre).  The points are projected into each
camera with **the clip's own calibration** — the f-theta polynomial the exporter writes
into ``calibration_estimate`` and the renderer reads back — and their camera-space depth
is compared with the captured depth image at that pixel:

    occluded  <=>  depth_image  <  point_depth - tol      (something nearer covers it)
    visible   <=>  depth_image >=  point_depth - tol
    tol = ``tol_m`` + ``tol_rel`` * point_depth           (0.5 m + 2 % by default)

``point_depth`` is the FLU camera-frame **x**, i.e. planar view-space depth, because that
is what CARLA's depth AOV stores (measured on the live server: reconstructing the ground
plane from a 90-degree camera gives the same camera height at the image centre and at the
edge under the planar reading, and heights that differ by 35 % under a radial one).

The tolerance is one-sided on purpose.  The sample points sit on the *bounding box*, which
is circumscribed around a mesh that is usually a little nearer, so the depth image at a
box point is legitimately in front of it by a few centimetres; only a surface clearly
nearer than the box counts as an occluder.

Self-occlusion is not special-cased: the lattice points on the far side of a car are
covered by the car itself, so an entirely unoccluded vehicle scores roughly a third of its
points rather than all of them.  That is why :attr:`VisibilityParams.min_visible_fraction`
defaults to 0.05 — the fraction separates "a few pixels of it are on screen" from "nothing
of it is", not "all of it" from "some of it".

Track segments
--------------
NVIDIA's ClipGT loader (``clipgt_loader._load_dynamic_objects`` -> ``DynamicObject.
get_pose_at_timestamp``) linearly interpolates a track between its observations for *any*
timestamp inside ``[t_first, t_last]``: a hole in the middle of a track is invisible to it,
the box is simply drawn along the straight line across the hole.  Outside the range it
extrapolates at constant velocity for up to ``max_extrapolation_us`` (500 ms in the ClipGT
loader) and only then drops the object.  So dropping rows is not enough — a track that
disappears and comes back must be split into a separate ``trackline_id`` per visible
segment, which is what :func:`segments` and :func:`segment_ids` are for.  The loader also
skips tracks with fewer than two observations, so single-tick segments are dropped.

Everything here is pure numpy: no CARLA, no clip on disk, no I/O.
"""

from __future__ import annotations

import logging
from dataclasses import dataclass, replace
from typing import Iterable, Sequence

import numpy as np

from .preview import FThetaCamera, PreviewCamera, euler_to_matrix, pose_matrix, transform_points

log = logging.getLogger(__name__)

MODES = ("depth", "none")
"""Accepted ``visibility`` values: z-buffer test, or export everything (the old behaviour)."""

BOX_GRID: np.ndarray = np.array(
    [[sx, sy, sz] for sx in (-0.5, 0.0, 0.5) for sy in (-0.5, 0.0, 0.5) for sz in (-0.5, 0.0, 0.5)],
    dtype=np.float64)
"""3x3x3 lattice in box-local units (27 points; contains the 8 corners and the centre)."""

ABSENT = -1
"""Per-tick state of a track whose actor was not in the snapshot."""


@dataclass(frozen=True)
class VisibilityParams:
    """Knobs of the occlusion filter (recorded in ``manifest.json`` and the sidecar)."""

    mode: str = "depth"
    min_visible_fraction: float = 0.05
    """Smallest visible-point fraction (over the points inside the frame) that still counts."""
    range_m: float = 150.0
    """Sample points farther than this from a camera do not count as seen by it."""
    hysteresis_frames: int = 3
    """Invisible runs shorter than this are bridged instead of splitting the track."""
    tol_m: float = 0.5
    tol_rel: float = 0.02
    grid: int = 3
    """Lattice resolution per axis (3 -> the 27 points of :data:`BOX_GRID`)."""

    def __post_init__(self) -> None:
        if self.mode not in MODES:
            raise ValueError(f"visibility must be one of {MODES}, not '{self.mode}'")
        if not 0.0 <= self.min_visible_fraction <= 1.0:
            raise ValueError("min_visible_fraction must be in [0, 1]")
        if self.range_m <= 0:
            raise ValueError("range_m must be > 0")
        if self.hysteresis_frames < 0:
            raise ValueError("hysteresis_frames must be >= 0")
        if self.grid < 2:
            raise ValueError("grid must be >= 2")

    @property
    def enabled(self) -> bool:
        return self.mode == "depth"

    def lattice(self) -> np.ndarray:
        """Box-local sample lattice for this ``grid`` (``BOX_GRID`` for the default 3)."""
        if self.grid == 3:
            return BOX_GRID
        t = np.linspace(-0.5, 0.5, self.grid)
        return np.array([[x, y, z] for x in t for y in t for z in t], dtype=np.float64)

    def as_dict(self) -> dict:
        return {"visibility": self.mode, "min_visible_fraction": self.min_visible_fraction,
                "range_m": self.range_m, "hysteresis_frames": self.hysteresis_frames,
                "tol_m": self.tol_m, "tol_rel": self.tol_rel, "grid": self.grid}

    @classmethod
    def from_dict(cls, data: dict) -> "VisibilityParams":
        known = {"min_visible_fraction", "range_m", "hysteresis_frames", "tol_m", "tol_rel", "grid"}
        kwargs = {k: v for k, v in data.items() if k in known}
        return cls(mode=str(data.get("visibility", data.get("mode", "depth"))), **kwargs)

    def with_mode(self, mode: str) -> "VisibilityParams":
        return replace(self, mode=mode)


# ----------------------------------------------------------------------------- geometry (pure)

def box_sample_points(center: Sequence[float], size: Sequence[float], quat_xyzw: Sequence[float],
                      lattice: np.ndarray = BOX_GRID) -> np.ndarray:
    """Sample points of one oriented box in world coordinates: ``(len(lattice), 3)``."""
    from .preview import quat_to_matrix

    return (np.asarray(lattice, float) * np.asarray(size, float)) @ quat_to_matrix(quat_xyzw).T \
        + np.asarray(center, float)


def stack_sample_points(boxes: Iterable[tuple[Sequence[float], Sequence[float], Sequence[float]]],
                        lattice: np.ndarray = BOX_GRID) -> np.ndarray:
    """Sample points of many ``(center, size, quat)`` boxes: ``(n_boxes, n_points, 3)``."""
    rows = [box_sample_points(c, s, q, lattice) for c, s, q in boxes]
    if not rows:
        return np.zeros((0, len(lattice), 3))
    return np.stack(rows)


def box_points_from_poses(rotations: np.ndarray, centers: np.ndarray, sizes: np.ndarray,
                          lattice: np.ndarray = BOX_GRID) -> np.ndarray:
    """Sample points of many boxes given as rotation matrices: ``(K, len(lattice), 3)``.

    The vectorised form of :func:`box_sample_points` for the per-tick path, which already
    holds the FLU pose of every obstacle and must not pay for a quaternion round trip.
    """
    R = np.asarray(rotations, float).reshape(-1, 3, 3)
    C = np.asarray(centers, float).reshape(-1, 3)
    S = np.asarray(sizes, float).reshape(-1, 3)
    if len(R) == 0:
        return np.zeros((0, len(lattice), 3))
    local = np.asarray(lattice, float)[None, :, :] * S[:, None, :]
    return np.einsum("kij,ksj->ksi", R, local) + C[:, None, :]


def camera_from_manifest(cam) -> PreviewCamera:
    """A :class:`~carla_cosmos.preview.PreviewCamera` for a :class:`CameraManifest`.

    Deliberately the *exported* camera model, not the CARLA sensor's pinhole: the f-theta
    polynomial fitted in :func:`carla_cosmos.coords.pinhole_ftheta_poly` is what lands in
    ``calibration_estimate`` and what NVIDIA's renderer projects with, so the filter and the
    renderer agree on where an obstacle is.  The fit residual is below 1e-3 rad (well under a
    pixel at 1280x720), so it indexes the pinhole depth image correctly too.
    """
    from . import coords

    poly, _resid = coords.pinhole_ftheta_poly(cam.width, cam.height, cam.hfov)
    lens = FThetaCamera(name=cam.name, width=int(cam.width), height=int(cam.height),
                        cx=cam.width / 2.0, cy=cam.height / 2.0, coefficients=tuple(poly))
    return PreviewCamera(name=cam.name, lens=lens,
                         sensor_to_rig=pose_matrix(euler_to_matrix(cam.rpy_flu), cam.t_flu))


# ----------------------------------------------------------------------------- the z-buffer test

def camera_visibility(points_world: np.ndarray, world_to_camera: np.ndarray, camera: PreviewCamera,
                      depth: np.ndarray, params: VisibilityParams = VisibilityParams()
                      ) -> tuple[np.ndarray, np.ndarray]:
    """Visible / in-frame point counts of every box in one camera.

    ``points_world`` is ``(n_boxes, n_points, 3)`` in the FLU world frame, ``depth`` the raw
    metric depth image ``(H, W)`` of that camera for the same tick.  Returns
    ``(n_visible, n_in_frame)``, both ``(n_boxes,)`` integer arrays.
    """
    P = np.asarray(points_world, float)
    if P.size == 0:
        return np.zeros(len(P), np.int64), np.zeros(len(P), np.int64)
    n_boxes, n_pts, _ = P.shape
    cam_pts = transform_points(P.reshape(-1, 3), world_to_camera)
    uv = camera.lens.project(cam_pts)
    d_pt = cam_pts[:, 0]  # planar view-space depth, the convention of CARLA's depth AOV
    h, w = depth.shape[:2]
    u = np.rint(uv[:, 0])
    v = np.rint(uv[:, 1])
    inside = (np.isfinite(u) & np.isfinite(v) & (u >= 0) & (u < w) & (v >= 0) & (v < h)
              & (d_pt > 0.0) & (d_pt <= params.range_m))
    ui = np.where(inside, u, 0.0).astype(np.intp)
    vi = np.where(inside, v, 0.0).astype(np.intp)
    d_img = np.asarray(depth, float)[vi, ui]
    tol = params.tol_m + params.tol_rel * d_pt
    visible = inside & (d_img >= d_pt - tol)
    return (visible.reshape(n_boxes, n_pts).sum(1).astype(np.int64),
            inside.reshape(n_boxes, n_pts).sum(1).astype(np.int64))


def visible_fractions(points_world: np.ndarray, ego_pose: np.ndarray,
                      cameras: Sequence[PreviewCamera], depths: dict[str, np.ndarray],
                      params: VisibilityParams = VisibilityParams()) -> np.ndarray:
    """Per-box visible fraction, unioned over the cameras (one decision per object per tick).

    ``-1.0`` means "no camera had a single sample point inside its frame", which is a
    different thing from "every point was covered" and is what the caller reports as
    *out of frame* rather than *occluded*.  ClipGT tracks are global, not per view, so the
    fraction of the camera that sees the object best is the one that decides.
    """
    P = np.asarray(points_world, float)
    best = np.full(len(P), -1.0)
    if P.size == 0:
        return best
    for cam in cameras:
        depth = depths.get(cam.name)
        if depth is None:
            continue
        n_vis, n_in = camera_visibility(P, cam.world_to_camera(ego_pose), cam, depth, params)
        seen = n_in > 0
        frac = np.where(seen, n_vis / np.maximum(n_in, 1), -1.0)
        best = np.maximum(best, frac)
    return best


# ----------------------------------------------------------------------------- decisions over time

def apply_hysteresis(states: np.ndarray, frames: int) -> np.ndarray:
    """Bridge invisible runs shorter than ``frames`` that sit between two visible runs.

    ``states`` is a per-tick array of ``1`` (visible), ``0`` (invisible) and :data:`ABSENT`
    (the actor was not in the snapshot).  Absences are never bridged — the actor is gone, the
    track legitimately ends — and leading/trailing invisible runs are not bridged either,
    since there is nothing on the other side to interpolate from.  Returns a new array.
    """
    out = np.asarray(states, np.int8).copy()
    if frames <= 1 or out.size == 0:
        return out
    i = 0
    n = out.size
    while i < n:
        if out[i] != 0:
            i += 1
            continue
        j = i
        while j < n and out[j] == 0:
            j += 1
        if (j - i) < frames and i > 0 and j < n and out[i - 1] == 1 and out[j] == 1:
            out[i:j] = 1
        i = j
    return out


def segments(states: np.ndarray) -> list[tuple[int, int]]:
    """Maximal half-open runs ``[start, stop)`` of ticks where ``states == 1``."""
    s = np.asarray(states, np.int8)
    out: list[tuple[int, int]] = []
    i = 0
    while i < s.size:
        if s[i] != 1:
            i += 1
            continue
        j = i
        while j < s.size and s[j] == 1:
            j += 1
        out.append((i, j))
        i = j
    return out


def segment_ids(track_id: str, n_segments: int) -> list[str]:
    """Track ids for ``n_segments`` visibility segments.

    A track that never disappears keeps its plain id (identical output to an unfiltered
    export); one that does gets ``<id>#<k>`` per segment, because the loader would otherwise
    interpolate straight across the gap.
    """
    if n_segments <= 1:
        return [track_id]
    return [f"{track_id}#{k}" for k in range(n_segments)]


def keep_segments(states: np.ndarray, min_observations: int = 2) -> list[tuple[int, int]]:
    """Visibility segments long enough for the loader (it skips 1-observation tracks)."""
    return [(a, b) for a, b in segments(states) if (b - a) >= min_observations]
