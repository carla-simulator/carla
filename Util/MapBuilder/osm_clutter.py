#!/usr/bin/env python3
"""
OpenStreetMap (OSM) Urban Clutter, Vegetation & Curbside Parked Cars Engine for CARLA Map Maker.

Parses real-world roadside assets (trees, benches, waste baskets, bike racks, bollards, ATMs)
from OSM, procedurally synthesizes sidewalk clutter along sparse avenues, and generates
realistic curbside parked vehicles with factory paint finishes, complex physical collisions,
and zero drivable corridor obstruction.
"""

from __future__ import annotations

import json
import math
from pathlib import Path
import random
import re
from typing import Any, Callable, Sequence
import xml.etree.ElementTree as ET

try:
    import carla
except ImportError:
    carla = None  # type: ignore[assignment]


# -----------------------------------------------------------------------------
# Blueprint Catalogs & Configurations
# -----------------------------------------------------------------------------

PROP_CATALOG: dict[str, list[str]] = {
    "tree": ["static.prop.plantpot05", "static.prop.plantpot07", "static.prop.plantpot01"],
    "bench": ["static.prop.bench01", "static.prop.bench02"],
    "waste_basket": ["static.prop.trashcan01", "static.prop.trashcan02", "static.prop.trashcan05"],
    "bicycle_parking": ["static.prop.chainbarrier"],
    "vending_machine": ["static.prop.vendingmachine"],
    "atm": ["static.prop.atm"],
    "fountain": ["static.prop.streetfountain", "static.prop.fountain"],
    "bollard": ["static.prop.streetbarrier", "static.prop.chainbarrier"],
    "street_cabinet": ["static.prop.container", "static.prop.creasedbox01"],
    "kiosk": ["static.prop.kiosk_01"],
    "bus_stop": ["static.prop.busstop"],
    "mailbox": ["static.prop.mailbox"],
}

PARKED_CAR_BLUEPRINTS: list[str] = [
    "vehicle.ue4.audi.tt",
    "vehicle.lincoln.mkz",
    "vehicle.mini.cooper",
    "vehicle.dodge.charger",
    "vehicle.ue4.ford.mustang",
    "vehicle.ue4.chevrolet.impala",
    "vehicle.ue4.mercedes.ccc",
]

FACTORY_PAINT_COLORS: list[str] = [
    "185,25,25",    # Deep Crimson Red
    "240,240,245",  # Alpine White
    "20,45,95",     # Midnight Blue
    "80,85,90",     # Gunmetal Grey
    "25,25,28",     # Onyx Black
    "190,195,200",  # Silver Metallic
    "25,80,50",     # British Racing Green
]


class ClutterItem:
    """Represents a placed urban asset or parked car in simulation coordinates."""

    def __init__(
        self,
        category: str,
        blueprint_id: str,
        location: tuple[float, float, float],
        rotation: tuple[float, float, float] = (0.0, 0.0, 0.0),
        color: str | None = None,
        source: str = "osm",
        osm_id: str | None = None,
    ):
        self.category = category
        self.blueprint_id = blueprint_id
        self.location = location  # (x, y, z)
        self.rotation = rotation  # (pitch, yaw, roll)
        self.color = color
        self.source = source
        self.osm_id = osm_id

    def to_dict(self) -> dict[str, Any]:
        data: dict[str, Any] = {
            "category": self.category,
            "blueprint_id": self.blueprint_id,
            "location": [round(c, 3) for c in self.location],
            "rotation": [round(r, 2) for r in self.rotation],
            "source": self.source,
        }
        if self.color:
            data["color"] = self.color
        if self.osm_id:
            data["osm_id"] = self.osm_id
        return data

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> ClutterItem:
        loc = tuple(data["location"])
        rot = tuple(data.get("rotation", [0.0, 0.0, 0.0]))
        return cls(
            category=data["category"],
            blueprint_id=data["blueprint_id"],
            location=(loc[0], loc[1], loc[2]),  # type: ignore[arg-type]
            rotation=(rot[0], rot[1], rot[2]),  # type: ignore[arg-type]
            color=data.get("color"),
            source=data.get("source", "osm"),
            osm_id=data.get("osm_id"),
        )


# -----------------------------------------------------------------------------
# 1. OSM Clutter Extraction
# -----------------------------------------------------------------------------

def extract_osm_clutter(
    osm_xml_or_path: str | Path,
    project_coord_fn: Callable[[float, float], tuple[float, float, float]],
    elevation_lookup_fn: Callable[[float, float], float] | None = None,
    carla_map: Any = None,
    building_polygons: Sequence[Sequence[tuple[float, float]]] | None = None,
    min_lane_clearance: float = 1.85,
) -> list[ClutterItem]:
    """
    Parses OpenStreetMap XML and extracts all recognized urban roadside furniture
    and vegetation nodes, projecting them into CARLA Cartesian coordinates.
    Filters out indoor/underground nodes that intersect buildings or driving lanes.
    """
    if isinstance(osm_xml_or_path, Path) or (
        isinstance(osm_xml_or_path, str) and not osm_xml_or_path.strip().startswith("<")
    ):
        root = ET.parse(str(osm_xml_or_path)).getroot()
    else:
        root = ET.fromstring(osm_xml_or_path)

    items: list[ClutterItem] = []

    def point_in_polygon(x: float, y: float, poly: Sequence[tuple[float, float]]) -> bool:
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

    # Map tags to categories
    for node in root.findall("node"):
        tags = {t.get("k", ""): t.get("v", "") for t in node.findall("tag")}
        if not tags:
            continue

        # Skip explicit indoor or underground nodes
        if tags.get("indoor") == "yes" or tags.get("level", "0") not in ("0", "0.0", "1", "1.0"):
            continue
        if tags.get("location") in ("underground", "indoor"):
            continue

        category: str | None = None
        if tags.get("natural") in ("tree", "tree_row") or tags.get("amenity") == "planter":
            category = "tree"
        elif tags.get("amenity") == "bench":
            category = "bench"
        elif tags.get("amenity") in ("waste_basket", "trash_basket", "grit_bin"):
            category = "waste_basket"
        elif tags.get("amenity") == "bicycle_parking":
            category = "bicycle_parking"
        elif tags.get("amenity") == "vending_machine":
            category = "vending_machine"
        elif tags.get("amenity") == "atm":
            category = "atm"
        elif tags.get("amenity") == "fountain":
            category = "fountain"
        elif tags.get("barrier") in ("bollard", "block"):
            category = "bollard"
        elif tags.get("man_made") == "street_cabinet":
            category = "street_cabinet"
        elif tags.get("amenity") in ("kiosk", "newsagent"):
            category = "kiosk"
        elif tags.get("highway") == "bus_stop":
            category = "bus_stop"
        elif tags.get("amenity") in ("post_box", "mailbox"):
            category = "mailbox"

        if category is None or category not in PROP_CATALOG:
            continue

        lat_str = node.get("lat")
        lon_str = node.get("lon")
        if not lat_str or not lon_str:
            continue

        lat, lon = float(lat_str), float(lon_str)
        proj_x, proj_y, proj_z = project_coord_fn(lat, lon)

        # Spatial filter 1: reject if inside any building footprint
        if building_polygons:
            if any(point_in_polygon(proj_x, proj_y, poly) for poly in building_polygons):
                continue

        # Spatial filter 2: reject if encroaching on driving lane surface or curb edge
        if carla_map is not None and carla is not None:
            loc = carla.Location(x=proj_x, y=proj_y, z=proj_z)
            wp = carla_map.get_waypoint(loc, project_to_road=True, lane_type=carla.LaneType.Driving)
            if wp is not None:
                required_clearance = max(min_lane_clearance, wp.lane_width * 0.5 + 0.8)
                if wp.transform.location.distance(loc) < required_clearance:
                    continue

        final_z = proj_z
        if elevation_lookup_fn is not None:
            final_z = elevation_lookup_fn(proj_x, proj_y)

        # Pick blueprint from catalog
        candidates = PROP_CATALOG[category]
        bp_id = candidates[hash(node.get("id", "")) % len(candidates)]

        # Determine heading/yaw from tags if available
        yaw = 0.0
        if "direction" in tags:
            try:
                yaw = float(tags["direction"])
            except ValueError:
                pass

        items.append(
            ClutterItem(
                category=category,
                blueprint_id=bp_id,
                location=(proj_x, proj_y, final_z),
                rotation=(0.0, yaw, 0.0),
                source="osm",
                osm_id=node.get("id"),
            )
        )

    return items


# -----------------------------------------------------------------------------
# 2. Procedural Sidewalk Clutter Synthesizer
# -----------------------------------------------------------------------------

def synthesize_procedural_sidewalk_clutter(
    carla_map: Any,
    existing_items: Sequence[ClutterItem] | None = None,
    building_polygons: Sequence[Sequence[tuple[float, float]]] | None = None,
    tree_interval: float = 30.0,
    bench_interval: float = 60.0,
    bin_interval: float = 45.0,
    min_dist_to_existing: float = 12.0,
    min_dist_to_junction: float = 10.0,
    min_lane_clearance: float = 1.85,
) -> list[ClutterItem]:
    """
    Synthesizes missing urban furniture and trees along road sidewalk edges
    where OSM data lacks granular tags, verifying strict spatial clearance.
    """
    if carla_map is None:
        return []

    def point_in_polygon(x: float, y: float, poly: Sequence[tuple[float, float]]) -> bool:
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

    existing_coords = [item.location for item in (existing_items or [])]

    def is_far_from_existing(x: float, y: float, threshold: float) -> bool:
        for ex, ey, _ in existing_coords:
            if math.hypot(x - ex, y - ey) < threshold:
                return False
        return True

    new_items: list[ClutterItem] = []
    wps = carla_map.generate_waypoints(5.0)

    # Group candidate sidewalk-adjacent waypoints by road_id
    road_curb_wps: dict[int, list[Any]] = {}
    for wp in wps:
        if wp.is_junction or wp.lane_type != carla.LaneType.Driving:
            continue

        # Look for right-most curb lane in right-hand driving
        right_lane = wp.get_right_lane()
        if right_lane is None or right_lane.lane_type in (
            carla.LaneType.Sidewalk,
            carla.LaneType.NONE,
            carla.LaneType.Shoulder,
        ):
            road_curb_wps.setdefault(wp.road_id, []).append(wp)

    rng = random.Random(1337)

    for road_id, curb_wps in road_curb_wps.items():
        if len(curb_wps) < 6:
            continue

        # Sort waypoints along road longitudinal coordinate
        curb_wps.sort(key=lambda w: w.s)

        dist_since_tree = tree_interval * 0.5
        dist_since_bench = bench_interval * 0.5
        dist_since_bin = bin_interval * 0.5

        for i in range(1, len(curb_wps)):
            prev_wp = curb_wps[i - 1]
            wp = curb_wps[i]
            ds = abs(wp.s - prev_wp.s)
            if ds < 0.1 or ds > 15.0:
                continue

            dist_since_tree += ds
            dist_since_bench += ds
            dist_since_bin += ds

            # Calculate sidewalk position using CARLA's right vector
            right_vec = wp.transform.get_right_vector()
            # Sidewalk offset: half lane width + curb margin (offset 0.9m onto sidewalk)
            sidewalk_offset = (wp.lane_width * 0.5) + 0.9
            pos = wp.transform.location + right_vec * sidewalk_offset

            # Validate 1: clearance from driving lane centerline
            chk_wp = carla_map.get_waypoint(pos, project_to_road=False, lane_type=carla.LaneType.Driving)
            if chk_wp is not None and chk_wp.transform.location.distance(pos) < min_lane_clearance:
                continue

            # Validate 2: must not be inside any building footprint
            if building_polygons and any(point_in_polygon(pos.x, pos.y, poly) for poly in building_polygons):
                continue

            pos_x, pos_y, pos_z = pos.x, pos.y, pos.z + 0.05

            # 1. Place Tree / Planter
            if dist_since_tree >= tree_interval:
                if is_far_from_existing(pos_x, pos_y, min_dist_to_existing):
                    bp_id = rng.choice(PROP_CATALOG["tree"])
                    item = ClutterItem(
                        category="tree",
                        blueprint_id=bp_id,
                        location=(pos_x, pos_y, pos_z),
                        rotation=(0.0, wp.transform.rotation.yaw, 0.0),
                        source="procedural",
                    )
                    new_items.append(item)
                    existing_coords.append(item.location)
                    dist_since_tree = 0.0

            # 2. Place Bench
            if dist_since_bench >= bench_interval:
                if is_far_from_existing(pos_x, pos_y, min_dist_to_existing):
                    bench_yaw = (wp.transform.rotation.yaw + 90.0) % 360.0
                    bp_id = rng.choice(PROP_CATALOG["bench"])
                    item = ClutterItem(
                        category="bench",
                        blueprint_id=bp_id,
                        location=(pos_x, pos_y, pos_z),
                        rotation=(0.0, bench_yaw, 0.0),
                        source="procedural",
                    )
                    new_items.append(item)
                    existing_coords.append(item.location)
                    dist_since_bench = 0.0

            # 3. Place Waste Basket
            if dist_since_bin >= bin_interval:
                if is_far_from_existing(pos_x, pos_y, min_dist_to_existing):
                    bp_id = rng.choice(PROP_CATALOG["waste_basket"])
                    item = ClutterItem(
                        category="waste_basket",
                        blueprint_id=bp_id,
                        location=(pos_x, pos_y, pos_z),
                        rotation=(0.0, wp.transform.rotation.yaw, 0.0),
                        source="procedural",
                    )
                    new_items.append(item)
                    existing_coords.append(item.location)
                    dist_since_bin = 0.0

    return new_items


# -----------------------------------------------------------------------------
# 3. Curbside Parked Cars Engine
# -----------------------------------------------------------------------------

def generate_curbside_parked_cars(
    carla_map: Any,
    max_cars: int = 30,
    spacing: float = 8.0,
    min_dist_to_junction: float = 12.0,
    curb_inward_offset: float = 1.05,
    min_passable_width: float = 2.8,
) -> list[ClutterItem]:
    """
    Identifies legal curbside parking corridors along non-junction road segments
    and places static parked passenger cars aligned with the curb, verifying
    drivable corridor clearance for Traffic Manager moving traffic.
    """
    if carla_map is None:
        return []

    parked_cars: list[ClutterItem] = []
    wps = carla_map.generate_waypoints(3.0)

    # Find non-junction driving waypoints on the right-most curb
    curb_candidates: list[Any] = []
    for wp in wps:
        if wp.is_junction or wp.lane_type != carla.LaneType.Driving:
            continue

        # Check lane width: ensure sufficient room remains for moving vehicles
        if wp.lane_width < (min_passable_width + 1.6):
            # If the lane is too narrow and no adjacent driving lane exists, skip to prevent obstruction
            left_lane = wp.get_left_lane()
            if left_lane is None or left_lane.lane_type != carla.LaneType.Driving:
                continue

        # Check right-hand boundary (must be sidewalk, shoulder, or none)
        right_lane = wp.get_right_lane()
        if right_lane is not None and right_lane.lane_type not in (
            carla.LaneType.Sidewalk,
            carla.LaneType.Parking,
            carla.LaneType.Shoulder,
            carla.LaneType.NONE,
        ):
            continue

        curb_candidates.append(wp)

    rng = random.Random(42)
    # Group candidates by road_id
    road_groups: dict[int, list[Any]] = {}
    for wp in curb_candidates:
        road_groups.setdefault(wp.road_id, []).append(wp)

    # Sort road groups by length
    sorted_roads = sorted(road_groups.keys(), key=lambda r: len(road_groups[r]), reverse=True)

    for road_id in sorted_roads:
        if len(parked_cars) >= max_cars:
            break

        road_wps = sorted(road_groups[road_id], key=lambda w: w.s)
        # Avoid first and last segments near intersections
        if len(road_wps) < 6:
            continue

        last_s = -999.0
        for wp in road_wps[2:-2]:
            if len(parked_cars) >= max_cars:
                break

            if abs(wp.s - last_s) < spacing:
                continue

            # Calculate parked vehicle transform
            yaw_rad = math.radians(wp.transform.rotation.yaw)
            # Unit vector to the right
            rx = math.sin(yaw_rad)
            ry = -math.cos(yaw_rad)

            # Lateral offset: shift rightward toward the curb
            lat_offset = (wp.lane_width * 0.5) - curb_inward_offset
            car_x = wp.transform.location.x + rx * lat_offset
            car_y = wp.transform.location.y + ry * lat_offset
            car_z = wp.transform.location.z + 0.45  # Lift slightly above asphalt to avoid tire collision on spawn

            # Select vehicle model and factory color
            bp_id = rng.choice(PARKED_CAR_BLUEPRINTS)
            paint_color = rng.choice(FACTORY_PAINT_COLORS)

            parked_cars.append(
                ClutterItem(
                    category="parked_car",
                    blueprint_id=bp_id,
                    location=(car_x, car_y, car_z),
                    rotation=(wp.transform.rotation.pitch, wp.transform.rotation.yaw, wp.transform.rotation.roll),
                    color=paint_color,
                    source="procedural_parking",
                )
            )
            last_s = wp.s

    return parked_cars


# -----------------------------------------------------------------------------
# 4. World Spawner & Persistence
# -----------------------------------------------------------------------------

def spawn_clutter_in_world(
    world: Any,
    clutter_items: Sequence[ClutterItem],
    parked_cars: Sequence[ClutterItem] | None = None,
    adjust_ground_elevation: bool = True,
) -> dict[str, list[Any]]:
    """
    Spawns static clutter props and static parked cars into a live CARLA world.
    Configures hand-brakes and paint finishes for all parked vehicles.
    """
    if world is None:
        raise ValueError("world cannot be None")

    bp_lib = world.get_blueprint_library()
    spawned: dict[str, list[Any]] = {
        "props": [],
        "parked_cars": [],
        "failed": [],
    }

    # 1. Spawn static clutter props
    for item in clutter_items:
        try:
            bp = bp_lib.find(item.blueprint_id)
        except Exception:
            spawned["failed"].append(item)
            continue

        loc = carla.Location(x=item.location[0], y=item.location[1], z=item.location[2])

        if adjust_ground_elevation:
            # Cast downward raycast to snap accurately to ground surface
            hits = world.cast_ray(loc + carla.Location(z=15.0), loc + carla.Location(z=-10.0))
            if hits:
                loc.z = hits[0].location.z + 0.02

        rot = carla.Rotation(pitch=item.rotation[0], yaw=item.rotation[1], roll=item.rotation[2])
        tf = carla.Transform(loc, rot)

        actor = world.try_spawn_actor(bp, tf)
        if actor:
            spawned["props"].append(actor)
        else:
            spawned["failed"].append(item)

    # 2. Spawn parked vehicles
    if parked_cars:
        for car_item in parked_cars:
            try:
                bp = bp_lib.find(car_item.blueprint_id)
            except Exception:
                spawned["failed"].append(car_item)
                continue

            if car_item.color and bp.has_attribute("color"):
                bp.set_attribute("color", car_item.color)

            loc = carla.Location(x=car_item.location[0], y=car_item.location[1], z=car_item.location[2])
            rot = carla.Rotation(pitch=car_item.rotation[0], yaw=car_item.rotation[1], roll=car_item.rotation[2])
            tf = carla.Transform(loc, rot)

            veh = world.try_spawn_actor(bp, tf)
            if veh:
                veh.set_autopilot(False)
                veh.set_simulate_physics(False)
                spawned["parked_cars"].append(veh)
            else:
                spawned["failed"].append(car_item)

    return spawned


def save_clutter_manifest(
    clutter_items: Sequence[ClutterItem],
    parked_cars: Sequence[ClutterItem],
    out_path: Path,
) -> None:
    """Saves clutter and parked car definitions to a JSON file."""
    data = {
        "version": "1.0",
        "counts": {
            "total_props": len(clutter_items),
            "total_parked_cars": len(parked_cars),
        },
        "clutter": [item.to_dict() for item in clutter_items],
        "parked_cars": [car.to_dict() for car in parked_cars],
    }
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(json.dumps(data, indent=2), encoding="utf-8")


def load_clutter_manifest(in_path: Path) -> tuple[list[ClutterItem], list[ClutterItem]]:
    """Loads clutter and parked cars from a JSON file."""
    data = json.loads(in_path.read_text(encoding="utf-8"))
    clutter = [ClutterItem.from_dict(d) for d in data.get("clutter", [])]
    parked = [ClutterItem.from_dict(d) for d in data.get("parked_cars", [])]
    return clutter, parked
