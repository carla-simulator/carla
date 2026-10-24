#!/usr/bin/env python3
"""
OpenStreetMap Traffic Signals Extraction, OpenDRIVE Signal Injection & Physical Light Management.

Ingests authentic traffic signal nodes from OpenStreetMap, projects coordinates to OpenDRIVE
road geometry (road_id, lane_id, s, t), injects compliant OpenDRIVE <signal> elements,
and generates physical 3D traffic light pole and signal head placements.
"""

from __future__ import annotations

from dataclasses import asdict, dataclass
import json
import math
from pathlib import Path
import sys
from typing import Any, Callable, Sequence
import xml.etree.ElementTree as ET

import carla

TRAFFIC_LIGHT_MODELS: dict[str, dict[str, Any]] = {
    "cantilever_mast": {
        "mesh_path": "/Game/Carla/Static/Pole/SM_TrafficLightPole01.SM_TrafficLightPole01",
        "blueprint_id": "traffic.traffic_light",
        "height": 6.8,
        "arm_reach": 4.5,
        "has_signal_head": True,
    },
    "vertical_post": {
        "mesh_path": "/Game/Carla/Static/Pole/SM_TrafficLightPole02.SM_TrafficLightPole02",
        "blueprint_id": "traffic.traffic_light",
        "height": 4.5,
        "arm_reach": 0.0,
        "has_signal_head": True,
    },
    "pedestrian_signal": {
        "mesh_path": "/Game/Carla/Static/TrafficLight/SM_TrafficLight_Pedestrian01_A.SM_TrafficLight_Pedestrian01_A",
        "blueprint_id": "static.prop.mesh",
        "height": 2.8,
        "arm_reach": 0.0,
        "has_signal_head": True,
    },
}


@dataclass
class TrafficSignal:
    """Represents a traffic signal device, OpenDRIVE landmark, and physical luminaire."""
    signal_id: str
    node_id: str = ""
    road_id: int = -1
    lane_id: int = -1
    from_lane: int = -1
    to_lane: int = -1
    s: float = 0.0
    t: float = 0.0
    location: tuple[float, float, float] = (0.0, 0.0, 0.0)
    rotation: tuple[float, float, float] = (0.0, 0.0, 0.0)  # (pitch, yaw, roll)
    model_type: str = "cantilever_mast"
    mesh_path: str = ""
    is_dynamic: bool = True
    source: str = "osm"
    is_pedestrian: bool = False

    def __post_init__(self) -> None:
        if not self.mesh_path and self.model_type in TRAFFIC_LIGHT_MODELS:
            self.mesh_path = TRAFFIC_LIGHT_MODELS[self.model_type]["mesh_path"]
        if self.model_type == "pedestrian_signal":
            self.is_pedestrian = True
        if self.from_lane == -1 and self.to_lane == -1 and self.lane_id != -1:
            self.from_lane = self.lane_id
            self.to_lane = self.lane_id

    def to_dict(self) -> dict[str, Any]:
        return {
            "signal_id": self.signal_id,
            "node_id": self.node_id,
            "road_id": self.road_id,
            "lane_id": self.lane_id,
            "from_lane": self.from_lane,
            "to_lane": self.to_lane,
            "s": round(self.s, 2),
            "t": round(self.t, 2),
            "location": [round(c, 3) for c in self.location],
            "rotation": [round(r, 2) for r in self.rotation],
            "model_type": self.model_type,
            "mesh_path": self.mesh_path,
            "is_dynamic": self.is_dynamic,
            "source": self.source,
            "is_pedestrian": self.is_pedestrian,
        }

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> TrafficSignal:
        return cls(
            signal_id=str(data["signal_id"]),
            node_id=str(data.get("node_id", "0")),
            road_id=int(data["road_id"]),
            lane_id=int(data["lane_id"]),
            from_lane=int(data.get("from_lane", data.get("lane_id", -1))),
            to_lane=int(data.get("to_lane", data.get("lane_id", -1))),
            s=float(data["s"]),
            t=float(data["t"]),
            location=tuple(data["location"]),  # type: ignore[arg-type]
            rotation=tuple(data["rotation"]),  # type: ignore[arg-type]
            model_type=data.get("model_type", "cantilever_mast"),
            mesh_path=data.get("mesh_path", TRAFFIC_LIGHT_MODELS["cantilever_mast"]["mesh_path"]),
            is_dynamic=data.get("is_dynamic", True),
            source=data.get("source", "osm"),
            is_pedestrian=data.get("is_pedestrian", data.get("model_type") == "pedestrian_signal"),
        )


def make_lat_lon_projector(
    carla_map: Any | None = None,
    xodr_xml: str | None = None,
) -> Callable[[float, float], tuple[float, float, float]]:
    """Creates a geographic-to-simulation projection function."""
    off_x = 0.0
    off_y = 0.0
    if not xodr_xml and carla_map is not None:
        try:
            xodr_xml = carla_map.to_opendrive()
        except Exception:
            pass

    if xodr_xml:
        try:
            root = ET.fromstring(xodr_xml)
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
                res = carla_map.geolocation_to_transform(carla.GeoLocation(lat, lon, 0.0))
                loc = res.location if hasattr(res, "location") else res
                return loc.x + off_x, -(loc.y + off_y), loc.z
            except Exception:
                pass
        return lon * 111320.0, lat * 110540.0, 0.0

    return _project


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


def extract_osm_traffic_signals(
    osm_path: Path | str,
    project_coord_fn: Callable[[float, float], tuple[float, float, float]] | None = None,
    carla_map: Any = None,
    building_polygons: list[list[tuple[float, float]]] | None = None,
) -> list[TrafficSignal]:
    """
    Extracts authentic highway=traffic_signals and crossing=traffic_signals nodes from OSM,
    projects geographic coordinates to simulation coordinates, projects to the nearest road waypoint,
    consolidates multi-lane stop lines into single master vehicular signals per approach,
    and sets lateral curb setback (t_pole) to keep poles outside driving lanes.
    """
    if project_coord_fn is None:
        project_coord_fn = make_lat_lon_projector(carla_map)

    tree = ET.parse(str(osm_path))
    root = tree.getroot()
    signals: list[TrafficSignal] = []

    hw_candidates: list[dict[str, Any]] = []
    ped_candidates: list[dict[str, Any]] = []
    seen_hw_coords: set[tuple[int, int]] = set()
    seen_ped_coords: set[tuple[int, int]] = set()

    for node in root.findall("node"):
        tags = {t.get("k", ""): t.get("v", "") for t in node.findall("tag")}
        is_hw_signal = tags.get("highway") == "traffic_signals"
        is_cr_signal = tags.get("crossing") == "traffic_signals"

        if not (is_hw_signal or is_cr_signal):
            continue

        node_id = node.get("id", "0")
        lat = float(node.get("lat", "0.0"))
        lon = float(node.get("lon", "0.0"))

        proj_x, proj_y, proj_z = project_coord_fn(lat, lon)

        # Filter out interior nodes inside buildings
        if building_polygons:
            if any(point_in_polygon(proj_x, proj_y, poly) for poly in building_polygons):
                continue

        item = {
            "node_id": node_id,
            "is_hw": is_hw_signal,
            "is_cr": is_cr_signal,
            "proj_pos": (proj_x, proj_y, proj_z),
            "wp": None,
        }

        if carla_map is not None:
            loc = carla.Location(x=proj_x, y=proj_y, z=proj_z)
            wp = carla_map.get_waypoint(loc, project_to_road=True, lane_type=carla.LaneType.Driving)
            item["wp"] = wp

        if is_hw_signal:
            coord_key = (int(round(proj_x / 3.0)), int(round(proj_y / 3.0)))
            if coord_key not in seen_hw_coords:
                seen_hw_coords.add(coord_key)
                hw_candidates.append(item)
        else:
            coord_key = (int(round(proj_x / 3.0)), int(round(proj_y / 3.0)))
            if coord_key not in seen_ped_coords:
                seen_ped_coords.add(coord_key)
                ped_candidates.append(item)

    # 1. Process Vehicular Signals: Directional Approach Clustering & Curb Setback
    valid_road_lanes = (
        carla.LaneType.Driving,
        carla.LaneType.Biking,
        carla.LaneType.Shoulder,
        carla.LaneType.Parking,
    ) if carla_map is not None else ()

    if carla_map is not None:
        from collections import defaultdict
        hw_groups: dict[tuple[int, int], list[dict[str, Any]]] = defaultdict(list)
        unmapped_hw: list[dict[str, Any]] = []

        for cand in hw_candidates:
            wp = cand["wp"]
            if wp is not None:
                lane_sign = -1 if wp.lane_id < 0 else 1
                hw_groups[(wp.road_id, lane_sign)].append(cand)
            else:
                unmapped_hw.append(cand)

        for (road_id, lane_sign), c_list in hw_groups.items():
            c_list.sort(key=lambda x: x["wp"].s)
            clusters: list[list[dict[str, Any]]] = []
            cur_cluster = [c_list[0]]
            for item in c_list[1:]:
                if item["wp"].s - cur_cluster[-1]["wp"].s <= 15.0:
                    cur_cluster.append(item)
                else:
                    clusters.append(cur_cluster)
                    cur_cluster = [item]
            clusters.append(cur_cluster)

            for cluster in clusters:
                # Pick earliest stop line on this approach
                rep = cluster[0]
                wp = rep["wp"]
                outermost_lane = wp
                driving_lane_ids = [wp.lane_id]

                if wp.lane_id < 0:
                    while (
                        outermost_lane.get_right_lane() is not None
                        and outermost_lane.get_right_lane().lane_type in valid_road_lanes
                    ):
                        outermost_lane = outermost_lane.get_right_lane()
                        if outermost_lane.lane_type == carla.LaneType.Driving:
                            driving_lane_ids.append(outermost_lane.lane_id)
                    carriageway_w = 0.0
                    check = outermost_lane
                    while check is not None and check.lane_id < 0:
                        carriageway_w += check.lane_width
                        check = check.get_left_lane()
                        if check is not None and check.lane_type == carla.LaneType.Driving:
                            if check.lane_id not in driving_lane_ids:
                                driving_lane_ids.append(check.lane_id)
                    t_pole = -(carriageway_w + 0.8)
                    right_v = outermost_lane.transform.get_right_vector()
                    pole_pos = outermost_lane.transform.location + right_v * (
                        outermost_lane.lane_width * 0.5 + 0.8
                    )
                else:
                    while (
                        outermost_lane.get_left_lane() is not None
                        and outermost_lane.get_left_lane().lane_type in valid_road_lanes
                    ):
                        outermost_lane = outermost_lane.get_left_lane()
                        if outermost_lane.lane_type == carla.LaneType.Driving:
                            driving_lane_ids.append(outermost_lane.lane_id)
                    carriageway_w = 0.0
                    check = outermost_lane
                    while check is not None and check.lane_id > 0:
                        carriageway_w += check.lane_width
                        check = check.get_right_lane()
                        if check is not None and check.lane_type == carla.LaneType.Driving:
                            if check.lane_id not in driving_lane_ids:
                                driving_lane_ids.append(check.lane_id)
                    t_pole = +(carriageway_w + 0.8)
                    left_v = -outermost_lane.transform.get_right_vector()
                    pole_pos = outermost_lane.transform.location + left_v * (
                        outermost_lane.lane_width * 0.5 + 0.8
                    )

                yaw = wp.transform.rotation.yaw + 90.0
                if yaw > 180.0:
                    yaw -= 360.0
                elif yaw < -180.0:
                    yaw += 360.0

                from_l = min(driving_lane_ids)
                to_l = max(driving_lane_ids)

                model_key = "cantilever_mast" if len(driving_lane_ids) > 1 or wp.lane_width > 3.2 else "vertical_post"
                spec = TRAFFIC_LIGHT_MODELS[model_key]

                signals.append(
                    TrafficSignal(
                        signal_id=f"tl_{rep['node_id']}",
                        node_id=rep["node_id"],
                        road_id=wp.road_id,
                        lane_id=wp.lane_id,
                        from_lane=from_l,
                        to_lane=to_l,
                        s=wp.s,
                        t=t_pole,
                        location=(pole_pos.x, pole_pos.y, pole_pos.z),
                        rotation=(0.0, yaw, 0.0),
                        model_type=model_key,
                        mesh_path=spec["mesh_path"],
                        is_dynamic=True,
                        source="osm",
                        is_pedestrian=False,
                    )
                )

        for cand in unmapped_hw:
            spec = TRAFFIC_LIGHT_MODELS["vertical_post"]
            signals.append(
                TrafficSignal(
                    signal_id=f"tl_{cand['node_id']}",
                    node_id=cand["node_id"],
                    road_id=-1,
                    lane_id=-1,
                    from_lane=-1,
                    to_lane=-1,
                    s=0.0,
                    t=0.0,
                    location=cand["proj_pos"],
                    rotation=(0.0, 0.0, 0.0),
                    model_type="vertical_post",
                    mesh_path=spec["mesh_path"],
                    is_dynamic=True,
                    source="osm",
                    is_pedestrian=False,
                )
            )
    else:
        # Fallback when carla_map is None
        for cand in hw_candidates:
            spec = TRAFFIC_LIGHT_MODELS["cantilever_mast"]
            signals.append(
                TrafficSignal(
                    signal_id=f"tl_{cand['node_id']}",
                    node_id=cand["node_id"],
                    road_id=-1,
                    lane_id=-1,
                    from_lane=-1,
                    to_lane=-1,
                    s=0.0,
                    t=0.0,
                    location=cand["proj_pos"],
                    rotation=(0.0, 0.0, 0.0),
                    model_type="cantilever_mast",
                    mesh_path=spec["mesh_path"],
                    is_dynamic=True,
                    source="osm",
                    is_pedestrian=False,
                )
            )

    # 2. Process Pedestrian Signals
    for cand in ped_candidates:
        wp = cand["wp"]
        road_id = -1
        lane_id = -1
        s_coord = 0.0
        t_coord = 0.0
        yaw = 0.0
        loc = cand["proj_pos"]

        if wp is not None:
            road_id = wp.road_id
            lane_id = wp.lane_id
            s_coord = wp.s
            outermost_lane = wp
            if wp.lane_id < 0:
                while (
                    outermost_lane.get_right_lane() is not None
                    and outermost_lane.get_right_lane().lane_type in valid_road_lanes
                ):
                    outermost_lane = outermost_lane.get_right_lane()
                carriageway_w = 0.0
                check = outermost_lane
                while check is not None and check.lane_id < 0:
                    carriageway_w += check.lane_width
                    check = check.get_left_lane()
                t_coord = -(carriageway_w + 0.5)
                right_v = outermost_lane.transform.get_right_vector()
                pos = outermost_lane.transform.location + right_v * (
                    outermost_lane.lane_width * 0.5 + 0.5
                )
                loc = (pos.x, pos.y, outermost_lane.transform.location.z)
            else:
                while (
                    outermost_lane.get_left_lane() is not None
                    and outermost_lane.get_left_lane().lane_type in valid_road_lanes
                ):
                    outermost_lane = outermost_lane.get_left_lane()
                carriageway_w = 0.0
                check = outermost_lane
                while check is not None and check.lane_id > 0:
                    carriageway_w += check.lane_width
                    check = check.get_right_lane()
                t_coord = +(carriageway_w + 0.5)
                left_v = -outermost_lane.transform.get_right_vector()
                pos = outermost_lane.transform.location + left_v * (
                    outermost_lane.lane_width * 0.5 + 0.5
                )
                loc = (pos.x, pos.y, outermost_lane.transform.location.z)

            yaw = wp.transform.rotation.yaw + 90.0
            if yaw > 180.0:
                yaw -= 360.0
            elif yaw < -180.0:
                yaw += 360.0

        spec = TRAFFIC_LIGHT_MODELS["pedestrian_signal"]
        signals.append(
            TrafficSignal(
                signal_id=f"tl_{cand['node_id']}",
                node_id=cand["node_id"],
                road_id=road_id,
                lane_id=lane_id,
                from_lane=lane_id,
                to_lane=lane_id,
                s=s_coord,
                t=t_coord,
                location=loc,
                rotation=(0.0, yaw, 0.0),
                model_type="pedestrian_signal",
                mesh_path=spec["mesh_path"],
                is_dynamic=True,
                source="osm",
                is_pedestrian=True,
            )
        )

    return signals


def inject_signals_into_opendrive(
    xodr_xml: str,
    signals: Sequence[TrafficSignal],
) -> str:
    """
    Ensures that OpenDRIVE XML contains compliant <signals><signal ... /></signals>
    elements for all extracted signals, including controller associations.
    Replaces any raw/unclustered pre-existing traffic light signals.
    """
    root = ET.fromstring(xodr_xml)

    # Clean existing signals and controllers to prevent duplicate unclustered signals and dangling signalReferences
    for road in root.findall("road"):
        sigs_elem = road.find("signals")
        if sigs_elem is not None:
            road.remove(sigs_elem)

    for ctrl in list(root.findall("controller")):
        root.remove(ctrl)

    roads_by_id: dict[str, ET.Element] = {r.get("id", ""): r for r in root.findall("road")}
    next_ctrl_id = 9000

    for sig in signals:
        if sig.road_id < 0:
            continue
        road_str = str(sig.road_id)
        if road_str not in roads_by_id:
            continue

        sig_id_str = str(sig.signal_id).replace("tl_", "")
        road_elem = roads_by_id[road_str]
        sigs_elem = road_elem.find("signals")
        if sigs_elem is None:
            sigs_elem = ET.SubElement(road_elem, "signals")

        # Vehicular traffic lights use type 1000001 (spawned by ATrafficLightManager)
        # Pedestrian signals use type 1000003 (OpenDRIVE landmark, not spawned as cantilever)
        sig_type = "1000003" if sig.is_pedestrian else "1000001"
        z_offset = "3.0" if sig.is_pedestrian else "5.0"

        signal_elem = ET.SubElement(
            sigs_elem,
            "signal",
            attrib={
                "id": sig_id_str,
                "s": f"{sig.s:.2f}",
                "t": f"{sig.t:.2f}",
                "name": f"signal:{sig_id_str}",
                "dynamic": "yes",
                "orientation": "+",
                "zOffset": z_offset,
                "country": "OpenDRIVE",
                "type": sig_type,
                "subtype": "-1",
                "value": "-1",
                "height": "0.0",
                "width": "0.50",
                "text": "",
                "hOffset": "0.0",
                "pitch": "0.0",
                "roll": "0.0",
            },
        )

        from_l = sig.from_lane if hasattr(sig, "from_lane") and sig.from_lane != -1 else sig.lane_id
        to_l = sig.to_lane if hasattr(sig, "to_lane") and sig.to_lane != -1 else sig.lane_id
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

        if not sig.is_pedestrian:
            ctrl_elem = ET.SubElement(
                root,
                "controller",
                attrib={
                    "id": str(next_ctrl_id),
                    "name": f"crtl_{sig_id_str}",
                    "sequence": "0",
                },
            )
            ET.SubElement(
                ctrl_elem,
                "control",
                attrib={
                    "signalId": sig_id_str,
                    "type": "",
                },
            )
            next_ctrl_id += 1

    return ET.tostring(root, encoding="utf-8").decode("utf-8")


def save_traffic_lights_manifest(
    signals: Sequence[TrafficSignal],
    output_path: Path | str,
) -> None:
    """Serializes traffic signals into a structured JSON manifest."""
    path = Path(output_path)
    path.parent.mkdir(parents=True, exist_ok=True)

    data = {
        "version": "1.0",
        "counts": {
            "total_signals": len(signals),
            "cantilever_masts": sum(1 for s in signals if s.model_type == "cantilever_mast"),
            "vertical_posts": sum(1 for s in signals if s.model_type == "vertical_post"),
            "pedestrian_signals": sum(1 for s in signals if s.model_type == "pedestrian_signal"),
        },
        "traffic_signals": [s.to_dict() for s in signals],
        "signals": [s.to_dict() for s in signals],
    }

    path.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")


def load_traffic_lights_manifest(json_path: Path | str) -> list[TrafficSignal]:
    """Loads traffic signals from a JSON manifest."""
    path = Path(json_path)
    if not path.is_file():
        return []

    data = json.loads(path.read_text(encoding="utf-8"))
    signals: list[TrafficSignal] = []
    for s_data in data.get("traffic_signals", []):
        signals.append(TrafficSignal.from_dict(s_data))
    return signals


def spawn_physical_traffic_lights(
    world: Any,
    signals: Sequence[TrafficSignal],
    adjust_ground_elevation: bool = True,
) -> list[Any]:
    """
    Spawns physical 3D traffic light static mesh geometry into a live CARLA simulator.
    Ensures downward raycasting snaps the base to the ground.
    """
    if world is None:
        return []

    bp_lib = world.get_blueprint_library()
    mesh_bp = bp_lib.find("static.prop.mesh")
    spawned_actors: list[Any] = []

    for sig in signals:
        mesh_bp.set_attribute("mesh_path", sig.mesh_path)
        loc = carla.Location(x=sig.location[0], y=sig.location[1], z=sig.location[2])

        if adjust_ground_elevation:
            hits = world.cast_ray(loc + carla.Location(z=15.0), loc + carla.Location(z=-10.0))
            if hits:
                loc.z = hits[0].location.z

        rot = carla.Rotation(pitch=sig.rotation[0], yaw=sig.rotation[1], roll=sig.rotation[2])
        tf = carla.Transform(loc, rot)

        actor = world.try_spawn_actor(mesh_bp, tf)
        if actor:
            spawned_actors.append(actor)

    return spawned_actors


def generate_stop_line_meshes(
    signals: Sequence[TrafficSignal],
    carla_map: Any = None,
    stop_bar_depth: float = 0.50,
    z_offset: float = 0.035,
) -> dict[str, Any]:
    """
    Generates procedural 3D physical stop line markings (solid white transverse bars)
    spanning across all approach driving lanes at each vehicular traffic signal.
    """
    sl_verts: list[tuple[float, float, float]] = []
    sl_tris: list[tuple[int, int, int]] = []
    sl_norms: list[tuple[float, float, float]] = []
    sl_uvs: list[tuple[float, float]] = []

    veh_signals = [s for s in signals if s.model_type in ("cantilever_mast", "vertical_post")]
    if not veh_signals:
        return {
            "name": "OSM_Stop_Lines",
            "semantic_label": "RoadLines",
            "material_preset": "crosswalk",
            "enable_collision": False,
            "vertices": [],
            "triangles": [],
            "normals": [],
            "uvs": [],
        }

    processed_lanes: set[tuple[int, int]] = set()
    half_d = stop_bar_depth * 0.5

    for sig in veh_signals:
        if carla_map is not None:
            s_stop = max(0.5, sig.s - 1.5)
            wp = carla_map.get_waypoint_xodr(sig.road_id, sig.lane_id, s_stop)
            if wp is None:
                try:
                    loc = carla.Location(x=sig.location[0], y=sig.location[1], z=sig.location[2])
                    wp = carla_map.get_waypoint(loc, project_to_road=True)
                except Exception:
                    wp = None

            if wp is not None:
                # Do NOT generate stop lines for signals located inside intersections / junctions
                if wp.is_junction:
                    continue
                lanes = [wp]
                curr = wp.get_right_lane()
                while curr and curr.lane_type == carla.LaneType.Driving and curr.lane_id * wp.lane_id > 0:
                    lanes.append(curr)
                    curr = curr.get_right_lane()
                curr = wp.get_left_lane()
                while curr and curr.lane_type == carla.LaneType.Driving and curr.lane_id * wp.lane_id > 0:
                    lanes.insert(0, curr)
                    curr = curr.get_left_lane()

                for l_wp in lanes:
                    lane_key = (l_wp.road_id, l_wp.lane_id)
                    if lane_key in processed_lanes:
                        continue
                    processed_lanes.add(lane_key)

                    cx, cy, cz = l_wp.transform.location.x, l_wp.transform.location.y, l_wp.transform.location.z
                    yaw_rad = math.radians(l_wp.transform.rotation.yaw)
                    tx, ty = math.cos(yaw_rad), math.sin(yaw_rad)
                    nx, ny = -math.sin(yaw_rad), math.cos(yaw_rad)
                    half_w = l_wp.lane_width * 0.5
                    elev_z = cz + z_offset

                    p0 = (cx - half_d * tx - half_w * nx, cy - half_d * ty - half_w * ny, elev_z)
                    p1 = (cx + half_d * tx - half_w * nx, cy + half_d * ty - half_w * ny, elev_z)
                    p2 = (cx + half_d * tx + half_w * nx, cy + half_d * ty + half_w * ny, elev_z)
                    p3 = (cx - half_d * tx + half_w * nx, cy - half_d * ty + half_w * ny, elev_z)

                    idx = len(sl_verts)
                    sl_verts.extend([p0, p1, p2, p3])
                    sl_tris.append((idx, idx + 2, idx + 1))
                    sl_tris.append((idx, idx + 3, idx + 2))
                    sl_norms.extend([(0.0, 0.0, 1.0)] * 4)
                    sl_uvs.extend([(0.0, 0.0), (1.0, 0.0), (1.0, 1.0), (0.0, 1.0)])
                continue

        # Fallback without carla_map
        lane_key = (sig.road_id, sig.lane_id)
        if lane_key in processed_lanes:
            continue
        processed_lanes.add(lane_key)

        cx, cy, cz = sig.location
        yaw_rad = math.radians(sig.rotation[1])
        tx, ty = math.cos(yaw_rad), math.sin(yaw_rad)
        nx, ny = -math.sin(yaw_rad), math.cos(yaw_rad)
        half_w = 3.5 * 0.5
        elev_z = cz + z_offset

        p0 = (cx - half_d * tx - half_w * nx, cy - half_d * ty - half_w * ny, elev_z)
        p1 = (cx + half_d * tx - half_w * nx, cy + half_d * ty - half_w * ny, elev_z)
        p2 = (cx + half_d * tx + half_w * nx, cy + half_d * ty + half_w * ny, elev_z)
        p3 = (cx - half_d * tx + half_w * nx, cy - half_d * ty + half_w * ny, elev_z)

        idx = len(sl_verts)
        sl_verts.extend([p0, p1, p2, p3])
        sl_tris.append((idx, idx + 2, idx + 1))
        sl_tris.append((idx, idx + 3, idx + 2))
        sl_norms.extend([(0.0, 0.0, 1.0)] * 4)
        sl_uvs.extend([(0.0, 0.0), (1.0, 0.0), (1.0, 1.0), (0.0, 1.0)])

    return {
        "name": "OSM_Stop_Lines",
        "semantic_label": "RoadLines",
        "material_preset": "crosswalk",
        "enable_collision": False,
        "vertices": sl_verts,
        "triangles": sl_tris,
        "normals": sl_norms,
        "uvs": sl_uvs,
    }
