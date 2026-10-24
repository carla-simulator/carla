#!/usr/bin/env python3
"""
Dedicated Bicycle Infrastructure & Cycleway Networks Engine for CARLA Map Maker.

Milestone 13 (M13):
  - Ingests OSM cycleway=* ways, highway=cycleway, bicycle=designated, and surface:colour=*.
  - Associates cycleways with OpenDRIVE road networks and calculates curbside lateral offsets.
  - Generates 3D cycleway corridor ribbon geometry with colored PBR surface presets:
      * Emerald Green (road_cycleway_green, Munich StVO standard)
      * Terracotta Red (road_cycleway_red, European standard)
      * Smooth Bitumen (road_asphalt)
  - Procedurally synthesizes retroreflective white demarcation lines:
      * Solid line (0.12m) for exclusive cycle tracks (cycleway:*:lane=exclusive)
      * Dashed line (1.0m stripe / 1.0m gap) for advisory lanes (cycleway:*:lane=advisory)
  - Synthesizes white bicycle silhouette stencil decals along tracks and inside Advance Stop Lines (Bike Boxes).
  - Populates static bicycle vehicle assets (Gazelle Omafiets, Diamondback Century, BH Crossbike)
    and bicycle parking loops.
"""

from __future__ import annotations

from dataclasses import asdict, dataclass
import json
import math
import os
from pathlib import Path
import random
import tempfile
from typing import Any, Callable, Sequence
import xml.etree.ElementTree as ET

try:
    import carla
except ImportError:
    carla = None  # type: ignore[assignment]

try:
    from Util.MapBuilder.cmesh import write_cmesh_file
    from Util.MapBuilder.osm_crosswalks import make_lat_lon_projector
    from Util.MapBuilder.osm_clutter import ClutterItem
except ImportError:
    from cmesh import write_cmesh_file
    from osm_crosswalks import make_lat_lon_projector
    from osm_clutter import ClutterItem

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


# =============================================================================
# 1. Bicycle Blueprints & Color Palettes
# =============================================================================

BICYCLE_BLUEPRINTS = [
    "vehicle.gazelle.omafiets",
    "vehicle.bh.crossbike",
    "vehicle.diamondback.century",
]

CYCLEWAY_COLOR_PRESETS: dict[str, list[int]] = {
    "green": [38, 138, 65],      # Munich Emerald Green
    "red": [195, 52, 42],        # Munich Terracotta / Traffic Red
    "asphalt": [55, 55, 60],     # Dark Bitumen
}


# =============================================================================
# 2. Dataclasses
# =============================================================================

@dataclass
class CyclewaySegment:
    """Represents a validated cycleway corridor or lane."""
    segment_id: str
    name: str
    side: str              # "right", "left", "both", "standalone"
    cycleway_type: str     # "lane", "track", "sidepath", "standalone"
    surface_color: str     # "green", "red", "asphalt"
    width_m: float
    lane_marking: str      # "solid", "dashed", "none"
    nodes_carla: list[tuple[float, float, float]]
    length_m: float
    road_id: int | None = None
    source: str = "osm"


@dataclass
class BikeBox:
    """Advance stop line / bike box allowing cyclists to queue ahead of vehicular traffic."""
    box_id: str
    road_id: int
    center_carla: tuple[float, float, float]
    heading_deg: float
    width_m: float
    length_m: float


@dataclass
class BikeParkingRack:
    """Bicycle parking stand with parked bikes."""
    rack_id: str
    location_carla: tuple[float, float, float]
    heading_deg: float
    num_parked_bikes: int
    parked_bikes: list[dict[str, Any]]


# =============================================================================
# 3. Extraction & Road Association Engine
# =============================================================================

def extract_osm_cycleways(
    osm_path: str | Path,
    project_coord_fn: Callable[[float, float], tuple[float, float, float]] | None = None,
    carla_map: Any | None = None,
    default_width: float = 2.0,
) -> tuple[list[CyclewaySegment], list[BikeBox], list[BikeParkingRack]]:
    """
    Extracts standalone cycleways, roadside cycle tracks, and bike lanes from OSM.
    Calculates 3D coordinates projected into CARLA simulation space.
    """
    osm_file = Path(osm_path)
    if not osm_file.is_file():
        raise FileNotFoundError(f"OSM file not found: {osm_path}")

    if project_coord_fn is None and carla_map is not None:
        project_coord_fn = make_lat_lon_projector(carla_map=carla_map)

    tree = ET.parse(osm_file)
    root = tree.getroot()

    nodes: dict[str, tuple[float, float]] = {
        n.get("id"): (float(n.get("lat")), float(n.get("lon"))) for n in root.findall("node")
    }

    cycleways: list[CyclewaySegment] = []
    bike_boxes: list[BikeBox] = []
    bike_racks: list[BikeParkingRack] = []

    # 1. Parse Candidate Ways
    raw_candidates: list[dict[str, Any]] = []
    for way in root.findall("way"):
        tags = {t.get("k"): t.get("v") for t in way.findall("tag")}
        refs = [nd.get("ref") for nd in way.findall("nd")]
        pts_geo = [nodes[r] for r in refs if r in nodes]
        if len(pts_geo) < 2:
            continue

        wid = way.get("id")
        name = tags.get("name", "Cycleway")
        highway = tags.get("highway", "")

        is_standalone = (highway == "cycleway") or (
            highway in ("path", "footway", "pedestrian", "service", "living_street", "track")
            and (tags.get("bicycle") in ("designated", "yes", "official") or tags.get("cycleway") in ("yes", "sidepath"))
        )

        on_road_sides: list[str] = []
        for k, s in [("cycleway:right", "right"), ("cycleway:left", "left"), ("cycleway:both", "both"), ("cycleway", "right")]:
            v = tags.get(k, "")
            if v in ("lane", "opposite_lane", "share_busway", "track", "opposite_track", "yes"):
                on_road_sides.append(s)

        is_on_road = not is_standalone and len(on_road_sides) > 0

        if not (is_standalone or is_on_road):
            continue

        # Project 3D centerline points
        pts_carla: list[tuple[float, float, float]] = []
        for lat, lon in pts_geo:
            if project_coord_fn is not None:
                x, y, z = project_coord_fn(lat, lon)
            else:
                x, y, z = lon * 111320.0, lat * 110540.0, 0.0
            pts_carla.append((round(x, 3), round(y, 3), round(z, 3)))

        raw_candidates.append({
            "wid": wid,
            "name": name,
            "highway": highway,
            "tags": tags,
            "refs": refs,
            "pts_carla": pts_carla,
            "is_standalone": is_standalone,
            "is_on_road": is_on_road,
            "on_road_sides": on_road_sides,
        })

    curbside_connection_points: dict[str, tuple[float, float, float]] = {}
    on_road_joint_points: dict[str, tuple[float, float, float]] = {}

    # Pre-index on-road candidates and compute baseline lateral offsets
    cand_by_start_node: dict[str, list[dict[str, Any]]] = {}
    cand_by_end_node: dict[str, list[dict[str, Any]]] = {}
    for cand in raw_candidates:
        if not cand["is_on_road"]:
            continue
        tags = cand["tags"]
        width_tag = tags.get("cycleway:width", tags.get("width", ""))
        try:
            width_m = float(width_tag)
        except Exception:
            width_m = default_width
        width_m = max(1.2, min(3.5, width_m))

        try:
            n_lanes = float(tags.get("lanes", "2"))
        except (ValueError, TypeError):
            n_lanes = 2.0
        n_lanes = max(1.0, n_lanes)
        lane_width = 3.5
        half_road = (n_lanes * lane_width) * 0.5
        d_offset = max(1.2, half_road - (width_m * 0.5))

        cand["d_offset"] = d_offset
        cand["width_m"] = width_m
        cand_by_start_node.setdefault(cand["refs"][0], []).append(cand)
        cand_by_end_node.setdefault(cand["refs"][-1], []).append(cand)

    # Pass 1: Process On-Road Cycleways with Curbside Lateral Offsets & Smooth Tapers
    for cand in raw_candidates:
        if not cand["is_on_road"]:
            continue

        tags = cand["tags"]
        pts_raw = list(cand["pts_carla"])
        d_offset = cand["d_offset"]
        width_m = cand["width_m"]

        if len(pts_raw) < 2:
            continue

        # Surface color
        color_tag = tags.get("cycleway:surface:colour", tags.get("surface:colour", "")).lower()
        if "green" in color_tag:
            surface_color = "green"
        elif "asphalt" in color_tag or "black" in color_tag:
            surface_color = "asphalt"
        else:
            surface_color = "red"

        # Check for smooth taper to adjoining on-road cycleways with different lane counts
        d_offsets = [d_offset] * len(pts_raw)
        end_ref = cand["refs"][-1]
        successors = [c for c in cand_by_start_node.get(end_ref, []) if c["wid"] != cand["wid"] and c["name"] == cand["name"]]
        if successors:
            succ = successors[0]
            d_next = succ["d_offset"]
            if abs(d_next - d_offset) > 0.1:
                p_end = pts_raw[-1]
                p_prev = pts_raw[-2]
                seg_len = math.hypot(p_end[0] - p_prev[0], p_end[1] - p_prev[1])
                taper_dist = min(18.0, seg_len * 0.8)
                if seg_len > 22.0:
                    alpha = (seg_len - taper_dist) / seg_len
                    p_taper = (
                        p_prev[0] + alpha * (p_end[0] - p_prev[0]),
                        p_prev[1] + alpha * (p_end[1] - p_prev[1]),
                        p_prev[2] + alpha * (p_end[2] - p_prev[2]),
                    )
                    pts_raw.insert(-1, p_taper)
                    d_offsets.insert(-1, d_offset)
                    d_offsets[-1] = d_next
                else:
                    d_offsets[-1] = d_next

        n = len(pts_raw)
        def compute_shifted(side_mult: float) -> list[tuple[float, float, float]]:
            shifted: list[tuple[float, float, float]] = []
            for i in range(n):
                if i == 0:
                    dx = pts_raw[1][0] - pts_raw[0][0]
                    dy = pts_raw[1][1] - pts_raw[0][1]
                elif i == n - 1:
                    dx = pts_raw[n - 1][0] - pts_raw[n - 2][0]
                    dy = pts_raw[n - 1][1] - pts_raw[n - 2][1]
                else:
                    dx = pts_raw[i + 1][0] - pts_raw[i - 1][0]
                    dy = pts_raw[i + 1][1] - pts_raw[i - 1][1]

                dist = math.hypot(dx, dy)
                if dist < 1e-4:
                    nrx, nry = -1.0, 0.0
                else:
                    ux, uy = dx / dist, dy / dist
                    nrx, nry = -uy, ux

                sx = pts_raw[i][0] + side_mult * nrx * d_offsets[i]
                sy = pts_raw[i][1] + side_mult * nry * d_offsets[i]
                sz = pts_raw[i][2]
                shifted.append((round(sx, 3), round(sy, 3), round(sz, 3)))
            return shifted

        sides_to_gen: set[tuple[str, float]] = set()
        for s_side in cand["on_road_sides"]:
            if s_side == "right":
                sides_to_gen.add(("right", 1.0))
            elif s_side == "left":
                sides_to_gen.add(("left", -1.0))
            elif s_side == "both":
                sides_to_gen.add(("right", 1.0))
                sides_to_gen.add(("left", -1.0))

        cand_id = cand["wid"]
        for s_name, s_mult in sides_to_gen:
            marking = "solid"
            if s_name == "right" and tags.get("cycleway:right:lane") == "advisory":
                marking = "dashed"
            elif s_name == "left" and tags.get("cycleway:left:lane") == "advisory":
                marking = "dashed"

            shifted_pts = compute_shifted(s_mult)

            # Snap adjoining on-road cycleway segments to exact shared joint coordinate
            start_joint_key = f"{cand['refs'][0]}_{s_name}"
            end_joint_key = f"{cand['refs'][-1]}_{s_name}"
            if start_joint_key in on_road_joint_points:
                shifted_pts[0] = on_road_joint_points[start_joint_key]
            else:
                on_road_joint_points[start_joint_key] = shifted_pts[0]

            if end_joint_key in on_road_joint_points:
                shifted_pts[-1] = on_road_joint_points[end_joint_key]
            else:
                on_road_joint_points[end_joint_key] = shifted_pts[-1]

            cum_len = sum(
                math.hypot(shifted_pts[j][0] - shifted_pts[j - 1][0], shifted_pts[j][1] - shifted_pts[j - 1][1])
                for j in range(1, len(shifted_pts))
            )
            if cum_len >= 3.0:
                seg_id = f"cyc_{cand_id}_{s_name}"
                cycleways.append(
                    CyclewaySegment(
                        segment_id=seg_id,
                        name=cand["name"],
                        side=s_name,
                        cycleway_type="lane",
                        surface_color=surface_color,
                        width_m=width_m,
                        lane_marking=marking,
                        nodes_carla=shifted_pts,
                        length_m=round(cum_len, 2),
                    )
                )
                curbside_connection_points[cand["refs"][0]] = shifted_pts[0]
                curbside_connection_points[cand["refs"][-1]] = shifted_pts[-1]

    # Collect road centerline nodes of roads without on-road cycleways
    road_centerline_nodes: set[str] = set()
    for way in root.findall("way"):
        t = {x.get("k"): x.get("v") for x in way.findall("tag")}
        hw = t.get("highway", "")
        if hw in ("motorway", "trunk", "primary", "secondary", "tertiary", "residential", "unclassified", "service"):
            has_cw = any(
                t.get(k) in ("lane", "opposite_lane", "share_busway", "track", "opposite_track", "yes")
                for k in ("cycleway", "cycleway:right", "cycleway:left", "cycleway:both")
            )
            if not has_cw:
                for nd in way.findall("nd"):
                    road_centerline_nodes.add(nd.get("ref"))

    # Pass 2: Process Standalone Cycleways and Transition Connector Ramps
    for cand in raw_candidates:
        if not cand["is_standalone"]:
            continue

        tags = cand["tags"]
        hw = cand["highway"]

        # Filter out unnamed path/footway routing link stubs that lead into roads without cycleways
        if hw in ("path", "footway", "pedestrian") and tags.get("name") is None and len(cand["refs"]) <= 3:
            if cand["refs"][0] in road_centerline_nodes or cand["refs"][-1] in road_centerline_nodes:
                continue

        pts_carla = list(cand["pts_carla"])
        n = len(pts_carla)
        if n < 2:
            continue

        # Snap connection endpoints if connecting to offset curbside cycleways
        if cand["refs"][0] in curbside_connection_points:
            pts_carla[0] = curbside_connection_points[cand["refs"][0]]
        if cand["refs"][-1] in curbside_connection_points:
            pts_carla[-1] = curbside_connection_points[cand["refs"][-1]]

        # Trim endpoints that penetrate into vehicular driving lanes on roads without cycleways
        if carla_map is not None:
            trimmed_pts = list(pts_carla)
            while len(trimmed_pts) >= 2:
                p = trimmed_pts[-1]
                loc = carla.Location(p[0], p[1], p[2])
                wp = carla_map.get_waypoint(loc, project_to_road=True, lane_type=carla.LaneType.Driving)
                if wp and not wp.is_junction and loc.distance(wp.transform.location) < wp.lane_width * 0.4:
                    if cand["refs"][-1] not in curbside_connection_points:
                        trimmed_pts.pop(-1)
                        continue
                break
            while len(trimmed_pts) >= 2:
                p = trimmed_pts[0]
                loc = carla.Location(p[0], p[1], p[2])
                wp = carla_map.get_waypoint(loc, project_to_road=True, lane_type=carla.LaneType.Driving)
                if wp and not wp.is_junction and loc.distance(wp.transform.location) < wp.lane_width * 0.4:
                    if cand["refs"][0] not in curbside_connection_points:
                        trimmed_pts.pop(0)
                        continue
                break
            pts_carla = trimmed_pts
            if len(pts_carla) < 2:
                continue

        # Determine width
        width_tag = tags.get("cycleway:width", tags.get("width", ""))
        try:
            width_m = float(width_tag)
        except Exception:
            width_m = default_width
        width_m = max(1.2, min(3.5, width_m))

        color_tag = tags.get("cycleway:surface:colour", tags.get("surface:colour", "")).lower()
        if "green" in color_tag:
            surface_color = "green"
        elif "asphalt" in color_tag or "black" in color_tag:
            surface_color = "asphalt"
        else:
            surface_color = "red"

        cum_len = sum(
            math.hypot(pts_carla[j][0] - pts_carla[j - 1][0], pts_carla[j][1] - pts_carla[j - 1][1])
            for j in range(1, len(pts_carla))
        )
        if cum_len >= 3.0:
            cand_id = cand["wid"]
            cycleways.append(
                CyclewaySegment(
                    segment_id=f"cyc_{cand_id}",
                    name=cand["name"],
                    side="standalone",
                    cycleway_type="standalone",
                    surface_color=surface_color,
                    width_m=width_m,
                    lane_marking="solid",
                    nodes_carla=pts_carla,
                    length_m=round(cum_len, 2),
                )
            )

    # 2. Synthesize Advance Stop Lines (Bike Boxes) at signalized approaches
    if carla_map is not None:
        try:
            roads = carla_map.get_topology()
            seen_box_locs: list[tuple[float, float]] = []

            for entry in roads:
                w0, w1 = entry[0], entry[1]
                # Transition from incoming approach road (not junction) into junction
                if (not w0.is_junction) and w1.is_junction:
                    # Step back 3.5m from the junction entry on the approach road
                    prev_wps = w1.previous(3.5)
                    for cand_wp in prev_wps:
                        # Advance Stop Lines must strictly sit on normal approach roads, never inside junctions
                        if cand_wp.is_junction:
                            continue

                        loc = cand_wp.transform.location
                        rot = cand_wp.transform.rotation

                        # Deduplicate: ensure minimum spacing of 10.0m between bike boxes
                        is_dup = any(math.hypot(b[0] - loc.x, b[1] - loc.y) < 10.0 for b in seen_box_locs)
                        if is_dup:
                            continue

                        seen_box_locs.append((loc.x, loc.y))
                        box_id = f"bikebox_{cand_wp.road_id}_{len(bike_boxes)+1:02d}"
                        bike_boxes.append(
                            BikeBox(
                                box_id=box_id,
                                road_id=cand_wp.road_id,
                                center_carla=(round(loc.x, 3), round(loc.y, 3), round(loc.z, 3)),
                                heading_deg=round(rot.yaw, 1),
                                width_m=round(cand_wp.lane_width, 2),
                                length_m=4.5,
                            )
                        )
        except Exception:
            pass

    # 3. Parse Bicycle Parking Nodes (Bicycle Clutter)
    rng = random.Random(77)
    for node in root.findall("node"):
        tags = {t.get("k"): t.get("v") for t in node.findall("tag")}
        is_bike_parking = (
            tags.get("amenity") == "bicycle_parking"
            or "bicycle_parking" in tags
        )
        if is_bike_parking:
            lat = float(node.get("lat"))
            lon = float(node.get("lon"))
            if project_coord_fn is not None:
                x, y, z = project_coord_fn(lat, lon)
            else:
                x, y, z = lon * 111320.0, lat * 110540.0, 0.0

            num_bikes = rng.randint(2, 5)
            parked_bike_items = []
            yaw = rng.uniform(0.0, 360.0)

            for b_idx in range(num_bikes):
                bp = rng.choice(BICYCLE_BLUEPRINTS)
                # Offset parked bikes in a neat row
                rad = math.radians(yaw)
                vx, vy = -math.sin(rad), math.cos(rad)
                bx = x + vx * (b_idx * 0.7 - (num_bikes - 1) * 0.35)
                by = y + vy * (b_idx * 0.7 - (num_bikes - 1) * 0.35)
                parked_bike_items.append({
                    "blueprint_id": bp,
                    "location": [round(bx, 3), round(by, 3), round(z + 0.15, 3)],
                    "rotation": [0.0, round(yaw + rng.uniform(-10.0, 10.0), 1), 0.0],
                })

            bike_racks.append(
                BikeParkingRack(
                    rack_id=f"rack_{node.get('id')}",
                    location_carla=(round(x, 3), round(y, 3), round(z, 3)),
                    heading_deg=round(yaw, 1),
                    num_parked_bikes=num_bikes,
                    parked_bikes=parked_bike_items,
                )
            )

    return cycleways, bike_boxes, bike_racks


# =============================================================================
# 4. 3D Mesh Synthesis: Cycleway Corridor Ribbons
# =============================================================================

def generate_cycleway_geometry(
    cycleways: Sequence[CyclewaySegment],
    default_color: str = "green",
    z_offset: float = 0.016,
) -> tuple[
    dict[str, Any],     # Layer geometry: OSM_Cycleways
    list[tuple[list[Any], list[Any]]],  # Walkable surfaces for Recast NavMesh
]:
    """
    Extrudes continuous 3D ribbon meshes for cycleway surfaces.
    Returns (cmesh_dict, walkable_surfaces).
    """
    vertices: list[tuple[float, float, float]] = []
    triangles: list[tuple[int, int, int]] = []
    normals: list[tuple[float, float, float]] = []
    uvs: list[tuple[float, float]] = []
    colors: list[tuple[int, int, int, int]] = []
    walkable_surfaces: list[tuple[list[Any], list[Any]]] = []

    for cyc in cycleways:
        pts = cyc.nodes_carla
        n = len(pts)
        if n < 2:
            continue

        w = cyc.width_m
        hw = w * 0.5
        seg_verts: list[tuple[float, float, float]] = []
        seg_tris: list[tuple[int, int, int]] = []

        # Color tint per vertex
        rgb = CYCLEWAY_COLOR_PRESETS.get(cyc.surface_color, [160, 48, 42])
        v_col = (int(rgb[0]), int(rgb[1]), int(rgb[2]), 255)

        # Calculate horizontal normal vectors at each node
        miter_normals: list[tuple[float, float]] = []
        for i in range(n):
            if i == 0:
                dx = pts[1][0] - pts[0][0]
                dy = pts[1][1] - pts[0][1]
            elif i == n - 1:
                dx = pts[-1][0] - pts[-2][0]
                dy = pts[-1][1] - pts[-2][1]
            else:
                dx = pts[i + 1][0] - pts[i - 1][0]
                dy = pts[i + 1][1] - pts[i - 1][1]

            length = math.hypot(dx, dy)
            if length < 1e-4:
                nx, ny = 0.0, 1.0
            else:
                # Perpendicular normal
                nx, ny = -dy / length, dx / length
            miter_normals.append((nx, ny))

        # Generate left and right ribbon edges
        cum_dist = 0.0
        base_v_idx = len(vertices)

        for i in range(n):
            px, py, pz = pts[i]
            nx, ny = miter_normals[i]

            if i > 0:
                cum_dist += math.hypot(px - pts[i - 1][0], py - pts[i - 1][1])

            vl = (px - nx * hw, py - ny * hw, pz + z_offset)
            vr = (px + nx * hw, py + ny * hw, pz + z_offset)

            vertices.append(vl)
            vertices.append(vr)
            normals.append((0.0, 0.0, 1.0))
            normals.append((0.0, 0.0, 1.0))
            colors.append(v_col)
            colors.append(v_col)

            # UV coordinates: U across width (0..1), V along corridor length
            v_coord = cum_dist / 4.0
            uvs.append((0.0, v_coord))
            uvs.append((1.0, v_coord))

            seg_verts.extend([vl, vr])

        # Stitch quad strips
        for i in range(n - 1):
            i0 = base_v_idx + i * 2
            i1 = base_v_idx + i * 2 + 1
            i2 = base_v_idx + (i + 1) * 2 + 1
            i3 = base_v_idx + (i + 1) * 2

            # Clockwise UE skyward triangles
            triangles.append((i0, i1, i2))
            triangles.append((i0, i2, i3))

            si0 = i * 2
            seg_tris.append((si0, si0 + 1, si0 + 2 + 1))
            seg_tris.append((si0, si0 + 2 + 1, si0 + 2))

        if seg_verts and seg_tris:
            walkable_surfaces.append((seg_verts, seg_tris))

    mesh_data = {
        "vertices": vertices,
        "triangles": triangles,
        "normals": normals,
        "uvs": uvs,
        "colors": colors,
    }
    return mesh_data, walkable_surfaces


# =============================================================================
# 5. 3D Mesh Synthesis: White Demarcation Line Markings
# =============================================================================

def generate_cycleway_demarcation_mesh(
    cycleways: Sequence[CyclewaySegment],
    line_width: float = 0.12,
    z_offset: float = 0.028,
) -> dict[str, Any]:
    """
    Synthesizes crisp white retroreflective demarcation lines separating cycle lanes from traffic:
      - Solid line for exclusive cycleways (0.12m width)
      - Dashed line (1.0m stripe / 1.0m gap) for advisory lanes
    """
    vertices: list[tuple[float, float, float]] = []
    triangles: list[tuple[int, int, int]] = []
    normals: list[tuple[float, float, float]] = []
    uvs: list[tuple[float, float]] = []

    hl = line_width * 0.5

    for cyc in cycleways:
        if cyc.lane_marking == "none":
            continue

        pts = cyc.nodes_carla
        n = len(pts)
        if n < 2:
            continue

        w = cyc.width_m
        hw = w * 0.5

        # Place line along the traffic-adjacent boundary (outer edge)
        # Offset along ribbon edge
        for i in range(n - 1):
            p0 = pts[i]
            p1 = pts[i + 1]

            dx = p1[0] - p0[0]
            dy = p1[1] - p0[1]
            seg_len = math.hypot(dx, dy)
            if seg_len < 1e-4:
                continue

            ux, uy = dx / seg_len, dy / seg_len
            nx, ny = -uy, ux

            # Shift to outer edge of cycleway
            sign = 1.0 if cyc.side == "left" else -1.0
            edge_p0 = (p0[0] + sign * nx * hw, p0[1] + sign * ny * hw, p0[2] + z_offset)
            edge_p1 = (p1[0] + sign * nx * hw, p1[1] + sign * ny * hw, p1[2] + z_offset)

            # Solid vs dashed handling
            if cyc.lane_marking == "dashed":
                dash_len = 1.0
                gap_len = 1.0
                period = dash_len + gap_len
                num_dashes = max(1, int(seg_len / period))

                for di in range(num_dashes):
                    d0 = di * period
                    d1 = min(seg_len, d0 + dash_len)
                    if d1 <= d0:
                        break

                    dp0 = (edge_p0[0] + ux * d0, edge_p0[1] + uy * d0, edge_p0[2])
                    dp1 = (edge_p0[0] + ux * d1, edge_p0[1] + uy * d1, edge_p1[2])

                    idx = len(vertices)
                    v0 = (dp0[0] - nx * hl, dp0[1] - ny * hl, dp0[2])
                    v1 = (dp0[0] + nx * hl, dp0[1] + ny * hl, dp0[2])
                    v2 = (dp1[0] + nx * hl, dp1[1] + ny * hl, dp1[2])
                    v3 = (dp1[0] - nx * hl, dp1[1] - ny * hl, dp1[2])

                    vertices.extend([v0, v1, v2, v3])
                    normals.extend([(0.0, 0.0, 1.0)] * 4)
                    uvs.extend([(0.0, 0.0), (1.0, 0.0), (1.0, 1.0), (0.0, 1.0)])
                    triangles.append((idx, idx + 1, idx + 2))
                    triangles.append((idx, idx + 2, idx + 3))
            else:
                # Solid continuous quad
                idx = len(vertices)
                v0 = (edge_p0[0] - nx * hl, edge_p0[1] - ny * hl, edge_p0[2])
                v1 = (edge_p0[0] + nx * hl, edge_p0[1] + ny * hl, edge_p0[2])
                v2 = (edge_p1[0] + nx * hl, edge_p1[1] + ny * hl, edge_p1[2])
                v3 = (edge_p1[0] - nx * hl, edge_p1[1] - ny * hl, edge_p1[2])

                vertices.extend([v0, v1, v2, v3])
                normals.extend([(0.0, 0.0, 1.0)] * 4)
                uvs.extend([(0.0, 0.0), (1.0, 0.0), (1.0, 1.0), (0.0, 1.0)])
                triangles.append((idx, idx + 1, idx + 2))
                triangles.append((idx, idx + 2, idx + 3))

    return {
        "vertices": vertices,
        "triangles": triangles,
        "normals": normals,
        "uvs": uvs,
    }


# =============================================================================
# 6. 3D Mesh Synthesis: Stencil Pictograms & Advance Stop Lines (Bike Boxes)
# =============================================================================

def generate_bike_stencil_geometry(
    cx: float,
    cy: float,
    cz: float,
    heading_deg: float,
    scale: float = 1.0,
) -> tuple[list[tuple[float, float, float]], list[tuple[int, int, int]]]:
    """
    Procedurally synthesizes an official white bicycle stencil pictogram (StVO 237).
    Constructs the two wheels, diamond frame, handlebars, and seat using geometric quads.
    """
    rad = math.radians(heading_deg)
    ux, uy = math.cos(rad), math.sin(rad)
    vx, vy = -uy, ux

    verts: list[tuple[float, float, float]] = []
    tris: list[tuple[int, int, int]] = []

    def add_line(p0: tuple[float, float], p1: tuple[float, float], width: float = 0.07):
        dx, dy = p1[0] - p0[0], p1[1] - p0[1]
        dist = math.hypot(dx, dy)
        if dist < 1e-4:
            return
        nx, ny = -dy / dist * (width * 0.5), dx / dist * (width * 0.5)
        idx = len(verts)
        verts.extend([
            (p0[0] - nx, p0[1] - ny, cz),
            (p0[0] + nx, p0[1] + ny, cz),
            (p1[0] + nx, p1[1] + ny, cz),
            (p1[0] - nx, p1[1] - ny, cz),
        ])
        tris.append((idx, idx + 1, idx + 2))
        tris.append((idx, idx + 2, idx + 3))

    def to_w(u_coord: float, v_coord: float) -> tuple[float, float]:
        return (
            cx + ux * u_coord + vx * v_coord,
            cy + uy * u_coord + vy * v_coord,
        )

    def add_ring(cu: float, cv: float, radius: float = 0.20 * scale, thickness: float = 0.05 * scale, segments: int = 16):
        for i in range(segments):
            a0 = 2.0 * math.pi * i / segments
            a1 = 2.0 * math.pi * (i + 1) / segments
            p0 = to_w(cu + math.cos(a0) * radius, cv + math.sin(a0) * radius)
            p1 = to_w(cu + math.cos(a1) * radius, cv + math.sin(a1) * radius)
            add_line(p0, p1, width=thickness)

    s = scale
    # 1. Wheels (16 segments each, proper circular geometry in bicycle plane)
    add_ring(-0.42 * s, 0.0, radius=0.20 * s, thickness=0.05 * s, segments=16)
    add_ring(+0.42 * s, 0.0, radius=0.20 * s, thickness=0.05 * s, segments=16)

    # 2. Diamond Frame & Saddle
    add_line(to_w(-0.42 * s, 0.0), to_w(-0.05 * s, -0.02 * s), width=0.05 * s)       # Chainstay
    add_line(to_w(-0.42 * s, 0.0), to_w(-0.22 * s, 0.26 * s), width=0.05 * s)        # Seatstay
    add_line(to_w(-0.05 * s, -0.02 * s), to_w(-0.22 * s, 0.36 * s), width=0.05 * s)   # Seat tube
    add_line(to_w(-0.28 * s, 0.36 * s), to_w(-0.16 * s, 0.36 * s), width=0.06 * s)   # Saddle horizontal bar
    add_line(to_w(-0.05 * s, -0.02 * s), to_w(0.26 * s, 0.26 * s), width=0.05 * s)    # Down tube
    add_line(to_w(-0.22 * s, 0.26 * s), to_w(0.26 * s, 0.26 * s), width=0.05 * s)    # Top tube
    add_line(to_w(0.42 * s, 0.0), to_w(0.26 * s, 0.26 * s), width=0.05 * s)           # Front fork

    # 3. Handlebars & Stem
    add_line(to_w(0.26 * s, 0.26 * s), to_w(0.26 * s, 0.40 * s), width=0.05 * s)      # Steerer stem
    add_line(to_w(0.20 * s, 0.40 * s), to_w(0.32 * s, 0.40 * s), width=0.06 * s)      # Handlebar crossbar

    return verts, tris


def generate_bike_stencils_and_boxes_mesh(
    cycleways: Sequence[CyclewaySegment],
    bike_boxes: Sequence[BikeBox],
    stencil_interval: float = 30.0,
    z_offset: float = 0.032,
) -> tuple[dict[str, Any], list[dict[str, Any]]]:
    """
    Places bicycle silhouette decals along cycleway tracks and generates
    advance stop line bike box perimeter markings.
    """
    vertices: list[tuple[float, float, float]] = []
    triangles: list[tuple[int, int, int]] = []
    normals: list[tuple[float, float, float]] = []
    uvs: list[tuple[float, float]] = []

    stencils_meta: list[dict[str, Any]] = []

    # 1. Stencils along cycleways
    for cyc in cycleways:
        pts = cyc.nodes_carla
        n = len(pts)
        if n < 2 or cyc.length_m < 10.0:
            continue

        cum_dist = 0.0
        next_stencil_dist = 6.0  # Place first stencil early in corridor

        for i in range(n - 1):
            p0 = pts[i]
            p1 = pts[i + 1]
            dx = p1[0] - p0[0]
            dy = p1[1] - p0[1]
            seg_len = math.hypot(dx, dy)
            if seg_len < 1e-4:
                continue

            heading_deg = math.degrees(math.atan2(dy, dx))

            while next_stencil_dist <= cum_dist + seg_len:
                t = (next_stencil_dist - cum_dist) / seg_len
                sx = p0[0] + dx * t
                sy = p0[1] + dy * t
                sz = p0[2] + z_offset

                # Generate stencil mesh
                s_verts, s_tris = generate_bike_stencil_geometry(sx, sy, sz, heading_deg, scale=1.1)
                idx = len(vertices)
                vertices.extend(s_verts)
                normals.extend([(0.0, 0.0, 1.0)] * len(s_verts))
                uvs.extend([(0.5, 0.5)] * len(s_verts))
                for st in s_tris:
                    triangles.append((idx + st[0], idx + st[1], idx + st[2]))

                stencils_meta.append({
                    "cycleway_id": cyc.segment_id,
                    "location": [round(sx, 3), round(sy, 3), round(sz, 3)],
                    "heading": round(heading_deg, 1),
                })

                next_stencil_dist += stencil_interval

            cum_dist += seg_len

    # 2. Advance Stop Lines (Bike Boxes)
    for bb in bike_boxes:
        bx, by, bz = bb.center_carla
        rad = math.radians(bb.heading_deg)
        ux, uy = math.cos(rad), math.sin(rad)
        vx, vy = -uy, ux

        hw = bb.width_m * 0.5
        hl = bb.length_m * 0.5
        z_mark = bz + z_offset

        # Stencil inside bike box
        s_verts, s_tris = generate_bike_stencil_geometry(bx, by, z_mark + 0.005, bb.heading_deg, scale=1.3)
        idx = len(vertices)
        vertices.extend(s_verts)
        normals.extend([(0.0, 0.0, 1.0)] * len(s_verts))
        uvs.extend([(0.5, 0.5)] * len(s_verts))
        for st in s_tris:
            triangles.append((idx + st[0], idx + st[1], idx + st[2]))

        # White border frame around bike box
        frame_lines = [
            ((bx - vx * hw - ux * hl, by - vy * hw - uy * hl), (bx + vx * hw - ux * hl, by + vy * hw - uy * hl)),
            ((bx - vx * hw + ux * hl, by - vy * hw + uy * hl), (bx + vx * hw + ux * hl, by + vy * hw + uy * hl)),
            ((bx - vx * hw - ux * hl, by - vy * hw - uy * hl), (bx - vx * hw + ux * hl, by - vy * hw + uy * hl)),
            ((bx + vx * hw - ux * hl, by + vy * hw - uy * hl), (bx + vx * hw + ux * hl, by + vy * hw + uy * hl)),
        ]
        line_w = 0.20
        h_line = line_w * 0.5

        for p0, p1 in frame_lines:
            dx, dy = p1[0] - p0[0], p1[1] - p0[1]
            dist = math.hypot(dx, dy)
            if dist < 1e-4:
                continue
            fx, fy = -dy / dist * h_line, dx / dist * h_line
            idx = len(vertices)
            vertices.extend([
                (p0[0] - fx, p0[1] - fy, z_mark),
                (p0[0] + fx, p0[1] + fy, z_mark),
                (p1[0] + fx, p1[1] + fy, z_mark),
                (p1[0] - fx, p1[1] - fy, z_mark),
            ])
            normals.extend([(0.0, 0.0, 1.0)] * 4)
            uvs.extend([(0.0, 0.0), (1.0, 0.0), (1.0, 1.0), (0.0, 1.0)])
            triangles.append((idx, idx + 1, idx + 2))
            triangles.append((idx, idx + 2, idx + 3))

    mesh_data = {
        "vertices": vertices,
        "triangles": triangles,
        "normals": normals,
        "uvs": uvs,
    }
    return mesh_data, stencils_meta


# =============================================================================
# 7. Clutter Placements: Parked Bicycles & Bike Racks
# =============================================================================

def generate_bike_parking_clutter(
    bike_racks: Sequence[BikeParkingRack],
    carla_map: carla.Map | None = None,
    min_clearance_m: float = 1.85,
) -> list[ClutterItem]:
    """Generates parked bicycle vehicle items from bicycle parking rack clusters."""
    items: list[ClutterItem] = []
    for rack in bike_racks:
        for bike in rack.parked_bikes:
            loc = tuple(bike["location"])
            rot = tuple(bike["rotation"])
            if carla_map is not None:
                c_loc = carla.Location(x=loc[0], y=loc[1], z=loc[2])
                wp = carla_map.get_waypoint(c_loc, project_to_road=False, lane_type=carla.LaneType.Driving)
                if wp is not None:
                    dist = wp.transform.location.distance(c_loc)
                    if dist < min_clearance_m:
                        continue
            items.append(
                ClutterItem(
                    category="parked_bicycle",
                    blueprint_id=bike["blueprint_id"],
                    location=loc,  # type: ignore[arg-type]
                    rotation=rot,  # type: ignore[arg-type]
                    source="osm_bicycle_parking",
                )
            )
    return items


# =============================================================================
# 8. Manifest Serialization & Auditing
# =============================================================================

def save_cycleways_manifest(
    cycleways: Sequence[CyclewaySegment],
    bike_boxes: Sequence[BikeBox],
    bike_racks: Sequence[BikeParkingRack],
    output_path: str | Path,
) -> None:
    """Saves bicycle infrastructure inventory to Placements/cycleways.json."""
    data = {
        "version": "1.0",
        "counts": {
            "total_cycleways": len(cycleways),
            "total_lane_km": round(sum(c.length_m for c in cycleways) / 1000.0, 3),
            "total_bike_boxes": len(bike_boxes),
            "total_bike_racks": len(bike_racks),
            "total_parked_bikes": sum(r.num_parked_bikes for r in bike_racks),
        },
        "cycleways": [asdict(c) for c in cycleways],
        "bike_boxes": [asdict(b) for b in bike_boxes],
        "bike_racks": [asdict(r) for r in bike_racks],
    }
    p = Path(output_path)
    p.parent.mkdir(parents=True, exist_ok=True)
    p.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")


def load_cycleways_manifest(manifest_path: str | Path) -> dict[str, Any]:
    """Loads Placements/cycleways.json."""
    p = Path(manifest_path)
    return json.loads(p.read_text(encoding="utf-8"))


def audit_cycleway_metrics(
    cycleways: Sequence[CyclewaySegment],
    bike_boxes: Sequence[BikeBox],
    bike_racks: Sequence[BikeParkingRack],
) -> dict[str, Any]:
    """Computes summary metrics for reporting and verification."""
    total_km = sum(c.length_m for c in cycleways) / 1000.0
    color_breakdown: dict[str, int] = {}
    type_breakdown: dict[str, int] = {}

    for c in cycleways:
        color_breakdown[c.surface_color] = color_breakdown.get(c.surface_color, 0) + 1
        type_breakdown[c.cycleway_type] = type_breakdown.get(c.cycleway_type, 0) + 1

    return {
        "total_segments": len(cycleways),
        "total_lane_km": round(total_km, 3),
        "color_breakdown": color_breakdown,
        "type_breakdown": type_breakdown,
        "bike_boxes": len(bike_boxes),
        "bike_racks": len(bike_racks),
        "parked_bikes": sum(r.num_parked_bikes for r in bike_racks),
    }

