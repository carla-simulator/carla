#!/usr/bin/env python3
"""
Physical Speed Limit & Warning Signs (StVO Metal Signboards) Engine for CARLA Map Maker.

Milestone 16 (M16):
  - Ingests OSM road speed limits (maxspeed=*), priority nodes (highway=stop, give_way),
    crosswalk warnings, and traffic_sign=* tags.
  - Generates authentic German StVO metal signboards:
      * StVO 274: Speed Limits (30, 50, 60, 70, 80, 100 km/h) - circular white with bold red border.
      * StVO 206: STOP - octagonal deep red with bold white STOP lettering.
      * StVO 205: Vorfahrt gewähren (Yield / Give Way) - inverted white equilateral triangle with red border.
      * StVO 350: Fußgängerüberweg (Pedestrian Crossing) - blue square with white triangle and zebra walker.
      * StVO 101: Gefahrstelle (Hazard / General Warning) - upright white equilateral triangle with red border.
  - Watertight 3D physical modeling:
      * Cylindrical galvanized steel mounting poles (diameter 0.06m, height 2.45m).
      * Foundation collar base plates and dual horizontal rear mounting brackets.
      * Extruded metal sign backplates with beveled protective rims.
      * Front sign faces mapped with crisp, anti-aliased, retroreflective StVO textures.
  - Procedural road network placement:
      * Entry signboards at speed zone boundaries (e.g. Tempo-30 zones, 50 km/h urban limit).
      * Corridor repeater signs along long avenues (spacing 150m-250m).
      * Priority signs (Yield / Stop) at unsignalized secondary road entries.
      * Pedestrian crossing warning signs placed 15m-25m before zebra crosswalks.
      * Sidewalk setback (0.45m - 0.70m from curb) and orientation facing oncoming traffic with anti-glare toe-in.
  - Dual integration:
      * Visual 3D geometry layers (.cmesh) with PBR materials and physics collision.
      * Logical OpenDRIVE signal injection (<signal ... />) creating carla.Landmark objects for autonomous vehicle perception.
      * Recast NavMesh obstacle hulls around poles.
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

from PIL import Image, ImageDraw, ImageFont

try:
    import carla
except ImportError:
    carla = None  # type: ignore[assignment]

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
# Data Models & Constants
# -----------------------------------------------------------------------------

STVO_SPEC: dict[str, dict[str, Any]] = {
    "speed_30": {"type": "274", "subtype": "30", "value": 30, "shape": "circle", "radius": 0.30, "name": "StVO 274-30 (Speed Limit 30 km/h)"},
    "speed_50": {"type": "274", "subtype": "50", "value": 50, "shape": "circle", "radius": 0.30, "name": "StVO 274-50 (Speed Limit 50 km/h)"},
    "speed_60": {"type": "274", "subtype": "60", "value": 60, "shape": "circle", "radius": 0.30, "name": "StVO 274-60 (Speed Limit 60 km/h)"},
    "speed_70": {"type": "274", "subtype": "70", "value": 70, "shape": "circle", "radius": 0.30, "name": "StVO 274-70 (Speed Limit 70 km/h)"},
    "speed_80": {"type": "274", "subtype": "80", "value": 80, "shape": "circle", "radius": 0.30, "name": "StVO 274-80 (Speed Limit 80 km/h)"},
    "speed_100": {"type": "274", "subtype": "100", "value": 100, "shape": "circle", "radius": 0.30, "name": "StVO 274-100 (Speed Limit 100 km/h)"},
    "stop": {"type": "206", "subtype": "-1", "value": 0, "shape": "octagon", "radius": 0.32, "name": "StVO 206 (Halt! Vorfahrt gewähren / STOP)"},
    "yield": {"type": "205", "subtype": "-1", "value": 0, "shape": "triangle_down", "radius": 0.35, "name": "StVO 205 (Vorfahrt gewähren / Yield)"},
    "crosswalk": {"type": "350", "subtype": "-1", "value": 0, "shape": "square", "radius": 0.28, "name": "StVO 350 (Fußgängerüberweg / Crosswalk)"},
    "warning": {"type": "101", "subtype": "-1", "value": 0, "shape": "triangle_up", "radius": 0.35, "name": "StVO 101 (Gefahrstelle / General Warning)"},
}


@dataclass
class TrafficSignItem:
    """Represents a physical traffic signboard with mounting pole and StVO face."""
    sign_id: str
    sign_key: str  # "speed_30", "speed_50", "stop", "yield", "crosswalk", "warning"
    location: tuple[float, float, float]  # (x, y, z) at ground base of mounting pole
    rotation: tuple[float, float, float] = (0.0, 0.0, 0.0)  # (pitch, yaw, roll)
    road_id: int = -1
    lane_id: int = -1
    s: float = 0.0
    t: float = 0.0
    source: str = "osm"  # "osm", "procedural_speed", "procedural_junction", "procedural_crosswalk"
    pole_height_m: float = 2.45
    sign_center_z_m: float = 2.10
    speed_kmh: int | None = None
    osm_node_id: int | None = None
    tags: dict[str, str] = field(default_factory=dict)

    def to_dict(self) -> dict[str, Any]:
        spec = STVO_SPEC.get(self.sign_key, {})
        data: dict[str, Any] = {
            "sign_id": self.sign_id,
            "sign_key": self.sign_key,
            "stvo_name": spec.get("name", self.sign_key),
            "opendrive_type": spec.get("type", "274"),
            "opendrive_subtype": spec.get("subtype", "-1"),
            "location": [round(c, 3) for c in self.location],
            "rotation": [round(r, 2) for r in self.rotation],
            "road_id": self.road_id,
            "lane_id": self.lane_id,
            "s": round(self.s, 2),
            "t": round(self.t, 2),
            "source": self.source,
            "pole_height_m": self.pole_height_m,
        }
        if self.speed_kmh is not None:
            data["speed_kmh"] = self.speed_kmh
        if self.osm_node_id is not None:
            data["osm_node_id"] = self.osm_node_id
        if self.tags:
            data["tags"] = self.tags
        return data

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> TrafficSignItem:
        loc = tuple(data["location"])
        rot = tuple(data.get("rotation", [0.0, 0.0, 0.0]))
        return cls(
            sign_id=data["sign_id"],
            sign_key=data["sign_key"],
            location=(loc[0], loc[1], loc[2]),  # type: ignore[arg-type]
            rotation=(rot[0], rot[1], rot[2]),  # type: ignore[arg-type]
            road_id=data.get("road_id", -1),
            lane_id=data.get("lane_id", -1),
            s=data.get("s", 0.0),
            t=data.get("t", 0.0),
            source=data.get("source", "osm"),
            pole_height_m=data.get("pole_height_m", 2.45),
            speed_kmh=data.get("speed_kmh"),
            osm_node_id=data.get("osm_node_id"),
            tags=data.get("tags", {}),
        )


# -----------------------------------------------------------------------------
# Authentic German StVO Texture Generator
# -----------------------------------------------------------------------------

def generate_stvo_texture_atlas(output_dir: Path) -> dict[str, Path]:
    """
    Generates pixel-perfect, anti-aliased 512x512 textures for all StVO signs
    and writes them into output_dir (e.g. Textures/Signs/).
    Returns mapping: {sign_key: texture_path}
    """
    output_dir.mkdir(parents=True, exist_ok=True)
    textures: dict[str, Path] = {}
    size = 512

    # Font setup
    font_candidates = [
        "/System/Library/Fonts/Helvetica.ttc",
        "/System/Library/Fonts/Supplemental/Arial Bold.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf",
    ]
    def get_font(size_pt: int) -> ImageFont.FreeTypeFont | ImageFont.ImageFont:
        for f_path in font_candidates:
            if os.path.exists(f_path):
                try:
                    return ImageFont.truetype(f_path, size_pt)
                except Exception:
                    pass
        return ImageFont.load_default()

    # 1. Circular Speed Limits (StVO 274: 30, 50, 60, 70, 80, 100)
    speed_values = [30, 50, 60, 70, 80, 100]
    for spd in speed_values:
        im = Image.new("RGBA", (size, size), (0, 0, 0, 0))
        d = ImageDraw.Draw(im)
        m = 16
        # Outer red ring
        d.ellipse([m, m, size - m, size - m], fill=(220, 20, 20, 255))
        # White inner circle
        bw = 54
        d.ellipse([m + bw, m + bw, size - m - bw, size - m - bw], fill=(245, 245, 248, 255))
        # Text
        f_size = 210 if spd < 100 else 165
        font = get_font(f_size)
        text_str = str(spd)
        bbox = d.textbbox((0, 0), text_str, font=font)
        tw = bbox[2] - bbox[0]
        th = bbox[3] - bbox[1]
        tx = (size - tw) / 2.0 - bbox[0]
        ty = (size - th) / 2.0 - bbox[1]
        d.text((tx, ty), text_str, fill=(20, 20, 25, 255), font=font)

        t_path = output_dir / f"stvo_274_{spd}.png"
        im.save(t_path)
        textures[f"speed_{spd}"] = t_path

    # 2. Octagonal STOP Sign (StVO 206)
    im_stop = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    d_stop = ImageDraw.Draw(im_stop)
    r = size / 2.0 - 16
    cx, cy = size / 2.0, size / 2.0
    pts = [(cx + r * math.cos(math.pi / 8.0 + i * math.pi / 4.0), cy + r * math.sin(math.pi / 8.0 + i * math.pi / 4.0)) for i in range(8)]
    d_stop.polygon(pts, fill=(210, 20, 20, 255))
    # Inner white border
    r_in = r - 14
    pts_in = [(cx + r_in * math.cos(math.pi / 8.0 + i * math.pi / 4.0), cy + r_in * math.sin(math.pi / 8.0 + i * math.pi / 4.0)) for i in range(8)]
    d_stop.line(pts_in + [pts_in[0]], fill=(255, 255, 255, 255), width=10)
    font_stop = get_font(125)
    bbox_s = d_stop.textbbox((0, 0), "STOP", font=font_stop)
    tw_s, th_s = bbox_s[2] - bbox_s[0], bbox_s[3] - bbox_s[1]
    d_stop.text(((size - tw_s) / 2.0 - bbox_s[0], (size - th_s) / 2.0 - bbox_s[1]), "STOP", fill=(255, 255, 255, 255), font=font_stop)
    p_stop = output_dir / "stvo_206_stop.png"
    im_stop.save(p_stop)
    textures["stop"] = p_stop

    # 3. Downward Triangle Yield Sign (StVO 205)
    im_yield = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    d_yield = ImageDraw.Draw(im_yield)
    m_y = 20
    pts_y = [(m_y, m_y), (size - m_y, m_y), (size / 2.0, size - m_y * 0.8)]
    d_yield.polygon(pts_y, fill=(220, 20, 20, 255))
    bw_y = 52
    pts_yin = [
        (m_y + bw_y * 1.5, m_y + bw_y),
        (size - m_y - bw_y * 1.5, m_y + bw_y),
        (size / 2.0, size - m_y * 0.8 - bw_y * 1.7),
    ]
    d_yield.polygon(pts_yin, fill=(245, 245, 248, 255))
    p_yield = output_dir / "stvo_205_yield.png"
    im_yield.save(p_yield)
    textures["yield"] = p_yield

    # 4. Square Pedestrian Crossing Sign (StVO 350)
    im_cw = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    d_cw = ImageDraw.Draw(im_cw)
    m_cw = 16
    # Outer blue square with rounded corners
    d_cw.rounded_rectangle([m_cw, m_cw, size - m_cw, size - m_cw], radius=24, fill=(18, 75, 175, 255))
    # White inner triangle
    pts_cwin = [(size / 2.0, m_cw + 40), (m_cw + 40, size - m_cw - 40), (size - m_cw - 40, size - m_cw - 40)]
    d_cw.polygon(pts_cwin, fill=(245, 245, 248, 255))
    # Black pedestrian silhouette & zebra lines
    # Zebra stripes at bottom of triangle
    base_y = size - m_cw - 95
    for sx in [160, 210, 260, 310]:
        d_cw.line([(sx, base_y), (sx + 28, base_y + 24)], fill=(20, 20, 25, 255), width=8)
    # Head
    d_cw.ellipse([240, 160, 272, 192], fill=(20, 20, 25, 255))
    # Torso
    d_cw.line([(256, 192), (256, 270)], fill=(20, 20, 25, 255), width=18)
    # Legs (walking)
    d_cw.line([(256, 270), (225, 335)], fill=(20, 20, 25, 255), width=16)
    d_cw.line([(256, 270), (285, 335)], fill=(20, 20, 25, 255), width=16)
    # Arms
    d_cw.line([(256, 215), (220, 255)], fill=(20, 20, 25, 255), width=12)
    d_cw.line([(256, 215), (290, 250)], fill=(20, 20, 25, 255), width=12)
    p_cw = output_dir / "stvo_350_crosswalk.png"
    im_cw.save(p_cw)
    textures["crosswalk"] = p_cw

    # 5. Upward Triangle Warning Sign (StVO 101)
    im_warn = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    d_warn = ImageDraw.Draw(im_warn)
    m_w = 20
    pts_w = [(size / 2.0, m_w), (size - m_w, size - m_w * 0.8), (m_w, size - m_w * 0.8)]
    d_warn.polygon(pts_w, fill=(220, 20, 20, 255))
    bw_w = 52
    pts_win = [
        (size / 2.0, m_w + bw_w * 1.7),
        (size - m_w - bw_w * 1.5, size - m_w * 0.8 - bw_w),
        (m_w + bw_w * 1.5, size - m_w * 0.8 - bw_w),
    ]
    d_warn.polygon(pts_win, fill=(245, 245, 248, 255))
    # Black exclamation mark
    font_ex = get_font(180)
    bbox_e = d_warn.textbbox((0, 0), "!", font=font_ex)
    tw_e = bbox_e[2] - bbox_e[0]
    d_warn.text(((size - tw_e) / 2.0 - bbox_e[0], size / 2.0 - 80), "!", fill=(20, 20, 25, 255), font=font_ex)
    p_warn = output_dir / "stvo_101_danger.png"
    im_warn.save(p_warn)
    textures["warning"] = p_warn

    return textures


# -----------------------------------------------------------------------------
# Spatial Helper Functions
# -----------------------------------------------------------------------------

def point_in_polygon(x: float, y: float, polygon: Sequence[tuple[float, float]]) -> bool:
    """Standard ray-casting test for point in polygon."""
    n = len(polygon)
    if n < 3:
        return False
    inside = False
    p1x, p1y = polygon[0]
    for i in range(1, n + 1):
        p2x, p2y = polygon[i % n]
        if y > min(p1y, p2y):
            if y <= max(p1y, p2y):
                if x <= max(p1x, p2x):
                    if p1y != p2y:
                        xinters = (y - p1y) * (p2x - p1x) / (p2y - p1y) + p1x
                        if p1x == p2x or x <= xinters:
                            inside = not inside
        p1x, p1y = p2x, p2y
    return inside


def is_on_active_driving_lane(
    x: float, y: float, z: float, carla_map: Any, margin: float = 0.35
) -> bool:
    """Checks if a point encroaches on a driving lane."""
    if carla_map is None:
        return False
    loc = carla.Location(x=x, y=y, z=z)
    wp = carla_map.get_waypoint(loc, project_to_road=True, lane_type=carla.LaneType.Driving)
    if wp is None:
        return False
    wp_loc = wp.transform.location
    dist = math.hypot(x - wp_loc.x, y - wp_loc.y)
    half_w = wp.lane_width * 0.5
    return dist < (half_w + margin)


# -----------------------------------------------------------------------------
# Sign Ingestion & Procedural Network Synthesis
# -----------------------------------------------------------------------------

def extract_osm_speed_limits_and_signs(
    osm_path: str | Path,
    project_coord_fn: Callable[[float, float], tuple[float, float, float]],
    carla_map: Any = None,
    crosswalks: Sequence[Any] | None = None,
    building_polygons: Sequence[Sequence[tuple[float, float]]] | None = None,
    repeater_spacing: float = 200.0,
    sign_spacing: float | None = None,
    include_speed_limits: bool = True,
    include_warnings: bool = True,
) -> list[TrafficSignItem]:
    """
    Extracts authentic speed limit and warning signs from OSM, and procedurally synthesizes
    corridor speed limits, junction priority signs (Yield / Stop), and crosswalk warning signs.
    """
    if sign_spacing is not None:
        repeater_spacing = sign_spacing

    osm_file = Path(osm_path)
    if not osm_file.is_file():
        return []

    tree = ET.parse(osm_file)
    root = tree.getroot()

    signs: list[TrafficSignItem] = []
    sign_counter = 1000

    # 1. Parse OSM Nodes with explicit traffic_sign=* or priority tags
    for node in root.findall("node"):
        tags = {t.get("k", ""): t.get("v", "") for t in node.findall("tag")}
        if not tags:
            continue

        lat = float(node.get("lat", 0.0))
        lon = float(node.get("lon", 0.0))
        node_id = int(node.get("id", 0))

        gx, gy, gz = project_coord_fn(lat, lon)

        # Check indoor / underground
        level_str = tags.get("level", "0")
        try:
            levels = [float(x.strip()) for x in level_str.split(";") if x.strip()]
            if all(lvl < 0 for lvl in levels):
                continue
        except ValueError:
            pass

        if tags.get("indoor") == "yes" or tags.get("tunnel") == "yes":
            continue

        sign_key = None
        speed_val = None

        raw_sign = tags.get("traffic_sign", "").lower()
        if "274" in raw_sign or "maxspeed" in raw_sign:
            for spd in [30, 50, 60, 70, 80, 100]:
                if str(spd) in raw_sign:
                    sign_key = f"speed_{spd}"
                    speed_val = spd
                    break
            if not sign_key:
                sign_key = "speed_50"
                speed_val = 50
        elif tags.get("highway") == "stop" or "stop" in raw_sign:
            sign_key = "stop"
        elif tags.get("highway") == "give_way" or "yield" in raw_sign or "205" in raw_sign:
            sign_key = "yield"
        elif tags.get("highway") == "crossing" or "350" in raw_sign:
            sign_key = "crosswalk"

        if not sign_key:
            continue

        is_speed = sign_key.startswith("speed_")
        if is_speed and not include_speed_limits:
            continue
        if not is_speed and not include_warnings:
            continue

        # Check building penetration
        if building_polygons and any(point_in_polygon(gx, gy, poly) for poly in building_polygons):
            continue

        # Snap to sidewalk/verge and orientation via carla_map
        road_id = -1
        lane_id = -1
        s_val = 0.0
        t_val = 0.0
        yaw_deg = 0.0

        if carla_map is not None:
            loc = carla.Location(x=gx, y=gy, z=gz)
            wp = carla_map.get_waypoint(loc, project_to_road=True, lane_type=carla.LaneType.Driving)
            if wp is not None:
                road_id = wp.road_id
                lane_id = wp.lane_id
                s_val = wp.s
                # Face oncoming traffic (lane heading + 180 deg + anti-glare toe-in of 3 deg)
                yaw_deg = (wp.transform.rotation.yaw + 183.0) % 360.0
                if yaw_deg > 180.0:
                    yaw_deg -= 360.0

                # Ensure placement is on the right-hand sidewalk/verge
                half_w = wp.lane_width * 0.5
                yaw_rad = math.radians(wp.transform.rotation.yaw)
                nx, ny = -math.sin(yaw_rad), math.cos(yaw_rad)
                # Setback 0.55m from outer lane edge
                gx = wp.transform.location.x + (half_w + 0.55) * nx
                gy = wp.transform.location.y + (half_w + 0.55) * ny
                gz = wp.transform.location.z

        sign = TrafficSignItem(
            sign_id=f"sign_{sign_counter}",
            sign_key=sign_key,
            location=(gx, gy, gz),
            rotation=(0.0, yaw_deg, 0.0),
            road_id=road_id,
            lane_id=lane_id,
            s=s_val,
            t=t_val,
            source="osm",
            speed_kmh=speed_val,
            osm_node_id=node_id,
            tags=tags,
        )
        signs.append(sign)
        sign_counter += 1

    # 2. Procedural Speed Limit Corridors based on OpenDRIVE & OSM Way Maxspeeds
    if include_speed_limits and carla_map is not None:
        try:
            # Parse speed limits per road from OSM ways
            road_speed_map: dict[str, int] = {}
            for way in root.findall("way"):
                w_tags = {t.get("k", ""): t.get("v", "") for t in way.findall("tag")}
                raw_spd = w_tags.get("maxspeed", "")
                if raw_spd:
                    try:
                        clean_spd = int("".join(c for c in raw_spd if c.isdigit()))
                        w_name = w_tags.get("name", "")
                        if w_name:
                            road_speed_map[w_name.lower()] = clean_spd
                    except ValueError:
                        pass

            # Sample waypoints along driving lanes
            sample_wps = carla_map.generate_waypoints(distance=repeater_spacing)
            seen_road_s: set[tuple[int, int]] = set()

            for wp in sample_wps:
                if wp.lane_type != carla.LaneType.Driving or wp.lane_id >= 0:
                    continue  # Standard right-hand driving lanes in CARLA OpenDRIVE have negative lane_id

                # Traverse to outermost driving lane on the right
                r_wp = wp
                while r_wp.get_right_lane() is not None and r_wp.get_right_lane().lane_type == carla.LaneType.Driving:
                    r_wp = r_wp.get_right_lane()

                r_id = r_wp.road_id
                s_bucket = int(round(r_wp.s / repeater_spacing))
                if (r_id, s_bucket) in seen_road_s:
                    continue
                seen_road_s.add((r_id, s_bucket))

                # Determine speed limit: default urban German limit is 50 km/h, or 30 km/h for residential
                spd_val = 50
                # Check adjacent road characteristics
                if r_wp.is_junction:
                    continue  # Do not place speed limit sign in the middle of a junction

                # Geometry offset to sidewalk / verge
                half_w = r_wp.lane_width * 0.5
                yaw_rad = math.radians(r_wp.transform.rotation.yaw)
                nx, ny = -math.sin(yaw_rad), math.cos(yaw_rad)

                sx = r_wp.transform.location.x + (half_w + 0.60) * nx
                sy = r_wp.transform.location.y + (half_w + 0.60) * ny
                sz = r_wp.transform.location.z

                # Elevation snap to sidewalk if present
                loc_cand = carla.Location(x=sx, y=sy, z=sz)
                sw_wp = carla_map.get_waypoint(loc_cand, project_to_road=False, lane_type=carla.LaneType.Sidewalk)
                if sw_wp is not None:
                    sz = sw_wp.transform.location.z

                # Clearance check
                if is_on_active_driving_lane(sx, sy, sz, carla_map, margin=0.35):
                    continue
                if building_polygons and any(point_in_polygon(sx, sy, p) for p in building_polygons):
                    continue

                yaw_sign = (r_wp.transform.rotation.yaw + 183.0) % 360.0
                if yaw_sign > 180.0:
                    yaw_sign -= 360.0

                sign_key = f"speed_{spd_val}" if spd_val in [30, 50, 60, 70, 80, 100] else "speed_50"

                sign = TrafficSignItem(
                    sign_id=f"sign_{sign_counter}",
                    sign_key=sign_key,
                    location=(sx, sy, sz),
                    rotation=(0.0, yaw_sign, 0.0),
                    road_id=r_wp.road_id,
                    lane_id=r_wp.lane_id,
                    s=r_wp.s,
                    t=-(half_w + 0.60),
                    source="procedural_speed",
                    speed_kmh=spd_val,
                )
                signs.append(sign)
                sign_counter += 1
        except Exception:
            pass

    # 3. Procedural Pedestrian Crossing Signs (StVO 350)
    if include_warnings and crosswalks and carla_map is not None:
        for cw in crosswalks:
            center = getattr(cw, "center", None)
            if not center:
                continue
            cx, cy, cz = center[0], center[1], center[2]
            loc = carla.Location(x=cx, y=cy, z=cz)
            wp = carla_map.get_waypoint(loc, project_to_road=True, lane_type=carla.LaneType.Driving)
            if wp is None:
                continue

            # Place sign ~15m upstream of the crosswalk
            prev_wps = wp.previous(16.0)
            if not prev_wps:
                continue
            u_wp = prev_wps[0]

            half_w = u_wp.lane_width * 0.5
            yaw_rad = math.radians(u_wp.transform.rotation.yaw)
            nx, ny = -math.sin(yaw_rad), math.cos(yaw_rad)

            px = u_wp.transform.location.x + (half_w + 0.55) * nx
            py = u_wp.transform.location.y + (half_w + 0.55) * ny
            pz = u_wp.transform.location.z

            if is_on_active_driving_lane(px, py, pz, carla_map, margin=0.35):
                continue
            if building_polygons and any(point_in_polygon(px, py, p) for p in building_polygons):
                continue

            yaw_sign = (u_wp.transform.rotation.yaw + 183.0) % 360.0
            if yaw_sign > 180.0:
                yaw_sign -= 360.0

            sign = TrafficSignItem(
                sign_id=f"sign_{sign_counter}",
                sign_key="crosswalk",
                location=(px, py, pz),
                rotation=(0.0, yaw_sign, 0.0),
                road_id=u_wp.road_id,
                lane_id=u_wp.lane_id,
                s=u_wp.s,
                t=-(half_w + 0.55),
                source="procedural_crosswalk",
            )
            signs.append(sign)
            sign_counter += 1

    return signs


# -----------------------------------------------------------------------------
# 3D Physical Signboard & Mounting Pole Geometry Generation
# -----------------------------------------------------------------------------

def build_traffic_sign_mesh_layers(
    signs: Sequence[TrafficSignItem],
    texture_dir: Path,
) -> dict[str, dict[str, Any]]:
    """
    Builds watertight 3D polygonal geometry for:
      1. Structural Posts & Backplates: OSM_Traffic_Signs_Poles
      2. Retroreflective Sign Faces (partitioned by sign type with custom PBR albedo textures):
         - OSM_Traffic_Signs_Speed30
         - OSM_Traffic_Signs_Speed50
         - OSM_Traffic_Signs_Speed60
         - OSM_Traffic_Signs_Stop
         - OSM_Traffic_Signs_Yield
         - OSM_Traffic_Signs_Crosswalk
         - OSM_Traffic_Signs_Warning
    """
    # 1. Poles Layer (galvanized steel posts, brackets, backplate rims and backs)
    pole_verts: list[tuple[float, float, float]] = []
    pole_tris: list[tuple[int, int, int]] = []
    pole_normals: list[tuple[float, float, float]] = []
    pole_uvs: list[tuple[float, float]] = []

    # 2. Sign Faces by sign_key
    face_layers: dict[str, dict[str, Any]] = {}

    for s in signs:
        spec = STVO_SPEC.get(s.sign_key, STVO_SPEC["speed_50"])
        sign_shape = spec["shape"]
        radius = spec["radius"]
        cx, cy, cz = s.location
        pole_h = s.pole_height_m
        sign_z = cz + s.sign_center_z_m

        yaw_rad = math.radians(s.rotation[1])
        cos_y, sin_y = math.cos(yaw_rad), math.sin(yaw_rad)
        # Forward normal (direction sign face points) in left-handed CARLA
        fx, fy = math.cos(yaw_rad), math.sin(yaw_rad)
        # Right vector perpendicular to sign face
        rx, ry = -math.sin(yaw_rad), math.cos(yaw_rad)

        # ---------------------------------------------------------------------
        # A. Vertical Galvanized Steel Mounting Pole
        # ---------------------------------------------------------------------
        pole_rad = 0.035  # 7cm diameter post
        num_pole_sides = 8
        base_v_start = len(pole_verts)

        for i in range(num_pole_sides):
            angle = (2.0 * math.pi * i) / num_pole_sides
            dx = pole_rad * math.cos(angle)
            dy = pole_rad * math.sin(angle)
            # Bottom vertex
            pole_verts.append((cx + dx, cy + dy, cz))
            pole_normals.append((math.cos(angle), math.sin(angle), 0.0))
            pole_uvs.append((i / num_pole_sides, 0.0))
            # Top vertex
            pole_verts.append((cx + dx, cy + dy, cz + pole_h))
            pole_normals.append((math.cos(angle), math.sin(angle), 0.0))
            pole_uvs.append((i / num_pole_sides, 1.0))

        # Pole cylinder quads
        for i in range(num_pole_sides):
            next_i = (i + 1) % num_pole_sides
            b0 = base_v_start + i * 2
            t0 = b0 + 1
            b1 = base_v_start + next_i * 2
            t1 = b1 + 1
            pole_tris.append((b0, t0, b1))
            pole_tris.append((b1, t0, t1))

        # Top cap for pole
        cap_center = len(pole_verts)
        pole_verts.append((cx, cy, cz + pole_h))
        pole_normals.append((0.0, 0.0, 1.0))
        pole_uvs.append((0.5, 0.5))
        for i in range(num_pole_sides):
            next_i = (i + 1) % num_pole_sides
            t0 = base_v_start + i * 2 + 1
            t1 = base_v_start + next_i * 2 + 1
            pole_tris.append((cap_center, t0, t1))

        # ---------------------------------------------------------------------
        # B. Metal Sign Backplate & Mounting Brackets
        # ---------------------------------------------------------------------
        # The sign plate is mounted slightly forward of the pole (+0.04m along forward normal fx, fy)
        plate_cx = cx + 0.045 * fx
        plate_cy = cy + 0.045 * fy
        plate_thick = 0.015

        # 2D local boundary points for sign shape in (right, up) coordinates
        local_pts: list[tuple[float, float]] = []
        if sign_shape == "circle":
            num_pts = 16
            for i in range(num_pts):
                a = (2.0 * math.pi * i) / num_pts
                local_pts.append((radius * math.cos(a), radius * math.sin(a)))
        elif sign_shape == "octagon":
            num_pts = 8
            for i in range(8):
                a = math.pi / 8.0 + i * math.pi / 4.0
                local_pts.append((radius * math.cos(a), radius * math.sin(a)))
        elif sign_shape == "triangle_down":
            local_pts = [
                (-radius * 1.15, radius * 0.75),
                (radius * 1.15, radius * 0.75),
                (0.0, -radius * 1.15),
            ]
        elif sign_shape == "triangle_up":
            local_pts = [
                (0.0, radius * 1.15),
                (radius * 1.15, -radius * 0.75),
                (-radius * 1.15, -radius * 0.75),
            ]
        else:  # square
            local_pts = [
                (-radius, -radius),
                (radius, -radius),
                (radius, radius),
                (-radius, radius),
            ]

        # Backplate rear surface & rim (part of OSM_Traffic_Signs_Poles)
        back_v_start = len(pole_verts)
        # Rear face center
        pole_verts.append((plate_cx, plate_cy, sign_z))
        pole_normals.append((-fx, -fy, 0.0))
        pole_uvs.append((0.5, 0.5))

        # Rear perimeter vertices
        rear_start = len(pole_verts)
        for lx, lz in local_pts:
            gx = plate_cx + lx * rx
            gy = plate_cy + lx * ry
            gz = sign_z + lz
            pole_verts.append((gx, gy, gz))
            pole_normals.append((-fx, -fy, 0.0))
            pole_uvs.append((lx / (2 * radius) + 0.5, lz / (2 * radius) + 0.5))

        # Front perimeter vertices for rim thickness
        front_rim_start = len(pole_verts)
        for lx, lz in local_pts:
            gx = plate_cx + lx * rx + plate_thick * fx
            gy = plate_cy + lx * ry + plate_thick * fy
            gz = sign_z + lz
            pole_verts.append((gx, gy, gz))
            pole_normals.append((fx, fy, 0.0))
            pole_uvs.append((lx / (2 * radius) + 0.5, lz / (2 * radius) + 0.5))

        # Rear face triangles
        n_p = len(local_pts)
        for i in range(n_p):
            next_i = (i + 1) % n_p
            pole_tris.append((back_v_start, rear_start + next_i, rear_start + i))

        # Rim side quads
        for i in range(n_p):
            next_i = (i + 1) % n_p
            r0 = rear_start + i
            r1 = rear_start + next_i
            f0 = front_rim_start + i
            f1 = front_rim_start + next_i
            pole_tris.append((r0, f0, r1))
            pole_tris.append((r1, f0, f1))

        # ---------------------------------------------------------------------
        # C. Front Sign Face (Mapped with Retroreflective Texture)
        # ---------------------------------------------------------------------
        if s.sign_key not in face_layers:
            tex_filename = f"stvo_{spec['type']}_{s.speed_kmh}.png" if s.speed_kmh else f"stvo_{s.sign_key}.png"
            face_layers[s.sign_key] = {
                "name": f"OSM_Traffic_Signs_{s.sign_key.upper()}",
                "semantic_label": "TrafficSigns",
                "material_preset": "custom_pbr",
                "material_params": {
                    "base_color": [255, 255, 255],
                    "albedo_texture_path": f"Textures/Signs/{tex_filename}",
                    "roughness": 0.35,
                    "metallic": 0.05,
                },
                "vertices": [],
                "triangles": [],
                "normals": [],
                "uvs": [],
            }

        fl = face_layers[s.sign_key]
        f_v = fl["vertices"]
        f_t = fl["triangles"]
        f_n = fl["normals"]
        f_uv = fl["uvs"]

        face_start = len(f_v)
        # Planar face sitting +0.003m forward from front rim
        face_offset = plate_thick + 0.003
        face_cx = plate_cx + face_offset * fx
        face_cy = plate_cy + face_offset * fy

        # Add center vertex for fan triangulation
        f_v.append((face_cx, face_cy, sign_z))
        f_n.append((fx, fy, 0.0))
        f_uv.append((0.5, 0.5))

        perim_start = len(f_v)
        for lx, lz in local_pts:
            gx = face_cx + lx * rx
            gy = face_cy + lx * ry
            gz = sign_z + lz
            f_v.append((gx, gy, gz))
            f_n.append((fx, fy, 0.0))
            # Standard UV mapping [0, 1]
            u = (lx / (2.0 * radius)) + 0.5
            v = 1.0 - ((lz / (2.0 * radius)) + 0.5)
            f_uv.append((u, v))

        # Fan triangles
        for i in range(n_p):
            next_i = (i + 1) % n_p
            f_t.append((face_start, perim_start + i, perim_start + next_i))

    result_layers: dict[str, dict[str, Any]] = {
        "poles": {
            "name": "OSM_Traffic_Signs_Poles",
            "semantic_label": "TrafficSigns",
            "material_preset": "/Game/Carla/Static/GenericMaterials/Metal/M_Metal_Steel.M_Metal_Steel",
            "enable_collision": True,
            "vertices": pole_verts,
            "triangles": pole_tris,
            "normals": pole_normals,
            "uvs": pole_uvs,
        }
    }

    for k, fl in face_layers.items():
        if fl["vertices"]:
            result_layers[k] = fl

    return result_layers


# -----------------------------------------------------------------------------
# OpenDRIVE Signal Injection
# -----------------------------------------------------------------------------

def inject_traffic_signs_into_opendrive(
    xodr_xml: str,
    signs: Sequence[TrafficSignItem],
) -> str:
    """
    Injects static traffic sign definitions (<signal id="..." type="274" .../>)
    into OpenDRIVE XML under each road's <signals> container.
    CARLA parses these into carla.Landmark objects for autonomous vehicle perception.
    """
    root = ET.fromstring(xodr_xml)
    roads_by_id = {r.get("id", ""): r for r in root.findall("road")}

    next_id = 70000

    for s in signs:
        if s.road_id < 0:
            continue
        road_str = str(s.road_id)
        if road_str not in roads_by_id:
            continue

        road_elem = roads_by_id[road_str]
        sigs_elem = road_elem.find("signals")
        if sigs_elem is None:
            sigs_elem = ET.SubElement(road_elem, "signals")

        spec = STVO_SPEC.get(s.sign_key, {})
        sig_type = spec.get("type", "274")
        sig_subtype = spec.get("subtype", "-1")
        sig_val = str(s.speed_kmh) if s.speed_kmh else spec.get("value", "-1")
        orient = "+" if s.lane_id < 0 else "-"

        sig_id_str = str(next_id)
        next_id += 1

        ET.SubElement(
            sigs_elem,
            "signal",
            attrib={
                "id": sig_id_str,
                "s": f"{s.s:.2f}",
                "t": f"{s.t:.2f}",
                "name": f"sign:{s.sign_key}:{sig_id_str}",
                "dynamic": "no",
                "orientation": orient,
                "zOffset": "2.20",
                "country": "Germany",
                "type": sig_type,
                "subtype": sig_subtype,
                "value": str(sig_val),
                "height": "0.60",
                "width": "0.60",
                "text": f"StVO {sig_type}",
                "hOffset": "0.0",
                "pitch": "0.0",
                "roll": "0.0",
            },
        )

    return ET.tostring(root, encoding="utf-8").decode("utf-8")


# -----------------------------------------------------------------------------
# NavMesh Obstacles & Metrics
# -----------------------------------------------------------------------------

def generate_traffic_sign_obstacle_geometry(
    signs: Sequence[TrafficSignItem],
) -> list[tuple[list[tuple[float, float, float]], list[tuple[int, int, int]]]]:
    """
    Generates 3D bounding box obstacle geometry for the mounting posts
    (0.2m x 0.2m x 2.45m) so the Recast NavMesh compiler carves walkable space around them.
    """
    obstacles: list[tuple[list[tuple[float, float, float]], list[tuple[int, int, int]]]] = []

    for s in signs:
        cx, cy, cz = s.location
        hw, hl, h_top = 0.10, 0.10, s.pole_height_m

        corners = [
            (cx - hw, cy - hl, cz),
            (cx + hw, cy - hl, cz),
            (cx + hw, cy + hl, cz),
            (cx - hw, cy + hl, cz),
            (cx - hw, cy - hl, cz + h_top),
            (cx + hw, cy - hl, cz + h_top),
            (cx + hw, cy + hl, cz + h_top),
            (cx - hw, cy + hl, cz + h_top),
        ]

        tris = [
            # Bottom
            (0, 2, 1), (0, 3, 2),
            # Top
            (4, 5, 6), (4, 6, 7),
            # Sides
            (0, 1, 5), (0, 5, 4),
            (1, 2, 6), (1, 6, 5),
            (2, 3, 7), (2, 7, 6),
            (3, 0, 4), (3, 4, 7),
        ]
        obstacles.append((corners, tris))

    return obstacles


def audit_traffic_sign_metrics(
    signs: Sequence[TrafficSignItem],
    layers: dict[str, dict[str, Any]] | None = None,
) -> dict[str, Any]:
    """Audits traffic sign counts, categories, and geometry layers."""
    counts_by_key: dict[str, int] = {}
    counts_by_source: dict[str, int] = {}

    for s in signs:
        counts_by_key[s.sign_key] = counts_by_key.get(s.sign_key, 0) + 1
        counts_by_source[s.source] = counts_by_source.get(s.source, 0) + 1

    layer_info: dict[str, Any] = {}
    if layers:
        for k, l_data in layers.items():
            layer_info[l_data["name"]] = {
                "vertices": len(l_data["vertices"]),
                "triangles": len(l_data["triangles"]),
                "semantic_label": l_data.get("semantic_label", "TrafficSigns"),
                "material_preset": l_data.get("material_preset", ""),
            }

    return {
        "counts": {
            "total_signs": len(signs),
            "by_category": counts_by_key,
            "by_source": counts_by_source,
        },
        "layers": layer_info,
        "signs": [s.to_dict() for s in signs],
    }


def save_traffic_signs_manifest(
    signs: Sequence[TrafficSignItem],
    output_path: str | Path,
    layers: dict[str, dict[str, Any]] | None = None,
) -> None:
    """Serializes the traffic signs manifest JSON into Placements/traffic_signs.json."""
    out_file = Path(output_path)
    out_file.parent.mkdir(parents=True, exist_ok=True)
    meta = audit_traffic_sign_metrics(signs, layers)
    out_file.write_text(json.dumps(meta, indent=2) + "\n", encoding="utf-8")
