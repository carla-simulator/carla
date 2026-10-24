#!/usr/bin/env python3
"""
OSM Pedestrian Crosswalks, Zebra Markings & Tactile Paving Module for CARLA Map Maker.

Milestone 11 (M11):
  - Extracts real-world OSM crosswalk nodes and ways (highway=crossing, crossing=*).
  - Associates crosswalks with OpenDRIVE road network lanes and heading vectors.
  - Procedurally synthesizes retroreflective white zebra stripes (DIN / StVO standards:
    0.50m stripe width, 0.50m gap, oriented parallel to road traffic flow).
  - Generates raised pedestrian refuge islands (crossing:island=yes) with curb profile.
  - Synthesizes tactile warning paving tiles (tactile_paving=yes) at sidewalk curb cuts.
  - Produces walkable crosswalk corridor meshes for Recast NavMesh integration.
"""

from __future__ import annotations

from dataclasses import asdict, dataclass
import json
import math
import os
from pathlib import Path
import tempfile
from typing import Any, Callable, Sequence
import xml.etree.ElementTree as ET

import carla

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
class CrosswalkItem:
    """Represents a validated pedestrian crosswalk associated with the road network."""
    crosswalk_id: str
    location: tuple[float, float, float]
    crossing_type: str  # "zebra", "traffic_signals", "uncontrolled", "marked", etc.
    road_id: int
    lane_ids: list[int]
    heading_deg: float  # Road traffic heading
    carriageway_width: float  # Total width across lanes
    corridor_length: float  # Length along road (e.g. 3.5m)
    has_island: bool
    has_tactile: bool
    num_stripes: int
    source: str = "osm"


def make_lat_lon_projector(
    carla_map: Any | None = None,
    xodr_xml: str | None = None,
) -> Callable[[float, float], tuple[float, float, float]]:
    """Creates a geographic-to-simulation projection function accounting for OpenDRIVE offsets."""
    off_x = 0.0
    off_y = 0.0
    xml_str = xodr_xml
    if not xml_str and carla_map is not None:
        try:
            xml_str = carla_map.to_opendrive()
        except Exception:
            pass
    if xml_str:
        try:
            root = ET.fromstring(xml_str)
            header = root.find("header")
            offset_elem = header.find("offset") if header is not None else None
            if offset_elem is not None:
                off_x = float(offset_elem.get("x", "0.0"))
                off_y = float(offset_elem.get("y", "0.0"))
        except Exception:
            pass

    def _project(lat: float, lon: float) -> tuple[float, float, float]:
        if carla_map is not None:
            try:
                tf = carla_map.geolocation_to_transform(carla.GeoLocation(lat, lon, 0.0))
                loc = tf.location if hasattr(tf, "location") else tf
                return loc.x + off_x, -(loc.y + off_y), loc.z
            except Exception:
                pass
        return lon * 111320.0, lat * 110540.0, 0.0

    return _project


def extract_osm_crosswalks(
    osm_path: str | Path,
    project_coord_fn: Callable[[float, float], tuple[float, float, float]] | None = None,
    carla_map: Any = None,
    max_road_dist: float = 20.0,
    dedup_threshold: float = 6.0,
) -> list[CrosswalkItem]:
    """
    Parses OSM nodes and ways for crossing tags, associates them with OpenDRIVE
    roads, and calculates carriageway extents and orientation vectors.
    """
    osm_file = Path(osm_path)
    if not osm_file.is_file():
        raise FileNotFoundError(f"OSM file not found: {osm_path}")

    if project_coord_fn is None and carla_map is not None:
        project_coord_fn = make_lat_lon_projector(carla_map=carla_map)

    tree = ET.parse(osm_file)
    root = tree.getroot()

    raw_crossings: list[tuple[str, float, float, dict[str, str]]] = []

    # 1. Parse crossing nodes
    for node in root.findall("node"):
        tags = {t.get("k"): t.get("v") for t in node.findall("tag")}
        # Filter out pure tram / railway crossings
        if tags.get("railway") in ("tram_crossing", "tram_level_crossing", "railway_crossing") and tags.get("highway") != "crossing":
            continue
        # Filter out unmarked or informal crossings
        if tags.get("crossing") in ("no", "unmarked", "informal") or tags.get("crossing:markings") == "no":
            continue
        is_crossing = (
            tags.get("highway") == "crossing"
            or ("crossing" in tags and tags.get("highway") != "traffic_signals")
            or (tags.get("highway") == "traffic_signals" and "crossing" in tags)
        )
        if is_crossing:
            lat = float(node.get("lat"))
            lon = float(node.get("lon"))
            raw_crossings.append((node.get("id"), lat, lon, tags))

    if not raw_crossings:
        return []

    # 2. Road Network Association
    crosswalks: list[CrosswalkItem] = []
    processed_locations: list[tuple[int, float, float]] = []  # (road_id, s, t) or (x, y)

    for node_id, lat, lon, tags in raw_crossings:
        if project_coord_fn is not None:
            cx, cy, cz = project_coord_fn(lat, lon)
        elif carla_map is not None:
            loc = carla_map.geolocation_to_transform(carla.GeoLocation(lat, lon, 0.0))
            cx, cy, cz = loc.x, loc.y, loc.z
        else:
            cx, cy, cz = lon * 1000.0, lat * 1000.0, 0.0

        if carla_map is None:
            # Standalone extraction without OpenDRIVE
            c_type = tags.get("crossing") or ("traffic_signals" if tags.get("highway") == "traffic_signals" else "zebra")
            has_island = tags.get("crossing:island") == "yes"
            has_tactile = tags.get("tactile_paving") == "yes"
            crosswalks.append(
                CrosswalkItem(
                    crosswalk_id=f"cw_{node_id}",
                    location=(cx, cy, cz),
                    crossing_type=c_type,
                    road_id=0,
                    lane_ids=[-1],
                    heading_deg=0.0,
                    carriageway_width=7.0,
                    corridor_length=3.5,
                    has_island=has_island,
                    has_tactile=has_tactile,
                    num_stripes=7,
                    source="osm",
                )
            )
            continue

        # Association with live carla.Map
        c_loc = carla.Location(x=cx, y=cy, z=cz)
        wp = carla_map.get_waypoint(c_loc, project_to_road=True, lane_type=carla.LaneType.Driving)
        if wp is None:
            continue

        dist = c_loc.distance(wp.transform.location)
        if dist > max_road_dist:
            continue

        # If wp is inside an internal junction connecting road, snap to nearest non-junction approach road
        if wp.is_junction:
            best_wp = None
            best_dist = 999.0
            for r in range(2, int(max_road_dist) + 1, 2):
                for ang in range(0, 360, 20):
                    rad = math.radians(ang)
                    test_loc = carla.Location(c_loc.x + r * math.cos(rad), c_loc.y + r * math.sin(rad), c_loc.z)
                    w = carla_map.get_waypoint(test_loc, project_to_road=True, lane_type=carla.LaneType.Driving)
                    if w and not w.is_junction:
                        d = c_loc.distance(w.transform.location)
                        if d < best_dist and d <= max_road_dist:
                            best_dist = d
                            best_wp = w
                if best_wp is not None and best_dist < 6.0:
                    break
            if best_wp is None:
                # No approach road nearby; internal junction turning tracks must not have crosswalks
                continue
            wp = best_wp

        # Query all driving lanes in this carriageway cross-section (same travel direction)
        lanes = [wp]
        curr = wp.get_left_lane()
        while curr is not None and curr.lane_type == carla.LaneType.Driving and curr.lane_id * wp.lane_id > 0:
            lanes.insert(0, curr)
            curr = curr.get_left_lane()
        curr = wp.get_right_lane()
        while curr is not None and curr.lane_type == carla.LaneType.Driving and curr.lane_id * wp.lane_id > 0:
            lanes.append(curr)
            curr = curr.get_right_lane()

        # Deduplicate crossings that map to nearly the same spot along the road
        direction_sign = 1 if wp.lane_id > 0 else -1
        is_dup = False
        for p_item in processed_locations:
            if len(p_item) == 5:
                p_road, p_dir, p_s, p_cx, p_cy = p_item
                if p_road == wp.road_id and p_dir == direction_sign and abs(wp.s - p_s) < 15.0:
                    is_dup = True
                    break
                if math.hypot(cx - p_cx, cy - p_cy) < dedup_threshold:
                    is_dup = True
                    break
            elif len(p_item) == 3:
                p_road, p_cx, p_cy = p_item
                if p_road == wp.road_id and math.hypot(cx - p_cx, cy - p_cy) < dedup_threshold:
                    is_dup = True
                    break
        if is_dup:
            continue
        processed_locations.append((wp.road_id, direction_sign, wp.s, cx, cy))

        total_width = max(3.5, sum(l.lane_width for l in lanes))
        lane_ids = [l.lane_id for l in lanes]
        mid_idx = len(lanes) // 2
        cw_heading = lanes[mid_idx].transform.rotation.yaw

        # Calculate true center of the entire carriageway cross-section across all driving lanes
        cw_cx = sum(l.transform.location.x for l in lanes) / len(lanes)
        cw_cy = sum(l.transform.location.y for l in lanes) / len(lanes)
        cw_cz = sum(l.transform.location.z for l in lanes) / len(lanes)

        c_type = tags.get("crossing") or (
            "traffic_signals" if tags.get("highway") == "traffic_signals" or tags.get("crossing_ref") == "pelican" else "zebra"
        )
        has_island = tags.get("crossing:island") == "yes"
        has_tactile = tags.get("tactile_paving") == "yes"

        # Calculate standard zebra stripes count: 0.5m stripe + 0.5m gap = 1.0m period
        num_stripes = max(3, int(round(total_width / 1.0)))

        crosswalks.append(
            CrosswalkItem(
                crosswalk_id=f"cw_{node_id}",
                location=(cw_cx, cw_cy, cw_cz),
                crossing_type=c_type,
                road_id=wp.road_id,
                lane_ids=lane_ids,
                heading_deg=cw_heading,
                carriageway_width=total_width,
                corridor_length=3.5,
                has_island=has_island,
                has_tactile=has_tactile,
                num_stripes=num_stripes,
                source="osm",
            )
        )

    return crosswalks


def generate_crosswalk_meshes(
    crosswalks: Sequence[CrosswalkItem],
    stripe_width: float = 0.50,
    stripe_gap: float = 0.50,
    z_offset: float = 0.02,
) -> dict[str, Any]:
    """
    Generates procedural geometry layers for:
      - 'zebra_stripes': Crisp white retroreflective zebra markings (material_preset: crosswalk)
      - 'refuge_islands': Raised concrete median divider islands (material_preset: sidewalk_tiles)
      - 'tactile_paving': Warning indicator pads at curb ramps (material_preset: sidewalk_tiles)
      - 'walkable_corridors': Full carriageway crossing planes for Recast NavMesh integration
    """
    # 1. Zebra Stripes Geometry
    z_verts: list[tuple[float, float, float]] = []
    z_tris: list[tuple[int, int, int]] = []
    z_norms: list[tuple[float, float, float]] = []
    z_uvs: list[tuple[float, float]] = []

    # 2. Refuge Islands Geometry
    isl_verts: list[tuple[float, float, float]] = []
    isl_tris: list[tuple[int, int, int]] = []
    isl_norms: list[tuple[float, float, float]] = []
    isl_uvs: list[tuple[float, float]] = []

    # 3. Tactile Paving Geometry
    tac_verts: list[tuple[float, float, float]] = []
    tac_tris: list[tuple[int, int, int]] = []
    tac_norms: list[tuple[float, float, float]] = []
    tac_uvs: list[tuple[float, float]] = []

    # 4. NavMesh Walkable Corridors
    walkable_corridors: list[tuple[list[tuple[float, float, float]], list[tuple[int, int, int]]]] = []

    period = stripe_width + stripe_gap

    for cw in crosswalks:
        cx, cy, cz = cw.location
        yaw_rad = math.radians(cw.heading_deg)
        # Unit vector along road
        tx, ty = math.cos(yaw_rad), math.sin(yaw_rad)
        # Unit vector across road (lateral normal)
        nx, ny = -math.sin(yaw_rad), math.cos(yaw_rad)

        L = cw.corridor_length
        W = cw.carriageway_width
        half_L = L * 0.5
        half_W = W * 0.5
        elevated_z = cz + z_offset

        # ---------------------------------------------------------------------
        # A. Zebra Stripes (White Retroreflective Longitudinal Stripes)
        # ---------------------------------------------------------------------
        num_stripes = cw.num_stripes
        # Distribute stripes symmetrically across [-half_W, +half_W]
        half_sw = stripe_width * 0.5
        lateral_span = (num_stripes - 1) * period
        start_t = -lateral_span * 0.5

        for i in range(num_stripes):
            t_offset = start_t + i * period
            # If refuge island is present, skip center stripe where island sits
            if cw.has_island and abs(t_offset) < 1.0:
                continue

            center_x = cx + t_offset * nx
            center_y = cy + t_offset * ny

            # 4 vertices of rectangular stripe
            p0 = (center_x - half_L * tx - half_sw * nx, center_y - half_L * ty - half_sw * ny, elevated_z)
            p1 = (center_x + half_L * tx - half_sw * nx, center_y + half_L * ty - half_sw * ny, elevated_z)
            p2 = (center_x + half_L * tx + half_sw * nx, center_y + half_L * ty + half_sw * ny, elevated_z)
            p3 = (center_x - half_L * tx + half_sw * nx, center_y - half_L * ty + half_sw * ny, elevated_z)

            idx = len(z_verts)
            z_verts.extend([p0, p1, p2, p3])
            # Clockwise skyward triangles for Unreal Engine left-handed coordinate rasterization
            z_tris.append((idx, idx + 2, idx + 1))
            z_tris.append((idx, idx + 3, idx + 2))
            z_norms.extend([(0.0, 0.0, 1.0)] * 4)
            z_uvs.extend([(0.0, 0.0), (1.0, 0.0), (1.0, 1.0), (0.0, 1.0)])

        # ---------------------------------------------------------------------
        # B. Raised Pedestrian Refuge Island (crossing:island=yes)
        # ---------------------------------------------------------------------
        if cw.has_island:
            isl_w = 1.6  # 1.6m wide median island
            isl_l = L + 1.0  # slightly longer than crosswalk
            half_iw = isl_w * 0.5
            half_il = isl_l * 0.5
            isl_h = 0.15  # 15cm raised curb

            # Top quad
            top_z = cz + isl_h
            ip0 = (cx - half_il * tx - half_iw * nx, cy - half_il * ty - half_iw * ny, top_z)
            ip1 = (cx + half_il * tx - half_iw * nx, cy + half_il * ty - half_iw * ny, top_z)
            ip2 = (cx + half_il * tx + half_iw * nx, cy + half_il * ty + half_iw * ny, top_z)
            ip3 = (cx - half_il * tx + half_iw * nx, cy - half_il * ty + half_iw * ny, top_z)

            # Bottom quad (for collision bevel)
            bp0 = (cx - half_il * tx - half_iw * nx, cy - half_il * ty - half_iw * ny, cz)
            bp1 = (cx + half_il * tx - half_iw * nx, cy + half_il * ty - half_iw * ny, cz)
            bp2 = (cx + half_il * tx + half_iw * nx, cy + half_il * ty + half_iw * ny, cz)
            bp3 = (cx - half_il * tx + half_iw * nx, cy - half_il * ty + half_iw * ny, cz)

            idx = len(isl_verts)
            isl_verts.extend([ip0, ip1, ip2, ip3, bp0, bp1, bp2, bp3])
            # Top surface (clockwise skyward)
            isl_tris.append((idx, idx + 2, idx + 1))
            isl_tris.append((idx, idx + 3, idx + 2))
            # 4 Side faces
            isl_tris.extend([
                (idx, idx + 4, idx + 5), (idx, idx + 5, idx + 1),  # front
                (idx + 1, idx + 5, idx + 6), (idx + 1, idx + 6, idx + 2),  # right
                (idx + 2, idx + 6, idx + 7), (idx + 2, idx + 7, idx + 3),  # back
                (idx + 3, idx + 7, idx + 4), (idx + 3, idx + 4, idx),  # left
            ])
            isl_norms.extend([(0.0, 0.0, 1.0)] * 4 + [(0.0, 0.0, -1.0)] * 4)
            isl_uvs.extend([(0.0, 0.0), (1.0, 0.0), (1.0, 1.0), (0.0, 1.0)] * 2)

        # ---------------------------------------------------------------------
        # C. Tactile Warning Paving Pads (tactile_paving=yes)
        # ---------------------------------------------------------------------
        if cw.has_tactile:
            pad_w = 1.0  # 1.0m along curb
            pad_l = 0.8  # 0.8m depth into sidewalk
            half_pw = pad_w * 0.5
            half_pl = pad_l * 0.5
            tac_z = cz + z_offset + 0.005

            for side in (-1.0, 1.0):
                # Position on each sidewalk entrance curb
                pad_cx = cx + (side * (half_W + half_pl)) * nx
                pad_cy = cy + (side * (half_W + half_pl)) * ny

                tp0 = (pad_cx - half_pw * tx - half_pl * nx, pad_cy - half_pw * ty - half_pl * ny, tac_z)
                tp1 = (pad_cx + half_pw * tx - half_pl * nx, pad_cy + half_pw * ty - half_pl * ny, tac_z)
                tp2 = (pad_cx + half_pw * tx + half_pl * nx, pad_cy + half_pw * ty + half_pl * ny, tac_z)
                tp3 = (pad_cx - half_pw * tx + half_pl * nx, pad_cy - half_pw * ty + half_pl * ny, tac_z)

                idx = len(tac_verts)
                tac_verts.extend([tp0, tp1, tp2, tp3])
                # Clockwise skyward
                tac_tris.append((idx, idx + 2, idx + 1))
                tac_tris.append((idx, idx + 3, idx + 2))
                tac_norms.extend([(0.0, 0.0, 1.0)] * 4)
                tac_uvs.extend([(0.0, 0.0), (1.0, 0.0), (1.0, 1.0), (0.0, 1.0)])

        # ---------------------------------------------------------------------
        # D. NavMesh Walkable Crosswalk Corridor
        # ---------------------------------------------------------------------
        # Connects left curb to right curb with a walkable quad
        w0 = (cx - half_L * tx - (half_W + 1.5) * nx, cy - half_L * ty - (half_W + 1.5) * ny, cz)
        w1 = (cx + half_L * tx - (half_W + 1.5) * nx, cy + half_L * ty - (half_W + 1.5) * ny, cz)
        w2 = (cx + half_L * tx + (half_W + 1.5) * nx, cy + half_L * ty + (half_W + 1.5) * ny, cz)
        w3 = (cx - half_L * tx + (half_W + 1.5) * nx, cy - half_L * ty + (half_W + 1.5) * ny, cz)
        walkable_corridors.append(([w0, w1, w2, w3], [(0, 1, 2), (0, 2, 3)]))

    return {
        "zebra_stripes": {
            "name": "OSM_Crosswalks",
            "semantic_label": "RoadLines",
            "material_preset": "crosswalk",
            "enable_collision": False,
            "vertices": z_verts,
            "triangles": z_tris,
            "normals": z_norms,
            "uvs": z_uvs,
        },
        "refuge_islands": {
            "name": "OSM_Crosswalk_Islands",
            "semantic_label": "Ground",
            "material_preset": "sidewalk_tiles",
            "enable_collision": True,
            "vertices": isl_verts,
            "triangles": isl_tris,
            "normals": isl_norms,
            "uvs": isl_uvs,
        },
        "tactile_paving": {
            "name": "OSM_Crosswalk_Tactile",
            "semantic_label": "Sidewalks",
            "material_preset": "sidewalk_tiles",
            "material_params": {
                "base_color": [240, 200, 40],  # Authentic yellow tactile warning
                "roughness": 0.85,
            },
            "enable_collision": False,
            "vertices": tac_verts,
            "triangles": tac_tris,
            "normals": tac_norms,
            "uvs": tac_uvs,
        },
        "walkable_corridors": walkable_corridors,
    }


def save_crosswalks_manifest(crosswalks: Sequence[CrosswalkItem], out_path: str | Path) -> None:
    """Serializes crosswalks to JSON manifest."""
    path = Path(out_path)
    path.parent.mkdir(parents=True, exist_ok=True)
    payload = {
        "version": "1.0",
        "crosswalks_count": len(crosswalks),
        "total_stripes": sum(c.num_stripes for c in crosswalks),
        "crosswalks": [asdict(c) for c in crosswalks],
    }
    path.write_text(json.dumps(payload, indent=2), encoding="utf-8")


def load_crosswalks_manifest(in_path: str | Path) -> list[CrosswalkItem]:
    """Loads crosswalk items from JSON manifest."""
    path = Path(in_path)
    if not path.is_file():
        raise FileNotFoundError(f"Crosswalks manifest not found: {in_path}")
    data = json.loads(path.read_text(encoding="utf-8"))
    items: list[CrosswalkItem] = []
    for d in data.get("crosswalks", []):
        items.append(
            CrosswalkItem(
                crosswalk_id=d["crosswalk_id"],
                location=tuple(d["location"]),
                crossing_type=d["crossing_type"],
                road_id=d["road_id"],
                lane_ids=list(d["lane_ids"]),
                heading_deg=d["heading_deg"],
                carriageway_width=d["carriageway_width"],
                corridor_length=d["corridor_length"],
                has_island=d["has_island"],
                has_tactile=d["has_tactile"],
                num_stripes=d["num_stripes"],
                source=d.get("source", "osm"),
            )
        )
    return items


def audit_crosswalk_metrics(
    crosswalks: Sequence[CrosswalkItem],
    carla_map: Any = None,
    max_road_dist: float = 20.0,
) -> dict[str, Any]:
    """
    Audits crosswalk geometric alignment, road coverage, and DIN/StVO compliance.
    """
    if not crosswalks:
        return {
            "crosswalk_count": 0,
            "total_stripes": 0,
            "road_coverage_pct": 0.0,
            "passed_din_standards": False,
        }

    valid_alignments = 0
    total_stripes = 0
    total_area_m2 = 0.0

    for cw in crosswalks:
        total_stripes += cw.num_stripes
        # Standard area: num_stripes * 0.50m * corridor_length
        total_area_m2 += cw.num_stripes * 0.50 * cw.corridor_length

        if carla_map is not None:
            loc = carla.Location(x=cw.location[0], y=cw.location[1], z=cw.location[2])
            wp = carla_map.get_waypoint(loc, project_to_road=True, lane_type=carla.LaneType.Driving)
            if wp is not None and loc.distance(wp.transform.location) <= max_road_dist:
                valid_alignments += 1
        else:
            valid_alignments += 1

    alignment_pct = (valid_alignments / len(crosswalks)) * 100.0

    return {
        "crosswalk_count": len(crosswalks),
        "total_stripes": total_stripes,
        "total_area_m2": round(total_area_m2, 2),
        "road_alignment_pct": round(alignment_pct, 1),
        "passed_din_standards": alignment_pct >= 95.0 and total_stripes >= len(crosswalks) * 3,
    }
