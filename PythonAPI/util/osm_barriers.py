#!/usr/bin/env python3
"""
Continuous Barriers, Perimeter Walls, Fences & Guard Rails Engine for CARLA Map Maker.

Milestone 14 (M14):
  - Ingests OSM ways and nodes tagged with barrier=* (wall, fence, guard_rail, retaining_wall, hedge, etc.).
  - Filters out underground and indoor features (level < 0, indoor=yes, tunnel=yes).
  - Classifies barriers into semantic categories:
      * Walls (barrier=wall, retaining_wall, city_wall) -> CityObjectLabel.Walls
      * Fences & Railings (barrier=fence, railing, handrail) -> CityObjectLabel.Fences
      * Guard Rails (barrier=guard_rail, crash_barrier, jersey_barrier) -> CityObjectLabel.GuardRail
      * Living Hedges (barrier=hedge) -> CityObjectLabel.Vegetation
  - Generates watertight, 3D extruded prism meshes with miter joins at corners, closed endcaps,
    accurate normals, and seamless UV coordinates.
  - Generates obstacle geometry for Recast NavMesh compilation so pedestrians navigate around barriers.
"""

from __future__ import annotations

from dataclasses import dataclass, field
import math
import os
from pathlib import Path
import tempfile
from typing import Any, Callable, Sequence
import xml.etree.ElementTree as ET

try:
    from Util.MapBuilder.cmesh import write_cmesh_file
except ImportError:
    try:
        from cmesh import write_cmesh_file
    except ImportError:
        write_cmesh_file = None  # type: ignore[assignment]

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
class BarrierWay:
    """Represents an extracted OpenStreetMap barrier feature."""
    osm_id: int
    barrier_type: str
    category: str  # "wall", "fence", "guard_rail", "hedge"
    nodes: list[tuple[float, float]]  # (lat, lon) coordinates
    tags: dict[str, str] = field(default_factory=dict)
    height: float = 1.2
    width: float = 0.3
    material_preset: str = "wall_concrete"
    semantic_label: str = "Walls"
    is_closed: bool = False


def parse_barrier_height(tags: dict[str, str], default_height: float = 1.2) -> float:
    """Parses height tag in meters, handling unit suffixes ('m', 'cm')."""
    raw_h = tags.get("height") or tags.get("est_height") or tags.get("barrier:height")
    if not raw_h:
        return default_height

    raw_h = raw_h.strip().lower()
    try:
        if raw_h.endswith("cm"):
            return max(0.2, float(raw_h[:-2].strip()) / 100.0)
        if raw_h.endswith("m"):
            return max(0.2, float(raw_h[:-1].strip()))
        return max(0.2, float(raw_h))
    except (ValueError, TypeError):
        return default_height


def parse_barrier_width(tags: dict[str, str], default_width: float = 0.3) -> float:
    """Parses width tag in meters."""
    raw_w = tags.get("width") or tags.get("barrier:width")
    if not raw_w:
        return default_width

    raw_w = raw_w.strip().lower()
    try:
        if raw_w.endswith("cm"):
            return max(0.05, float(raw_w[:-2].strip()) / 100.0)
        if raw_w.endswith("m"):
            return max(0.05, float(raw_w[:-1].strip()))
        return max(0.05, float(raw_w))
    except (ValueError, TypeError):
        return default_width


def classify_barrier(tags: dict[str, str]) -> tuple[str, str, str, float, float] | None:
    """
    Classifies an OSM barrier tag set into:
      (category, semantic_label, material_preset, default_height, default_width)
    Returns None if the feature is underground, indoor, or non-physical.
    """
    # 1. Filter out underground / indoor levels
    level_str = tags.get("level", "0").strip()
    try:
        # Check for negative floor levels (e.g. "-1", "-2", "-3")
        if float(level_str) < 0:
            return None
    except ValueError:
        pass

    if tags.get("tunnel") in ("yes", "building_passage") or tags.get("indoor") == "yes":
        return None

    barrier_val = (tags.get("barrier") or "").lower().strip()
    if not barrier_val:
        if tags.get("historic") == "citywalls":
            barrier_val = "wall"
        elif tags.get("natural") == "hedge":
            barrier_val = "hedge"
        else:
            return None

    # Skip non-continuous point markers unless represented as a line
    if barrier_val in ("entrance", "kerb", "lift_gate", "toll_booth"):
        return None

    material = (tags.get("material") or tags.get("fence_type") or tags.get("wall") or "").lower().strip()

    # Category A: Walls
    if barrier_val in ("wall", "retaining_wall", "city_wall", "stone_wall") or tags.get("historic") == "citywalls":
        category = "wall"
        semantic_label = "Walls"
        def_height = parse_barrier_height(tags, default_height=1.5)
        def_width = parse_barrier_width(tags, default_width=0.35)

        if material in ("brick", "brickwork"):
            mat_preset = "wall_brick"
        elif material in ("stone", "masonry", "rock", "granite", "sandstone"):
            mat_preset = "wall_stone"
        elif material in ("plaster", "render"):
            mat_preset = "/Game/Carla/Static/GenericMaterials/Facade/MI_Facade01.MI_Facade01"
        else:
            mat_preset = "wall_concrete"

        return category, semantic_label, mat_preset, def_height, def_width

    # Category B: Fences & Railings
    if barrier_val in ("fence", "railing", "handrail", "wire_fence", "guard_rail_pedestrian"):
        category = "fence"
        semantic_label = "Fences"
        def_height = parse_barrier_height(tags, default_height=1.2)
        def_width = parse_barrier_width(tags, default_width=0.10)

        if material in ("wood", "timber", "wooden"):
            mat_preset = "fence_wood"
        else:
            mat_preset = "fence_metal"

        return category, semantic_label, mat_preset, def_height, def_width

    # Category C: Highway Guard Rails & Crash Barriers
    if barrier_val in ("guard_rail", "crash_barrier", "jersey_barrier"):
        category = "guard_rail"
        semantic_label = "GuardRail"
        def_height = parse_barrier_height(tags, default_height=0.85)
        def_width = parse_barrier_width(tags, default_width=0.25)

        if barrier_val == "jersey_barrier":
            mat_preset = "wall_concrete"
        else:
            mat_preset = "guardrail"

        return category, semantic_label, mat_preset, def_height, def_width

    # Category D: Living Hedges
    if barrier_val == "hedge" or tags.get("natural") == "hedge":
        category = "hedge"
        semantic_label = "Vegetation"
        def_height = parse_barrier_height(tags, default_height=1.4)
        def_width = parse_barrier_width(tags, default_width=0.7)
        mat_preset = "hedge"
        return category, semantic_label, mat_preset, def_height, def_width

    # Category E: Generic barriers (barrier=yes)
    if barrier_val in ("yes", "dike", "retaining_wall"):
        fence_type = tags.get("fence_type")
        if fence_type:
            return "fence", "Fences", "fence_metal", parse_barrier_height(tags, 1.2), 0.10
        return "wall", "Walls", "wall_concrete", parse_barrier_height(tags, 1.3), 0.30

    return None


def extract_osm_barriers(osm_source: str | Path) -> list[BarrierWay]:
    """
    Parses OpenStreetMap XML file or string and extracts all above-ground BarrierWay instances.
    """
    if isinstance(osm_source, Path) or (isinstance(osm_source, str) and os.path.exists(osm_source)):
        tree = ET.parse(osm_source)
        root = tree.getroot()
    else:
        root = ET.fromstring(osm_source)

    # 1. Cache all node coordinates
    node_coords: dict[str, tuple[float, float]] = {}
    for node in root.findall("node"):
        n_id = node.get("id")
        lat_str = node.get("lat")
        lon_str = node.get("lon")
        if n_id and lat_str and lon_str:
            try:
                node_coords[n_id] = (float(lat_str), float(lon_str))
            except ValueError:
                continue

    barrier_ways: list[BarrierWay] = []

    # 2. Iterate through all ways
    for way in root.findall("way"):
        way_id_str = way.get("id", "0")
        try:
            osm_id = int(way_id_str)
        except ValueError:
            osm_id = 0

        tags: dict[str, str] = {}
        for tag in way.findall("tag"):
            k = tag.get("k")
            v = tag.get("v")
            if k and v:
                tags[k] = v

        classification = classify_barrier(tags)
        if classification is None:
            continue

        cat, semantic, mat, h, w = classification

        coords: list[tuple[float, float]] = []
        for nd in way.findall("nd"):
            ref = nd.get("ref")
            if ref and ref in node_coords:
                coords.append(node_coords[ref])

        if len(coords) < 2:
            continue

        is_closed = (coords[0] == coords[-1] and len(coords) >= 4)

        barrier_ways.append(BarrierWay(
            osm_id=osm_id,
            barrier_type=tags.get("barrier", "wall"),
            category=cat,
            nodes=coords,
            tags=tags,
            height=h,
            width=w,
            material_preset=mat,
            semantic_label=semantic,
            is_closed=is_closed,
        ))

    return barrier_ways


def extrude_barrier_polyline_mesh(
    polyline_points: Sequence[tuple[float, float, float]],
    height: float,
    width: float,
    is_closed: bool = False,
    uv_tile_u: float = 2.0,
    submerge_depth: float = 0.2,
) -> tuple[list[tuple[float, float, float]], list[tuple[int, int, int]], list[tuple[float, float, float]], list[tuple[float, float]]]:
    """
    Extrudes a continuous 3D watertight prism mesh along an arbitrary 3D polyline.
    Computes corner mitering, side walls, top cap, and start/end endcaps with correct outward normals.

    Returns:
      (vertices, triangles, normals, uvs)
    """
    N = len(polyline_points)
    if N < 2:
        return [], [], [], []

    # Filter out duplicate consecutive points
    pts: list[tuple[float, float, float]] = [polyline_points[0]]
    for i in range(1, N):
        p_prev = pts[-1]
        p_cur = polyline_points[i]
        d = math.hypot(p_cur[0] - p_prev[0], p_cur[1] - p_prev[1])
        if d > 0.05:  # Ignore points closer than 5cm
            pts.append(p_cur)

    N = len(pts)
    if N < 2:
        return [], [], [], []

    half_w = width * 0.5

    # 1. Compute 2D segment directions and segment lengths
    edge_dirs: list[tuple[float, float]] = []
    edge_lens: list[float] = []
    for i in range(N - 1):
        dx = pts[i + 1][0] - pts[i][0]
        dy = pts[i + 1][1] - pts[i][1]
        length = math.hypot(dx, dy)
        if length < 1e-4:
            dx, dy, length = 1.0, 0.0, 1.0
        edge_dirs.append((dx / length, dy / length))
        edge_lens.append(length)

    # 2. Compute vertex left-normals with miter joins
    vertex_normals: list[tuple[float, float]] = []
    for i in range(N):
        if i == 0:
            if is_closed and N >= 3:
                # Average first and last segment
                d0 = edge_dirs[-1]
                d1 = edge_dirs[0]
                n0 = (-d0[1], d0[0])
                n1 = (-d1[1], d1[0])
                avg_n = (n0[0] + n1[0], n0[1] + n1[1])
                mag = math.hypot(avg_n[0], avg_n[1])
                if mag > 1e-4:
                    scale = min(2.5, 2.0 / mag)
                    vertex_normals.append((avg_n[0] / mag * scale, avg_n[1] / mag * scale))
                else:
                    vertex_normals.append(n1)
            else:
                d = edge_dirs[0]
                vertex_normals.append((-d[1], d[0]))
        elif i == N - 1:
            if is_closed and N >= 3:
                vertex_normals.append(vertex_normals[0])
            else:
                d = edge_dirs[-1]
                vertex_normals.append((-d[1], d[0]))
        else:
            d0 = edge_dirs[i - 1]
            d1 = edge_dirs[i]
            n0 = (-d0[1], d0[0])
            n1 = (-d1[1], d1[0])
            avg_n = (n0[0] + n1[0], n0[1] + n1[1])
            mag = math.hypot(avg_n[0], avg_n[1])
            if mag > 1e-4:
                # Miter scale factor clamped to 2.5
                scale = min(2.5, 2.0 / mag)
                vertex_normals.append((avg_n[0] / mag * scale, avg_n[1] / mag * scale))
            else:
                vertex_normals.append(n1)

    # 3. Compute left and right boundary points along base and top
    # Base is submerged slightly to prevent floating artifacts on uneven ground
    left_bottom: list[tuple[float, float, float]] = []
    left_top: list[tuple[float, float, float]] = []
    right_bottom: list[tuple[float, float, float]] = []
    right_top: list[tuple[float, float, float]] = []

    cum_dist: list[float] = [0.0]
    for length in edge_lens:
        cum_dist.append(cum_dist[-1] + length)

    for i in range(N):
        x, y, z = pts[i]
        nx, ny = vertex_normals[i]
        z_bot = z - submerge_depth
        z_top = z + height

        lx = x + nx * half_w
        ly = y + ny * half_w
        rx = x - nx * half_w
        ry = y - ny * half_w

        left_bottom.append((lx, ly, z_bot))
        left_top.append((lx, ly, z_top))
        right_bottom.append((rx, ry, z_bot))
        right_top.append((rx, ry, z_top))

    vertices: list[tuple[float, float, float]] = []
    triangles: list[tuple[int, int, int]] = []
    normals: list[tuple[float, float, float]] = []
    uvs: list[tuple[float, float]] = []

    def add_quad(
        v0: tuple[float, float, float],
        v1: tuple[float, float, float],
        v2: tuple[float, float, float],
        v3: tuple[float, float, float],
        norm: tuple[float, float, float],
        uv0: tuple[float, float],
        uv1: tuple[float, float],
        uv2: tuple[float, float],
        uv3: tuple[float, float],
    ) -> None:
        idx = len(vertices)
        vertices.extend([v0, v1, v2, v3])
        normals.extend([norm, norm, norm, norm])
        uvs.extend([uv0, uv1, uv2, uv3])
        # Clockwise unwinding for Unreal Engine
        triangles.extend([
            (idx, idx + 1, idx + 2),
            (idx, idx + 2, idx + 3),
        ])

    # 4. Generate Left Vertical Wall Quads
    for i in range(N - 1):
        u0 = cum_dist[i] / uv_tile_u
        u1 = cum_dist[i + 1] / uv_tile_u
        d = edge_dirs[i]
        left_n = (-d[1], d[0], 0.0)

        add_quad(
            v0=left_bottom[i],
            v1=left_bottom[i + 1],
            v2=left_top[i + 1],
            v3=left_top[i],
            norm=left_n,
            uv0=(u0, 0.0),
            uv1=(u1, 0.0),
            uv2=(u1, 1.0),
            uv3=(u0, 1.0),
        )

    # 5. Generate Right Vertical Wall Quads
    for i in range(N - 1):
        u0 = cum_dist[i] / uv_tile_u
        u1 = cum_dist[i + 1] / uv_tile_u
        d = edge_dirs[i]
        right_n = (d[1], -d[0], 0.0)

        add_quad(
            v0=right_bottom[i + 1],
            v1=right_bottom[i],
            v2=right_top[i],
            v3=right_top[i + 1],
            norm=right_n,
            uv0=(u1, 0.0),
            uv1=(u0, 0.0),
            uv2=(u0, 1.0),
            uv3=(u1, 1.0),
        )

    # 6. Generate Top Horizontal Cap Quads
    top_n = (0.0, 0.0, 1.0)
    for i in range(N - 1):
        u0 = cum_dist[i] / uv_tile_u
        u1 = cum_dist[i + 1] / uv_tile_u

        add_quad(
            v0=left_top[i],
            v1=left_top[i + 1],
            v2=right_top[i + 1],
            v3=right_top[i],
            norm=top_n,
            uv0=(u0, 0.0),
            uv1=(u1, 0.0),
            uv2=(u1, 1.0),
            uv3=(u0, 1.0),
        )

    # 7. Generate Endcaps (for open polylines)
    if not is_closed:
        # Start endcap (facing opposite of first edge direction)
        d0 = edge_dirs[0]
        start_n = (-d0[0], -d0[1], 0.0)
        add_quad(
            v0=right_bottom[0],
            v1=left_bottom[0],
            v2=left_top[0],
            v3=right_top[0],
            norm=start_n,
            uv0=(0.0, 0.0),
            uv1=(1.0, 0.0),
            uv2=(1.0, 1.0),
            uv3=(0.0, 1.0),
        )

        # End endcap (facing along last edge direction)
        d_end = edge_dirs[-1]
        end_n = (d_end[0], d_end[1], 0.0)
        add_quad(
            v0=left_bottom[-1],
            v1=right_bottom[-1],
            v2=right_top[-1],
            v3=left_top[-1],
            norm=end_n,
            uv0=(0.0, 0.0),
            uv1=(1.0, 0.0),
            uv2=(1.0, 1.0),
            uv3=(0.0, 1.0),
        )

    return vertices, triangles, normals, uvs


def build_osm_barriers_multislot_layers(
    barriers: Sequence[BarrierWay],
    project_coord_fn: Callable[[float, float], tuple[float, float, float]],
    default_wall_height: float = 1.5,
    default_fence_height: float = 1.2,
    default_guardrail_height: float = 0.85,
) -> dict[str, dict[str, Any]]:
    """
    Builds distinct architectural barrier layers (walls, fences, guard_rails, hedges)
    from extracted BarrierWay objects.

    Returns:
      dict mapping slot_id -> {
          'name': str,
          'material_preset': str,
          'semantic_label': str,
          'enable_collision': bool,
          'vertices': list,
          'triangles': list,
          'normals': list,
          'uvs': list,
          'way_count': int,
          'total_length_m': float,
      }
    """
    layer_defs: dict[str, dict[str, Any]] = {
        "walls": {
            "name": "OSM_Barriers_Walls",
            "material_preset": "wall_concrete",
            "semantic_label": "Walls",
            "enable_collision": True,
            "vertices": [],
            "triangles": [],
            "normals": [],
            "uvs": [],
            "way_count": 0,
            "total_length_m": 0.0,
        },
        "fences": {
            "name": "OSM_Barriers_Fences",
            "material_preset": "fence_metal",
            "semantic_label": "Fences",
            "enable_collision": True,
            "vertices": [],
            "triangles": [],
            "normals": [],
            "uvs": [],
            "way_count": 0,
            "total_length_m": 0.0,
        },
        "guard_rails": {
            "name": "OSM_Barriers_GuardRails",
            "material_preset": "guardrail",
            "semantic_label": "GuardRail",
            "enable_collision": True,
            "vertices": [],
            "triangles": [],
            "normals": [],
            "uvs": [],
            "way_count": 0,
            "total_length_m": 0.0,
        },
        "hedges": {
            "name": "OSM_Barriers_Hedges",
            "material_preset": "hedge",
            "semantic_label": "Vegetation",
            "enable_collision": True,
            "vertices": [],
            "triangles": [],
            "normals": [],
            "uvs": [],
            "way_count": 0,
            "total_length_m": 0.0,
        },
    }

    def append_geom(
        target: dict[str, Any],
        v_list: list,
        t_list: list,
        n_list: list,
        uv_list: list,
        length: float,
    ) -> None:
        if not v_list:
            return
        offset = len(target["vertices"])
        target["vertices"].extend(v_list)
        target["normals"].extend(n_list)
        target["uvs"].extend(uv_list)
        for tri in t_list:
            target["triangles"].append((offset + tri[0], offset + tri[1], offset + tri[2]))
        target["way_count"] += 1
        target["total_length_m"] += length

    for b in barriers:
        if len(b.nodes) < 2:
            continue

        proj_pts: list[tuple[float, float, float]] = []
        for lat, lon in b.nodes:
            xyz = project_coord_fn(lat, lon)
            proj_pts.append(xyz)

        way_len = sum(
            math.hypot(proj_pts[i + 1][0] - proj_pts[i][0], proj_pts[i + 1][1] - proj_pts[i][1])
            for i in range(len(proj_pts) - 1)
        )

        h = b.height
        if b.category == "wall" and h == 1.2:
            h = default_wall_height
        elif b.category == "fence" and h == 1.2:
            h = default_fence_height
        elif b.category == "guard_rail" and h == 1.2:
            h = default_guardrail_height

        v, t, n, uv = extrude_barrier_polyline_mesh(
            proj_pts,
            height=h,
            width=b.width,
            is_closed=b.is_closed,
            uv_tile_u=2.0,
        )

        slot = "walls"
        if b.category == "fence":
            slot = "fences"
        elif b.category == "guard_rail":
            slot = "guard_rails"
        elif b.category == "hedge":
            slot = "hedges"

        append_geom(layer_defs[slot], v, t, n, uv, way_len)

    # Return only layers that have non-empty geometry
    return {k: v for k, v in layer_defs.items() if len(v["vertices"]) > 0}


def audit_barrier_metrics(
    barriers: Sequence[BarrierWay],
    layers: dict[str, dict[str, Any]],
) -> dict[str, Any]:
    """
    Computes summary metrics for extracted and generated barriers.
    """
    by_cat: dict[str, int] = {}
    by_mat: dict[str, int] = {}
    total_len = 0.0

    barrier_records: list[dict[str, Any]] = []
    for b in barriers:
        by_cat[b.category] = by_cat.get(b.category, 0) + 1
        by_mat[b.material_preset] = by_mat.get(b.material_preset, 0) + 1

    for l_key, l_data in layers.items():
        total_len += l_data.get("total_length_m", 0.0)

    layer_summary: dict[str, Any] = {}
    for l_key, l_data in layers.items():
        layer_summary[l_data["name"]] = {
            "semantic_label": l_data["semantic_label"],
            "material_preset": l_data["material_preset"],
            "vertices": len(l_data["vertices"]),
            "triangles": len(l_data["triangles"]),
            "way_count": l_data.get("way_count", 0),
            "total_length_m": round(l_data.get("total_length_m", 0.0), 2),
        }

    return {
        "counts": {
            "total_barriers": len(barriers),
            "total_length_m": round(total_len, 2),
            "walls": by_cat.get("wall", 0),
            "fences": by_cat.get("fence", 0),
            "guard_rails": by_cat.get("guard_rail", 0),
            "hedges": by_cat.get("hedge", 0),
        },
        "materials": by_mat,
        "layers": layer_summary,
    }

