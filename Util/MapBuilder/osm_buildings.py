#!/usr/bin/env python3
"""
OpenStreetMap (OSM) 3D Building Extrusion Engine for CARLA Map Maker.

Extracts building footprints from OSM XML, projects geographic coordinates
(lat, lon) into CARLA Cartesian coordinates (meters) matching the map's
OpenDRIVE projection, and extrudes watertight 3D wall and roof meshes with
UV coordinates and ground-truth semantic labels (CityObjectLabel.Buildings).
"""

from __future__ import annotations

import math
from pathlib import Path
import re
from typing import Any, Callable, Sequence
import xml.etree.ElementTree as ET


def triangulate_polygon_2d(pts: Sequence[tuple[float, float]]) -> list[tuple[int, int, int]]:
    """
    Ear-clipping triangulation algorithm for arbitrary simple 2D polygons (including non-convex).
    Returns a list of 0-based vertex index triples [(i0, i1, i2), ...].
    """
    n = len(pts)
    if n < 3:
        return []
    if n == 3:
        return [(0, 1, 2)]

    # Compute signed area to determine vertex winding order (positive = counter-clockwise)
    area = 0.0
    for i in range(n):
        j = (i + 1) % n
        area += pts[i][0] * pts[j][1] - pts[j][0] * pts[i][1]

    # Maintain vertex indices; ensure counter-clockwise orientation
    indices = list(range(n))
    if area < 0:
        indices.reverse()

    def is_convex(i0: int, i1: int, i2: int) -> bool:
        p0, p1, p2 = pts[i0], pts[i1], pts[i2]
        cross = (p1[0] - p0[0]) * (p2[1] - p0[1]) - (p1[1] - p0[1]) * (p2[0] - p0[0])
        return cross > 1e-7

    def point_in_triangle(p: tuple[float, float], p0: tuple[float, float], p1: tuple[float, float], p2: tuple[float, float]) -> bool:
        def sign(a: tuple[float, float], b: tuple[float, float], c: tuple[float, float]) -> float:
            return (a[0] - c[0]) * (b[1] - c[1]) - (b[0] - c[0]) * (a[1] - c[1])

        d1 = sign(p, p0, p1)
        d2 = sign(p, p1, p2)
        d3 = sign(p, p2, p0)

        has_neg = (d1 < -1e-7) or (d2 < -1e-7) or (d3 < -1e-7)
        has_pos = (d1 > 1e-7) or (d2 > 1e-7) or (d3 > 1e-7)
        return not (has_neg and has_pos)

    triangles: list[tuple[int, int, int]] = []
    max_iterations = len(indices) * 4
    iters = 0

    while len(indices) > 2 and iters < max_iterations:
        iters += 1
        m = len(indices)
        ear_found = False

        for k in range(m):
            i0 = indices[(k - 1) % m]
            i1 = indices[k]
            i2 = indices[(k + 1) % m]

            if not is_convex(i0, i1, i2):
                continue

            p0, p1, p2 = pts[i0], pts[i1], pts[i2]
            any_inside = False
            for j in range(m):
                if j in ((k - 1) % m, k, (k + 1) % m):
                    continue
                if point_in_triangle(pts[indices[j]], p0, p1, p2):
                    any_inside = True
                    break

            if not any_inside:
                triangles.append((i0, i1, i2))
                indices.pop(k)
                ear_found = True
                break

        if not ear_found and len(indices) > 2:
            # Fallback clipping for degenerate self-intersecting polygons
            triangles.append((indices[0], indices[1], indices[2]))
            indices.pop(1)

    return triangles


def parse_building_height(tags: dict[str, str], default_height: float = 8.0, level_height: float = 3.5) -> float:
    """Parses building height from OSM tags (height, building:levels, etc.)."""
    if "height" in tags:
        val = tags["height"].strip().lower()
        m = re.match(r"^([\d.]+)\s*(?:m|meters)?$", val)
        if m:
            try:
                h = float(m.group(1))
                if h > 1.0:
                    return h
            except ValueError:
                pass

    if "building:levels" in tags:
        val = tags["building:levels"].strip()
        m = re.match(r"^([\d.]+)$", val)
        if m:
            try:
                levels = float(m.group(1))
                if levels >= 1.0:
                    return max(3.0, levels * level_height)
            except ValueError:
                pass

    return default_height


class BuildingFootprint:
    """Represents a single 2D building footprint with geographic nodes."""

    def __init__(self, osm_id: str, tags: dict[str, str], nodes: list[tuple[float, float]]):
        self.osm_id = osm_id
        self.tags = tags
        self.nodes = nodes  # list of (lat, lon)


def extract_osm_building_footprints(osm_xml_or_path: str | Path) -> list[BuildingFootprint]:
    """Parses OSM XML and extracts all building polygons."""
    if isinstance(osm_xml_or_path, Path) or (isinstance(osm_xml_or_path, str) and not osm_xml_or_path.strip().startswith("<")):
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

    # 2. Collect building ways
    footprints: list[BuildingFootprint] = []
    for way in root.findall("way"):
        tags = {tag.get("k", ""): tag.get("v", "") for tag in way.findall("tag")}
        if "building" not in tags:
            continue

        way_nodes: list[tuple[float, float]] = []
        for nd in way.findall("nd"):
            ref = nd.get("ref")
            if ref and ref in nodes:
                way_nodes.append(nodes[ref])

        # Remove duplicate closing node if present
        if len(way_nodes) >= 4 and way_nodes[0] == way_nodes[-1]:
            way_nodes.pop()

        if len(way_nodes) >= 3:
            footprints.append(BuildingFootprint(way.get("id", ""), tags, way_nodes))

    return footprints


def classify_building_material(tags: dict[str, str], osm_id: str = "") -> tuple[str, str]:
    """
    Classifies a building into (facade_preset, roof_preset) based on OSM tags.
    Returns e.g. ('building_plaster', 'roof_tiles'), ('building_glass', 'roof_bitumen').
    """
    # 1. Roof preset determination
    roof_mat = tags.get("roof:material", "").strip().lower()
    if roof_mat in ("tiles", "ceramic", "clay", "slate", "shingle"):
        roof_preset = "roof_tiles"
    elif roof_mat in ("metal", "tin", "zinc", "copper", "steel"):
        roof_preset = "roof_metal"
    elif roof_mat in ("bitumen", "tar_paper", "felt", "asphalt", "rubber"):
        roof_preset = "roof_bitumen"
    elif roof_mat in ("concrete",):
        roof_preset = "building_concrete"
    elif roof_mat in ("glass",):
        roof_preset = "building_glass"
    else:
        # Default based on building function
        bldg_type = tags.get("building", "").strip().lower()
        if bldg_type in ("office", "commercial", "retail", "supermarket", "industrial", "warehouse"):
            roof_preset = "roof_bitumen"
        else:
            roof_preset = "roof_tiles"

    # 2. Facade preset determination
    facade_mat = tags.get("building:material", "").strip().lower()
    if facade_mat in ("brick", "red_brick"):
        return "building_brick", roof_preset
    if facade_mat in ("plaster", "stucco", "rendered"):
        return "building_plaster", roof_preset
    if facade_mat in ("glass", "curtain_wall"):
        return "building_concrete", roof_preset
    if facade_mat in ("stone", "sandstone", "granite", "marble", "limestone"):
        return "building_stone", roof_preset
    if facade_mat in ("concrete",):
        return "building_concrete", roof_preset
    if facade_mat in ("wood", "timber"):
        return "building_wood", roof_preset

    # 3. Infer from functional building tags
    bldg_val = tags.get("building", "").strip().lower()
    if bldg_val in ("office", "commercial", "retail", "bank", "supermarket", "department_store"):
        return "building_concrete", roof_preset
    if bldg_val in ("church", "cathedral", "monument", "historic", "castle", "civic"):
        return "building_stone", roof_preset
    if bldg_val in ("garages", "industrial", "warehouse", "transportation"):
        return "building_concrete", roof_preset
    if bldg_val in ("apartments", "residential", "house", "terrace", "detached", "dormitory"):
        return "building_plaster", roof_preset

    # 4. Fallback distribution based on deterministic hash of OSM ID
    h = 0
    for ch in osm_id:
        h = (h * 31 + ord(ch)) & 0xFFFFFFFF
    bucket = h % 10
    if bucket < 5:
        return "building_plaster", roof_preset
    elif bucket < 8:
        return "building_brick", roof_preset
    elif bucket < 9:
        return "building_stone", roof_preset
    else:
        return "building_concrete", roof_preset


def parse_building_colour(tags: dict[str, str]) -> tuple[int, int, int] | None:
    """Parses optional RGB color tint from OSM building:colour or colour tags."""
    val = tags.get("building:colour", tags.get("colour", "")).strip().lower()
    if not val:
        return None

    named_colors: dict[str, tuple[int, int, int]] = {
        "white": (245, 245, 245),
        "grey": (180, 180, 180),
        "gray": (180, 180, 180),
        "cream": (245, 235, 215),
        "beige": (245, 235, 215),
        "yellow": (230, 215, 160),
        "red": (190, 80, 70),
        "brick": (180, 80, 60),
        "brown": (140, 100, 75),
        "blue": (100, 140, 180),
        "green": (110, 150, 120),
    }
    if val in named_colors:
        return named_colors[val]

    m = re.match(r"^#?([0-9a-f]{2})([0-9a-f]{2})([0-9a-f]{2})$", val)
    if m:
        return int(m.group(1), 16), int(m.group(2), 16), int(m.group(3), 16)

    return None


def extrude_building_mesh_multislot(
    footprint_coords: Sequence[tuple[float, float]],
    height: float,
    base_z: float = 0.0,
    uv_tile_u: float = 4.0,
    uv_floor_height: float = 3.5,
) -> dict[str, tuple[list[tuple[float, float, float]], list[tuple[int, int, int]], list[tuple[float, float, float]], list[tuple[float, float]]]]:
    """
    Extrudes a 2D polygonal footprint into separate vertical facade walls and horizontal roof meshes.
    Returns a dict with 'walls' and 'roofs' keys, each containing (vertices, triangles, normals, uvs).
    Vertical facade UVs are modular per-story (v = height / 3.5m) to align window/floor grids.
    Horizontal roof UVs are planar top-down (u = x / 5.0m, v = y / 5.0m).
    """
    n = len(footprint_coords)
    empty = ([], [], [], [])
    if n < 3:
        return {"walls": empty, "roofs": empty}

    # Ensure footprint polygon is oriented counter-clockwise (positive signed area)
    signed_area = 0.0
    for i in range(n):
        j = (i + 1) % n
        signed_area += footprint_coords[i][0] * footprint_coords[j][1] - footprint_coords[j][0] * footprint_coords[i][1]
    if signed_area < 0:
        footprint_coords = list(reversed(footprint_coords))

    # -------------------------------------------------------------------------
    # 1. Extrude Vertical Facade Walls
    # -------------------------------------------------------------------------
    wall_verts: list[tuple[float, float, float]] = []
    wall_tris: list[tuple[int, int, int]] = []
    wall_norms: list[tuple[float, float, float]] = []
    wall_uvs: list[tuple[float, float]] = []

    u_coord = 0.0
    for i in range(n):
        j = (i + 1) % n
        p0 = footprint_coords[i]
        p1 = footprint_coords[j]

        dx = p1[0] - p0[0]
        dy = p1[1] - p0[1]
        edge_len = math.hypot(dx, dy)
        if edge_len < 1e-4:
            continue

        # Normal vector perpendicular to wall, pointing outwards
        # For CARLA coordinates: X right, Y forward -> outward normal is (dy, -dx, 0)
        nx = dy / edge_len
        ny = -dx / edge_len
        nz = 0.0

        v_base = len(wall_verts)

        # 4 vertices per wall quad
        v_bl = (p0[0], p0[1], base_z)
        v_br = (p1[0], p1[1], base_z)
        v_tr = (p1[0], p1[1], base_z + height)
        v_tl = (p0[0], p0[1], base_z + height)

        wall_verts.extend([v_bl, v_br, v_tr, v_tl])
        wall_norms.extend([(nx, ny, nz)] * 4)

        u0 = u_coord / uv_tile_u
        u1 = (u_coord + edge_len) / uv_tile_u
        v0 = 0.0
        v1 = height / uv_floor_height

        wall_uvs.extend([(u0, v0), (u1, v0), (u1, v1), (u0, v1)])
        u_coord += edge_len

        # 2 triangles for the wall quad (Unreal Engine clockwise outward face)
        wall_tris.append((v_base, v_base + 2, v_base + 1))
        wall_tris.append((v_base, v_base + 3, v_base + 2))

    # -------------------------------------------------------------------------
    # 2. Extrude Horizontal Roof
    # -------------------------------------------------------------------------
    roof_verts: list[tuple[float, float, float]] = []
    roof_tris: list[tuple[int, int, int]] = []
    roof_norms: list[tuple[float, float, float]] = []
    roof_uvs: list[tuple[float, float]] = []

    roof_pts = [(p[0], p[1]) for p in footprint_coords]
    roof_tris_2d = triangulate_polygon_2d(roof_pts)

    roof_scale = 5.0  # 5m planar tile repeat for roof materials
    for p in footprint_coords:
        roof_verts.append((p[0], p[1], base_z + height))
        roof_norms.append((0.0, 0.0, 1.0))
        roof_uvs.append((p[0] / roof_scale, p[1] / roof_scale))

    for tri in roof_tris_2d:
        # Flip last two indices for Unreal Engine skyward (+Z) clockwise face
        roof_tris.append((tri[0], tri[2], tri[1]))

    return {
        "walls": (wall_verts, wall_tris, wall_norms, wall_uvs),
        "roofs": (roof_verts, roof_tris, roof_norms, roof_uvs),
    }


def extrude_building_mesh(
    footprint_coords: Sequence[tuple[float, float]],
    height: float,
    base_z: float = 0.0,
    uv_tile_u: float = 4.0,
    uv_tile_v: float = 3.5,
) -> tuple[list[tuple[float, float, float]], list[tuple[int, int, int]], list[tuple[float, float, float]], list[tuple[float, float]]]:
    """
    Extrudes a 2D polygonal footprint into a single combined 3D building mesh (walls + roof).
    Returns (vertices, triangles, normals, uvs). Provided for backward compatibility.
    """
    slots = extrude_building_mesh_multislot(
        footprint_coords,
        height=height,
        base_z=base_z,
        uv_tile_u=uv_tile_u,
        uv_floor_height=uv_tile_v,
    )
    wall_v, wall_t, wall_n, wall_uv = slots["walls"]
    roof_v, roof_t, roof_n, roof_uv = slots["roofs"]

    combined_v = list(wall_v)
    combined_n = list(wall_n)
    combined_uv = list(wall_uv)
    combined_t = list(wall_t)

    offset = len(combined_v)
    combined_v.extend(roof_v)
    combined_n.extend(roof_n)
    combined_uv.extend(roof_uv)
    for tri in roof_t:
        combined_t.append((offset + tri[0], offset + tri[1], offset + tri[2]))

    return combined_v, combined_t, combined_n, combined_uv


def build_osm_buildings_multislot_layers(
    footprints: Sequence[BuildingFootprint],
    project_coord_fn: Callable[[float, float], tuple[float, float, float]],
    default_height: float = 8.0,
    level_height: float = 3.5,
) -> dict[str, dict[str, Any]]:
    """
    Builds separated, multi-slot architectural layers from OSM building footprints:
    - 'walls': Standard residential/plaster facades (material_preset: building_plaster)
    - 'roofs': Horizontal roofs (material_preset: roof_tiles)
    - 'modern_walls': Commercial/office facades (material_preset: building_glass)
    - 'stone_walls': Historic/civic stone facades (material_preset: building_stone)
    - 'brick_walls': Classic brick facades (material_preset: building_brick)
    """
    layer_defs: dict[str, dict[str, Any]] = {
        "walls": {
            "name": "OSM_Buildings",
            "material_preset": "building_plaster",
            "semantic_label": "Buildings",
            "enable_collision": True,
            "vertices": [],
            "triangles": [],
            "normals": [],
            "uvs": [],
        },
        "roofs": {
            "name": "OSM_Buildings_Roofs",
            "material_preset": "roof_tiles",
            "semantic_label": "Buildings",
            "enable_collision": True,
            "vertices": [],
            "triangles": [],
            "normals": [],
            "uvs": [],
        },
        "modern_walls": {
            "name": "OSM_Buildings_Modern",
            "material_preset": "building_concrete",
            "semantic_label": "Buildings",
            "enable_collision": True,
            "vertices": [],
            "triangles": [],
            "normals": [],
            "uvs": [],
        },
        "stone_walls": {
            "name": "OSM_Buildings_Stone",
            "material_preset": "building_stone",
            "semantic_label": "Buildings",
            "enable_collision": True,
            "vertices": [],
            "triangles": [],
            "normals": [],
            "uvs": [],
        },
        "brick_walls": {
            "name": "OSM_Buildings_Brick",
            "material_preset": "building_brick",
            "semantic_label": "Buildings",
            "enable_collision": True,
            "vertices": [],
            "triangles": [],
            "normals": [],
            "uvs": [],
        },
    }

    def append_geometry(target: dict[str, Any], verts: list, tris: list, norms: list, uvs: list) -> None:
        if not verts:
            return
        offset = len(target["vertices"])
        target["vertices"].extend(verts)
        target["normals"].extend(norms)
        target["uvs"].extend(uvs)
        for tri in tris:
            target["triangles"].append((offset + tri[0], offset + tri[1], offset + tri[2]))

    for fp in footprints:
        height = parse_building_height(fp.tags, default_height, level_height)

        local_coords: list[tuple[float, float]] = []
        base_z = 0.0
        for lat, lon in fp.nodes:
            xyz = project_coord_fn(lat, lon)
            local_coords.append((xyz[0], xyz[1]))
            base_z = xyz[2]

        slots = extrude_building_mesh_multislot(
            local_coords,
            height=height,
            base_z=base_z,
            uv_tile_u=8.0,
            uv_floor_height=7.0,
        )

        wall_v, wall_t, wall_n, wall_uv = slots["walls"]
        roof_v, roof_t, roof_n, roof_uv = slots["roofs"]

        facade_preset, _ = classify_building_material(fp.tags, fp.osm_id)

        # Route wall facade to appropriate architectural slot
        if facade_preset in ("building_concrete", "building_modern", "building_glass"):
            target_wall = layer_defs["modern_walls"]
        elif facade_preset == "building_stone":
            target_wall = layer_defs["stone_walls"]
        elif facade_preset == "building_brick":
            target_wall = layer_defs["brick_walls"]
        else:
            target_wall = layer_defs["walls"]

        append_geometry(target_wall, wall_v, wall_t, wall_n, wall_uv)
        append_geometry(layer_defs["roofs"], roof_v, roof_t, roof_n, roof_uv)

    # Return only layers that have non-empty geometry
    return {k: v for k, v in layer_defs.items() if len(v["vertices"]) > 0}


def build_osm_buildings_layer(
    footprints: Sequence[BuildingFootprint],
    project_coord_fn: Callable[[float, float], tuple[float, float, float]],
    default_height: float = 8.0,
    level_height: float = 3.5,
) -> tuple[list[tuple[float, float, float]], list[tuple[int, int, int]], list[tuple[float, float, float]], list[tuple[float, float]]]:
    """
    Builds a single combined 3D mesh layer from all OSM building footprints.
    Maintained for full backward compatibility.
    """
    all_layers = build_osm_buildings_multislot_layers(
        footprints,
        project_coord_fn=project_coord_fn,
        default_height=default_height,
        level_height=level_height,
    )
    all_vertices: list[tuple[float, float, float]] = []
    all_triangles: list[tuple[int, int, int]] = []
    all_normals: list[tuple[float, float, float]] = []
    all_uvs: list[tuple[float, float]] = []

    for l_data in all_layers.values():
        offset = len(all_vertices)
        all_vertices.extend(l_data["vertices"])
        all_normals.extend(l_data["normals"])
        all_uvs.extend(l_data["uvs"])
        for tri in l_data["triangles"]:
            all_triangles.append((offset + tri[0], offset + tri[1], offset + tri[2]))

    return all_vertices, all_triangles, all_normals, all_uvs


CARLA_3D_BUILDING_CATALOG: list[tuple[str, tuple[float, float, float]]] = [
    ("/Game/Carla/Static/Building/Buildings/BuildingComplete/SM_Apt32_A.SM_Apt32_A", (6.66, 16.32, 17.59)),
    ("/Game/Carla/Static/Building/Buildings/BuildingComplete/SM_Apt32_B.SM_Apt32_B", (4.33, 2.13, 14.53)),
    ("/Game/Carla/Static/Building/Buildings/BuildingComplete/SM_Apt33_A.SM_Apt33_A", (6.36, 16.30, 19.67)),
    ("/Game/Carla/Static/Building/Buildings/BuildingComplete/SM_Apt34_A.SM_Apt34_A", (6.03, 16.24, 18.79)),
    ("/Game/Carla/Static/Building/Buildings/BuildingComplete/SM_Apt35_A.SM_Apt35_A", (6.29, 16.32, 19.67)),
    ("/Game/Carla/Static/Building/BackgroundBuildings/00_IndividualComponents/Apt20/SM_Apt20_D.SM_Apt20_D", (15.42, 15.11, 15.73)),
    ("/Game/Carla/Static/Building/BackgroundBuildings/00_IndividualComponents/Apt20/SM_Apt20_B.SM_Apt20_B", (18.71, 18.79, 20.47)),
    ("/Game/Carla/Static/Building/BackgroundBuildings/00_IndividualComponents/Apt20/SM_Apt20_A.SM_Apt20_A", (22.30, 29.20, 15.73)),
    ("/Game/Carla/Static/Building/BackgroundBuildings/00_IndividualComponents/Apt20/SM_Apt21_A.SM_Apt21_A", (16.46, 31.46, 29.27)),
    ("/Game/Carla/Static/Building/BackgroundBuildings/00_IndividualComponents/Apt12/SM_Apt12_A.SM_Apt12_A", (30.42, 15.42, 20.32)),
    ("/Game/Carla/Static/Building/BackgroundBuildings/00_IndividualComponents/Apt12/SM_Apt12_B.SM_Apt12_B", (34.05, 28.56, 27.32)),
    ("/Game/Carla/Static/Building/BackgroundBuildings/00_IndividualComponents/Apt20/SM_Apt20_C.SM_Apt20_C", (18.71, 46.89, 20.45)),
    ("/Game/Carla/Static/Building/BackgroundBuildings/00_IndividualComponents/Apt20/SM_Apt20_E.SM_Apt20_E", (32.94, 30.96, 29.89)),
    ("/Game/Carla/Static/Building/BackgroundBuildings/SM_Apt19_A.SM_Apt19_A", (47.98, 20.00, 24.00)),
    ("/Game/Carla/Static/Building/BackgroundBuildings/00_IndividualComponents/Apt19/SM_Apt19.SM_Apt19", (48.21, 16.11, 24.00)),
    ("/Game/Carla/Static/Building/BackgroundBuildings/00_IndividualComponents/Apt20/SM_Apt20_F.SM_Apt20_F", (45.21, 45.24, 39.33)),
]


def generate_3d_building_placements(
    footprints: Sequence[BuildingFootprint],
    project_coord_fn: Callable[[float, float], tuple[float, float, float]],
    default_height: float = 8.0,
    level_height: float = 3.5,
) -> list[dict[str, Any]]:
    """
    Places photorealistic 3D static mesh buildings from CARLA's built-in architectural catalog
    matching parcel footprints (centroid, orientation, scale, and base elevation).
    Returns a list of placement dicts formatted for carla-map.json.
    """
    placements_by_mesh: dict[str, list[dict[str, Any]]] = {
        m[0]: [] for m in CARLA_3D_BUILDING_CATALOG
    }

    for fp in footprints:
        pts = [project_coord_fn(lat, lon) for lat, lon in fp.nodes]
        if len(pts) < 3:
            continue

        xs = [p[0] for p in pts]
        ys = [p[1] for p in pts]
        base_z = min(p[2] for p in pts)
        cx = sum(xs) / len(xs)
        cy = sum(ys) / len(ys)

        # 1. Compute dominant edge orientation
        max_edge_len = 0.0
        best_dx, best_dy = 1.0, 0.0
        n = len(pts)
        for i in range(n):
            j = (i + 1) % n
            dx = pts[j][0] - pts[i][0]
            dy = pts[j][1] - pts[i][1]
            dist = math.hypot(dx, dy)
            if dist > max_edge_len:
                max_edge_len = dist
                best_dx, best_dy = dx, dy

        theta = math.atan2(best_dy, best_dx)
        cos_t = math.cos(-theta)
        sin_t = math.sin(-theta)

        # 2. Project vertices onto oriented local frame
        rot_xs = [(x - cx) * cos_t - (y - cy) * sin_t for x, y in zip(xs, ys)]
        rot_ys = [(x - cx) * sin_t + (y - cy) * cos_t for x, y in zip(xs, ys)]

        u_min, u_max = min(rot_xs), max(rot_xs)
        v_min, v_max = min(rot_ys), max(rot_ys)
        length_dim = max(4.0, u_max - u_min)
        width_dim = max(4.0, v_max - v_min)
        height_dim = parse_building_height(fp.tags, default_height, level_height)

        u_mid = (u_min + u_max) * 0.5
        v_mid = (v_min + v_max) * 0.5
        world_cx = cx + u_mid * math.cos(theta) - v_mid * math.sin(theta)
        world_cy = cy + u_mid * math.sin(theta) + v_mid * math.cos(theta)

        yaw_deg = math.degrees(theta)
        if width_dim > length_dim:
            length_dim, width_dim = width_dim, length_dim
            yaw_deg += 90.0

        # 3. Model matching based on footprint area and aspect ratio
        target_area = length_dim * width_dim
        h_hash = hash(fp.osm_id) & 0x7FFFFFFF

        scores: list[tuple[float, int]] = []
        for idx, (mesh_path, (mx, my, mz)) in enumerate(CARLA_3D_BUILDING_CATALOG):
            m_area = mx * my
            area_diff = abs(math.log(max(1e-3, target_area) / m_area))
            aspect_target = length_dim / max(1.0, width_dim)
            aspect_model = max(mx, my) / max(1.0, min(mx, my))
            aspect_diff = abs(math.log(aspect_target / aspect_model))
            score = area_diff * 1.5 + aspect_diff
            scores.append((score, idx))

        scores.sort(key=lambda s: s[0])
        # Pick with pseudo-random variety among top candidate models
        top_candidates = scores[:min(3, len(scores))]
        chosen_idx = top_candidates[h_hash % len(top_candidates)][1]
        chosen_path, (mx, my, mz) = CARLA_3D_BUILDING_CATALOG[chosen_idx]

        sx = max(0.4, min(2.5, length_dim / mx))
        sy = max(0.4, min(2.5, width_dim / my))
        sz = max(0.5, min(2.5, height_dim / mz))

        placements_by_mesh[chosen_path].append({
            "location": [round(world_cx, 2), round(world_cy, 2), round(base_z, 2)],
            "rotation": [0.0, round(yaw_deg % 360.0, 1), 0.0],
            "scale": [round(sx, 2), round(sy, 2), round(sz, 2)],
        })

    result: list[dict[str, Any]] = []
    for mesh_path, transforms in placements_by_mesh.items():
        if transforms:
            result.append({
                "asset_path": mesh_path,
                "transforms": transforms,
            })
    return result


