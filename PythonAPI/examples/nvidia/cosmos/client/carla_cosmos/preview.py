"""Local ground-truth preview: draw the exported ClipGT scene back onto the captured RGB.

    carla-cosmos preview --clip clips/<id> [--cameras ...] [--frames a:b] [--out DIR] [--grid]
                         [--show-occluded] [--png-every N] [--no-horizon]

This is the check you can run on a laptop, with no GPU node and no model in the loop: if the
overlay sits on the image for every camera, the scene package we upload (and NVIDIA's renderer
consumes) describes the drive we captured.  It reprojects the exported tables — obstacles, lane
lines, road boundaries, crosswalks, wait lines, poles, traffic lights and signs — through the
exported calibration and egomotion, so a mistake anywhere in the export shows up as an overlay
that slides off the world.

The maths is NVIDIA's ClipGT loader, verbatim (it was validated against their renderer's output):

* calibration: ``nominalSensor2Rig_FLU`` = ``Rotation.from_euler("xyz", radians(roll-pitch-yaw))``
  plus ``t``; ego pose from ``egomotion_estimate`` (quaternion + location); world -> camera is
  ``inv(T_rig @ T_sensor2rig)``;
* lens: the ``pixeldistance-to-angle`` polynomial maps pixel radius to angle, so projection needs
  it inverted — done by interpolation over the *monotonic* part of the polynomial only (past its
  first maximum the polynomial turns around and interpolation would fold the image back on itself);
* camera frame is FLU (x forward, y left, z up): ``u = cx + r·(−y/ρ)``, ``v = cy + r·(−z/ρ)`` with
  ``ρ = hypot(y, z)`` and ``θ = atan2(ρ, x)``; points behind the camera or beyond the lens range
  are dropped (NaN) rather than mirrored to the front;
* polylines are densified to 0.5 m so the clipping is per sub-segment: a line that leaves the
  frustum in the middle is cut there instead of being folded across the image (that fold was the
  visible bug on the cross cameras before densification);
* boxes are drawn edge by edge, and their edges are densified and clipped exactly like the
  polylines: a box that straddles the camera plane (any parked car the ego drives past) keeps the
  visible part of every edge instead of losing the whole edge, which used to leave only the far
  face drawn and made near obstacles look as if they had been exported too close to the horizon;
* parked obstacles — tracks the exporter writes with exactly two rows (first and last timestamp)
  — are held constant between those two timestamps (the whole clip, unless the occlusion filter
  cut the track into visible segments);
* ``--show-occluded`` reads the exporter's visibility sidecar and draws everything the occlusion
  filter dropped as dashed grey boxes, which is how that filter is inspected: a grey box over a
  building is the filter working, a grey box over a plainly visible car is not;
* map polylines are hidden behind the map's own terrain (:class:`Horizon`).  There is no depth
  buffer for them, and on flat ground that never mattered — distant boundaries pile up at the
  horizon row and read as the horizon.  With relief it is glaring: on the San Francisco NuRec
  clip the road boundaries a hundred metres ahead sit thirty metres down the far side of a
  crest, so they used to be painted straight across the road in front of the car (they are
  *correct* there — the same rows of NVIDIA's own ClipGT project to the same pixels — they are
  simply not visible from the car).  ``--no-horizon`` turns the test off.

Everything above lives in small pure functions (:class:`FThetaCamera`, :func:`densify`,
:func:`polyline_segments`, :func:`box_corners`, …) that need neither CARLA nor a clip on disk, so
they are unit-tested directly.  :func:`preview_clip` is the thin I/O shell around them.
"""

from __future__ import annotations

import json
import logging
import math
from dataclasses import dataclass, field
from pathlib import Path
from typing import Iterable, Iterator, Sequence

import numpy as np

log = logging.getLogger(__name__)

# ----------------------------------------------------------------------------- layers

@dataclass(frozen=True)
class Layer:
    """One ClipGT table and how it is drawn.  ``color`` is BGR (OpenCV order)."""

    table: str
    kind: str
    """``polyline`` (a list of world points per row) or ``box`` (centre/size/orientation)."""
    color: tuple[int, int, int]
    closed: bool = False
    """Polylines only: join the last point back to the first (crosswalk quads)."""


LAYERS: dict[str, Layer] = {
    layer.table: layer for layer in (
        Layer("lane_line", "polyline", (0, 220, 220)),
        Layer("road_boundary", "polyline", (255, 255, 255)),
        Layer("crosswalk", "polyline", (255, 80, 255), closed=True),
        Layer("wait_line", "polyline", (0, 120, 255)),
        Layer("road_marking", "polyline", (180, 180, 0)),
        Layer("pole", "polyline", (200, 255, 0)),
        Layer("obstacle", "box", (255, 160, 80)),
        Layer("traffic_light", "box", (60, 255, 60)),
        Layer("traffic_sign", "box", (140, 140, 255)),
    )
}
"""Every table the preview knows how to draw, keyed by table name."""

DEFAULT_LAYERS: tuple[str, ...] = tuple(LAYERS)

PERSON_COLOR = (0, 200, 255)
"""Obstacles with category ``person`` are drawn in their own colour."""

OCCLUDED_COLOR = (140, 140, 140)
"""``--show-occluded``: obstacles the occlusion filter dropped, drawn dashed and grey."""

BOX_EDGES = ((0, 1), (0, 2), (1, 3), (2, 3), (4, 5), (4, 6), (5, 7), (6, 7),
             (0, 4), (1, 5), (2, 6), (3, 7))
"""Edges of :func:`box_corners`' corner ordering."""

_UNIT_CORNERS = np.array([[sx, sy, sz] for sx in (-1, 1) for sy in (-1, 1) for sz in (-1, 1)]) / 2


# ----------------------------------------------------------------------------- geometry (pure)

def euler_to_matrix(roll_pitch_yaw_deg: Sequence[float]) -> np.ndarray:
    """ClipGT's ``roll-pitch-yaw`` (degrees) -> 3x3, i.e. ``Rotation.from_euler("xyz", radians)``."""
    from scipy.spatial.transform import Rotation

    return Rotation.from_euler("xyz", np.radians(np.asarray(roll_pitch_yaw_deg, float))).as_matrix()


def quat_to_matrix(quat_xyzw: Sequence[float]) -> np.ndarray:
    """Quaternion in ClipGT order (x, y, z, w) -> 3x3."""
    from scipy.spatial.transform import Rotation

    return Rotation.from_quat(np.asarray(quat_xyzw, float)).as_matrix()


def pose_matrix(rotation: np.ndarray, translation: Sequence[float]) -> np.ndarray:
    """3x3 + 3 -> 4x4 homogeneous transform."""
    T = np.eye(4)
    T[:3, :3] = rotation
    T[:3, 3] = np.asarray(translation, float)
    return T


def transform_points(points: np.ndarray, T: np.ndarray) -> np.ndarray:
    """Apply a 4x4 to (N, 3) points."""
    P = np.asarray(points, float).reshape(-1, 3)
    return P @ T[:3, :3].T + T[:3, 3]


def densify(points: np.ndarray, step: float = 0.5) -> np.ndarray:
    """Resample an (N, 3) polyline so no segment is longer than ``step`` metres.

    Clipping happens per segment, so a polyline that crosses the camera plane must be fine enough
    that the part in front stays and only the part behind is dropped."""
    P = np.asarray(points, float).reshape(-1, 3)
    if len(P) < 2:
        return P
    out = [P[:1]]
    for a, b in zip(P[:-1], P[1:]):
        n = max(1, int(np.ceil(np.linalg.norm(b - a) / step)))
        out.append(a + (b - a)[None] * np.linspace(0, 1, n + 1)[1:, None])
    return np.vstack(out)


def box_corners(center: Sequence[float], size: Sequence[float], quat_xyzw: Sequence[float]) -> np.ndarray:
    """The 8 world corners of an oriented box, ordered for :data:`BOX_EDGES`."""
    return (_UNIT_CORNERS * np.asarray(size, float)) @ quat_to_matrix(quat_xyzw).T + np.asarray(center, float)


def box_edge_points(corners: np.ndarray, step: float = 0.5) -> np.ndarray:
    """The 12 edges of :func:`box_corners`, densified to ``step`` metres: ``(12, n + 1, 3)``.

    Boxes need the same treatment as polylines.  A box drawn straight from its 8 projected corners
    loses every edge with an endpoint behind the camera or outside the lens range, and for a parked
    car the ego is level with that is *most* of them: what survives is the far face alone, floating
    near the image centre, which reads as a metre-scale export error and is not one.  Densifying
    first means the clip happens where the edge actually leaves the frustum."""
    C = np.asarray(corners, float).reshape(-1, 3)
    a = C[[i for i, _ in BOX_EDGES]]
    b = C[[j for _, j in BOX_EDGES]]
    n = max(1, int(np.ceil(float(np.linalg.norm(b - a, axis=1).max()) / max(step, 1e-6))))
    t = np.linspace(0.0, 1.0, n + 1)[None, :, None]
    return a[:, None, :] + (b - a)[:, None, :] * t


def polyline_segments(uv: np.ndarray) -> Iterator[tuple[np.ndarray, np.ndarray]]:
    """Consecutive pixel pairs of a projected polyline, skipping any pair touching a NaN.

    That is the clipping: the invisible middle of a line simply produces no segments, instead of
    one long segment folded across the image."""
    for a, b in zip(uv[:-1], uv[1:]):
        if not (np.isnan(a).any() or np.isnan(b).any()):
            yield a, b


# ----------------------------------------------------------------------------- terrain horizon (pure)

GROUND_TABLES: tuple[str, ...] = ("road_boundary", "lane_line", "crosswalk", "wait_line", "road_marking")
"""Polyline tables that lie *on* the road surface, and so are the occluders of :class:`Horizon`.

``pole`` is a polyline table too but stands up off the ground, so it is drawn (and horizon-tested)
without ever occluding: one pole would otherwise blank a whole azimuth bin behind it."""

HORIZON_BINS = 360
"""Azimuth bins of the horizon profile (1 degree each)."""

HORIZON_SLACK_M = 2.0
"""Only surface nearer than ``range - this`` may hide a point: a polyline never occludes itself."""

HORIZON_MARGIN_DEG = 0.25
"""Elevation tolerance, so a point exactly *on* the skyline is kept rather than flickering."""

_HORIZON_KEY_SCALE = 1e6
"""``bin * this + range`` sorts by (bin, range) in one float64 key; ranges are metres, so safe."""


def _polar(points: np.ndarray, eye: np.ndarray, bins: int) -> tuple[np.ndarray, np.ndarray, np.ndarray]:
    """``(azimuth bin, horizontal range, elevation angle)`` of world points seen from ``eye``."""
    P = np.asarray(points, float).reshape(-1, 3)
    d = P[:, :2] - np.asarray(eye, float)[:2]
    rng = np.hypot(d[:, 0], d[:, 1])
    az = (np.arctan2(d[:, 1], d[:, 0]) * (bins / (2.0 * np.pi))).astype(np.int64) % bins
    return az, rng, np.arctan2(P[:, 2] - float(eye[2]), np.maximum(rng, 1e-6))


class Horizon:
    """The skyline the map's own road surface builds up around one eye point.

    The classic terrain horizon test, on the only surface a scene package carries: walking outward
    along a bearing, the running maximum of the elevation angle of every surface sample already
    passed *is* the skyline, and a point below it is behind a crest and cannot be seen.  The
    occluders are the map's own points, which is what makes this safe to leave on by default —
    on a flat clip the running maximum never gets ahead of anything, so nothing is culled at all.
    It fails open in both directions that matter: an azimuth the map has no sample on hides
    nothing, and a point nearer than the closest sample on its bearing is always drawn.
    """

    def __init__(self, occluders: np.ndarray, eye: Sequence[float], bins: int = HORIZON_BINS,
                 slack_m: float = HORIZON_SLACK_M, margin_deg: float = HORIZON_MARGIN_DEG) -> None:
        self.eye = np.asarray(eye, float)
        self.bins, self.slack_m = int(bins), float(slack_m)
        self.margin = math.radians(margin_deg)
        az, rng, elev = _polar(occluders, self.eye, self.bins)
        keep = rng > 1e-3
        order = np.lexsort((rng[keep], az[keep]))
        self._az, self._rng, sky = az[keep][order], rng[keep][order], elev[keep][order]
        # running max of the elevation angle within each azimuth bin, outward
        edges = np.flatnonzero(np.diff(self._az)) + 1
        for a, b in zip(np.r_[0, edges], np.r_[edges, len(sky)]):
            if b - a > 1:
                sky[a:b] = np.maximum.accumulate(sky[a:b])
        self._sky = sky
        self._key = self._az * _HORIZON_KEY_SCALE + self._rng

    def visible(self, points: np.ndarray) -> np.ndarray:
        """Boolean mask of the world points that are not hidden below this skyline."""
        P = np.asarray(points, float).reshape(-1, 3)
        if len(self._sky) == 0:
            return np.ones(len(P), bool)
        az, rng, elev = _polar(P, self.eye, self.bins)
        query = az * _HORIZON_KEY_SCALE + np.maximum(rng - self.slack_m, 0.0)
        i = np.searchsorted(self._key, query, "right") - 1
        hit = (i >= 0) & (self._az[np.clip(i, 0, len(self._az) - 1)] == az)
        sky = np.where(hit, self._sky[np.clip(i, 0, len(self._sky) - 1)], -np.inf)
        return elev >= sky - self.margin


# ----------------------------------------------------------------------------- camera model (pure)

@dataclass
class FThetaCamera:
    """An f-theta (``pixeldistance-to-angle``) camera from a ClipGT rig ``properties`` block."""

    name: str
    width: int
    height: int
    cx: float
    cy: float
    coefficients: tuple[float, ...]
    """Polynomial coefficients, lowest order first: ``theta = sum(c_i * r**i)``."""
    max_radius: float = 1200.0
    """Pixel radius the inversion table covers."""
    samples: int = 24001
    max_angle_deg: float = 89.0
    _radii: np.ndarray = field(init=False, repr=False)
    _thetas: np.ndarray = field(init=False, repr=False)
    theta_max: float = field(init=False, default=0.0)

    def __post_init__(self) -> None:
        radii = np.linspace(0.0, self.max_radius, self.samples)
        thetas = np.polyval(list(self.coefficients)[::-1], radii)
        mono = int(np.argmax(thetas)) + 1  # invert only the monotonic part of the polynomial
        self._radii, self._thetas = radii[:mono], thetas[:mono]
        self.theta_max = float(min(self._thetas[-1], np.radians(self.max_angle_deg)))

    @classmethod
    def from_rig_sensor(cls, sensor: dict) -> "FThetaCamera":
        """Build from one entry of ``rig_json["rig"]["sensors"]``."""
        p = sensor["properties"]
        return cls(name=sensor["name"], width=int(p["width"]), height=int(p["height"]),
                   cx=float(p["cx"]), cy=float(p["cy"]),
                   coefficients=tuple(float(c) for c in p["polynomial"].split()))

    def project(self, points_flu: np.ndarray) -> np.ndarray:
        """(N, 3) camera-frame FLU points -> (N, 2) pixels; NaN behind the camera / past the lens.

        FLU means x forward, y left, z up, so +y goes to the *left* of the image (smaller u) and
        +z goes *up* (smaller v)."""
        P = np.asarray(points_flu, float).reshape(-1, 3)
        x, y, z = P[:, 0], P[:, 1], P[:, 2]
        rho = np.hypot(y, z)
        theta = np.arctan2(rho, x)
        r = np.interp(theta, self._thetas, self._radii)
        safe = np.maximum(rho, 1e-9)
        uv = np.stack([self.cx + r * (-y / safe), self.cy + r * (-z / safe)], axis=1)
        uv[(x <= 0.05) | (theta > self.theta_max)] = np.nan
        return uv


@dataclass
class PreviewCamera:
    """An f-theta camera with its rig mounting."""

    name: str
    lens: FThetaCamera
    sensor_to_rig: np.ndarray

    @classmethod
    def from_rig_sensor(cls, sensor: dict) -> "PreviewCamera":
        ext = sensor["nominalSensor2Rig_FLU"]
        return cls(name=sensor["name"], lens=FThetaCamera.from_rig_sensor(sensor),
                   sensor_to_rig=pose_matrix(euler_to_matrix(ext["roll-pitch-yaw"]), ext["t"]))

    def world_to_camera(self, rig_pose: np.ndarray) -> np.ndarray:
        """4x4 world -> camera for one ego pose (rig -> world)."""
        return np.linalg.inv(rig_pose @ self.sensor_to_rig)

    def project_world(self, points: np.ndarray, world_to_camera: np.ndarray) -> np.ndarray:
        """(N, 3) world points -> (N, 2) pixels."""
        return self.lens.project(transform_points(points, world_to_camera))


def cameras_from_rig_json(rig_json: str | dict) -> dict[str, PreviewCamera]:
    """``calibration_estimate.rig_json`` -> ``{camera name: PreviewCamera}``."""
    rig = json.loads(rig_json) if isinstance(rig_json, str) else rig_json
    return {s["name"]: PreviewCamera.from_rig_sensor(s) for s in rig["rig"]["sensors"]}


# ----------------------------------------------------------------------------- scene package

@dataclass
class Box:
    """One oriented box of a box-shaped table."""

    center: np.ndarray
    size: np.ndarray
    quat_xyzw: np.ndarray
    category: str = ""
    track_id: str = ""

    @classmethod
    def from_payload(cls, payload: dict) -> "Box":
        size = payload.get("size") or payload["dimensions"]
        return cls(center=np.array([payload["center"][k] for k in "xyz"], float),
                   size=np.array([size[k] for k in "xyz"], float),
                   quat_xyzw=np.array([payload["orientation"][k] for k in "xyzw"], float),
                   category=str(payload.get("category", "")),
                   track_id=str(payload.get("trackline_id", "")))

    def corners(self) -> np.ndarray:
        return box_corners(self.center, self.size, self.quat_xyzw)


def split_tracks(rows: Sequence[dict], table: str = "obstacle") -> tuple[list[tuple[Box, int, int]],
                                                                        dict[int, list[Box]]]:
    """Split a per-timestamp box table into parked and moving obstacles.

    The exporter writes a parked vehicle once at the first and once at the last timestamp it is
    visible for (two rows), and a moving one at every timestamp.  Two-row tracks are therefore
    constant, and are returned as ``(box, first timestamp, last timestamp)``: a parked car the
    occlusion filter hides for part of the clip is exported as one two-row track per visible
    segment, so the timestamps say when to draw it (for an unfiltered export they span the whole
    clip, which is every frame, as before).  Everything else is keyed by its own timestamp."""
    by_track: dict[str, list[dict]] = {}
    for r in rows:
        by_track.setdefault(str(r[table]["trackline_id"]), []).append(r)
    static = []
    for rs in by_track.values():
        if len(rs) == 2:
            ts = sorted(int(r["key"]["timestamp_micros"]) for r in rs)
            static.append((Box.from_payload(rs[0][table]), ts[0], ts[1]))
    dynamic: dict[int, list[Box]] = {}
    for r in rows:
        if len(by_track[str(r[table]["trackline_id"])]) != 2:
            dynamic.setdefault(int(r["key"]["timestamp_micros"]), []).append(Box.from_payload(r[table]))
    return static, dynamic


def load_visibility(scene_dir: str | Path, clip_id: str) -> dict[int, list[Box]] | None:
    """``<clip>.visibility.json`` -> ``{timestamp_micros: [dropped Box, ...]}``.

    The sidecar the exporter writes next to the tables when the occlusion filter is on: it holds
    the per-tick decisions and, because those rows are by construction missing from
    ``obstacle.parquet``, the geometry of everything the filter dropped.  ``None`` when the clip
    was exported without the filter."""
    path = Path(scene_dir) / f"{clip_id}.visibility.json"
    if not path.is_file():
        return None
    data = json.loads(path.read_text())
    stamps = data["timestamps_micros"]
    out: dict[int, list[Box]] = {}
    for entry in data.get("occluded", []):
        a, b = entry["ticks"]
        box = Box.from_payload(entry["obstacle"])
        for t in range(max(0, a), min(b, len(stamps))):
            out.setdefault(int(stamps[t]), []).append(box)
    log.info("preview: %s: %d frame(s) carry occluded obstacles (params %s)",
             path.name, len(out), data.get("params"))
    return out


def _points_of(payload: dict) -> list[dict]:
    """The list of ``{x, y, z}`` in a polyline payload (``line_rail``, ``location``, …)."""
    for value in payload.values():
        if isinstance(value, list) and value and isinstance(value[0], dict) and "x" in value[0]:
            return value
    return []


@dataclass
class SceneGT:
    """The exported ClipGT tables, in memory, ready to project."""

    clip_id: str
    cameras: dict[str, PreviewCamera]
    timestamps: list[int]
    ego_poses: list[np.ndarray]
    polylines: dict[str, list[np.ndarray]] = field(default_factory=dict)
    static_boxes: dict[str, list[tuple[Box, int | None, int | None]]] = field(default_factory=dict)
    """``table -> [(box, first timestamp, last timestamp)]``; ``None`` bounds mean "every frame"."""
    dynamic_boxes: dict[str, dict[int, list[Box]]] = field(default_factory=dict)
    occluded_boxes: dict[int, list[Box]] = field(default_factory=dict)
    """Obstacles the occlusion filter dropped, from the visibility sidecar (``--show-occluded``)."""
    _ground: np.ndarray | None = field(default=None, repr=False)
    _horizon: tuple[tuple[float, ...], "Horizon"] | None = field(default=None, repr=False)

    @property
    def frames(self) -> int:
        return len(self.ego_poses)

    def horizon(self, eye: Sequence[float]) -> Horizon | None:
        """The skyline the loaded road-surface layers build around ``eye`` (``None`` if there is none)."""
        if self._ground is None:
            ground = [p for table in GROUND_TABLES for p in self.polylines.get(table, [])]
            self._ground = np.vstack(ground) if ground else np.zeros((0, 3))
        if not len(self._ground):
            return None
        key = tuple(np.round(np.asarray(eye, float), 3))
        if self._horizon is None or self._horizon[0] != key:
            self._horizon = (key, Horizon(self._ground, eye))
        return self._horizon[1]

    def boxes_at(self, table: str, timestamp: int) -> list[Box]:
        """Parked boxes alive at ``timestamp`` plus whatever the table has at it."""
        static = [b for b, t0, t1 in self.static_boxes.get(table, [])
                  if t0 is None or t0 <= timestamp <= t1]
        return static + self.dynamic_boxes.get(table, {}).get(timestamp, [])

    @classmethod
    def load(cls, scene_dir: str | Path, clip_id: str | None = None,
             layers: Iterable[str] = DEFAULT_LAYERS, show_occluded: bool = False) -> "SceneGT":
        """Read ``<scene_dir>/<clip_id>.<table>.parquet`` for the requested layers."""
        import pyarrow.parquet as pq

        d = Path(scene_dir)
        if clip_id is None:
            names = sorted(p.name for p in d.glob("*.calibration_estimate.parquet"))
            if not names:
                raise FileNotFoundError(f"no calibration_estimate parquet in {d}")
            clip_id = names[0].split(".calibration_estimate.parquet")[0]

        def table(name: str) -> list[dict] | None:
            p = d / f"{clip_id}.{name}.parquet"
            return pq.read_table(p).to_pylist() if p.is_file() else None

        calib = table("calibration_estimate")
        if not calib:
            raise FileNotFoundError(f"{d} has no calibration_estimate for clip '{clip_id}'")
        cameras = cameras_from_rig_json(calib[0]["calibration_estimate"]["rig_json"])
        ego_rows = table("egomotion_estimate") or []
        timestamps = [int(r["key"]["timestamp_micros"]) for r in ego_rows]
        poses = []
        for r in ego_rows:
            e = r["egomotion_estimate"]
            poses.append(pose_matrix(quat_to_matrix([e["orientation"][k] for k in "xyzw"]),
                                     [e["location"][k] for k in "xyz"]))
        scene = cls(clip_id=clip_id, cameras=cameras, timestamps=timestamps, ego_poses=poses)
        for name in layers:
            layer = LAYERS.get(name)
            if layer is None:
                raise ValueError(f"unknown preview layer '{name}' (known: {', '.join(DEFAULT_LAYERS)})")
            rows = table(layer.table)
            if not rows:
                continue
            if layer.kind == "polyline":
                scene.polylines[layer.table] = [
                    np.array([[p["x"], p["y"], p["z"]] for p in _points_of(r[layer.table])], float)
                    for r in rows if len(_points_of(r[layer.table])) > 1]
            elif "timestamp_micros" in rows[0]["key"]:
                static, dynamic = split_tracks(rows, layer.table)
                scene.static_boxes[layer.table], scene.dynamic_boxes[layer.table] = static, dynamic
            else:
                scene.static_boxes[layer.table] = [(Box.from_payload(r[layer.table]), None, None)
                                                   for r in rows]
        if show_occluded:
            scene.occluded_boxes = load_visibility(d, clip_id) or {}
        return scene


# ----------------------------------------------------------------------------- drawing

def draw_box(frame: np.ndarray, camera: PreviewCamera, world_to_camera: np.ndarray, box: Box,
             color: tuple[int, int, int], thickness: int = 2, step: float = 0.5,
             dashed: bool = False) -> bool:
    """Draw one oriented box; ``dashed`` keeps every other densified sub-segment.

    Returns whether anything was drawn (a box entirely outside the lens range draws nothing)."""
    import cv2

    edges = box_edge_points(box.corners(), step)
    uv = camera.project_world(edges.reshape(-1, 3), world_to_camera).reshape(edges.shape[0], -1, 2)
    if np.isnan(uv).all():
        return False
    drawn = False
    for edge in uv:
        for i, (a, b) in enumerate(polyline_segments(edge)):
            if dashed and i % 2:
                continue
            cv2.line(frame, tuple(a.astype(int)), tuple(b.astype(int)), color, thickness)
            drawn = True
    return drawn


def draw_scene(frame: np.ndarray, camera: PreviewCamera, world_to_camera: np.ndarray, scene: SceneGT,
               timestamp: int, layers: Iterable[str] = DEFAULT_LAYERS, thickness: int = 2,
               dim: float = 0.6, step: float = 0.5, show_occluded: bool = False,
               horizon: bool = True) -> np.ndarray:
    """Draw the scene onto one BGR frame (in place) and return it.

    ``dim`` darkens the RGB first so the overlay reads on bright frames; pass 1.0 for no dimming.
    ``show_occluded`` additionally draws whatever the occlusion filter dropped at this timestamp
    as dashed grey boxes, so the filter can be inspected against the pixels it was derived from.
    ``horizon`` hides map polylines that the map's own road surface occludes (:class:`Horizon`);
    it is a no-op on flat ground and only bites where the clip has real relief."""
    if dim != 1.0:
        frame[:] = (frame * dim).astype(np.uint8)
    if show_occluded:
        for box in scene.occluded_boxes.get(timestamp, []):
            draw_box(frame, camera, world_to_camera, box, OCCLUDED_COLOR, thickness, step, dashed=True)
    skyline = scene.horizon(np.linalg.inv(world_to_camera)[:3, 3]) if horizon else None
    for name in layers:
        layer = LAYERS[name]
        if layer.kind == "polyline":
            import cv2

            for points in scene.polylines.get(layer.table, []):
                P = densify(points, step)
                if layer.closed and len(P) > 2:
                    P = np.vstack([P, densify(np.vstack([points[-1], points[0]]), step)])
                uv = camera.project_world(P, world_to_camera)
                if skyline is not None:
                    uv[~skyline.visible(P)] = np.nan
                for a, b in polyline_segments(uv):
                    cv2.line(frame, tuple(a.astype(int)), tuple(b.astype(int)), layer.color, thickness)
        else:
            for box in scene.boxes_at(layer.table, timestamp):
                color = PERSON_COLOR if box.category == "person" else layer.color
                draw_box(frame, camera, world_to_camera, box, color, thickness, step)
    return frame


# ----------------------------------------------------------------------------- labels (PIL, not ffmpeg)

_FONT_CANDIDATES = ("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
                    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
                    "/usr/share/fonts/TTF/DejaVuSans-Bold.ttf")


def label_patch(text: str, height: int = 22, color: tuple[int, int, int] = (255, 255, 80)) -> np.ndarray:
    """Render ``text`` to a small BGR patch on a black plate (drawn with PIL, blitted by us).

    ffmpeg's ``drawtext`` crashes when combined with ``xstack`` in this environment, so the grid
    labels are burned in here instead."""
    from PIL import Image, ImageDraw, ImageFont

    font = None
    for path in _FONT_CANDIDATES:
        if Path(path).is_file():
            font = ImageFont.truetype(path, max(10, int(height * 0.68)))
            break
    if font is None:  # pragma: no cover - only on hosts without DejaVu
        font = ImageFont.load_default()
    probe = ImageDraw.Draw(Image.new("RGB", (1, 1)))
    w = int(probe.textlength(text, font=font)) + 12
    img = Image.new("RGB", (w, height), (0, 0, 0))
    ImageDraw.Draw(img).text((6, max(0, (height - font.size) // 2 - 1)), text, font=font,
                             fill=(color[2], color[1], color[0]))
    return np.array(img)[:, :, ::-1].copy()


def blit(frame: np.ndarray, patch: np.ndarray, x: int = 6, y: int = 6) -> np.ndarray:
    """Paste a patch onto a frame, clipped to its bounds."""
    h = min(patch.shape[0], frame.shape[0] - y)
    w = min(patch.shape[1], frame.shape[1] - x)
    if h > 0 and w > 0:
        frame[y:y + h, x:x + w] = patch[:h, :w]
    return frame


# ----------------------------------------------------------------------------- clip preview (I/O)

GRID_COLUMNS = 4
"""Cameras per row in the grid video."""


def parse_frames(spec: str | None, total: int) -> tuple[int, int]:
    """``"a:b"`` (either side optional) -> a half-open frame range clamped to ``total``."""
    if not spec:
        return 0, total
    a, _, b = spec.partition(":")
    start = int(a) if a.strip() else 0
    stop = int(b) if b.strip() else total
    start = max(0, min(total, start))
    stop = max(start, min(total, stop))
    return start, stop


def _open(path: Path):
    import cv2

    cap = cv2.VideoCapture(str(path))
    if not cap.isOpened():
        raise FileNotFoundError(f"cannot open {path}")
    return cap


def preview_clip(clip_dir: str | Path, cameras: Sequence[str] | None = None, frames: str | None = None,
                 out_dir: str | Path | None = None, grid: bool = False,
                 layers: Sequence[str] = DEFAULT_LAYERS, dim: float = 0.6, thickness: int = 2,
                 grid_scale: float = 0.5, progress=None, show_occluded: bool = False,
                 png_every: int = 0, horizon: bool = True) -> dict[str, Path]:
    """Write ``<out>/<camera>.mp4`` (and ``<out>/grid.mp4``) with the scene drawn on the RGB.

    Returns ``{camera name (or "grid"): path}``.  ``progress`` is called with
    ``(camera, frame index, frame count)`` if given."""
    import cv2

    from .clip import Clip
    from .contracts import canonical_camera_name

    clip = Clip.load(clip_dir)
    scene_dir = clip.scene_dir
    if scene_dir is None or not Path(scene_dir).is_dir():
        raise FileNotFoundError(f"clip {clip.manifest.clip_id} has no scene package to preview")
    scene = SceneGT.load(scene_dir, clip.manifest.clip_id, layers, show_occluded=show_occluded)
    names = list(cameras) if cameras else list(clip.manifest.camera_names)
    unknown = [c for c in names if c not in scene.cameras]
    if unknown:
        raise ValueError(f"camera(s) {unknown} are not in the exported calibration "
                         f"({', '.join(scene.cameras)})")
    out = Path(out_dir) if out_dir else Path(clip.path) / "preview"
    out.mkdir(parents=True, exist_ok=True)
    start, stop = parse_frames(frames, min(clip.manifest.frames, scene.frames) or clip.manifest.frames)
    written: dict[str, Path] = {}

    def render(name: str, index: int, frame: np.ndarray) -> np.ndarray:
        cam = scene.cameras[name]
        return draw_scene(frame, cam, cam.world_to_camera(scene.ego_poses[index]), scene,
                          scene.timestamps[index], layers, thickness, dim,
                          show_occluded=show_occluded, horizon=horizon)

    for name in names:
        src = clip.video("rgb", name)
        cap = _open(src)
        w, h, fps = int(cap.get(3)), int(cap.get(4)), cap.get(5) or clip.manifest.fps
        dst = out / f"{canonical_camera_name(name)}.mp4"
        writer = cv2.VideoWriter(str(dst), cv2.VideoWriter_fourcc(*"mp4v"), fps, (w, h))
        if start:
            cap.set(cv2.CAP_PROP_POS_FRAMES, start)
        for i in range(start, stop):
            ok, frame = cap.read()
            if not ok:
                break
            drawn = render(name, i, frame)
            writer.write(drawn)
            if png_every and (i - start) % png_every == 0:
                cv2.imwrite(str(out / f"{canonical_camera_name(name)}_{i:04d}.png"), drawn)
            if progress:
                progress(name, i - start + 1, stop - start)
        cap.release()
        writer.release()
        written[name] = dst
        log.info("preview %s -> %s", name, dst)

    if grid and names:
        written["grid"] = _write_grid(clip, scene, names, out, start, stop, render, grid_scale, progress)
    index = {"clip_id": clip.manifest.clip_id, "frames": [start, stop], "layers": list(layers),
             "show_occluded": bool(show_occluded), "horizon": bool(horizon),
             "videos": {n: p.name for n, p in written.items() if n != "grid"},
             "grid": written["grid"].name if "grid" in written else None}
    (out / "preview.json").write_text(json.dumps(index, indent=2))
    return written


def _write_grid(clip, scene: SceneGT, names: Sequence[str], out: Path, start: int, stop: int,
                render, scale: float, progress=None) -> Path:
    """One video with every camera in a ``GRID_COLUMNS``-wide grid, each tile labelled."""
    import cv2

    from .contracts import canonical_camera_name

    caps = [_open(clip.video("rgb", n)) for n in names]
    fps = caps[0].get(5) or clip.manifest.fps
    tw = max(16, int(int(caps[0].get(3)) * scale) // 2 * 2)
    th = max(16, int(int(caps[0].get(4)) * scale) // 2 * 2)
    cols = min(GRID_COLUMNS, len(names))
    rows = (len(names) + cols - 1) // cols
    patches = [label_patch(f"{canonical_camera_name(n).removeprefix('camera_')} · scene GT",
                           height=max(14, th // 18)) for n in names]
    dst = out / "grid.mp4"
    writer = cv2.VideoWriter(str(dst), cv2.VideoWriter_fourcc(*"mp4v"), fps, (tw * cols, th * rows))
    for cap in caps:
        if start:
            cap.set(cv2.CAP_PROP_POS_FRAMES, start)
    for i in range(start, stop):
        tiles = []
        for name, cap, patch in zip(names, caps, patches):
            ok, frame = cap.read()
            if not ok:
                tiles.append(np.zeros((th, tw, 3), np.uint8))
                continue
            tile = cv2.resize(render(name, i, frame), (tw, th), interpolation=cv2.INTER_AREA)
            tiles.append(blit(tile, patch))
        tiles += [np.zeros((th, tw, 3), np.uint8)] * (rows * cols - len(tiles))
        writer.write(np.vstack([np.hstack(tiles[r * cols:(r + 1) * cols]) for r in range(rows)]))
        if progress:
            progress("grid", i - start + 1, stop - start)
    for cap in caps:
        cap.release()
    writer.release()
    log.info("preview grid -> %s", dst)
    return dst
