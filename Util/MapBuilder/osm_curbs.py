#!/usr/bin/env python3
"""
Procedural 3D Sidewalk Curbs Engine for CARLA Map Maker.

Generates photorealistic, watertight 3D concrete/stone curbs along all elevated
sidewalk edges from OpenDRIVE and OpenStreetMap road networks:
  - Inner Curb (Roadway / Cycleway facing):
      * 16cm top stone surface (DIN 483 / European curb stone standard)
      * 3cm 45-degree beveled chamfer
      * Vertical curb face dropping from sidewalk height (+15.24cm) to gutter level
      * 25-32cm gutter channel sealing the road/cycleway asphalt seam
      * Submerged foundation drop anchoring 8cm below ground to prevent any see-through gaps
  - Outer Curb (Terrain / Lawn / Plaza facing):
      * 12cm horizontal stone border
      * Vertical foundation skirt dropping 8cm into ground
  - Transverse End Caps:
      * Watertight closing caps at sidewalk run endpoints and pedestrian junctions
"""

from __future__ import annotations

from dataclasses import dataclass, field
import json
import math
import os
from pathlib import Path
import tempfile
from typing import Any, Sequence
import xml.etree.ElementTree as ET

try:
    import carla
except ImportError:
    carla = None

try:
    from Util.MapBuilder.cmesh import write_cmesh_file, read_cmesh_file
except ImportError:
    try:
        from cmesh import write_cmesh_file, read_cmesh_file
    except ImportError:
        write_cmesh_file = None  # type: ignore[assignment]
        read_cmesh_file = None   # type: ignore[assignment]

# Local repo root and temp setup
_current_dir = Path(__file__).resolve().parent
_repo_root = (
    _current_dir.parent.parent
    if _current_dir.name in ("MapBuilder", "util") and _current_dir.parent.name in ("Util", "PythonAPI")
    else Path.cwd()
)
_local_tmp = _repo_root / ".tmp"
try:
    _local_tmp.mkdir(parents=True, exist_ok=True)
    tempfile.tempdir = str(_local_tmp)
    os.environ["TMPDIR"] = str(_local_tmp)
except Exception:
    pass


@dataclass
class CurbRunPoint:
    s: float
    x: float
    y: float
    z: float
    lane_width: float
    inner_ux: float
    inner_uy: float


@dataclass
class CurbConfig:
    enabled: bool = True
    inner_curbs: bool = True
    outer_curbs: bool = True
    curb_top_width: float = 0.16       # 16cm top stone width
    outer_border_width: float = 0.12   # 12cm outer rim width
    gutter_width: float = 0.32         # 32cm gutter channel towards roadway
    chamfer_size: float = 0.03         # 3cm 45 deg chamfer bevel
    curb_elevation: float = 0.155      # Sidewalk top offset (+15.5cm)
    gutter_elevation: float = 0.012    # Gutter channel offset (+1.2cm above road)
    submerge_depth: float = 0.08       # Foundation drop depth (-8cm below road)
    step: float = 0.5                  # Longitudinal sampling interval (meters)
    material_preset: str = "/Game/Carla/Static/SideWalk/Curbs/MI_Curb_01.MI_Curb_01"


def _dist_pt_seg(px: float, py: float, ax: float, ay: float, bx: float, by: float) -> float:
    dx, dy = bx - ax, by - ay
    l2 = dx * dx + dy * dy
    if l2 == 0:
        return math.hypot(px - ax, py - ay)
    t = max(0.0, min(1.0, ((px - ax) * dx + (py - ay) * dy) / l2))
    return math.hypot(px - (ax + t * dx), py - (ay + t * dy))


def _dist_pt_polyline(px: float, py: float, poly: Sequence[tuple[float, float]]) -> float:
    d_min = 1e9
    for i in range(len(poly) - 1):
        d = _dist_pt_seg(px, py, poly[i][0], poly[i][1], poly[i + 1][0], poly[i + 1][1])
        if d < d_min:
            d_min = d
    return d_min


def extract_sidewalk_curb_runs(
    carla_map: Any,
    xodr_xml_content: str,
    step: float = 0.5,
    cycleways: list[Any] | None = None,
) -> list[list[CurbRunPoint]]:
    """
    Extracts continuous runs of sidewalk edge sample points from OpenDRIVE,
    ensuring that no curb or gutter encroaches on active cycleway corridors.
    """
    if carla_map is None:
        return []

    cyc_corridors: list[tuple[list[tuple[float, float]], float]] = []
    if cycleways:
        for c in cycleways:
            nodes = (
                c.nodes_carla
                if hasattr(c, "nodes_carla")
                else (c.get("nodes_carla", []) if isinstance(c, dict) else [])
            )
            w = (
                c.width_m
                if hasattr(c, "width_m")
                else (c.get("width_m", 2.0) if isinstance(c, dict) else 2.0)
            )
            if len(nodes) >= 2:
                cyc_corridors.append(([(float(n[0]), float(n[1])) for n in nodes], float(w) * 0.5))

    root = ET.fromstring(xodr_xml_content)
    runs: list[list[CurbRunPoint]] = []

    for road in root.findall("road"):
        rid = int(road.get("id", "0"))
        length = float(road.get("length", "0.0"))
        if length < 2.0:
            continue

        lanes_elem = road.find("lanes")
        if lanes_elem is None:
            continue

        for side in ("left", "right"):
            side_elem = lanes_elem.find(f"laneSection/{side}")
            if side_elem is None:
                continue

            for lane in side_elem.findall("lane"):
                if lane.get("type") != "sidewalk":
                    continue
                lid = int(lane.get("id", "0"))

                num_steps = int(math.ceil(length / step))
                pts: list[CurbRunPoint] = []
                for i in range(num_steps + 1):
                    s = min(float(i * step), length)
                    wp = carla_map.get_waypoint_xodr(rid, lid, s)
                    if not wp or wp.lane_width < 0.1:
                        continue

                    loc = wp.transform.location
                    w = wp.lane_width

                    # Calculate unit vector towards inner adjacent lane (roadway center)
                    inner_lid = lid + 1 if lid < 0 else lid - 1
                    wp_in = (
                        carla_map.get_waypoint_xodr(rid, inner_lid, s)
                        if inner_lid != 0
                        else None
                    )

                    if wp_in:
                        vx = wp_in.transform.location.x - loc.x
                        vy = wp_in.transform.location.y - loc.y
                        vlen = math.hypot(vx, vy)
                        ux, uy = (vx / vlen, vy / vlen) if vlen > 0.0 else (0.0, 0.0)
                    else:
                        yaw = math.radians(wp.transform.rotation.yaw)
                        ux = math.sin(yaw) if lid < 0 else -math.sin(yaw)
                        uy = -math.cos(yaw) if lid < 0 else math.cos(yaw)

                    # Check if sidewalk inner edge or gutter encroaches on any cycleway corridor
                    if cyc_corridors:
                        in_x = loc.x + 0.5 * w * ux
                        in_y = loc.y + 0.5 * w * uy
                        gut_x = in_x + 0.32 * ux
                        gut_y = in_y + 0.32 * uy
                        encroaching = False
                        for poly, r_cyc in cyc_corridors:
                            if _dist_pt_polyline(in_x, in_y, poly) < (r_cyc - 0.05) or \
                               _dist_pt_polyline(gut_x, gut_y, poly) < (r_cyc - 0.05):
                                encroaching = True
                                break
                        if encroaching:
                            continue

                    pts.append(
                        CurbRunPoint(
                            s=s,
                            x=loc.x,
                            y=loc.y,
                            z=loc.z,
                            lane_width=w,
                            inner_ux=ux,
                            inner_uy=uy,
                        )
                    )

                if len(pts) < 2:
                    continue

                # Partition into continuous runs
                curr_run = [pts[0]]
                for k in range(1, len(pts)):
                    if pts[k].s - pts[k - 1].s <= (step * 2.5):
                        curr_run.append(pts[k])
                    else:
                        if len(curr_run) >= 2:
                            runs.append(curr_run)
                        curr_run = [pts[k]]
                if len(curr_run) >= 2:
                    runs.append(curr_run)

    return runs


def build_sidewalk_curbs_geometry(
    runs: list[list[CurbRunPoint]],
    config: CurbConfig | None = None,
) -> dict[str, Any]:
    """
    Builds 3D mesh geometry for inner curbs, outer curbs, and end caps.
    Returns vertices, triangles, normals, uvs, and audit metrics.
    """
    if config is None:
        config = CurbConfig()

    all_verts: list[tuple[float, float, float]] = []
    all_tris: list[tuple[int, int, int]] = []
    all_norms: list[tuple[float, float, float]] = []
    all_uvs: list[tuple[float, float]] = []

    W_curb_top = config.curb_top_width
    W_gutter = config.gutter_width
    B = config.chamfer_size
    z_top_off = config.curb_elevation
    z_gut_off = config.gutter_elevation
    z_bot_off = -config.submerge_depth
    W_outer = config.outer_border_width

    total_curb_length = 0.0
    inner_curbs_count = 0
    outer_curbs_count = 0

    for run in runs:
        N = len(run)
        if N < 2:
            continue

        run_len = run[-1].s - run[0].s
        total_curb_length += run_len

        # ---------------------------------------------------------------------
        # 1. Inner Curb (Roadway / Cycleway facing)
        # ---------------------------------------------------------------------
        if config.inner_curbs:
            inner_curbs_count += 1
            for i in range(N - 1):
                p0 = run[i]
                p1 = run[i + 1]

                in0_x = p0.x + 0.5 * p0.lane_width * p0.inner_ux
                in0_y = p0.y + 0.5 * p0.lane_width * p0.inner_uy
                in1_x = p1.x + 0.5 * p1.lane_width * p1.inner_ux
                in1_y = p1.y + 0.5 * p1.lane_width * p1.inner_uy

                z_top0, z_top1 = p0.z + z_top_off, p1.z + z_top_off
                z_gut0, z_gut1 = p0.z + z_gut_off, p1.z + z_gut_off
                z_bot0, z_bot1 = p0.z + z_bot_off, p1.z + z_bot_off

                # Face 1: Gutter Channel
                p_gut_out0 = (in0_x + W_gutter * p0.inner_ux, in0_y + W_gutter * p0.inner_uy, z_gut0)
                p_gut_in0 = (in0_x + 0.01 * p0.inner_ux, in0_y + 0.01 * p0.inner_uy, z_gut0)
                p_gut_out1 = (in1_x + W_gutter * p1.inner_ux, in1_y + W_gutter * p1.inner_uy, z_gut1)
                p_gut_in1 = (in1_x + 0.01 * p1.inner_ux, in1_y + 0.01 * p1.inner_uy, z_gut1)

                base_idx = len(all_verts)
                all_verts.extend([p_gut_out0, p_gut_in0, p_gut_in1, p_gut_out1])
                all_norms.extend([(0.0, 0.0, 1.0)] * 4)
                all_uvs.extend([(p0.s * 2.0, 0.0), (p0.s * 2.0, 0.5), (p1.s * 2.0, 0.5), (p1.s * 2.0, 0.0)])
                all_tris.append((base_idx, base_idx + 1, base_idx + 2))
                all_tris.append((base_idx, base_idx + 2, base_idx + 3))

                # Gutter Outer Submerged Drop (prevents any gaps on road side)
                p_gut_drop0 = (in0_x + W_gutter * p0.inner_ux, in0_y + W_gutter * p0.inner_uy, z_bot0)
                p_gut_drop1 = (in1_x + W_gutter * p1.inner_ux, in1_y + W_gutter * p1.inner_uy, z_bot1)
                norm_drop = (0.5 * (p0.inner_ux + p1.inner_ux), 0.5 * (p0.inner_uy + p1.inner_uy), 0.0)
                base_idx = len(all_verts)
                all_verts.extend([p_gut_drop0, p_gut_out0, p_gut_out1, p_gut_drop1])
                all_norms.extend([norm_drop] * 4)
                all_uvs.extend([(p0.s * 2.0, 0.0), (p0.s * 2.0, 0.2), (p1.s * 2.0, 0.2), (p1.s * 2.0, 0.0)])
                all_tris.append((base_idx, base_idx + 1, base_idx + 2))
                all_tris.append((base_idx, base_idx + 2, base_idx + 3))

                # Face 2: Vertical Curb Face (rising from gutter to chamfer bottom)
                p_curb_cham_bot0 = (in0_x + 0.01 * p0.inner_ux, in0_y + 0.01 * p0.inner_uy, z_top0 - B)
                p_curb_cham_bot1 = (in1_x + 0.01 * p1.inner_ux, in1_y + 0.01 * p1.inner_uy, z_top1 - B)
                norm_face = (0.5 * (p0.inner_ux + p1.inner_ux), 0.5 * (p0.inner_uy + p1.inner_uy), 0.0)
                base_idx = len(all_verts)
                all_verts.extend([p_gut_in0, p_curb_cham_bot0, p_curb_cham_bot1, p_gut_in1])
                all_norms.extend([norm_face] * 4)
                all_uvs.extend([(p0.s * 2.0, 0.0), (p0.s * 2.0, 0.8), (p1.s * 2.0, 0.8), (p1.s * 2.0, 0.0)])
                all_tris.append((base_idx, base_idx + 1, base_idx + 2))
                all_tris.append((base_idx, base_idx + 2, base_idx + 3))

                # Face 3: Beveled Chamfer (45 deg)
                p_cham_top0 = (in0_x - B * p0.inner_ux, in0_y - B * p0.inner_uy, z_top0)
                p_cham_top1 = (in1_x - B * p1.inner_ux, in1_y - B * p1.inner_uy, z_top1)
                norm_cham = (
                    0.5 * (p0.inner_ux + p1.inner_ux) * 0.707,
                    0.5 * (p0.inner_uy + p1.inner_uy) * 0.707,
                    0.707,
                )
                base_idx = len(all_verts)
                all_verts.extend([p_curb_cham_bot0, p_cham_top0, p_cham_top1, p_curb_cham_bot1])
                all_norms.extend([norm_cham] * 4)
                all_uvs.extend([(p0.s * 2.0, 0.0), (p0.s * 2.0, 0.2), (p1.s * 2.0, 0.2), (p1.s * 2.0, 0.0)])
                all_tris.append((base_idx, base_idx + 1, base_idx + 2))
                all_tris.append((base_idx, base_idx + 2, base_idx + 3))

                # Face 4: Horizontal Top Stone Surface
                p_back_top0 = (in0_x - W_curb_top * p0.inner_ux, in0_y - W_curb_top * p0.inner_uy, z_top0)
                p_back_top1 = (in1_x - W_curb_top * p1.inner_ux, in1_y - W_curb_top * p1.inner_uy, z_top1)
                norm_top = (0.0, 0.0, 1.0)
                base_idx = len(all_verts)
                all_verts.extend([p_cham_top0, p_back_top0, p_back_top1, p_cham_top1])
                all_norms.extend([norm_top] * 4)
                all_uvs.extend([(p0.s * 2.0, 0.0), (p0.s * 2.0, 0.5), (p1.s * 2.0, 0.5), (p1.s * 2.0, 0.0)])
                all_tris.append((base_idx, base_idx + 1, base_idx + 2))
                all_tris.append((base_idx, base_idx + 2, base_idx + 3))

        # ---------------------------------------------------------------------
        # 2. Outer Curb (Terrain / Lawn / Plaza facing)
        # ---------------------------------------------------------------------
        if config.outer_curbs:
            outer_curbs_count += 1
            for i in range(N - 1):
                p0 = run[i]
                p1 = run[i + 1]
                vx0, vy0 = -p0.inner_ux, -p0.inner_uy
                vx1, vy1 = -p1.inner_ux, -p1.inner_uy

                out0_x = p0.x + 0.5 * p0.lane_width * vx0
                out0_y = p0.y + 0.5 * p0.lane_width * vy0
                out1_x = p1.x + 0.5 * p1.lane_width * vx1
                out1_y = p1.y + 0.5 * p1.lane_width * vy1

                z_top0, z_top1 = p0.z + z_top_off, p1.z + z_top_off
                z_bot0, z_bot1 = p0.z + z_bot_off, p1.z + z_bot_off

                # Vertical Skirt Dropping into Ground
                p_out_bot0 = (out0_x + 0.02 * vx0, out0_y + 0.02 * vy0, z_bot0)
                p_out_top0 = (out0_x + 0.02 * vx0, out0_y + 0.02 * vy0, z_top0)
                p_out_top1 = (out1_x + 0.02 * vx1, out1_y + 0.02 * vy1, z_top1)
                p_out_bot1 = (out1_x + 0.02 * vx1, out1_y + 0.02 * vy1, z_bot1)

                norm_out = (0.5 * (vx0 + vx1), 0.5 * (vy0 + vy1), 0.0)
                base_idx = len(all_verts)
                all_verts.extend([p_out_bot0, p_out_top0, p_out_top1, p_out_bot1])
                all_norms.extend([norm_out] * 4)
                all_uvs.extend([(p0.s * 2.0, 0.0), (p0.s * 2.0, 1.0), (p1.s * 2.0, 1.0), (p1.s * 2.0, 0.0)])
                all_tris.append((base_idx, base_idx + 1, base_idx + 2))
                all_tris.append((base_idx, base_idx + 2, base_idx + 3))

                # Horizontal Outer Border
                p_out_back0 = (out0_x - W_outer * vx0, out0_y - W_outer * vy0, z_top0)
                p_out_back1 = (out1_x - W_outer * vx1, out1_y - W_outer * vy1, z_top1)
                base_idx = len(all_verts)
                all_verts.extend([p_out_top0, p_out_back0, p_out_back1, p_out_top1])
                all_norms.extend([(0.0, 0.0, 1.0)] * 4)
                all_uvs.extend([(p0.s * 2.0, 0.0), (p0.s * 2.0, 0.5), (p1.s * 2.0, 0.5), (p1.s * 2.0, 0.0)])
                all_tris.append((base_idx, base_idx + 1, base_idx + 2))
                all_tris.append((base_idx, base_idx + 2, base_idx + 3))

        # ---------------------------------------------------------------------
        # 3. Transverse End Caps (Watertight run ends)
        # ---------------------------------------------------------------------
        for endpoint, is_start in [(run[0], True), (run[-1], False)]:
            xe, ye, ze, we = endpoint.x, endpoint.y, endpoint.z, endpoint.lane_width
            uxe, uye = endpoint.inner_ux, endpoint.inner_uy
            vxe, vye = -uxe, -uye
            in_x, in_y = xe + 0.5 * we * uxe, ye + 0.5 * we * uye
            out_x, out_y = xe + 0.5 * we * vxe, ye + 0.5 * we * vye
            z_te = ze + z_top_off
            z_be = ze + z_bot_off

            p_in_bot = (in_x + W_gutter * uxe, in_y + W_gutter * uye, z_be)
            p_in_top = (in_x + 0.01 * uxe, in_y + 0.01 * uye, z_te)
            p_out_top = (out_x + 0.02 * vxe, out_y + 0.02 * vye, z_te)
            p_out_bot = (out_x + 0.02 * vxe, out_y + 0.02 * vye, z_be)

            norm_cap = (-uye, uxe, 0.0) if is_start else (uye, -uxe, 0.0)
            base_idx = len(all_verts)
            all_verts.extend([p_in_bot, p_in_top, p_out_top, p_out_bot])
            all_norms.extend([norm_cap] * 4)
            all_uvs.extend([(0.0, 0.0), (0.0, 1.0), (we, 1.0), (we, 0.0)])
            if is_start:
                all_tris.append((base_idx, base_idx + 1, base_idx + 2))
                all_tris.append((base_idx, base_idx + 2, base_idx + 3))
            else:
                all_tris.append((base_idx, base_idx + 3, base_idx + 2))
                all_tris.append((base_idx, base_idx + 2, base_idx + 1))

    metrics = {
        "sidewalk_runs": len(runs),
        "total_curb_length_m": round(total_curb_length, 2),
        "inner_curbs_count": inner_curbs_count,
        "outer_curbs_count": outer_curbs_count,
        "vertices_count": len(all_verts),
        "triangles_count": len(all_tris),
    }

    return {
        "vertices": all_verts,
        "triangles": all_tris,
        "normals": all_norms,
        "uvs": all_uvs,
        "metrics": metrics,
    }


def generate_sidewalk_curbs_mesh(
    carla_map: Any,
    xodr_xml_content: str,
    output_cmesh_path: Path | str | None = None,
    config: CurbConfig | None = None,
    cycleways: list[Any] | None = None,
) -> dict[str, Any]:
    """
    High-level generator: extracts sidewalk runs, generates 3D geometry,
    and optionally writes out a .cmesh file.
    """
    runs = extract_sidewalk_curb_runs(
        carla_map,
        xodr_xml_content,
        step=config.step if config else 0.5,
        cycleways=cycleways,
    )

    geom = build_sidewalk_curbs_geometry(runs, config=config)

    if output_cmesh_path is not None and geom["vertices"] and write_cmesh_file is not None:
        p = Path(output_cmesh_path)
        p.parent.mkdir(parents=True, exist_ok=True)
        write_cmesh_file(
            p,
            vertices=geom["vertices"],
            triangles=geom["triangles"],
            normals=geom["normals"],
            uvs=geom["uvs"],
        )

    return geom


def audit_curb_metrics(metrics: dict[str, Any]) -> dict[str, Any]:
    """Formats and structures curb metrics for serialization into Placements/curbs.json."""
    return {
        "version": "1.0",
        "feature": "osm_curbs",
        "counts": {
            "sidewalk_runs": metrics.get("sidewalk_runs", 0),
            "total_length_m": metrics.get("total_curb_length_m", 0.0),
            "inner_curbs": metrics.get("inner_curbs_count", 0),
            "outer_curbs": metrics.get("outer_curbs_count", 0),
        },
        "mesh": {
            "vertices": metrics.get("vertices_count", 0),
            "triangles": metrics.get("triangles_count", 0),
        },
    }
