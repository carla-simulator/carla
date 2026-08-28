"""ClipGT scene exporter: CARLA ground truth -> NVIDIA world-scenario Parquet.

Produces the exact tables consumed by
``cosmos_transfer2/_src/imaginaire/auxiliary/world_scenario/dataloaders/clipgt_loader.py``
(schemas dumped from NVIDIA's ``multiview_example1`` and verified geometrically
in the Phase 0 spike): ``egomotion_estimate``, ``obstacle``,
``calibration_estimate``, ``lane_line``, ``road_boundary``, ``crosswalk``,
``pole``, ``traffic_light``, ``traffic_sign``, ``wait_line``, ``road_marking``.

World frame: right-handed FLU anchored at the first ego rear-axle-on-ground
pose.  Ego origin: rear axle on ground.  Hardened over the spike:

* junction lane segments are skipped (CARLA marks some connectors ``Solid``,
  which renders as a phantom line through the intersection);
* static parked vehicles (level bounding boxes tagged Car/Truck/Bus/
  Motorcycle/Bicycle, not actors) are exported as constant-pose obstacles,
  de-duplicated against the live actors (the hero included) and against the
  second, smaller box every parked vehicle mesh carries for its interior;
* per-frame traffic-light states go to a sidecar JSON (the stock ClipGT loader
  ignores states and renders grey; the sidecar feeds the loader extension);
* obstacles no camera can see are dropped, and tracks are split per visible
  segment (:mod:`carla_cosmos.visibility`, ``visibility="depth"``).
"""

from __future__ import annotations

import json
import logging
import time
from pathlib import Path
from typing import Iterable, Sequence

import numpy as np

import carla

from . import clipgt, coords, visibility as vis
from .clipgt import SCHEMAS, VERSION  # the ClipGT format, CARLA-free (re-exported: tests import them here)
from .contracts import CameraManifest

log = logging.getLogger(__name__)

VERSION = 1725328440
"""Constant ``version`` column value (same convention as NVIDIA's example)."""

# Lane marking enums -> NVIDIA lane-line styles/colours (audit section 5.4).
_STYLE = {
    carla.LaneMarkingType.Solid: "SOLID_SINGLE",
    carla.LaneMarkingType.Broken: "DASHED_SINGLE",
    carla.LaneMarkingType.SolidSolid: "SOLID_DOUBLE",
    carla.LaneMarkingType.SolidBroken: "SOLID_DASHED",
    carla.LaneMarkingType.BrokenSolid: "DASHED_SOLID",
    carla.LaneMarkingType.BrokenBroken: "DASHED_DOUBLE",
    carla.LaneMarkingType.BottsDots: "DOT_DASHED_SINGLE",
    carla.LaneMarkingType.Other: "UNKNOWN",
}
_COLOR = {
    carla.LaneMarkingColor.Standard: "WHITE",
    carla.LaneMarkingColor.White: "WHITE",
    carla.LaneMarkingColor.Yellow: "YELLOW",
    carla.LaneMarkingColor.Blue: "BLUE",
}
_ROADLIKE = {carla.LaneType.Driving, carla.LaneType.Parking, carla.LaneType.Shoulder,
             carla.LaneType.Biking, carla.LaneType.Bidirectional, carla.LaneType.Stop,
             carla.LaneType.Entry, carla.LaneType.Exit, carla.LaneType.OnRamp, carla.LaneType.OffRamp}
_CURBLIKE = (carla.LaneType.Sidewalk, carla.LaneType.Border, carla.LaneType.Median)

# Level-bb labels exported as static parked obstacles -> NVIDIA category.
STATIC_OBSTACLE_LABELS = {
    carla.CityObjectLabel.Car: "automobile",
    carla.CityObjectLabel.Truck: "heavy_truck",
    carla.CityObjectLabel.Bus: "bus",
    carla.CityObjectLabel.Motorcycle: "rider",
    carla.CityObjectLabel.Bicycle: "rider",
}


def actor_category(actor: carla.Actor) -> str:
    """NVIDIA obstacle category of a CARLA actor (audit section 2)."""
    tid = actor.type_id
    if tid.startswith("walker."):
        return "person"
    attrs = actor.attributes
    base = attrs.get("base_type", "")
    if base in ("motorcycle", "bicycle", "bycicle") or attrs.get("number_of_wheels") == "2":
        return "rider"
    if base == "truck":
        return "heavy_truck"
    if base == "bus":
        return "bus"
    if base == "van":
        return "other_vehicle"
    if tid.startswith("vehicle."):
        return "automobile"
    return "protruding_object"


class WorldFrame:
    """FLU world frame anchored at the first ego rear-axle pose."""

    def __init__(self, m0_flu: np.ndarray) -> None:
        self.inv = np.linalg.inv(m0_flu)

    def pose(self, m_ue: np.ndarray) -> np.ndarray:
        """UE world pose -> FLU pose in this frame."""
        return self.inv @ coords.ue_to_flu(m_ue)

    def point(self, loc: carla.Location) -> dict[str, float]:
        """UE world point -> FLU point in this frame."""
        p = self.inv @ np.array([loc.x, -loc.y, loc.z, 1.0])
        return coords.xyz(p[:3])

    def bbox(self, bb: carla.BoundingBox) -> tuple[dict, dict, dict]:
        """World-space bounding box -> (center, dimensions, quaternion) in this frame."""
        mf = self.pose(coords.bbox_matrix_ue(bb))
        return (coords.xyz(mf[:3, 3]),
                coords.xyz((2 * bb.extent.x, 2 * bb.extent.y, 2 * bb.extent.z)),
                coords.quat_xyzw(mf))


_mapkey = clipgt.mapkey


def _runs(ticks: Iterable[int]) -> list[tuple[int, int]]:
    """Consecutive integers as half-open ``[start, stop)`` runs."""
    out: list[tuple[int, int]] = []
    for t in sorted(ticks):
        if out and out[-1][1] == t:
            out[-1] = (out[-1][0], t + 1)
        else:
            out.append((t, t + 1))
    return out


def _side_point(wp: carla.Waypoint, sign: int) -> carla.Location:
    loc = wp.transform.location
    _, rv, _ = coords.ue_forward_right_up(wp.transform.rotation)  # not get_right_vector(): see coords.ue_rotation_matrix
    w = wp.lane_width / 2.0
    return carla.Location(loc.x + sign * rv[0] * w, loc.y + sign * rv[1] * w, loc.z + sign * rv[2] * w)


class SceneExporter:
    """Accumulates GT over a capture and writes the ClipGT package.

    Usage: construct, :meth:`begin` after the ego is settled (anchors the world
    frame and snapshots static layers), :meth:`record_tick` once per world tick,
    :meth:`write` at the end.
    """

    def __init__(self, world: carla.World, clip_id: str, hero: carla.Vehicle,
                 axle_local_ue: np.ndarray, lane_step: float = 1.0, *,
                 cameras: Sequence[CameraManifest] | None = None,
                 visibility: vis.VisibilityParams | str = "none") -> None:
        self.world = world
        self.clip_id = clip_id
        self.hero = hero
        self.lane_step = lane_step
        self.t_axle = np.eye(4)
        self.t_axle[:3, 3] = axle_local_ue
        self.frame: WorldFrame | None = None
        self._static: dict[str, list[dict]] = {}
        self._static_obstacles: list[dict] = []
        self.ego_rows: list[dict] = []
        self.obstacle_rows: list[dict] = []
        self.timestamps: list[int] = []
        self.tl_states: list[dict[str, str]] = []
        self._actors: list[tuple[int, str, carla.BoundingBox]] = []
        self._lights: list[carla.TrafficLight] = []
        # ---- occlusion filter (see carla_cosmos.visibility)
        self.visibility = (vis.VisibilityParams(mode=visibility) if isinstance(visibility, str)
                           else visibility)
        self.camera_manifests: list[CameraManifest] = list(cameras or [])
        self._cameras: list = []
        self._lattice = self.visibility.lattice()
        self._obs_index: dict[str, list[tuple[int, int]]] = {}
        """dynamic track id -> [(tick index, index into :attr:`obstacle_rows`)]"""
        self._state: dict[str, list[int]] = {}
        """track id (dynamic and ``static:*``) -> per-tick 1 / 0 / :data:`visibility.ABSENT`"""
        self._fraction: dict[str, list[float]] = {}
        self._static_points: np.ndarray = np.zeros((0, len(self._lattice), 3))
        self.visibility_micros: list[float] = []
        """Wall-clock cost of the z-buffer test per tick (microseconds), for the capture report."""

    # ------------------------------------------------------------------ setup
    def begin(self) -> None:
        """Anchor the world frame at the current ego pose and export static layers."""
        m0 = coords.ue_matrix(self.hero.get_transform()) @ self.t_axle
        self.frame = WorldFrame(coords.ue_to_flu(m0))
        self._scan_actors()
        self._export_static_layers()
        self._export_static_obstacles()
        self._begin_visibility()

    def _begin_visibility(self) -> None:
        """Build the camera models and the constant sample points of the parked obstacles."""
        if not self.visibility.enabled:
            return
        if not self.camera_manifests:
            log.warning("scene: visibility='depth' but no camera calibration was given; "
                        "exporting every obstacle")
            self.visibility = self.visibility.with_mode("none")
            return
        self._cameras = [vis.camera_from_manifest(c) for c in self.camera_manifests]
        self._static_points = vis.stack_sample_points(
            ([o["center"][k] for k in "xyz"], [o["size"][k] for k in "xyz"],
             [o["orientation"][k] for k in "xyzw"]) for o in self._static_obstacles)
        log.info("scene: occlusion filter on (%s) over %d camera(s), %d parked + %d live obstacles",
                 self.visibility.as_dict(), len(self._cameras), len(self._static_obstacles),
                 len(self._actors))

    def _scan_actors(self) -> None:
        for a in self.world.get_actors():
            if a.id == self.hero.id:
                continue
            if a.type_id.startswith("vehicle.") or a.type_id.startswith("walker.pedestrian"):
                self._actors.append((a.id, actor_category(a), a.bounding_box))
        self._lights = list(self.world.get_actors().filter("traffic.traffic_light*"))
        log.info("scene: tracking %d dynamic obstacles, %d traffic lights", len(self._actors), len(self._lights))

    # ------------------------------------------------------------------ static layers
    def _export_static_layers(self) -> None:
        assert self.frame is not None
        cmap = self.world.get_map()
        lane_rows, boundary_rows = self._export_lanes(cmap)
        self._static = {
            "lane_line": lane_rows,
            "road_boundary": boundary_rows,
            "crosswalk": self._export_crosswalks(cmap),
            "wait_line": self._export_wait_lines(),
            "road_marking": [],
        }
        poles, lights, signs = self._export_level_statics()
        self._static.update({"pole": poles, "traffic_light": lights, "traffic_sign": signs})
        for name, rows in self._static.items():
            log.info("scene: static %-14s %5d rows", name, len(rows))

    def _export_lanes(self, cmap: carla.Map) -> tuple[list[dict], list[dict]]:
        assert self.frame is not None
        lane_rows: list[dict] = []
        boundary_rows: list[dict] = []
        seen: set[tuple] = set()
        centre_done: set[tuple] = set()
        for wp0, _ in cmap.get_topology():
            key = (wp0.road_id, wp0.section_id, wp0.lane_id)
            if key in seen or wp0.is_junction:
                continue
            seen.add(key)
            pts = [wp0] + list(wp0.next_until_lane_end(self.lane_step))
            if len(pts) < 2:
                continue
            self._emit_lane_lines(wp0, pts, lane_rows, centre_done)
            self._emit_boundary(wp0, pts, boundary_rows)
        return lane_rows, boundary_rows

    def _emit_lane_lines(self, wp0: carla.Waypoint, pts: list[carla.Waypoint],
                         out: list[dict], centre_done: set[tuple]) -> None:
        """Outer (right) marking of every lane; the centre line once per road section."""
        assert self.frame is not None
        sides: list[tuple[int, object]] = [(+1, lambda w: w.right_lane_marking)]
        if abs(wp0.lane_id) == 1:
            ck = (wp0.road_id, wp0.section_id)
            if ck not in centre_done:
                centre_done.add(ck)
                sides.append((-1, lambda w: w.left_lane_marking))
        for sign, pick in sides:
            rail: list[dict] = []
            styles: list[str] = []
            colors: list[str] = []
            cur: tuple[str, str] | None = None

            def flush() -> None:
                if len(rail) >= 2:
                    out.append({"key": _mapkey(self.clip_id, "lanelines"),
                                "lane_line": {"line_rail": rail[:], "styles": styles[:], "colors": colors[:],
                                              "left_driving_direction": [], "right_driving_direction": []},
                                "version": VERSION})
                rail.clear(); styles.clear(); colors.clear()

            for wp in pts:
                mk = pick(wp)
                style = _STYLE.get(mk.type) if mk is not None else None
                if style is None or wp.is_junction:
                    flush()
                    cur = None
                    continue
                color = _COLOR.get(mk.color, "UNKNOWN")
                if cur is not None and cur != (style, color):
                    flush()
                cur = (style, color)
                rail.append(self.frame.point(_side_point(wp, sign)))
                styles.append(style)
                colors.append(color)
            flush()

    def _emit_boundary(self, wp0: carla.Waypoint, pts: list[carla.Waypoint], out: list[dict]) -> None:
        """Outer edge of the outermost road-like lane on the right."""
        assert self.frame is not None
        if wp0.lane_type != carla.LaneType.Driving:
            return
        outer: list[dict] = []
        category = "road_boundary"
        for wp in pts:
            w = wp
            while True:
                nxt = w.get_right_lane()
                if nxt is None or nxt.lane_type not in _ROADLIKE or nxt.lane_id * w.lane_id < 0:
                    if nxt is not None and nxt.lane_type in _CURBLIKE:
                        category = "tall_curb"
                    break
                w = nxt
            outer.append(self.frame.point(_side_point(w, +1)))
        if len(outer) >= 2:
            out.append({"key": _mapkey(self.clip_id, "road-boundaries"),
                        "road_boundary": {"category": category, "location": outer}, "version": VERSION})

    def _export_crosswalks(self, cmap: carla.Map) -> list[dict]:
        """Crosswalk polygons (flat vertex list delimited by a repeated first vertex)."""
        assert self.frame is not None
        pts = cmap.get_crosswalks()
        rows: list[dict] = []
        i = 0
        while i < len(pts):
            start = pts[i]
            poly = [start]
            i += 1
            while i < len(pts) and pts[i].distance(start) > 1e-3:
                poly.append(pts[i])
                i += 1
            i += 1  # closing duplicate
            if len(poly) >= 3:
                rows.append({"key": _mapkey(self.clip_id, "crosswalks"),
                             "crosswalk": {"category": "PEDESTRIAN",
                                           "location": [self.frame.point(p) for p in poly]},
                             "version": VERSION})
        return rows

    def _pole_polyline(self, bb: carla.BoundingBox) -> list[dict]:
        assert self.frame is not None
        _, _, up = coords.ue_forward_right_up(bb.rotation)  # not get_up_vector(): see coords.ue_rotation_matrix
        base = carla.Location(bb.location.x - up[0] * bb.extent.z, bb.location.y - up[1] * bb.extent.z,
                              bb.location.z - up[2] * bb.extent.z)
        top = carla.Location(bb.location.x + up[0] * bb.extent.z, bb.location.y + up[1] * bb.extent.z,
                             bb.location.z + up[2] * bb.extent.z)
        return [self.frame.point(top), self.frame.point(base)]

    def _export_level_statics(self) -> tuple[list[dict], list[dict], list[dict]]:
        """Poles, traffic-light heads (poles classified by aspect ratio) and signs."""
        assert self.frame is not None
        poles: list[dict] = []
        lights: list[dict] = []
        signs: list[dict] = []
        for bb in self.world.get_level_bbs(carla.CityObjectLabel.Poles):
            poles.append({"key": _mapkey(self.clip_id, "poles"),
                          "pole": {"category": "UNKNOWN", "location": self._pole_polyline(bb)},
                          "version": VERSION})
        for bb in self.world.get_level_bbs(carla.CityObjectLabel.TrafficLight):
            if bb.extent.z > 2.0 * max(bb.extent.x, bb.extent.y) and max(bb.extent.x, bb.extent.y) < 0.4:
                poles.append({"key": _mapkey(self.clip_id, "poles"),
                              "pole": {"category": "LIGHT", "location": self._pole_polyline(bb)},
                              "version": VERSION})
            else:
                c, d, q = self.frame.bbox(bb)
                lights.append({"key": _mapkey(self.clip_id, "traffic-lights"),
                               "traffic_light": {"center": c, "dimensions": d, "orientation": q,
                                                 "category": "traffic_light"},
                               "version": VERSION})
        for bb in self.world.get_level_bbs(carla.CityObjectLabel.TrafficSigns):
            c, d, q = self.frame.bbox(bb)
            signs.append({"key": _mapkey(self.clip_id, "traffic-signs"),
                          "traffic_sign": {"center": c, "dimensions": d, "orientation": q,
                                           "category": "TRAFFIC_SIGN_INFORMATION"},
                          "version": VERSION})
        return poles, lights, signs

    def _export_wait_lines(self) -> list[dict]:
        assert self.frame is not None
        rows: list[dict] = []
        for tl in self._lights:
            for wp in tl.get_stop_waypoints():
                rows.append({"key": _mapkey(self.clip_id, "wait-lines"),
                             "wait_line": {"category": "STOP",
                                           "location": [self.frame.point(_side_point(wp, -1)),
                                                        self.frame.point(_side_point(wp, +1))],
                                           "is_implicit": True, "intersection_subtype": "ENTRY"},
                             "version": VERSION})
        return rows

    def _actor_centres_ue(self) -> np.ndarray:
        """World centres of every live vehicle/walker, **including the hero**.

        The hero is deliberately absent from :attr:`_actors` (it is the rig, not an obstacle), but
        its mesh *is* in ``get_level_bbs``: without it here the ego's own body was exported as a
        parked car frozen at the start pose, sitting on top of the camera for the whole clip.
        """
        actors = [self.world.get_actor(aid) for aid, _cat, _bb in self._actors] + [self.hero]
        centres = [(a.get_transform().location.x, a.get_transform().location.y, a.get_transform().location.z)
                   for a in actors if a is not None]
        return np.array(centres, dtype=np.float64) if centres else np.zeros((0, 3))

    def _export_static_obstacles(self, min_extent_z: float = 0.2, dedup_m: float = 2.0,
                                 same_centre_m: float = 0.3) -> None:
        """Parked vehicles that are not actors, from the level bounding boxes.

        Emitted later as constant-pose obstacles at the first and last clip timestamps (the loader
        needs >= 2 observations per track).  Two things have to be de-duplicated first:

        * **live actors.**  A level bb whose centre is within ``dedup_m`` of a live vehicle or
          walker is that actor's mesh; the actor is already exported per tick.  The hero counts:
          see :meth:`_actor_centres_ue`.
        * **multi-mesh props.**  Every parked vehicle in the UE5 towns is built from two meshes -
          the body (``..._SM_0``) and the interior/glass shell (``..._SM_1``, ~15 % smaller) - and
          ``get_level_bbs`` returns a box for each, at the same centre.  Of the boxes sharing a
          centre within ``same_centre_m`` only the largest by volume survives.
        """
        assert self.frame is not None
        actor_centres = self._actor_centres_ue()
        n_actor = n_dup = 0
        for label, category in STATIC_OBSTACLE_LABELS.items():
            kept: list[tuple[int, carla.BoundingBox, np.ndarray, float]] = []
            for i, bb in enumerate(self.world.get_level_bbs(label)):
                if bb.extent.z < min_extent_z:
                    continue
                centre = np.array([bb.location.x, bb.location.y, bb.location.z])
                if len(actor_centres) and np.min(np.linalg.norm(actor_centres - centre, axis=1)) < dedup_m:
                    n_actor += 1
                    continue
                volume = float(bb.extent.x * bb.extent.y * bb.extent.z)
                twin = next((k for k, (_j, _b, c, _v) in enumerate(kept)
                             if np.linalg.norm(c - centre) < same_centre_m), None)
                if twin is None:
                    kept.append((i, bb, centre, volume))
                    continue
                n_dup += 1
                if volume > kept[twin][3]:
                    kept[twin] = (i, bb, centre, volume)
            for i, bb, _centre, _volume in kept:
                c, d, q = self.frame.bbox(bb)
                self._static_obstacles.append(
                    {"trackline_id": f"static:{label.name.lower()}:{i}",
                     "center": c, "size": d, "orientation": q, "category": category})
        log.info("scene: %d static parked obstacles (%d level bbs matched live actors, "
                 "%d duplicate meshes dropped)", len(self._static_obstacles), n_actor, n_dup)

    # ------------------------------------------------------------------ per tick
    def record_tick(self, snapshot: carla.WorldSnapshot,
                    depth: dict[str, np.ndarray] | None = None) -> None:
        """Record ego pose, dynamic obstacles and traffic-light states for one tick.

        ``depth`` maps camera name -> the raw metric depth image of this tick; it is what the
        occlusion filter tests against (``visibility="depth"``) and is ignored otherwise.
        """
        assert self.frame is not None, "call begin() first"
        ts = int(round(snapshot.timestamp.elapsed_seconds * 1e6))
        tick = len(self.timestamps)
        self.timestamps.append(ts)
        hero_snap = snapshot.find(self.hero.id)
        if hero_snap is None:
            raise RuntimeError(f"hero actor {self.hero.id} missing from snapshot frame {snapshot.frame}")
        m_ego = self.frame.pose(coords.ue_matrix(hero_snap.get_transform()) @ self.t_axle)
        self.ego_rows.append({"key": {"clip_id": self.clip_id, "timestamp_micros": ts},
                              "egomotion_estimate": {"name": "egomotion_carla",
                                                     "location": coords.xyz(m_ego[:3, 3]),
                                                     "orientation": coords.quat_xyzw(m_ego)},
                              "version": VERSION})
        present: list[str] = []
        rot = []
        centre = []
        size = []
        for aid, category, bb in self._actors:
            asnap = snapshot.find(aid)
            if asnap is None:
                continue  # actor destroyed mid-clip; its track simply ends
            m_world = coords.ue_matrix(asnap.get_transform()) @ coords.ue_matrix(
                carla.Transform(bb.location, bb.rotation))
            mf = self.frame.pose(m_world)
            dims = (2 * bb.extent.x, 2 * bb.extent.y, 2 * bb.extent.z)
            self._obs_index.setdefault(str(aid), []).append((tick, len(self.obstacle_rows)))
            self.obstacle_rows.append(
                {"key": {"clip_id": self.clip_id, "timestamp_micros": ts,
                         "label_class_id": "scene:obstacles:carla:v0"},
                 "obstacle": {"trackline_id": str(aid), "center": coords.xyz(mf[:3, 3]),
                              "size": coords.xyz(dims),
                              "orientation": coords.quat_xyzw(mf), "category": category},
                 "version": VERSION})
            present.append(str(aid))
            rot.append(mf[:3, :3])
            centre.append(mf[:3, 3])
            size.append(dims)
        self._record_visibility(tick, m_ego, present, rot, centre, size, depth)
        self.tl_states.append({str(tl.id): str(tl.get_state()) for tl in self._lights})

    def _record_visibility(self, tick: int, m_ego: np.ndarray, present: list[str], rot: list[np.ndarray],
                           centre: list[np.ndarray], size: list[tuple[float, float, float]],
                           depth: dict[str, np.ndarray] | None) -> None:
        """One z-buffer decision per obstacle for this tick (live actors and parked meshes)."""
        if not self.visibility.enabled:
            return
        t0 = time.perf_counter()
        ids = present + [o["trackline_id"] for o in self._static_obstacles]
        if depth:
            points = vis.box_points_from_poses(np.asarray(rot), np.asarray(centre), np.asarray(size),
                                               self._lattice) if present else \
                np.zeros((0, len(self._lattice), 3))
            if len(self._static_points):
                points = np.concatenate([points, self._static_points]) if len(points) else self._static_points
            fractions = vis.visible_fractions(points, m_ego, self._cameras, depth, self.visibility)
        else:  # no depth for this tick: nothing can be judged, keep everything
            fractions = np.ones(len(ids))
        seen = fractions >= 0.0
        state = np.where(seen & (fractions >= self.visibility.min_visible_fraction), 1, 0)
        for i, tid in enumerate(ids):
            self._state.setdefault(tid, [vis.ABSENT] * tick).append(int(state[i]))
            self._fraction.setdefault(tid, [-1.0] * tick).append(float(fractions[i]))
        for tid in self._state:  # actors that were not in this snapshot
            if len(self._state[tid]) == tick:
                self._state[tid].append(vis.ABSENT)
                self._fraction[tid].append(-1.0)
        self.visibility_micros.append((time.perf_counter() - t0) * 1e6)

    # ------------------------------------------------------------------ output
    def write(self, out_dir: str | Path, cameras: Iterable[CameraManifest]) -> Path:
        """Write all Parquet tables plus the timestamp and traffic-light sidecars."""
        if not self.timestamps:
            raise RuntimeError("no ticks recorded")
        out = Path(out_dir)
        out.mkdir(parents=True, exist_ok=True)
        obstacle_rows, occluded, decisions = self._obstacle_rows()
        if self.visibility.enabled:
            self._write_visibility_sidecar(out, occluded, decisions)
        self._write_table(out, "egomotion_estimate", self.ego_rows)
        self._write_table(out, "obstacle", obstacle_rows)
        self._write_table(out, "calibration_estimate", [self._calibration_row(cameras)])
        for name, rows in self._static.items():
            self._write_table(out, name, rows)
        clipgt.write_timestamps(out, self.clip_id, self.timestamps)
        tl_file = out / f"{self.clip_id}.traffic_light_states.json"
        tl_file.write_text(json.dumps(
            {"timestamps_micros": self.timestamps, "states": self.tl_states}, indent=1))
        return out

    # ------------------------------------------------------------------ obstacle table
    def _obstacle_row(self, ts: int, payload: dict) -> dict:
        return {"key": {"clip_id": self.clip_id, "timestamp_micros": ts,
                        "label_class_id": "scene:obstacles:carla:v0"},
                "obstacle": payload, "version": VERSION}

    def _obstacle_rows(self) -> tuple[list[dict], list[dict], dict[str, dict]]:
        """The obstacle table, plus what the filter dropped and why.

        With ``visibility="none"`` this is the plain export: every live actor at every tick it
        was in the snapshot, every parked mesh at the first and last timestamp.  With
        ``"depth"`` each track is cut into visibility segments (see :mod:`carla_cosmos.
        visibility`): segments get their own ``trackline_id`` so NVIDIA's loader cannot
        interpolate across the hole, segments shorter than two observations are dropped (the
        loader skips them anyway), and a parked obstacle that is hidden for part of the clip
        stops being a clip-wide two-row track and becomes one two-row track per segment.
        """
        if not self.visibility.enabled:
            rows = list(self.obstacle_rows)
            for proto in self._static_obstacles:
                for ts in (self.timestamps[0], self.timestamps[-1]):
                    rows.append(self._obstacle_row(ts, proto))
            return rows, [], {}
        rows: list[dict] = []
        occluded: list[dict] = []
        decisions: dict[str, dict] = {}
        n_ticks = len(self.timestamps)
        for tid, static in self._tracks():
            raw = np.array(self._state.get(tid, [vis.ABSENT] * n_ticks), np.int8)
            state = vis.apply_hysteresis(raw, self.visibility.hysteresis_frames)
            segs = vis.keep_segments(state)
            ids = vis.segment_ids(tid, len(segs))
            kept: set[int] = set()
            for (a, b), sid in zip(segs, ids):
                kept.update(range(a, b))
                rows.extend(self._segment_rows(tid, sid, static, a, b))
            occluded.extend(self._occluded_entries(tid, static, kept))
            decisions[tid] = {"static": static, "category": self._category(tid, static),
                              "raw": [int(v) for v in raw], "state": [int(v) for v in state],
                              "fraction": [round(float(f), 4) for f in
                                           self._fraction.get(tid, [-1.0] * n_ticks)],
                              "segments": [[int(a), int(b)] for a, b in segs],
                              "trackline_ids": list(ids[:len(segs)])}
        n_dropped = sum(1 for t in decisions.values() if not t["segments"])
        log.info("scene: occlusion filter kept %d of %d obstacle observations, %d of %d tracks "
                 "(%d track(s) split into segments)", len(rows) - self._static_row_count(rows),
                 len(self.obstacle_rows), len(decisions) - n_dropped, len(decisions),
                 sum(1 for t in decisions.values() if len(t["segments"]) > 1))
        return rows, occluded, decisions

    def _tracks(self) -> list[tuple[str, bool]]:
        """``(track id, is a parked level-bb obstacle)`` for every track the clip knows."""
        return ([(str(aid), False) for aid, _c, _bb in self._actors]
                + [(o["trackline_id"], True) for o in self._static_obstacles])

    def _category(self, tid: str, static: bool) -> str:
        if static:
            return next(o["category"] for o in self._static_obstacles if o["trackline_id"] == tid)
        return next((c for aid, c, _bb in self._actors if str(aid) == tid), "")

    def _static_proto(self, tid: str) -> dict:
        return next(o for o in self._static_obstacles if o["trackline_id"] == tid)

    def _observations(self, tid: str, static: bool):
        """``(tick, payload)`` of every tick this obstacle existed at."""
        if static:
            proto = self._static_proto(tid)
            return [(t, proto) for t in range(len(self.timestamps))]
        return [(tick, self.obstacle_rows[i]["obstacle"]) for tick, i in self._obs_index.get(tid, [])]

    def _occluded_entries(self, tid: str, static: bool, kept: set[int]) -> list[dict]:
        """Dropped observations of one track, as ``{"ticks": [a, b), "obstacle": payload}``.

        A parked obstacle keeps the same pose for its whole occluded run, so the run is one
        entry; a live actor moves, so each dropped tick is its own.  This is only for
        ``carla-cosmos preview --show-occluded`` — nothing the renderer sees.
        """
        out: list[dict] = []
        if static:
            proto = self._static_proto(tid)
            hidden = [t for t in range(len(self.timestamps)) if t not in kept]
            for a, b in _runs(hidden):
                out.append({"ticks": [a, b], "obstacle": proto})
            return out
        for tick, payload in self._observations(tid, static):
            if tick not in kept:
                out.append({"ticks": [tick, tick + 1], "obstacle": payload})
        return out

    def _segment_rows(self, tid: str, sid: str, static: bool, a: int, b: int) -> list[dict]:
        """Rows of one visibility segment.

        A parked obstacle does not move, so two rows (the ends of the segment) carry it: the
        loader interpolates linearly between them and reproduces the constant pose, at a
        fraction of the table size.  A live actor needs every tick it was seen at.
        """
        if static:
            proto = self._static_proto(tid)
            return [self._obstacle_row(self.timestamps[t], dict(proto, trackline_id=sid))
                    for t in (a, b - 1)]
        rows = []
        for tick, i in self._obs_index.get(tid, []):
            if a <= tick < b:
                payload = dict(self.obstacle_rows[i]["obstacle"], trackline_id=sid)
                rows.append(self._obstacle_row(self.timestamps[tick], payload))
        return rows

    @staticmethod
    def _static_row_count(rows: list[dict]) -> int:
        return sum(1 for r in rows if str(r["obstacle"]["trackline_id"]).startswith("static:"))

    def _write_visibility_sidecar(self, out: Path, occluded: list[dict], decisions: dict) -> None:
        """``<clip>.visibility.parquet``-in-JSON: what the filter decided, for ``preview
        --show-occluded``.

        It carries the geometry of the dropped observations too, since those rows are by
        construction absent from ``obstacle.parquet`` and the preview has nowhere else to get
        them from.  NVIDIA's loader addresses its tables by exact file name, so the extra file
        is inert on the server side.
        """
        path = out / f"{self.clip_id}.visibility.json"
        path.write_text(json.dumps({
            "clip_id": self.clip_id,
            "params": self.visibility.as_dict(),
            "timestamps_micros": self.timestamps,
            "cost_micros_per_tick": round(float(np.mean(self.visibility_micros)), 1)
            if self.visibility_micros else None,
            "tracks": decisions,
            "occluded": occluded,
        }))
        log.info("scene: wrote %-22s tracks=%5d occluded observations=%6d",
                 "visibility", len(decisions), len(occluded))

    def _write_table(self, out: Path, name: str, rows: list[dict]) -> None:
        clipgt.write_table(out, self.clip_id, name, rows)
        log.info("scene: wrote %-22s rows=%6d", name, len(rows))

    def _calibration_row(self, cameras: Iterable[CameraManifest]) -> dict:
        return clipgt.calibration_row(self.clip_id, self.timestamps[0], cameras)
