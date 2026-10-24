#!/usr/bin/env python3
"""
OpenStreetMap (OSM) Urban Terrain, Landuse Ground & Sub-Base Generator for CARLA Map Maker.

Parses OSM polygons and multipolygon relations for:
  - Green spaces, parks, lawns, flowerbeds (landuse=grass, leisure=park, natural=wood)
  - Water basins, canals, fountains (natural=water, waterway=*, amenity=fountain)
  - Urban pedestrian plazas, squares, traffic islands (highway=pedestrian, place=square)
  - Sub-base ground plane spanning the entire map bounding box

Triangulates 2D polygons with support for inner holes, downward perimeter foundation
skirts (preventing light leaks and edge abysses), seamless UV coordinates, and ground-truth
CityObjectLabel semantic tags.
"""

from __future__ import annotations

import math
from pathlib import Path
import re
from typing import Any, Callable, Sequence
import xml.etree.ElementTree as ET

import numpy as np

try:
    import matplotlib.tri as mtri
    HAS_MATPLOTLIB = True
except ImportError:
    HAS_MATPLOTLIB = False

try:
    from Util.MapBuilder.osm_buildings import triangulate_polygon_2d
except ImportError:
    from osm_buildings import triangulate_polygon_2d


# =============================================================================
# Category Definitions & Material Mappings
# =============================================================================

TERRAIN_CATEGORIES = {
    "grass": {
        "material_preset": "grass",
        "semantic_label": "Vegetation",
        "default_z_offset": -0.02,
        "skirt_depth": 0.6,
        "uv_scale": 8.0,
    },
    "water": {
        "material_preset": "water",
        "semantic_label": "Water",
        "default_z_offset": -0.15,
        "skirt_depth": 0.5,
        "uv_scale": 10.0,
    },
    "plaza": {
        "material_preset": "sidewalk_tiles",
        "semantic_label": "Ground",
        "default_z_offset": 0.005,
        "skirt_depth": 0.4,
        "uv_scale": 6.0,
    },
    "subbase": {
        "material_preset": "dirt",
        "semantic_label": "Terrain",
        "default_z_offset": -0.10,
        "skirt_depth": 1.0,
        "uv_scale": 12.0,
    },
}


def classify_terrain_tags(tags: dict[str, str]) -> str | None:
    """Classifies an OSM feature's tags into a terrain category (grass, water, plaza), or None."""
    # 1. Water features
    if tags.get("natural") in ("water", "wetland"):
        return "water"
    if "waterway" in tags or tags.get("water") in ("lake", "pond", "basin", "fountain"):
        return "water"
    if tags.get("amenity") == "fountain":
        return "water"

    # 2. Vegetation / Grass / Parks
    if tags.get("landuse") in ("grass", "flowerbed", "meadow", "forest", "village_green", "recreation_ground"):
        return "grass"
    if tags.get("leisure") in ("park", "garden", "pitch", "playground"):
        return "grass"
    if tags.get("natural") in ("wood", "grassland", "scrub", "tree_row"):
        return "grass"

    # 3. Plazas / Pedestrian Squares / Paved urban grounds
    if tags.get("place") == "square":
        return "plaza"
    if tags.get("highway") in ("pedestrian", "footway") and tags.get("area") == "yes":
        return "plaza"
    if tags.get("landuse") in ("traffic_island", "commercial", "retail", "plaza"):
        return "plaza"
    if tags.get("amenity") in ("parking", "marketplace"):
        return "plaza"

    return None


# =============================================================================
# OSM Terrain Polygon Extraction
# =============================================================================

class TerrainPolygon:
    """Represents a classified 2D terrain polygon with outer perimeter and optional inner holes."""

    def __init__(
        self,
        osm_id: str,
        category: str,
        tags: dict[str, str],
        outer_nodes: list[tuple[float, float]],
        inner_holes: list[list[tuple[float, float]]] | None = None,
    ):
        self.osm_id = osm_id
        self.category = category
        self.tags = tags
        self.outer_nodes = outer_nodes  # list of (lat, lon)
        self.inner_holes = inner_holes or []  # list of lists of (lat, lon)


def assemble_way_rings(
    member_way_ids: list[str],
    ways_dict: dict[str, list[str]],
    nodes_dict: dict[str, tuple[float, float]],
) -> list[list[tuple[float, float]]]:
    """Assembles unordered way member references into continuous closed coordinate loops."""
    segments = [
        [nodes_dict[ref] for ref in ways_dict[wid] if ref in nodes_dict]
        for wid in member_way_ids
        if wid in ways_dict and len(ways_dict[wid]) >= 2
    ]
    segments = [s for s in segments if len(s) >= 2]

    rings: list[list[tuple[float, float]]] = []
    while segments:
        current = segments.pop(0)
        extended = True
        while extended and len(current) >= 2:
            if current[0] == current[-1]:
                break
            extended = False
            for i, seg in enumerate(segments):
                if seg[0] == current[-1]:
                    current.extend(seg[1:])
                    segments.pop(i)
                    extended = True
                    break
                elif seg[-1] == current[-1]:
                    current.extend(reversed(seg[:-1]))
                    segments.pop(i)
                    extended = True
                    break
                elif seg[-1] == current[0]:
                    current = seg[:-1] + current
                    segments.pop(i)
                    extended = True
                    break
                elif seg[0] == current[0]:
                    current = list(reversed(seg[1:])) + current
                    segments.pop(i)
                    extended = True
                    break

        if len(current) >= 4 and current[0] == current[-1]:
            current.pop()  # remove duplicate closing point
        if len(current) >= 3:
            rings.append(current)

    return rings


def extract_osm_terrain_polygons(osm_xml_or_path: str | Path) -> list[TerrainPolygon]:
    """Parses OSM XML and extracts all landuse, park, water, and plaza terrain polygons."""
    if isinstance(osm_xml_or_path, Path) or (
        isinstance(osm_xml_or_path, str) and not osm_xml_or_path.strip().startswith("<")
    ):
        root = ET.parse(str(osm_xml_or_path)).getroot()
    else:
        root = ET.fromstring(osm_xml_or_path)

    # 1. Collect all node coordinates: id -> (lat, lon)
    nodes: dict[str, tuple[float, float]] = {}
    for node in root.findall("node"):
        node_id = node.get("id")
        lat = node.get("lat")
        lon = node.get("lon")
        if node_id and lat and lon:
            nodes[node_id] = (float(lat), float(lon))

    # 2. Collect ways
    ways: dict[str, list[str]] = {}
    way_tags_dict: dict[str, dict[str, str]] = {}
    for way in root.findall("way"):
        wid = way.get("id")
        if not wid:
            continue
        nd_refs = [nd.get("ref", "") for nd in way.findall("nd") if nd.get("ref", "") in nodes]
        ways[wid] = nd_refs
        tags = {t.get("k", ""): t.get("v", "") for t in way.findall("tag")}
        way_tags_dict[wid] = tags

    terrain_polygons: list[TerrainPolygon] = []

    # 3. Collect multipolygon relations first
    handled_way_ids: set[str] = set()
    for rel in root.findall("relation"):
        tags = {t.get("k", ""): t.get("v", "") for t in rel.findall("tag")}
        cat = classify_terrain_tags(tags)
        if not cat:
            continue

        outers = [
            m.get("ref", "")
            for m in rel.findall("member")
            if m.get("type") == "way" and m.get("role") in ("outer", "")
        ]
        inners = [
            m.get("ref", "")
            for m in rel.findall("member")
            if m.get("type") == "way" and m.get("role") == "inner"
        ]

        outer_rings = assemble_way_rings(outers, ways, nodes)
        inner_rings = assemble_way_rings(inners, ways, nodes)

        for ring in outer_rings:
            terrain_polygons.append(TerrainPolygon(rel.get("id", ""), cat, tags, ring, inner_rings))

        handled_way_ids.update(outers)

    # 4. Collect standalone closed ways
    for wid, nd_refs in ways.items():
        if wid in handled_way_ids:
            continue
        if len(nd_refs) < 4 or nd_refs[0] != nd_refs[-1]:
            continue

        tags = way_tags_dict.get(wid, {})
        # Skip buildings here as they are handled by building extrusion
        if "building" in tags:
            continue

        cat = classify_terrain_tags(tags)
        if not cat:
            continue

        coords = [nodes[ref] for ref in nd_refs[:-1]]
        if len(coords) >= 3:
            terrain_polygons.append(TerrainPolygon(wid, cat, tags, coords))

    return terrain_polygons


# =============================================================================
# Geometric Triangulation & Foundation Skirts
# =============================================================================

def point_in_polygon_2d(p: tuple[float, float], poly: Sequence[tuple[float, float]]) -> bool:
    """Ray casting point-in-polygon algorithm."""
    x, y = p
    n = len(poly)
    inside = False
    p1x, p1y = poly[0]
    for i in range(n + 1):
        p2x, p2y = poly[i % n]
        if y > min(p1y, p2y) and y <= max(p1y, p2y):
            if x <= max(p1x, p2x):
                if p1y != p2y:
                    xinters = (y - p1y) * (p2x - p1x) / (p2y - p1y) + p1x
                if p1x == p2x or x <= xinters:
                    inside = not inside
        p1x, p1y = p2x, p2y
    return inside


def triangulate_terrain_polygon(
    outer_pts: Sequence[tuple[float, float]],
    inner_holes: Sequence[Sequence[tuple[float, float]]] = (),
    base_z: float = 0.0,
    skirt_depth: float = 0.6,
    uv_scale: float = 10.0,
) -> tuple[
    list[tuple[float, float, float]],
    list[tuple[int, int, int]],
    list[tuple[float, float, float]],
    list[tuple[float, float]],
]:
    """
    Triangulates a 2D terrain polygon with inner holes and generates perimeter downward foundation skirts.
    Returns (vertices, triangles, normals, uvs).
    """
    n = len(outer_pts)
    if n < 3:
        return [], [], [], []

    # Ensure outer polygon is CCW (positive signed area)
    area = 0.0
    for i in range(n):
        j = (i + 1) % n
        area += outer_pts[i][0] * outer_pts[j][1] - outer_pts[j][0] * outer_pts[i][1]
    if area < 0:
        outer_pts = list(reversed(outer_pts))

    vertices: list[tuple[float, float, float]] = []
    triangles: list[tuple[int, int, int]] = []
    normals: list[tuple[float, float, float]] = []
    uvs: list[tuple[float, float]] = []

    # -------------------------------------------------------------------------
    # 1. Top Surface Triangulation
    # -------------------------------------------------------------------------
    all_pts_2d: list[tuple[float, float]] = list(outer_pts)
    hole_ranges: list[list[tuple[float, float]]] = []

    for hole in inner_holes:
        if len(hole) >= 3:
            hole_pts = list(hole)
            hole_ranges.append(hole_pts)
            all_pts_2d.extend(hole_pts)

    top_base_idx = len(vertices)

    # Use Delaunay triangulation if holes are present or matplotlib is available
    triangulation_success = False
    if hole_ranges and HAS_MATPLOTLIB and len(all_pts_2d) >= 3:
        try:
            x_arr = np.array([p[0] for p in all_pts_2d], dtype=np.float64)
            y_arr = np.array([p[1] for p in all_pts_2d], dtype=np.float64)
            delaunay = mtri.Triangulation(x_arr, y_arr)

            for p in all_pts_2d:
                vertices.append((p[0], p[1], base_z))
                normals.append((0.0, 0.0, 1.0))
                uvs.append((p[0] / uv_scale, p[1] / uv_scale))

            for t in delaunay.triangles:
                i0, i1, i2 = int(t[0]), int(t[1]), int(t[2])
                cx = (all_pts_2d[i0][0] + all_pts_2d[i1][0] + all_pts_2d[i2][0]) / 3.0
                cy = (all_pts_2d[i0][1] + all_pts_2d[i1][1] + all_pts_2d[i2][1]) / 3.0

                # Must be inside outer polygon and outside all holes
                if not point_in_polygon_2d((cx, cy), outer_pts):
                    continue
                in_any_hole = any(point_in_polygon_2d((cx, cy), h) for h in hole_ranges)
                if in_any_hole:
                    continue

                # Ensure Unreal Engine clockwise skyward face
                cross = (all_pts_2d[i1][0] - all_pts_2d[i0][0]) * (all_pts_2d[i2][1] - all_pts_2d[i0][1]) - (
                    all_pts_2d[i1][1] - all_pts_2d[i0][1]
                ) * (all_pts_2d[i2][0] - all_pts_2d[i0][0])
                if cross > 0:
                    triangles.append((top_base_idx + i0, top_base_idx + i2, top_base_idx + i1))
                else:
                    triangles.append((top_base_idx + i0, top_base_idx + i1, top_base_idx + i2))

            triangulation_success = True
        except Exception:
            triangulation_success = False

    if not triangulation_success:
        # Fallback to simple ear-clipping without holes
        tris_2d = triangulate_polygon_2d(outer_pts)
        for p in outer_pts:
            vertices.append((p[0], p[1], base_z))
            normals.append((0.0, 0.0, 1.0))
            uvs.append((p[0] / uv_scale, p[1] / uv_scale))

        for tri in tris_2d:
            triangles.append((top_base_idx + tri[0], top_base_idx + tri[2], top_base_idx + tri[1]))

    # -------------------------------------------------------------------------
    # 2. Perimeter Downward Foundation Skirt
    # -------------------------------------------------------------------------
    if skirt_depth > 0.01:
        u_cum = 0.0
        for i in range(n):
            j = (i + 1) % n
            p0 = outer_pts[i]
            p1 = outer_pts[j]

            dx = p1[0] - p0[0]
            dy = p1[1] - p0[1]
            seg_len = math.hypot(dx, dy)
            if seg_len < 1e-4:
                continue

            # Outward normal
            nx = dy / seg_len
            ny = -dx / seg_len

            v_skirt = len(vertices)
            # 4 vertices for vertical quad
            v_tl = (p0[0], p0[1], base_z)
            v_tr = (p1[0], p1[1], base_z)
            v_br = (p1[0], p1[1], base_z - skirt_depth)
            v_bl = (p0[0], p0[1], base_z - skirt_depth)

            vertices.extend([v_tl, v_tr, v_br, v_bl])
            normals.extend([(nx, ny, 0.0)] * 4)

            u0 = u_cum / uv_scale
            u1 = (u_cum + seg_len) / uv_scale
            uvs.extend([(u0, 0.0), (u1, 0.0), (u1, skirt_depth / uv_scale), (u0, skirt_depth / uv_scale)])
            u_cum += seg_len

            # UE clockwise outward faces
            triangles.append((v_skirt, v_skirt + 1, v_skirt + 2))
            triangles.append((v_skirt, v_skirt + 2, v_skirt + 3))

    return vertices, triangles, normals, uvs


# =============================================================================
# Continuous Sub-Base Ground Plane
# =============================================================================

def generate_subbase_grid(
    min_x: float,
    max_x: float,
    min_y: float,
    max_y: float,
    base_z: float = -0.10,
    margin: float = 50.0,
    cell_size: float = 25.0,
    uv_scale: float = 12.0,
) -> tuple[
    list[tuple[float, float, float]],
    list[tuple[int, int, int]],
    list[tuple[float, float, float]],
    list[tuple[float, float]],
]:
    """
    Generates a regular 2D grid sub-base ground mesh covering the map extent plus margin.
    Guarantees 100% surface coverage across the map bounding box.
    """
    x0 = min_x - margin
    x1 = max_x + margin
    y0 = min_y - margin
    y1 = max_y + margin

    nx = max(2, int(math.ceil((x1 - x0) / cell_size)) + 1)
    ny = max(2, int(math.ceil((y1 - y0) / cell_size)) + 1)

    xs = np.linspace(x0, x1, nx)
    ys = np.linspace(y0, y1, ny)

    vertices: list[tuple[float, float, float]] = []
    triangles: list[tuple[int, int, int]] = []
    normals: list[tuple[float, float, float]] = []
    uvs: list[tuple[float, float]] = []

    for iy in range(ny):
        for ix in range(nx):
            x_val = float(xs[ix])
            y_val = float(ys[iy])
            vertices.append((x_val, y_val, base_z))
            normals.append((0.0, 0.0, 1.0))
            uvs.append((x_val / uv_scale, y_val / uv_scale))

    for iy in range(ny - 1):
        for ix in range(nx - 1):
            i0 = iy * nx + ix
            i1 = iy * nx + (ix + 1)
            i2 = (iy + 1) * nx + (ix + 1)
            i3 = (iy + 1) * nx + ix

            # Clockwise UE skyward faces
            triangles.append((i0, i2, i1))
            triangles.append((i0, i3, i2))

    return vertices, triangles, normals, uvs


# =============================================================================
# Build Pipeline: Grouped Layers Generation
# =============================================================================

def build_osm_terrain_layers(
    polygons: Sequence[TerrainPolygon],
    project_coord_fn: Callable[[float, float], tuple[float, float, float]],
    map_bounds: tuple[float, float, float, float] | None = None,
    include_subbase: bool = True,
    subbase_margin: float = 50.0,
) -> dict[str, dict[str, Any]]:
    """
    Builds categorized 3D terrain meshes (grass, water, plaza, subbase).
    Returns a dictionary mapping layer category name to mesh geometry and metadata:
      category -> {"vertices": [...], "triangles": [...], "normals": [...], "uvs": [...], "config": {...}}
    """
    categorized_polys: dict[str, list[TerrainPolygon]] = {
        "grass": [],
        "water": [],
        "plaza": [],
    }

    for poly in polygons:
        cat = poly.category
        if cat in categorized_polys:
            categorized_polys[cat].append(poly)

    all_x: list[float] = []
    all_y: list[float] = []
    output_layers: dict[str, dict[str, Any]] = {}

    for cat, polys in categorized_polys.items():
        if not polys:
            continue

        cfg = TERRAIN_CATEGORIES[cat]
        z_offset = cfg["default_z_offset"]
        skirt_depth = cfg["skirt_depth"]
        uv_scale = cfg["uv_scale"]

        layer_verts: list[tuple[float, float, float]] = []
        layer_tris: list[tuple[int, int, int]] = []
        layer_norms: list[tuple[float, float, float]] = []
        layer_uvs: list[tuple[float, float]] = []

        for poly in polys:
            # Project outer nodes
            outer_projected: list[tuple[float, float]] = []
            base_z = 0.0
            for lat, lon in poly.outer_nodes:
                xyz = project_coord_fn(lat, lon)
                outer_projected.append((xyz[0], xyz[1]))
                base_z = xyz[2]
                all_x.append(xyz[0])
                all_y.append(xyz[1])

            # Project inner holes
            inner_projected: list[list[tuple[float, float]]] = []
            for hole in poly.inner_holes:
                h_proj: list[tuple[float, float]] = []
                for lat, lon in hole:
                    xyz = project_coord_fn(lat, lon)
                    h_proj.append((xyz[0], xyz[1]))
                    all_x.append(xyz[0])
                    all_y.append(xyz[1])
                if len(h_proj) >= 3:
                    inner_projected.append(h_proj)

            verts, tris, norms, uvs = triangulate_terrain_polygon(
                outer_projected,
                inner_holes=inner_projected,
                base_z=base_z + z_offset,
                skirt_depth=skirt_depth,
                uv_scale=uv_scale,
            )

            if not verts:
                continue

            v_offset = len(layer_verts)
            layer_verts.extend(verts)
            layer_norms.extend(norms)
            layer_uvs.extend(uvs)
            for tri in tris:
                layer_tris.append((v_offset + tri[0], v_offset + tri[1], v_offset + tri[2]))

        if layer_verts:
            output_layers[cat] = {
                "name": f"OSM_Terrain_{cat.capitalize()}",
                "vertices": layer_verts,
                "triangles": layer_tris,
                "normals": layer_norms,
                "uvs": layer_uvs,
                "material_preset": cfg["material_preset"],
                "semantic_label": cfg["semantic_label"],
                "enable_collision": True,
            }

    # Generate continuous Sub-Base Ground Plane
    if include_subbase:
        if map_bounds is not None:
            min_x, max_x, min_y, max_y = map_bounds
        elif all_x and all_y:
            min_x, max_x = min(all_x), max(all_x)
            min_y, max_y = min(all_y), max(all_y)
        else:
            min_x, max_x = -200.0, 200.0
            min_y, max_y = -200.0, 200.0

        sb_cfg = TERRAIN_CATEGORIES["subbase"]
        sb_verts, sb_tris, sb_norms, sb_uvs = generate_subbase_grid(
            min_x=min_x,
            max_x=max_x,
            min_y=min_y,
            max_y=max_y,
            base_z=sb_cfg["default_z_offset"],
            margin=subbase_margin,
            cell_size=25.0,
            uv_scale=sb_cfg["uv_scale"],
        )

        output_layers["subbase"] = {
            "name": "OSM_Terrain_Subbase",
            "vertices": sb_verts,
            "triangles": sb_tris,
            "normals": sb_norms,
            "uvs": sb_uvs,
            "material_preset": sb_cfg["material_preset"],
            "semantic_label": sb_cfg["semantic_label"],
            "enable_collision": True,
        }

    return output_layers
