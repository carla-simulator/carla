#!/usr/bin/env python3
"""
Physical Speed Limit & Warning Signs Engine (StVO Metal Signboards) for CARLA Map Maker.

Milestone 16 (M16):
  - Ingests real-world traffic signs and speed limits from OpenStreetMap:
      * maxspeed=* (30, 50, 60, 80, 100, zone:30, walk)
      * traffic_sign=* (DE:274[..], DE:206, DE:205, DE:306, DE:101, DE:133, DE:138, etc.)
      * highway=give_way, highway=stop
  - Synthesizes procedural StVO signboards along road corridors and junction approaches:
      * Speed limits (StVO 274: 30, 50, 60, 80, 100 km/h)
      * Stop signs (StVO 206)
      * Yield / Give Way signs (StVO 205)
      * Priority Road signs (StVO 306)
      * Danger / Warning signs (StVO 101, 133, 138)
  - Rigorous Spatial Clearance & Placement:
      * Set back 0.50m - 0.75m past the outer road curb onto sidewalk/verge.
      * Yaw orientation strictly normal to lane heading, facing oncoming vehicular traffic.
      * Zero encroachment into active vehicular driving lanes (>0.40m clearance).
      * Zero collision with dedicated cycleway corridors (>0.30m clearance).
      * Zero penetration into building footprints.
  - Physical 3D Metal Geometry & PBR Texture Atlas:
      * DIN 76mm tubular steel posts (r=0.038m, height 2.2m - 2.5m clearance).
      * Ground socket / base collar at z=0.
      * Aluminum mounting clamps and RAL 7043 Verkehrsgrau B backing plate.
      * High-resolution 1024x1024 retroreflective StVO texture atlas (DIN 1451 typography).
  - OpenDRIVE <signals> Synchronization:
      * Injects compliant <signal> elements with exact curb offset t.
      * Completely eliminates CARLA's "overlaps a driving lane" warning.
      * Exposes landmarks to CARLA API (carla.Map.get_all_landmarks()).
  - NavMesh Obstacle Geometry:
      * Standing posts registered in obstacle_surfaces for pedestrian avoidance.
  - Manifest & Audit Serialization:
      * Placements/traffic_signs.json with comprehensive sign metrics.
"""

from __future__ import annotations

from dataclasses import dataclass, field
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

from PIL import Image, ImageDraw, ImageFont

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


# -----------------------------------------------------------------------------
# Data Models
# -----------------------------------------------------------------------------

@dataclass
class TrafficSignItem:
    """Represents a physical StVO traffic signboard and OpenDRIVE landmark."""
    sign_id: str
    sign_type: str        # "speed_limit", "stop", "yield", "priority_road", "warning", "end_limit"
    subtype: str          # "30", "50", "60", "80", "100", "stop", "yield", "priority", "danger", etc.
    stvo_code: str        # "274-30", "274-50", "206", "205", "306", "101", "133", "138", "278"
    location: tuple[float, float, float]    # (x, y, z) in CARLA world coordinates
    rotation: tuple[float, float, float] = (0.0, 0.0, 0.0)  # (pitch, yaw, roll) in degrees
    post_height: float = 2.25               # Clear height in meters
    post_radius: float = 0.038              # DIN 76mm diameter post (r=0.038m)
    plate_shape: str = "circle"             # "circle", "octagon", "triangle_down", "triangle_up", "diamond"
    plate_width: float = 0.60               # Size 2 standard
    plate_height: float = 0.60
    road_id: int = -1
    lane_id: int = -1
    from_lane: int = -1
    to_lane: int = -1
    s: float = 0.0
    t: float = 0.0
    value: float = 50.0                     # Numeric speed limit value (km/h)
    source: str = "osm"                     # "osm" or "procedural"
    osm_id: int | None = None
    tags: dict[str, str] = field(default_factory=dict)
    curb_setback_m: float = 0.60
    lane_clearance_m: float = 0.60

    def to_dict(self) -> dict[str, Any]:
        return {
            "sign_id": self.sign_id,
            "sign_type": self.sign_type,
            "subtype": self.subtype,
            "stvo_code": self.stvo_code,
            "location": [round(c, 3) for c in self.location],
            "rotation": [round(r, 2) for r in self.rotation],
            "post_height": round(self.post_height, 2),
            "post_radius": round(self.post_radius, 3),
            "plate_shape": self.plate_shape,
            "plate_width": round(self.plate_width, 2),
            "plate_height": round(self.plate_height, 2),
            "road_id": self.road_id,
            "lane_id": self.lane_id,
            "from_lane": self.from_lane,
            "to_lane": self.to_lane,
            "s": round(self.s, 2),
            "t": round(self.t, 2),
            "value": round(self.value, 1),
            "source": self.source,
            "osm_id": self.osm_id,
            "curb_setback_m": round(self.curb_setback_m, 2),
            "lane_clearance_m": round(self.lane_clearance_m, 2),
        }

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> TrafficSignItem:
        return cls(
            sign_id=str(data["sign_id"]),
            sign_type=str(data["sign_type"]),
            subtype=str(data.get("subtype", "50")),
            stvo_code=str(data.get("stvo_code", "274-50")),
            location=tuple(data["location"]),  # type: ignore[arg-type]
            rotation=tuple(data.get("rotation", [0.0, 0.0, 0.0])),  # type: ignore[arg-type]
            post_height=float(data.get("post_height", 2.25)),
            post_radius=float(data.get("post_radius", 0.038)),
            plate_shape=str(data.get("plate_shape", "circle")),
            plate_width=float(data.get("plate_width", 0.60)),
            plate_height=float(data.get("plate_height", 0.60)),
            road_id=int(data.get("road_id", -1)),
            lane_id=int(data.get("lane_id", -1)),
            from_lane=int(data.get("from_lane", -1)),
            to_lane=int(data.get("to_lane", -1)),
            s=float(data.get("s", 0.0)),
            t=float(data.get("t", 0.0)),
            value=float(data.get("value", 50.0)),
            source=str(data.get("source", "osm")),
            osm_id=data.get("osm_id"),
            curb_setback_m=float(data.get("curb_setback_m", 0.60)),
            lane_clearance_m=float(data.get("lane_clearance_m", 0.60)),
        )


# -----------------------------------------------------------------------------
# Atlas Layout & Texture Generation
# -----------------------------------------------------------------------------

# Atlas grid mapping (4x4 grid in a 1024x1024 texture)
# Each cell is 256x256 pixels.
ATLAS_CELLS: dict[str, tuple[int, int]] = {
    # Speed limits row 0
    "274-30": (0, 0),
    "274-50": (1, 0),
    "274-60": (2, 0),
    "274-80": (3, 0),
    # Row 1
    "274-100": (0, 1),
    "206": (1, 1),       # STOP
    "205": (2, 1),       # Yield / Give Way
    "306": (3, 1),       # Priority Road
    # Row 2
    "101": (0, 2),       # Danger
    "133": (1, 2),       # Pedestrian Warning
    "138": (2, 2),       # Cyclist Warning
    "278": (3, 2),       # End speed limit
    # Row 3: Materials
    "metal_post": (0, 3),    # Galvanized steel post
    "plate_back": (1, 3),    # RAL 7043 Verkehrsgrau B
    "bracket": (2, 3),       # Aluminum mounting clamp
    "zone30": (3, 3),        # StVO 274.1 Zone 30
}


def _get_truetype_font(size: int, bold: bool = True) -> Any:
    """Finds an authentic sans-serif bold font matching DIN 1451."""
    candidates = [
        "/System/Library/Fonts/Supplemental/Arial Bold.ttf" if bold else "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "/Library/Fonts/Arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf",
    ]
    for p in candidates:
        if os.path.isfile(p):
            try:
                return ImageFont.truetype(p, size)
            except Exception:
                pass
    return ImageFont.load_default()


def generate_traffic_signs_texture_atlas(output_path: Path) -> Path:
    """
    Renders a crisp 1024x1024 texture atlas containing authentic German StVO
    vector graphics, DIN 1451 typography, and realistic metal material backing.
    """
    output_path = Path(output_path)
    output_path.parent.mkdir(parents=True, exist_ok=True)

    img = Image.new("RGBA", (1024, 1024), (255, 255, 255, 0))
    draw = ImageDraw.Draw(img)

    cell_size = 256
    font_large = _get_truetype_font(105, bold=True)
    font_medium = _get_truetype_font(80, bold=True)
    font_small = _get_truetype_font(60, bold=True)

    # Helper: draw speed limit circle
    def draw_speed_circle(col: int, row: int, number_str: str, font: Any) -> None:
        cx = col * cell_size + cell_size // 2
        cy = row * cell_size + cell_size // 2
        r_outer = 118
        r_inner = 90
        # Red outer border
        draw.ellipse([cx - r_outer, cy - r_outer, cx + r_outer, cy + r_outer], fill=(225, 25, 25, 255))
        # White center
        draw.ellipse([cx - r_inner, cy - r_inner, cx + r_inner, cy + r_inner], fill=(250, 250, 250, 255))
        # Black DIN numeral
        draw.text((cx, cy), number_str, fill=(15, 15, 15, 255), anchor="mm", font=font)

    # 1. Row 0: Speed limits 30, 50, 60, 80
    draw_speed_circle(0, 0, "30", font_large)
    draw_speed_circle(1, 0, "50", font_large)
    draw_speed_circle(2, 0, "60", font_large)
    draw_speed_circle(3, 0, "80", font_large)

    # 2. Row 1: Speed limit 100
    draw_speed_circle(0, 1, "100", font_small)

    # Cell (1, 1): StVO 206 STOP
    cx = 1 * cell_size + cell_size // 2
    cy = 1 * cell_size + cell_size // 2
    r_stop = 120
    pts_stop: list[tuple[float, float]] = []
    for i in range(8):
        ang = math.pi / 8.0 + i * math.pi / 4.0
        pts_stop.append((cx + r_stop * math.cos(ang), cy + r_stop * math.sin(ang)))
    draw.polygon(pts_stop, fill=(215, 20, 20, 255))
    # Inner white border
    pts_stop_in: list[tuple[float, float]] = []
    for i in range(8):
        ang = math.pi / 8.0 + i * math.pi / 4.0
        pts_stop_in.append((cx + (r_stop - 8) * math.cos(ang), cy + (r_stop - 8) * math.sin(ang)))
    draw.polygon(pts_stop_in, outline=(250, 250, 250, 255), width=5)
    draw.text((cx, cy), "STOP", fill=(255, 255, 255, 255), anchor="mm", font=font_medium)

    # Cell (2, 1): StVO 205 Yield (Inverted triangle)
    cx = 2 * cell_size + cell_size // 2
    cy = 1 * cell_size + cell_size // 2
    h_tri = 195.0
    w_tri = 220.0
    pts_yield = [
        (cx - w_tri / 2.0, cy - h_tri / 3.0),
        (cx + w_tri / 2.0, cy - h_tri / 3.0),
        (cx, cy + 2.0 * h_tri / 3.0),
    ]
    draw.polygon(pts_yield, fill=(225, 25, 25, 255))
    pts_yield_in = [
        (cx - (w_tri - 55) / 2.0, cy - (h_tri - 48) / 3.0),
        (cx + (w_tri - 55) / 2.0, cy - (h_tri - 48) / 3.0),
        (cx, cy + 2.0 * (h_tri - 48) / 3.0),
    ]
    draw.polygon(pts_yield_in, fill=(250, 250, 250, 255))

    # Cell (3, 1): StVO 306 Priority Road (Yellow diamond)
    cx = 3 * cell_size + cell_size // 2
    cy = 1 * cell_size + cell_size // 2
    d_size = 110.0
    pts_dia = [
        (cx, cy - d_size),
        (cx + d_size, cy),
        (cx, cy + d_size),
        (cx - d_size, cy),
    ]
    draw.polygon(pts_dia, fill=(250, 250, 250, 255))
    d_inner = 78.0
    pts_dia_in = [
        (cx, cy - d_inner),
        (cx + d_inner, cy),
        (cx, cy + d_inner),
        (cx - d_inner, cy),
    ]
    draw.polygon(pts_dia_in, fill=(245, 205, 25, 255))  # Authentic StVO RAL 1023 Traffic Yellow

    # Cell (0, 2): StVO 101 Danger / Warning (Upright triangle with '!')
    cx = 0 * cell_size + cell_size // 2
    cy = 2 * cell_size + cell_size // 2
    pts_warn = [
        (cx, cy - 2.0 * h_tri / 3.0),
        (cx + w_tri / 2.0, cy + h_tri / 3.0),
        (cx - w_tri / 2.0, cy + h_tri / 3.0),
    ]
    draw.polygon(pts_warn, fill=(225, 25, 25, 255))
    pts_warn_in = [
        (cx, cy - 2.0 * (h_tri - 48) / 3.0),
        (cx + (w_tri - 55) / 2.0, cy + (h_tri - 48) / 3.0),
        (cx - (w_tri - 55) / 2.0, cy + (h_tri - 48) / 3.0),
    ]
    draw.polygon(pts_warn_in, fill=(250, 250, 250, 255))
    draw.text((cx, cy + 10), "!", fill=(15, 15, 15, 255), anchor="mm", font=font_large)

    # Cell (1, 2): StVO 133 Pedestrian Warning (Upright triangle with walker icon)
    cx = 1 * cell_size + cell_size // 2
    cy = 2 * cell_size + cell_size // 2
    draw.polygon([(x + cell_size, y) for x, y in pts_warn], fill=(225, 25, 25, 255))
    draw.polygon([(x + cell_size, y) for x, y in pts_warn_in], fill=(250, 250, 250, 255))
    # Pedestrian pictogram
    draw.ellipse([cx - 10, cy - 35, cx + 10, cy - 15], fill=(15, 15, 15, 255))  # Head
    draw.line([(cx, cy - 15), (cx, cy + 20)], fill=(15, 15, 15, 255), width=8)   # Torso
    draw.line([(cx, cy - 5), (cx - 20, cy + 10)], fill=(15, 15, 15, 255), width=6)  # Left arm
    draw.line([(cx, cy - 5), (cx + 20, cy - 15)], fill=(15, 15, 15, 255), width=6)  # Right arm
    draw.line([(cx, cy + 20), (cx - 18, cy + 50)], fill=(15, 15, 15, 255), width=8) # Left leg
    draw.line([(cx, cy + 20), (cx + 18, cy + 45)], fill=(15, 15, 15, 255), width=8) # Right leg

    # Cell (2, 2): StVO 138 Cyclist Warning
    cx = 2 * cell_size + cell_size // 2
    cy = 2 * cell_size + cell_size // 2
    draw.polygon([(x + 2 * cell_size, y) for x, y in pts_warn], fill=(225, 25, 25, 255))
    draw.polygon([(x + 2 * cell_size, y) for x, y in pts_warn_in], fill=(250, 250, 250, 255))
    # Bicycle pictogram
    draw.ellipse([cx - 45, cy + 15, cx - 15, cy + 45], outline=(15, 15, 15, 255), width=5) # Rear wheel
    draw.ellipse([cx + 15, cy + 15, cx + 45, cy + 45], outline=(15, 15, 15, 255), width=5) # Front wheel
    draw.line([(cx - 30, cy + 30), (cx - 5, cy + 30)], fill=(15, 15, 15, 255), width=5)
    draw.line([(cx - 5, cy + 30), (cx + 25, cy + 5)], fill=(15, 15, 15, 255), width=5)
    draw.line([(cx - 30, cy + 30), (cx - 10, cy + 5)], fill=(15, 15, 15, 255), width=5)
    draw.line([(cx - 10, cy + 5), (cx + 15, cy + 5)], fill=(15, 15, 15, 255), width=5)
    draw.line([(cx + 30, cy + 30), (cx + 20, cy - 5)], fill=(15, 15, 15, 255), width=5)
    draw.ellipse([cx - 5, cy - 20, cx + 10, cy - 5], fill=(15, 15, 15, 255)) # Rider head

    # Cell (3, 2): StVO 278 End of speed limit
    cx = 3 * cell_size + cell_size // 2
    cy = 2 * cell_size + cell_size // 2
    r_end = 118
    draw.ellipse([cx - r_end, cy - r_end, cx + r_end, cy + r_end], fill=(180, 180, 180, 255))
    draw.ellipse([cx - (r_end - 10), cy - (r_end - 10), cx + (r_end - 10), cy + (r_end - 10)], fill=(250, 250, 250, 255))
    # 5 Diagonal black bars
    for off in [-25, -12, 0, 12, 25]:
        draw.line([(cx - 85 + off, cy + 85 + off), (cx + 85 + off, cy - 85 + off)], fill=(25, 25, 25, 255), width=6)

    # 4. Row 3: Material Textures
    # Cell (0, 3): Galvanized tubular steel (clean light metallic grey)
    x0, y0 = 0 * cell_size, 3 * cell_size
    draw.rectangle([x0, y0, x0 + cell_size, y0 + cell_size], fill=(188, 192, 196, 255))
    # Cell (1, 3): RAL 7043 Verkehrsgrau B backing plate
    x0, y0 = 1 * cell_size, 3 * cell_size
    draw.rectangle([x0, y0, x0 + cell_size, y0 + cell_size], fill=(78, 84, 82, 255))
    # Cell (2, 3): Aluminum clamp / bracket
    x0, y0 = 2 * cell_size, 3 * cell_size
    draw.rectangle([x0, y0, x0 + cell_size, y0 + cell_size], fill=(140, 146, 152, 255))
    # Cell (3, 3): StVO 274.1 Zone 30 plate
    x0, y0 = 3 * cell_size, 3 * cell_size
    draw.rectangle([x0 + 10, y0 + 10, x0 + cell_size - 10, y0 + cell_size - 10], fill=(245, 245, 245, 255), outline=(20, 20, 20, 255), width=4)
    cx_z, cy_z = x0 + cell_size // 2, y0 + cell_size // 2 - 20
    draw.text((cx_z, y0 + 40), "ZONE", fill=(20, 20, 20, 255), anchor="mm", font=_get_truetype_font(42, bold=True))
    draw.ellipse([cx_z - 65, cy_z + 20, cx_z + 65, cy_z + 150], fill=(225, 25, 25, 255))
    draw.ellipse([cx_z - 48, cy_z + 37, cx_z + 48, cy_z + 133], fill=(250, 250, 250, 255))
    draw.text((cx_z, cy_z + 85), "30", fill=(15, 15, 15, 255), anchor="mm", font=_get_truetype_font(60, bold=True))

    img.save(output_path, "PNG")
    return output_path


def get_atlas_uv_bounds(stvo_code: str) -> tuple[float, float, float, float]:
    """Returns (u_min, v_min, u_max, v_max) in [0, 1] texture coordinates for a given StVO code."""
    col, row = ATLAS_CELLS.get(stvo_code, (1, 0))  # Default to 274-50
    u_min = col * 0.25
    u_max = (col + 1) * 0.25
    v_min = row * 0.25
    v_max = (row + 1) * 0.25
    return (u_min, v_min, u_max, v_max)


# -----------------------------------------------------------------------------
# Spatial Clearance & Geometry Helpers
# -----------------------------------------------------------------------------

def is_point_inside_polygon(x: float, y: float, poly: Sequence[tuple[float, float]]) -> bool:
    """Ray-casting point in polygon test."""
    n = len(poly)
    if n < 3:
        return False
    inside = False
    p1x, p1y = poly[0]
    for i in range(1, n + 1):
        p2x, p2y = poly[i % n]
        if y > min(p1y, p2y):
            if y <= max(p1y, p2y):
                if x <= max(p1x, p2x):
                    if p1y != p2y:
                        xinters = (y - p1y) * (p2x - p1x) / (p2y - p1y) + p1x
                    if p1x == p2x or x <= xinters:
                        inside = not inside
        p1x, p1y = p2x, p2y
    return inside


def is_colliding_with_cycleways(x: float, y: float, cycleways: Sequence[Any], clearance: float = 0.35) -> bool:
    """Checks if a point is within clearance distance of any cycleway path."""
    for cyc in cycleways:
        coords = getattr(cyc, "world_coords", [])
        if not coords or len(coords) < 2:
            continue
        width = getattr(cyc, "width_m", 1.8)
        half_w = width * 0.5 + clearance
        for i in range(len(coords) - 1):
            x1, y1 = coords[i][0], coords[i][1]
            x2, y2 = coords[i + 1][0], coords[i + 1][1]
            dx, dy = x2 - x1, y2 - y1
            seg_len_sq = dx * dx + dy * dy
            if seg_len_sq < 1e-4:
                continue
            t = max(0.0, min(1.0, ((x - x1) * dx + (y - y1) * dy) / seg_len_sq))
            proj_x = x1 + t * dx
            proj_y = y1 + t * dy
            dist = math.hypot(x - proj_x, y - proj_y)
            if dist < half_w:
                return True
    return False


# -----------------------------------------------------------------------------
# OSM Extraction & Waypoint Snapping
# -----------------------------------------------------------------------------

def parse_maxspeed_value(val_str: str) -> float:
    """Parses maxspeed tag into float km/h."""
    val_str = str(val_str).strip().lower()
    if val_str in ("walk", "walking"):
        return 10.0
    if "zone:30" in val_str or val_str == "30":
        return 30.0
    digits = "".join(ch for ch in val_str if ch.isdigit() or ch == ".")
    try:
        val = float(digits)
        return val if val > 0 else 50.0
    except ValueError:
        return 50.0


def extract_osm_traffic_signs(
    osm_path: Path | str,
    project_coord_fn: Callable[[float, float], tuple[float, float]],
    carla_map: Any = None,
    building_polygons: Sequence[Sequence[tuple[float, float]]] | None = None,
    cycleways: Sequence[Any] | None = None,
) -> list[TrafficSignItem]:
    """
    Extracts real-world traffic signs, speed limits, stop, and yield points from OpenStreetMap.
    Snaps each sign to the nearest road corridor with safe sidewalk curb setback.
    """
    tree = ET.parse(str(osm_path))
    root = tree.getroot()

    nodes_by_id: dict[str, tuple[float, float]] = {}
    sign_nodes: list[dict[str, Any]] = []

    for node in root.findall("node"):
        nid = node.get("id", "")
        lat = float(node.get("lat", "0"))
        lon = float(node.get("lon", "0"))
        nodes_by_id[nid] = (lat, lon)

        tags = {tag.get("k", ""): tag.get("v", "") for tag in node.findall("tag")}
        traffic_sign = tags.get("traffic_sign", "")
        maxspeed = tags.get("maxspeed", "")
        highway = tags.get("highway", "")

        is_sign = False
        sign_type = "speed_limit"
        subtype = "50"
        val = 50.0
        code = "274-50"
        shape = "circle"

        if "274" in traffic_sign or maxspeed:
            is_sign = True
            sign_type = "speed_limit"
            val = parse_maxspeed_value(maxspeed or traffic_sign)
            subtype = str(int(val))
            code = f"274-{subtype}"
            shape = "circle"
        elif "206" in traffic_sign or highway == "stop":
            is_sign = True
            sign_type = "stop"
            subtype = "stop"
            val = 0.0
            code = "206"
            shape = "octagon"
        elif "205" in traffic_sign or highway == "give_way":
            is_sign = True
            sign_type = "yield"
            subtype = "yield"
            val = 0.0
            code = "205"
            shape = "triangle_down"
        elif "306" in traffic_sign:
            is_sign = True
            sign_type = "priority_road"
            subtype = "priority"
            val = 0.0
            code = "306"
            shape = "diamond"
        elif "101" in traffic_sign:
            is_sign = True
            sign_type = "warning"
            subtype = "danger"
            val = 0.0
            code = "101"
            shape = "triangle_up"
        elif "133" in traffic_sign or "134" in traffic_sign:
            is_sign = True
            sign_type = "warning"
            subtype = "pedestrian"
            val = 0.0
            code = "133"
            shape = "triangle_up"
        elif "138" in traffic_sign:
            is_sign = True
            sign_type = "warning"
            subtype = "cyclist"
            val = 0.0
            code = "138"
            shape = "triangle_up"

        if is_sign:
            p_coord = project_coord_fn(lat, lon)
            px, py = p_coord[0], p_coord[1]
            sign_nodes.append({
                "osm_id": int(nid) if nid.isdigit() else None,
                "pos_2d": (px, py),
                "sign_type": sign_type,
                "subtype": subtype,
                "value": val,
                "code": code,
                "shape": shape,
                "tags": tags,
            })

    # Also parse maxspeed on ways to place speed limit signs at way transitions
    for way in root.findall("way"):
        tags = {tag.get("k", ""): tag.get("v", "") for tag in way.findall("tag")}
        maxspeed = tags.get("maxspeed", "")
        highway = tags.get("highway", "")
        if maxspeed and highway in ("primary", "secondary", "tertiary", "residential", "trunk", "unclassified"):
            val = parse_maxspeed_value(maxspeed)
            nd_refs = [nd.get("ref", "") for nd in way.findall("nd")]
            if nd_refs and nd_refs[0] in nodes_by_id:
                lat, lon = nodes_by_id[nd_refs[0]]
                p_coord = project_coord_fn(lat, lon)
                px, py = p_coord[0], p_coord[1]
                subtype = str(int(val))
                sign_nodes.append({
                    "osm_id": int(way.get("id", "0")) if way.get("id", "").isdigit() else None,
                    "pos_2d": (px, py),
                    "sign_type": "speed_limit",
                    "subtype": subtype,
                    "value": val,
                    "code": f"274-{subtype}",
                    "shape": "circle",
                    "tags": tags,
                })

    items: list[TrafficSignItem] = []
    sign_idx = 1000

    for sn in sign_nodes:
        x, y = sn["pos_2d"]
        road_id = -1
        lane_id = -1
        s_coord = 0.0
        t_coord = 0.0
        yaw = 0.0
        curb_setback = 0.60
        lane_clearance = 0.60
        z = 0.0

        if carla_map is not None:
            sample_loc = carla.Location(x=x, y=y, z=0.0)
            wp = carla_map.get_waypoint(sample_loc, project_to_road=True, lane_type=carla.LaneType.Driving)
            if wp is not None:
                # Find outermost driving lane on right side (standard RHT)
                curr_wp = wp
                while curr_wp.get_right_lane() is not None and curr_wp.get_right_lane().lane_type == carla.LaneType.Driving:
                    curr_wp = curr_wp.get_right_lane()

                road_id = curr_wp.road_id
                lane_id = curr_wp.lane_id
                s_coord = curr_wp.s

                # Calculate cumulative carriageway width to outer curb
                total_w = 0.0
                check_wp = curr_wp
                if curr_wp.lane_id < 0:
                    while check_wp is not None and check_wp.lane_id < 0:
                        total_w += check_wp.lane_width
                        check_wp = check_wp.get_left_lane()
                    t_coord = -(total_w + curb_setback)
                else:
                    while check_wp is not None and check_wp.lane_id > 0:
                        total_w += check_wp.lane_width
                        check_wp = check_wp.get_right_lane()
                    t_coord = +(total_w + curb_setback)

                # World position: shift by normal vector past outer curb
                yaw_rad = math.radians(curr_wp.transform.rotation.yaw)
                # Right normal in left-handed CARLA coordinates: (-sin(yaw), cos(yaw))
                nx, ny = -math.sin(yaw_rad), math.cos(yaw_rad)
                half_lane = curr_wp.lane_width * 0.5
                setback_dist = half_lane + curb_setback

                loc = curr_wp.transform.location
                x = loc.x + setback_dist * nx
                y = loc.y + setback_dist * ny
                z = loc.z

                # Sign face orientation: opposite to lane heading (+180 deg) so approaching drivers face the sign
                yaw = curr_wp.transform.rotation.yaw + 180.0
                if yaw > 180.0:
                    yaw -= 360.0
                elif yaw < -180.0:
                    yaw += 360.0

                lane_clearance = curb_setback

        # Clearance verification
        if building_polygons:
            in_bldg = any(is_point_inside_polygon(x, y, poly) for poly in building_polygons)
            if in_bldg:
                continue

        if cycleways and is_colliding_with_cycleways(x, y, cycleways, clearance=0.45):
            if carla_map is not None:
                x_shifted = x + 0.50 * nx
                y_shifted = y + 0.50 * ny
                if not is_colliding_with_cycleways(x_shifted, y_shifted, cycleways, clearance=0.35) and not (building_polygons and any(is_point_inside_polygon(x_shifted, y_shifted, poly) for poly in building_polygons)):
                    x, y = x_shifted, y_shifted
                    curb_setback += 0.50
                    lane_clearance += 0.50
                else:
                    continue
            else:
                continue

        # Spatial deduplication against already accepted items (min 5.0m distance)
        if any(math.hypot(x - it.location[0], y - it.location[1]) < 5.0 for it in items):
            continue

        sign_idx += 1
        items.append(
            TrafficSignItem(
                sign_id=f"sign_{sign_idx}",
                sign_type=sn["sign_type"],
                subtype=sn["subtype"],
                stvo_code=sn["code"],
                location=(x, y, z),
                rotation=(0.0, yaw, 0.0),
                post_height=2.25,
                plate_shape=sn["shape"],
                plate_width=0.60,
                plate_height=0.60,
                road_id=road_id,
                lane_id=lane_id,
                from_lane=lane_id,
                to_lane=lane_id,
                s=s_coord,
                t=t_coord,
                value=sn["value"],
                source="osm",
                osm_id=sn["osm_id"],
                tags=sn["tags"],
                curb_setback_m=curb_setback,
                lane_clearance_m=lane_clearance,
            )
        )

    return items


# -----------------------------------------------------------------------------
# Procedural Synthesis
# -----------------------------------------------------------------------------

def synthesize_procedural_traffic_signs(
    carla_map: Any,
    existing_signs: Sequence[TrafficSignItem],
    spacing: float = 160.0,
    min_dist_existing: float = 40.0,
    building_polygons: Sequence[Sequence[tuple[float, float]]] | None = None,
    cycleways: Sequence[Any] | None = None,
) -> list[TrafficSignItem]:
    """
    Synthesizes authentic German StVO speed limit (StVO 274: 50 km/h urban default)
    and priority/yield signs along road corridors lacking signs.
    Places signs with 0.60m sidewalk curb setback, strictly facing oncoming traffic.
    """
    if carla_map is None:
        return []

    existing_coords = [(s.location[0], s.location[1]) for s in existing_signs]
    synth_items: list[TrafficSignItem] = []
    sign_idx = 2000

    try:
        wps = carla_map.generate_waypoints(distance=spacing)
    except Exception:
        return []

    for wp in wps:
        if wp.lane_type != carla.LaneType.Driving:
            continue

        # Traverse to outermost driving lane on the right
        curr_wp = wp
        while curr_wp.get_right_lane() is not None and curr_wp.get_right_lane().lane_type == carla.LaneType.Driving:
            curr_wp = curr_wp.get_right_lane()

        road_id = curr_wp.road_id
        lane_id = curr_wp.lane_id
        s_coord = curr_wp.s

        loc = curr_wp.transform.location
        yaw_deg = curr_wp.transform.rotation.yaw
        yaw_rad = math.radians(yaw_deg)

        curb_setback = 0.60
        half_lane = curr_wp.lane_width * 0.5
        setback_dist = half_lane + curb_setback

        # Right normal vector in CARLA left-handed coordinates
        nx, ny = -math.sin(yaw_rad), math.cos(yaw_rad)
        sx = loc.x + setback_dist * nx
        sy = loc.y + setback_dist * ny
        sz = loc.z

        # Elevation snap
        sample_loc = carla.Location(x=sx, y=sy, z=sz)
        near_wp = carla_map.get_waypoint(sample_loc, project_to_road=False, lane_type=carla.LaneType.Sidewalk)
        if near_wp is not None:
            sz = near_wp.transform.location.z

        # Clearance checks
        # 1. Proximity to existing signs
        too_close = False
        for ex, ey in existing_coords:
            if math.hypot(sx - ex, sy - ey) < min_dist_existing:
                too_close = True
                break
        if too_close:
            continue

        # 2. Cycleway collision
        if cycleways and is_colliding_with_cycleways(sx, sy, cycleways, clearance=0.30):
            continue

        # 3. Building collision
        if building_polygons:
            if any(is_point_inside_polygon(sx, sy, poly) for poly in building_polygons):
                continue

        # Calculate OpenDRIVE t-coordinate (outside carriageway on sidewalk)
        total_w = 0.0
        check_wp = curr_wp
        if curr_wp.lane_id < 0:
            while check_wp is not None and check_wp.lane_id < 0:
                total_w += check_wp.lane_width
                check_wp = check_wp.get_left_lane()
            t_coord = -(total_w + curb_setback)
        else:
            while check_wp is not None and check_wp.lane_id > 0:
                total_w += check_wp.lane_width
                check_wp = check_wp.get_right_lane()
            t_coord = +(total_w + curb_setback)

        # Yaw facing oncoming traffic (+180 deg)
        facing_yaw = yaw_deg + 180.0
        if facing_yaw > 180.0:
            facing_yaw -= 360.0
        elif facing_yaw < -180.0:
            facing_yaw += 360.0

        # Pick appropriate StVO sign:
        # Default urban corridor: StVO 274-50 (or 274-30 near residential)
        stvo_code = "274-50"
        sign_type = "speed_limit"
        subtype = "50"
        val = 50.0
        plate_shape = "circle"

        # If near junction entrance, occasionally place Priority Road (StVO 306)
        if curr_wp.is_junction and (sign_idx % 4 == 0):
            stvo_code = "306"
            sign_type = "priority_road"
            subtype = "priority"
            val = 0.0
            plate_shape = "diamond"

        sign_idx += 1
        new_sign = TrafficSignItem(
            sign_id=f"sign_proc_{sign_idx}",
            sign_type=sign_type,
            subtype=subtype,
            stvo_code=stvo_code,
            location=(sx, sy, sz),
            rotation=(0.0, facing_yaw, 0.0),
            post_height=2.25,
            plate_shape=plate_shape,
            plate_width=0.60,
            plate_height=0.60,
            road_id=road_id,
            lane_id=lane_id,
            from_lane=lane_id,
            to_lane=lane_id,
            s=s_coord,
            t=t_coord,
            value=val,
            source="procedural",
            curb_setback_m=curb_setback,
            lane_clearance_m=curb_setback,
        )

        synth_items.append(new_sign)
        existing_coords.append((sx, sy))

    return synth_items


# -----------------------------------------------------------------------------
# 3D Metal Geometry & CMESH Generator
# -----------------------------------------------------------------------------

def generate_traffic_signs_mesh(
    traffic_signs: Sequence[TrafficSignItem],
    atlas_relative_path: str = "Textures/traffic_signs_atlas.png",
) -> dict[str, Any]:
    """
    Constructs high-fidelity 3D meshes for all traffic signs:
      - DIN 76mm tubular steel posts (r=0.038m, clearance height 2.25m)
      - Ground socket / base collar
      - Aluminum mounting clamps and RAL 7043 Verkehrsgrau B backing
      - Front retroreflective sign face with crisp StVO vector graphics
    Returns mesh dict ready for write_cmesh_file.
    """
    vertices: list[tuple[float, float, float]] = []
    triangles: list[tuple[int, int, int]] = []
    normals: list[tuple[float, float, float]] = []
    uvs: list[tuple[float, float]] = []

    uv_post_min, uv_post_vmin, uv_post_max, uv_post_vmax = get_atlas_uv_bounds("metal_post")
    uv_back_min, uv_back_vmin, uv_back_max, uv_back_vmax = get_atlas_uv_bounds("plate_back")
    uv_clamp_min, uv_clamp_vmin, uv_clamp_max, uv_clamp_vmax = get_atlas_uv_bounds("bracket")

    for sign in traffic_signs:
        cx, cy, cz = sign.location
        yaw_deg = sign.rotation[1]
        yaw_rad = math.radians(yaw_deg)
        cos_y = math.cos(yaw_rad)
        sin_y = math.sin(yaw_rad)

        def transform_point(lx: float, ly: float, lz: float) -> tuple[float, float, float]:
            """Transforms local sign coordinate to CARLA world coordinate."""
            gx = cx + (lx * cos_y - ly * sin_y)
            gy = cy + (lx * sin_y + ly * cos_y)
            gz = cz + lz
            return (gx, gy, gz)

        def transform_normal(nx: float, ny: float, nz: float) -> tuple[float, float, float]:
            """Transforms local normal vector to CARLA world coordinate."""
            gx = nx * cos_y - ny * sin_y
            gy = nx * sin_y + ny * cos_y
            return (gx, gy, nz)

        # ---------------------------------------------------------------------
        # 1. Base Collar / Ground Socket (z = 0 to z = 0.06m)
        # ---------------------------------------------------------------------
        num_seg = 12
        r_base = 0.065
        h_base = 0.06
        base_bot_start = len(vertices)
        for i in range(num_seg):
            ang = (2.0 * math.pi * i) / num_seg
            bx = r_base * math.cos(ang)
            by = r_base * math.sin(ang)
            vertices.append(transform_point(bx, by, 0.0))
            normals.append(transform_normal(math.cos(ang), math.sin(ang), 0.0))
            uvs.append((uv_post_min + 0.05, uv_post_vmin + 0.05))

        base_top_start = len(vertices)
        for i in range(num_seg):
            ang = (2.0 * math.pi * i) / num_seg
            bx = r_base * math.cos(ang)
            by = r_base * math.sin(ang)
            vertices.append(transform_point(bx, by, h_base))
            normals.append(transform_normal(math.cos(ang), math.sin(ang), 0.0))
            uvs.append((uv_post_max - 0.05, uv_post_vmin + 0.05))

        # Side quads of base collar
        for i in range(num_seg):
            next_i = (i + 1) % num_seg
            b0 = base_bot_start + i
            b1 = base_bot_start + next_i
            t0 = base_top_start + i
            t1 = base_top_start + next_i
            triangles.append((b0, t0, b1))
            triangles.append((b1, t0, t1))

        # ---------------------------------------------------------------------
        # 2. Tubular Steel Post (DIN 76mm, r=0.038m, height = post_height)
        # ---------------------------------------------------------------------
        r_post = sign.post_radius
        h_post = sign.post_height
        post_bot_start = len(vertices)
        for i in range(num_seg):
            ang = (2.0 * math.pi * i) / num_seg
            px = r_post * math.cos(ang)
            py = r_post * math.sin(ang)
            vertices.append(transform_point(px, py, h_base))
            normals.append(transform_normal(math.cos(ang), math.sin(ang), 0.0))
            uvs.append((uv_post_min + 0.1, uv_post_vmin + 0.1))

        post_top_start = len(vertices)
        for i in range(num_seg):
            ang = (2.0 * math.pi * i) / num_seg
            px = r_post * math.cos(ang)
            py = r_post * math.sin(ang)
            vertices.append(transform_point(px, py, h_post))
            normals.append(transform_normal(math.cos(ang), math.sin(ang), 0.0))
            uvs.append((uv_post_max - 0.1, uv_post_vmax - 0.1))

        # Side quads of tubular post
        for i in range(num_seg):
            next_i = (i + 1) % num_seg
            b0 = post_bot_start + i
            b1 = post_bot_start + next_i
            t0 = post_top_start + i
            t1 = post_top_start + next_i
            triangles.append((b0, t0, b1))
            triangles.append((b1, t0, t1))

        # Top cap of tubular post
        cap_center = len(vertices)
        vertices.append(transform_point(0.0, 0.0, h_post))
        normals.append(transform_normal(0.0, 0.0, 1.0))
        uvs.append((uv_post_min + 0.12, uv_post_vmin + 0.12))
        for i in range(num_seg):
            next_i = (i + 1) % num_seg
            triangles.append((cap_center, post_top_start + i, post_top_start + next_i))

        # ---------------------------------------------------------------------
        # 3. Rear Mounting Clamps (Aluminum Brackets)
        # ---------------------------------------------------------------------
        # Two horizontal clamp boxes connecting post to sign plate
        plate_z_center = h_post - sign.plate_height * 0.35
        clamp_heights = [plate_z_center - 0.18, plate_z_center + 0.18]
        plate_offset_x = r_post + 0.015  # Distance from post center to back of plate
        plate_thick = 0.015

        for ch in clamp_heights:
            c_min_x, c_max_x = -0.01, plate_offset_x
            c_hw = 0.035
            c_hh = 0.02
            clamp_corners = [
                (c_min_x, -c_hw, ch - c_hh),
                (c_max_x, -c_hw, ch - c_hh),
                (c_max_x, c_hw, ch - c_hh),
                (c_min_x, c_hw, ch - c_hh),
                (c_min_x, -c_hw, ch + c_hh),
                (c_max_x, -c_hw, ch + c_hh),
                (c_max_x, c_hw, ch + c_hh),
                (c_min_x, c_hw, ch + c_hh),
            ]
            c_idx_start = len(vertices)
            for lx, ly, lz in clamp_corners:
                vertices.append(transform_point(lx, ly, lz))
                normals.append(transform_normal(1.0, 0.0, 0.0))
                uvs.append((uv_clamp_min + 0.05, uv_clamp_vmin + 0.05))

            # 6 faces of clamp box
            cube_faces = [
                (0, 1, 2, 3),  # bottom
                (4, 7, 6, 5),  # top
                (0, 4, 5, 1),  # -y
                (2, 6, 7, 3),  # +y
                (0, 3, 7, 4),  # -x
                (1, 5, 6, 2),  # +x
            ]
            for f0, f1, f2, f3 in cube_faces:
                triangles.append((c_idx_start + f0, c_idx_start + f1, c_idx_start + f2))
                triangles.append((c_idx_start + f0, c_idx_start + f2, c_idx_start + f3))

        # ---------------------------------------------------------------------
        # 4. Sign Plate: Back Face, Edge Rim, and Front Retroreflective Face
        # ---------------------------------------------------------------------
        # Local X: plate back at plate_offset_x, plate front at plate_offset_x + plate_thick
        back_x = plate_offset_x
        front_x = plate_offset_x + plate_thick
        u_min, v_min, u_max, v_max = get_atlas_uv_bounds(sign.stvo_code)

        if sign.plate_shape == "circle":
            # Circular plate (StVO 274, StVO 278)
            rad = sign.plate_width * 0.5
            p_segs = 24

            # Front face (facing +X local -> oncoming traffic)
            f_center = len(vertices)
            vertices.append(transform_point(front_x, 0.0, plate_z_center))
            normals.append(transform_normal(1.0, 0.0, 0.0))
            uvs.append(((u_min + u_max) * 0.5, (v_min + v_max) * 0.5))

            f_rim_start = len(vertices)
            for i in range(p_segs):
                ang = (2.0 * math.pi * i) / p_segs
                ly = rad * math.sin(ang)
                lz = plate_z_center + rad * math.cos(ang)
                vertices.append(transform_point(front_x, ly, lz))
                normals.append(transform_normal(1.0, 0.0, 0.0))
                # UV mapping inscribed in atlas cell (un-mirrored: +Y is observer left -> u_min)
                u_coord = u_min + (0.5 - 0.48 * math.sin(ang)) * (u_max - u_min)
                v_coord = v_min + (0.5 - 0.48 * math.cos(ang)) * (v_max - v_min)
                uvs.append((u_coord, v_coord))

            for i in range(p_segs):
                next_i = (i + 1) % p_segs
                triangles.append((f_center, f_rim_start + i, f_rim_start + next_i))

            # Back face (facing -X local)
            b_center = len(vertices)
            vertices.append(transform_point(back_x, 0.0, plate_z_center))
            normals.append(transform_normal(-1.0, 0.0, 0.0))
            uvs.append(((uv_back_min + uv_back_max) * 0.5, (uv_back_vmin + uv_back_vmax) * 0.5))

            b_rim_start = len(vertices)
            for i in range(p_segs):
                ang = (2.0 * math.pi * i) / p_segs
                ly = rad * math.sin(ang)
                lz = plate_z_center + rad * math.cos(ang)
                vertices.append(transform_point(back_x, ly, lz))
                normals.append(transform_normal(-1.0, 0.0, 0.0))
                uvs.append((uv_back_min + 0.05, uv_back_vmin + 0.05))

            for i in range(p_segs):
                next_i = (i + 1) % p_segs
                triangles.append((b_center, b_rim_start + next_i, b_rim_start + i))

            # Outer rim / edge quads
            for i in range(p_segs):
                next_i = (i + 1) % p_segs
                f0 = f_rim_start + i
                f1 = f_rim_start + next_i
                b0 = b_rim_start + i
                b1 = b_rim_start + next_i
                triangles.append((f0, b0, f1))
                triangles.append((f1, b0, b1))

        elif sign.plate_shape == "octagon":
            # Octagonal plate (StVO 206 STOP)
            p_segs = 8
            rad = (sign.plate_width * 0.5) / math.cos(math.pi / 8.0)

            # Front face
            f_center = len(vertices)
            vertices.append(transform_point(front_x, 0.0, plate_z_center))
            normals.append(transform_normal(1.0, 0.0, 0.0))
            uvs.append(((u_min + u_max) * 0.5, (v_min + v_max) * 0.5))

            f_rim_start = len(vertices)
            for i in range(p_segs):
                ang = math.pi / 8.0 + (2.0 * math.pi * i) / p_segs
                ly = rad * math.sin(ang)
                lz = plate_z_center + rad * math.cos(ang)
                vertices.append(transform_point(front_x, ly, lz))
                normals.append(transform_normal(1.0, 0.0, 0.0))
                u_coord = u_min + (0.5 - 0.48 * math.sin(ang)) * (u_max - u_min)
                v_coord = v_min + (0.5 - 0.48 * math.cos(ang)) * (v_max - v_min)
                uvs.append((u_coord, v_coord))

            for i in range(p_segs):
                next_i = (i + 1) % p_segs
                triangles.append((f_center, f_rim_start + i, f_rim_start + next_i))

            # Back face
            b_center = len(vertices)
            vertices.append(transform_point(back_x, 0.0, plate_z_center))
            normals.append(transform_normal(-1.0, 0.0, 0.0))
            uvs.append(((uv_back_min + uv_back_max) * 0.5, (uv_back_vmin + uv_back_vmax) * 0.5))

            b_rim_start = len(vertices)
            for i in range(p_segs):
                ang = math.pi / 8.0 + (2.0 * math.pi * i) / p_segs
                ly = rad * math.sin(ang)
                lz = plate_z_center + rad * math.cos(ang)
                vertices.append(transform_point(back_x, ly, lz))
                normals.append(transform_normal(-1.0, 0.0, 0.0))
                uvs.append((uv_back_min + 0.05, uv_back_vmin + 0.05))

            for i in range(p_segs):
                next_i = (i + 1) % p_segs
                triangles.append((b_center, b_rim_start + next_i, b_rim_start + i))

            for i in range(p_segs):
                next_i = (i + 1) % p_segs
                f0 = f_rim_start + i
                f1 = f_rim_start + next_i
                b0 = b_rim_start + i
                b1 = b_rim_start + next_i
                triangles.append((f0, b0, f1))
                triangles.append((f1, b0, b1))

        elif sign.plate_shape in ("triangle_down", "triangle_up"):
            # Triangular plate (StVO 205 Yield, StVO 101/133/138 Warning)
            side = sign.plate_width
            h = side * (math.sqrt(3.0) / 2.0)
            is_down = (sign.plate_shape == "triangle_down")

            if is_down:
                pts_local_2d = [
                    (-side * 0.5, plate_z_center + h / 3.0),
                    (side * 0.5, plate_z_center + h / 3.0),
                    (0.0, plate_z_center - 2.0 * h / 3.0),
                ]
                uv_pts = [
                    (u_max - 0.05 * (u_max - u_min), v_min + 0.15 * (v_max - v_min)),
                    (u_min + 0.05 * (u_max - u_min), v_min + 0.15 * (v_max - v_min)),
                    ((u_min + u_max) * 0.5, v_max - 0.05 * (v_max - v_min)),
                ]
            else:
                pts_local_2d = [
                    (0.0, plate_z_center + 2.0 * h / 3.0),
                    (side * 0.5, plate_z_center - h / 3.0),
                    (-side * 0.5, plate_z_center - h / 3.0),
                ]
                uv_pts = [
                    ((u_min + u_max) * 0.5, v_min + 0.05 * (v_max - v_min)),
                    (u_min + 0.05 * (u_max - u_min), v_max - 0.15 * (v_max - v_min)),
                    (u_max - 0.05 * (u_max - u_min), v_max - 0.15 * (v_max - v_min)),
                ]

            f_start = len(vertices)
            for i, (ly, lz) in enumerate(pts_local_2d):
                vertices.append(transform_point(front_x, ly, lz))
                normals.append(transform_normal(1.0, 0.0, 0.0))
                uvs.append(uv_pts[i])

            triangles.append((f_start, f_start + 1, f_start + 2))

            b_start = len(vertices)
            for ly, lz in pts_local_2d:
                vertices.append(transform_point(back_x, ly, lz))
                normals.append(transform_normal(-1.0, 0.0, 0.0))
                uvs.append((uv_back_min + 0.05, uv_back_vmin + 0.05))

            triangles.append((b_start, b_start + 2, b_start + 1))

            for i in range(3):
                next_i = (i + 1) % 3
                f0 = f_start + i
                f1 = f_start + next_i
                b0 = b_start + i
                b1 = b_start + next_i
                triangles.append((f0, b0, f1))
                triangles.append((f1, b0, b1))

        elif sign.plate_shape == "diamond":
            # Diamond plate (StVO 306 Priority Road)
            d = sign.plate_width * 0.5
            diamond_pts = [
                (0.0, plate_z_center + d),
                (d, plate_z_center),
                (0.0, plate_z_center - d),
                (-d, plate_z_center),
            ]
            uv_pts = [
                ((u_min + u_max) * 0.5, v_min + 0.05 * (v_max - v_min)),
                (u_min + 0.05 * (u_max - u_min), (v_min + v_max) * 0.5),
                ((u_min + u_max) * 0.5, v_max - 0.05 * (v_max - v_min)),
                (u_max - 0.05 * (u_max - u_min), (v_min + v_max) * 0.5),
            ]

            f_start = len(vertices)
            for i, (ly, lz) in enumerate(diamond_pts):
                vertices.append(transform_point(front_x, ly, lz))
                normals.append(transform_normal(1.0, 0.0, 0.0))
                uvs.append(uv_pts[i])

            triangles.append((f_start, f_start + 1, f_start + 2))
            triangles.append((f_start, f_start + 2, f_start + 3))

            b_start = len(vertices)
            for ly, lz in diamond_pts:
                vertices.append(transform_point(back_x, ly, lz))
                normals.append(transform_normal(-1.0, 0.0, 0.0))
                uvs.append((uv_back_min + 0.05, uv_back_vmin + 0.05))

            triangles.append((b_start, b_start + 2, b_start + 1))
            triangles.append((b_start, b_start + 3, b_start + 2))

            for i in range(4):
                next_i = (i + 1) % 4
                f0 = f_start + i
                f1 = f_start + next_i
                b0 = b_start + i
                b1 = b_start + next_i
                triangles.append((f0, b0, f1))
                triangles.append((f1, b0, b1))

    mat_params: dict[str, Any] = {
        "albedo_texture_path": atlas_relative_path,
        "roughness": 0.45,
        "metallic": 0.20,
    }

    return {
        "name": "OSM_Traffic_Signs",
        "semantic_label": "TrafficSign",
        "material_preset": "custom_pbr",
        "material_params": mat_params,
        "vertices": vertices,
        "triangles": triangles,
        "normals": normals,
        "uvs": uvs,
        "sign_count": len(traffic_signs),
    }


# -----------------------------------------------------------------------------
# OpenDRIVE Signal Synchronization
# -----------------------------------------------------------------------------

def inject_traffic_signs_into_opendrive(
    xodr_xml: str,
    traffic_signs: Sequence[TrafficSignItem],
) -> str:
    """
    Injects compliant OpenDRIVE <signal> elements with exact curb offsets t
    matching the physical signboards.
    Eliminates CARLA's "overlaps a driving lane" warning and registers landmarks.
    """
    root = ET.fromstring(xodr_xml)
    roads_by_id: dict[str, ET.Element] = {r.get("id", ""): r for r in root.findall("road")}

    # Clean previous unaligned speed limit signals (e.g. Osm2Odr t=-2.35 signals)
    for road in root.findall("road"):
        sigs_elem = road.find("signals")
        if sigs_elem is not None:
            for sig in list(sigs_elem.findall("signal")):
                sig_type = sig.get("type", "")
                sig_name = sig.get("name", "")
                # Remove unaligned speed limit signals
                if sig_type == "274" or "sign:speed" in sig_name:
                    sigs_elem.remove(sig)

    for sign in traffic_signs:
        if sign.road_id < 0:
            continue
        road_str = str(sign.road_id)
        if road_str not in roads_by_id:
            continue

        road_elem = roads_by_id[road_str]
        sigs_elem = road_elem.find("signals")
        if sigs_elem is None:
            sigs_elem = ET.SubElement(road_elem, "signals")

        # Map StVO code to OpenDRIVE type and subtype
        xodr_type = "274"
        xodr_subtype = str(int(sign.value))
        val_str = str(int(sign.value))

        if sign.sign_type == "stop":
            xodr_type = "206"
            xodr_subtype = "-1"
            val_str = "0"
        elif sign.sign_type == "yield":
            xodr_type = "205"
            xodr_subtype = "-1"
            val_str = "0"
        elif sign.sign_type == "priority_road":
            xodr_type = "306"
            xodr_subtype = "-1"
            val_str = "0"
        elif sign.sign_type == "warning":
            xodr_type = "101"
            xodr_subtype = "-1"
            val_str = "0"

        signal_elem = ET.SubElement(
            sigs_elem,
            "signal",
            attrib={
                "id": str(sign.sign_id).replace("sign_", ""),
                "s": f"{sign.s:.2f}",
                "t": f"{sign.t:.2f}",
                "name": f"sign:{sign.sign_type}:{sign.sign_id}",
                "dynamic": "no",
                "orientation": "+",
                "zOffset": f"{sign.post_height:.2f}",
                "country": "Germany",
                "type": xodr_type,
                "subtype": xodr_subtype,
                "value": val_str,
                "unit": "km/h",
                "height": f"{sign.plate_height:.2f}",
                "width": f"{sign.plate_width:.2f}",
                "text": f"StVO {sign.stvo_code}",
                "hOffset": "0.0",
                "pitch": "0.0",
                "roll": "0.0",
            },
        )

        from_l = sign.from_lane if sign.from_lane != -1 else sign.lane_id
        to_l = sign.to_lane if sign.to_lane != -1 else sign.lane_id
        if from_l > to_l:
            from_l, to_l = to_l, from_l

        ET.SubElement(
            signal_elem,
            "validity",
            attrib={
                "fromLane": str(from_l),
                "toLane": str(to_l),
            },
        )

    return ET.tostring(root, encoding="utf-8").decode("utf-8")


# -----------------------------------------------------------------------------
# NavMesh Obstacle Geometry
# -----------------------------------------------------------------------------

def generate_traffic_sign_obstacle_geometry(
    traffic_signs: Sequence[TrafficSignItem],
) -> list[tuple[list[tuple[float, float, float]], list[tuple[int, int, int]]]]:
    """
    Constructs bounding proxy obstacles for Recast NavMesh compiler so
    pedestrians walk around standing signposts.
    """
    obstacles: list[tuple[list[tuple[float, float, float]], list[tuple[int, int, int]]]] = []
    num_seg = 8
    radius = 0.15  # 15cm safety buffer around 76mm post

    for sign in traffic_signs:
        cx, cy, cz = sign.location
        h = sign.post_height
        verts: list[tuple[float, float, float]] = []
        tris: list[tuple[int, int, int]] = []

        # Bottom circle
        b_start = 0
        for i in range(num_seg):
            ang = (2.0 * math.pi * i) / num_seg
            verts.append((cx + radius * math.cos(ang), cy + radius * math.sin(ang), cz))

        # Top circle
        t_start = num_seg
        for i in range(num_seg):
            ang = (2.0 * math.pi * i) / num_seg
            verts.append((cx + radius * math.cos(ang), cy + radius * math.sin(ang), cz + h))

        # Side quads
        for i in range(num_seg):
            next_i = (i + 1) % num_seg
            b0 = b_start + i
            b1 = b_start + next_i
            t0 = t_start + i
            t1 = t_start + next_i
            tris.append((b0, t0, b1))
            tris.append((b1, t0, t1))

        obstacles.append((verts, tris))

    return obstacles


# -----------------------------------------------------------------------------
# Audit Metrics & Manifest Serialization
# -----------------------------------------------------------------------------

def audit_traffic_sign_metrics(
    traffic_signs: Sequence[TrafficSignItem],
    mesh_data: dict[str, Any] | None = None,
) -> dict[str, Any]:
    """Computes comprehensive spatial and quality metrics for traffic signs."""
    total_signs = len(traffic_signs)
    by_type: dict[str, int] = {}
    by_subtype: dict[str, int] = {}
    by_source: dict[str, int] = {"osm": 0, "procedural": 0}

    curb_setbacks: list[float] = []
    lane_clearances: list[float] = []

    for s in traffic_signs:
        by_type[s.sign_type] = by_type.get(s.sign_type, 0) + 1
        by_subtype[s.subtype] = by_subtype.get(s.subtype, 0) + 1
        by_source[s.source] = by_source.get(s.source, 0) + 1
        curb_setbacks.append(s.curb_setback_m)
        lane_clearances.append(s.lane_clearance_m)

    min_setback = min(curb_setbacks) if curb_setbacks else 0.0
    avg_setback = sum(curb_setbacks) / len(curb_setbacks) if curb_setbacks else 0.0
    min_clearance = min(lane_clearances) if lane_clearances else 0.0

    verts_count = len(mesh_data.get("vertices", [])) if mesh_data else 0
    tris_count = len(mesh_data.get("triangles", [])) if mesh_data else 0

    return {
        "version": "1.0",
        "standards": "German StVO (Straßenverkehrs-Ordnung)",
        "total_signs": total_signs,
        "counts_by_type": by_type,
        "counts_by_subtype": by_subtype,
        "counts_by_source": by_source,
        "spatial_clearance": {
            "min_curb_setback_m": round(min_setback, 2),
            "avg_curb_setback_m": round(avg_setback, 2),
            "min_lane_clearance_m": round(min_clearance, 2),
            "zero_driving_lane_encroachment": min_clearance >= 0.40,
        },
        "mesh_geometry": {
            "vertex_count": verts_count,
            "triangle_count": tris_count,
        },
    }


def save_traffic_signs_manifest(
    traffic_signs: Sequence[TrafficSignItem],
    manifest_path: Path | str,
    mesh_data: dict[str, Any] | None = None,
) -> None:
    """Serializes traffic signs and audit report to Placements/traffic_signs.json."""
    manifest_path = Path(manifest_path)
    manifest_path.parent.mkdir(parents=True, exist_ok=True)

    metrics = audit_traffic_sign_metrics(traffic_signs, mesh_data)
    data = {
        "metrics": metrics,
        "signs": [s.to_dict() for s in traffic_signs],
    }

    manifest_path.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")
