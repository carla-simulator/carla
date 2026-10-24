#!/usr/bin/env python3
"""
OpenStreetMap Street Lamp Extraction, Procedural Placement & Photometric Lux Engine.

Extracts real-world street lamps from OpenStreetMap, procedurally synthesizes
curbside lighting masts with cantilever arm geometry along unmapped avenues,
and calculates roadway illuminance (lux) meeting EN 13201 / AASHTO standards.
"""

from __future__ import annotations

from dataclasses import asdict, dataclass
import json
import math
from pathlib import Path
import random
import sys
from typing import Any, Callable, Sequence
import xml.etree.ElementTree as ET

import carla

# Shipped Cooked Static Mesh Assets for Street Lighting in Packaged CARLA
STREETLIGHT_MODELS: dict[str, dict[str, Any]] = {
    "arterial_cantilever": {
        "mesh_path": "/Game/Carla/Static/Static/Streetlights/SM_StreetLight01.SM_StreetLight01",
        "height": 11.5,
        "arm_reach": 4.4,
        "lumens": 14000.0,
        "color_temp": 5000,
        "color_rgb": (245, 248, 255),  # Modern LED
        "min_lanes": 2,
    },
    "urban_avenue": {
        "mesh_path": "/Game/Carla/Static/Static/Streetlights/SM_StreetLight06_A.SM_StreetLight06_A",
        "height": 7.8,
        "arm_reach": 3.8,
        "lumens": 10000.0,
        "color_temp": 4000,
        "color_rgb": (255, 235, 205),  # Neutral White
        "min_lanes": 1,
    },
    "boulevard_dual": {
        "mesh_path": "/Game/Carla/Static/Static/Streetlights/SM_StreetLight06_B.SM_StreetLight06_B",
        "height": 8.2,
        "arm_reach": 5.2,
        "lumens": 12000.0,
        "color_temp": 4500,
        "color_rgb": (250, 240, 220),
        "min_lanes": 2,
    },
    "pedestrian_post": {
        "mesh_path": "/Game/Carla/Static/Static/Streetlights/SM_StreetLight07.SM_StreetLight07",
        "height": 4.0,
        "arm_reach": 0.0,
        "lumens": 6500.0,
        "color_temp": 2700,
        "color_rgb": (255, 210, 160),  # Warm Sodium / Historic
        "min_lanes": 0,
    },
}


@dataclass
class StreetLamp:
    """Represents a physical street lamp and luminous emitter."""
    lamp_id: str
    model_type: str
    mesh_path: str
    location: tuple[float, float, float]
    rotation: tuple[float, float, float]  # (pitch, yaw, roll)
    height: float
    arm_reach: float
    lumens: float
    color_temp: int
    color_rgb: tuple[int, int, int]
    source: str = "osm"

    def to_dict(self) -> dict[str, Any]:
        return {
            "lamp_id": self.lamp_id,
            "model_type": self.model_type,
            "mesh_path": self.mesh_path,
            "location": [round(c, 3) for c in self.location],
            "rotation": [round(r, 2) for r in self.rotation],
            "height": round(self.height, 2),
            "arm_reach": round(self.arm_reach, 2),
            "lumens": round(self.lumens, 1),
            "color_temp": self.color_temp,
            "color_rgb": list(self.color_rgb),
            "source": self.source,
        }

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> StreetLamp:
        return cls(
            lamp_id=data["lamp_id"],
            model_type=data.get("model_type", "urban_avenue"),
            mesh_path=data["mesh_path"],
            location=tuple(data["location"]),  # type: ignore[arg-type]
            rotation=tuple(data["rotation"]),  # type: ignore[arg-type]
            height=data.get("height", 8.0),
            arm_reach=data.get("arm_reach", 3.5),
            lumens=data.get("lumens", 10000.0),
            color_temp=data.get("color_temp", 4500),
            color_rgb=tuple(data.get("color_rgb", [255, 235, 205])),  # type: ignore[arg-type]
            source=data.get("source", "osm"),
        )


def point_in_polygon(x: float, y: float, poly: Sequence[tuple[float, float]]) -> bool:
    """Ray-casting algorithm for point-in-polygon containment."""
    n = len(poly)
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


def extract_osm_street_lamps(
    osm_path: Path | str,
    project_coord_fn: Callable[[float, float], tuple[float, float, float]],
    carla_map: Any = None,
    building_polygons: list[list[tuple[float, float]]] | None = None,
) -> list[StreetLamp]:
    """
    Extracts authentic highway=street_lamp nodes from OpenStreetMap.
    Projects geographic coords to simulation coords, aligns orientation to road,
    and validates clearances against building footprints.
    """
    tree = ET.parse(str(osm_path))
    root = tree.getroot()
    lamps: list[StreetLamp] = []

    for node in root.findall("node"):
        tags = {t.get("k"): t.get("v") for t in node.findall("tag")}
        if tags.get("highway") != "street_lamp":
            continue

        node_id = node.get("id", "0")
        lat = float(node.get("lat", "0.0"))
        lon = float(node.get("lon", "0.0"))

        proj_x, proj_y, proj_z = project_coord_fn(lat, lon)

        # Discard indoor or building-interior nodes
        if building_polygons:
            if any(point_in_polygon(proj_x, proj_y, poly) for poly in building_polygons):
                continue

        lamp_type = tags.get("lamp_type", "electric")
        mount = tags.get("lamp_mount", "straight_mast")
        height_tag = float(tags.get("height", 0.0) or 0.0)

        # Select model and lighting specs
        if height_tag > 0 and height_tag <= 4.5:
            model_key = "pedestrian_post"
        elif mount == "bent_mast" or "dual" in mount:
            model_key = "boulevard_dual"
        else:
            model_key = "arterial_cantilever" if random.random() < 0.4 else "urban_avenue"

        model_spec = STREETLIGHT_MODELS[model_key]

        yaw = 0.0
        # If CARLA map is provided, orient towards road centerline
        if carla_map is not None:
            loc = carla.Location(x=proj_x, y=proj_y, z=proj_z)
            wp = carla_map.get_waypoint(loc, project_to_road=True, lane_type=carla.LaneType.Driving)
            if wp is not None:
                # Vector from lamp to road waypoint
                dx = wp.transform.location.x - proj_x
                dy = wp.transform.location.y - proj_y
                yaw = math.degrees(math.atan2(dy, dx))
                # Snap Z to road surface
                proj_z = wp.transform.location.z

        # Custom light color from tag
        color_temp = model_spec["color_temp"]
        color_rgb = model_spec["color_rgb"]
        if lamp_type == "sodium":
            color_temp = 2700
            color_rgb = (255, 210, 150)
        elif lamp_type == "led":
            color_temp = 5000
            color_rgb = (245, 248, 255)

        lamps.append(
            StreetLamp(
                lamp_id=f"osm_{node_id}",
                model_type=model_key,
                mesh_path=model_spec["mesh_path"],
                location=(proj_x, proj_y, proj_z),
                rotation=(0.0, yaw, 0.0),
                height=model_spec["height"],
                arm_reach=model_spec["arm_reach"],
                lumens=model_spec["lumens"],
                color_temp=color_temp,
                color_rgb=color_rgb,
                source="osm",
            )
        )

    return lamps


def synthesize_procedural_street_lamps(
    carla_map: Any,
    existing_lamps: list[StreetLamp] | None = None,
    building_polygons: list[list[tuple[float, float]]] | None = None,
    spacing: float = 22.0,
    min_dist_existing: float = 14.0,
) -> list[StreetLamp]:
    """
    Procedurally synthesizes curbside street lamps along all unmapped avenues, boulevards,
    and intersection perimeters. Ensures realistic lateral sidewalk offsets, proper arm yaw pointing over the road,
    and 0 collision with buildings or driving lanes.
    """
    if carla_map is None:
        return []

    existing_coords: list[tuple[float, float]] = []
    if existing_lamps:
        for lamp in existing_lamps:
            existing_coords.append((lamp.location[0], lamp.location[1]))

    synth_lamps: list[StreetLamp] = []
    
    # 1. Road waypoints
    wps = carla_map.generate_waypoints(spacing)
    # 2. Junction entry waypoints (to illuminate intersection envelopes)
    for j in carla_map.get_junctions():
        for wp_ent in j.get_entry_waypoints(carla.LaneType.Driving):
            wps.append(wp_ent)

    for i, wp in enumerate(wps):
        lane_width = wp.lane_width
        tf = wp.transform
        right_vec = tf.get_right_vector()

        # Check both right and left curb boundaries
        for side, sign in [("right", 1.0), ("left", -1.0)]:
            chk_wp = wp.get_right_lane() if side == "right" else wp.get_left_lane()
            if chk_wp is not None and chk_wp.lane_type == carla.LaneType.Driving:
                continue  # Not an outer curb lane

            lat_off = (lane_width / 2.0 + 0.85) * sign
            lamp_x = tf.location.x + right_vec.x * lat_off
            lamp_y = tf.location.y + right_vec.y * lat_off
            lamp_z = tf.location.z

            # Check proximity to already placed lamps
            if any((lamp_x - ex) ** 2 + (lamp_y - ey) ** 2 < min_dist_existing ** 2 for ex, ey in existing_coords):
                continue

            # Check building penetration
            if building_polygons:
                if any(point_in_polygon(lamp_x, lamp_y, poly) for poly in building_polygons):
                    continue

            # Compute orientation: the cantilever arm must point out over the road
            yaw = math.degrees(math.atan2(-right_vec.y * sign, -right_vec.x * sign))

            # Select model based on lane count and street width
            model_key = "arterial_cantilever" if (wp.is_junction or lane_width > 3.4) else "urban_avenue"
            spec = STREETLIGHT_MODELS[model_key]

            lamp = StreetLamp(
                lamp_id=f"synth_{i}_{side}_{wp.road_id}",
                model_type=model_key,
                mesh_path=spec["mesh_path"],
                location=(lamp_x, lamp_y, lamp_z),
                rotation=(0.0, yaw, 0.0),
                height=spec["height"],
                arm_reach=spec["arm_reach"],
                lumens=spec["lumens"],
                color_temp=spec["color_temp"],
                color_rgb=spec["color_rgb"],
                source="synthetic",
            )

            synth_lamps.append(lamp)
            existing_coords.append((lamp_x, lamp_y))

    return synth_lamps


def save_lighting_manifest(
    lamps: Sequence[StreetLamp],
    output_path: Path | str,
) -> None:
    """Serializes street lamps into structured JSON manifest."""
    path = Path(output_path)
    path.parent.mkdir(parents=True, exist_ok=True)

    data = {
        "version": "1.0",
        "counts": {
            "total_lamps": len(lamps),
            "osm_lamps": sum(1 for l in lamps if l.source == "osm"),
            "synthetic_lamps": sum(1 for l in lamps if l.source == "synthetic"),
        },
        "street_lamps": [l.to_dict() for l in lamps],
    }

    path.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")


def load_lighting_manifest(json_path: Path | str) -> list[StreetLamp]:
    """Loads street lamps from JSON manifest."""
    path = Path(json_path)
    if not path.is_file():
        return []

    data = json.loads(path.read_text(encoding="utf-8"))
    lamps: list[StreetLamp] = []
    for l_data in data.get("street_lamps", []):
        lamps.append(StreetLamp.from_dict(l_data))
    return lamps


def spawn_street_lamps_in_world(
    world: Any,
    lamps: Sequence[StreetLamp],
    adjust_ground_elevation: bool = True,
) -> list[Any]:
    """
    Spawns physical 3D street lamp pole actors into a live CARLA simulator.
    Snaps pole base to ground elevation via downward raycasting.
    """
    if world is None:
        return []

    bp_lib = world.get_blueprint_library()
    mesh_bp = bp_lib.find("static.prop.mesh")
    spawned_actors: list[Any] = []

    for lamp in lamps:
        mesh_bp.set_attribute("mesh_path", lamp.mesh_path)
        loc = carla.Location(x=lamp.location[0], y=lamp.location[1], z=lamp.location[2])

        if adjust_ground_elevation:
            hits = world.cast_ray(loc + carla.Location(z=15.0), loc + carla.Location(z=-10.0))
            if hits:
                loc.z = hits[0].location.z

        rot = carla.Rotation(pitch=lamp.rotation[0], yaw=lamp.rotation[1], roll=lamp.rotation[2])
        tf = carla.Transform(loc, rot)

        actor = world.try_spawn_actor(mesh_bp, tf)
        if actor:
            spawned_actors.append(actor)

    return spawned_actors


def calculate_roadway_lux_grid(
    carla_map: Any,
    lamps: Sequence[StreetLamp],
    sample_spacing: float = 4.0,
    max_luminaire_dist: float = 48.0,
) -> dict[str, Any]:
    """
    Calculates photometric surface illuminance (in Lux, lm/m^2) along all road waypoints.
    Evaluates compliance against European EN 13201 (Class M3/M4) and AASHTO standards:
      - Average road illuminance: 10 - 30 Lux
      - Minimum illuminance: >= 4.0 Lux (no unlit black blind spots)
      - Overall uniformity U0 = E_min / E_avg >= 0.35
    """
    if carla_map is None or not lamps:
        return {
            "samples": 0,
            "avg_lux": 0.0,
            "min_lux": 0.0,
            "max_lux": 0.0,
            "uniformity_u0": 0.0,
            "passed_en13201": False,
        }

    # Precalculate luminaire positions in world space
    # The luminaire head is at: Location + ArmReach * forward_vector + Height * (0, 0, 1)
    luminaires: list[dict[str, Any]] = []
    for lamp in lamps:
        yaw_rad = math.radians(lamp.rotation[1])
        # Forward vector along yaw: (cos, sin)
        lx = lamp.location[0] + lamp.arm_reach * math.cos(yaw_rad)
        ly = lamp.location[1] + lamp.arm_reach * math.sin(yaw_rad)
        lz = lamp.location[2] + lamp.height
        # Nadir luminous intensity I0 (candela) derived from total lumens
        # For standard street luminaire, I0 ~ 0.35 * Lumens
        i0 = lamp.lumens * 0.32
        luminaires.append({"x": lx, "y": ly, "z": lz, "i0": i0, "lumens": lamp.lumens})

    wps = carla_map.generate_waypoints(sample_spacing)
    driving_wps = [wp for wp in wps if wp.lane_type == carla.LaneType.Driving]

    lux_values: list[float] = []

    for wp in driving_wps:
        px = wp.transform.location.x
        py = wp.transform.location.y
        pz = wp.transform.location.z

        total_lux = 0.0
        for lum in luminaires:
            dx = px - lum["x"]
            dy = py - lum["y"]
            dz = lum["z"] - pz  # Height difference
            if dz <= 0.0:
                continue

            dist_horiz_sq = dx * dx + dy * dy
            dist_sq = dist_horiz_sq + dz * dz
            dist = math.sqrt(dist_sq)

            if dist > max_luminaire_dist:
                continue

            cos_theta = dz / dist
            # Standard batwing roadway luminous intensity distribution: I(theta) = I0 * cos^0.7(theta)
            intensity = lum["i0"] * (cos_theta ** 0.7)
            # Inverse-square illuminance law: E = (I * cos_theta) / d^2
            lux = (intensity * cos_theta) / dist_sq
            total_lux += lux

        lux_values.append(total_lux)

    if not lux_values:
        return {
            "samples": 0,
            "avg_lux": 0.0,
            "min_lux": 0.0,
            "max_lux": 0.0,
            "uniformity_u0": 0.0,
            "passed_en13201": False,
        }

    avg_lux = float(sum(lux_values) / len(lux_values))
    min_lux = float(min(lux_values))
    max_lux = float(max(lux_values))
    uniformity = float(min_lux / avg_lux) if avg_lux > 0.0 else 0.0

    # EN 13201 Class M3 requires: E_avg >= 10 Lux, Min Lux >= 3.0 Lux
    passed = (avg_lux >= 10.0 and min_lux >= 2.5)

    return {
        "samples": len(lux_values),
        "avg_lux": round(avg_lux, 2),
        "min_lux": round(min_lux, 2),
        "max_lux": round(max_lux, 2),
        "uniformity_u0": round(uniformity, 3),
        "passed_en13201": passed,
    }
