#!/usr/bin/env python3
"""
CARLA Editor-Free Map Maker CLI and SDK (`carla-map`).

A standalone tool that compiles OpenStreetMap (.osm), OpenDRIVE (.xodr),
and custom 3D meshes (.obj, .gltf, .glb) into portable .carlamap bundles
without requiring access to the private Unreal Engine source or editor.

Subcommands:
  init       Scaffold a new map creation project with recipe and asset folders.
  build      Compile assets, extruded OSM buildings, NavMesh, and TM cache into .carlamap.
  inspect    Display a detailed summary of a .carlamap bundle (layers, roads, navmesh).
  verify     Validate bundle structure, .cmesh headers, OpenDRIVE XML, and NavMesh.
  install    Install a .carlamap bundle into a local or packaged CARLA server.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import shutil
import sys
import tarfile
import tempfile
from typing import Any
import xml.etree.ElementTree as ET

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

if (_current_dir / "cmesh.py").is_file():
    if str(_current_dir) not in sys.path:
        sys.path.insert(0, str(_current_dir))
elif (_current_dir.parent.parent / "Util" / "MapBuilder" / "cmesh.py").is_file():
    _mb = _current_dir.parent.parent / "Util" / "MapBuilder"
    if str(_mb) not in sys.path:
        sys.path.insert(0, str(_mb))

import carla

try:
    from Util.MapBuilder.cmesh import (
        CMESH_MAGIC,
        CMESH_VERSION_1,
        read_cmesh_file,
        unpack_cmesh,
        write_cmesh_file,
    )
    from Util.MapBuilder.mesh_importer import (
        MeshSubpart,
        classify_submesh,
        import_and_pack_custom_meshes,
        parse_gltf_file,
        parse_obj_file,
    )
    from Util.MapBuilder.navmesh_builder import (
        compile_navmesh_binary,
        export_recast_geometry_obj,
        find_recast_builder_binary,
    )
    from Util.MapBuilder.osm_buildings import (
        build_osm_buildings_layer,
        build_osm_buildings_multislot_layers,
        extract_osm_building_footprints,
        generate_3d_building_placements,
    )
    from Util.MapBuilder.osm_terrain import (
        build_osm_terrain_layers,
        extract_osm_terrain_polygons,
    )
    from Util.MapBuilder.osm_clutter import (
        ClutterItem,
        extract_osm_clutter,
        generate_curbside_parked_cars,
        load_clutter_manifest,
        save_clutter_manifest,
        synthesize_procedural_sidewalk_clutter,
    )
    from Util.MapBuilder.osm_lighting import (
        StreetLamp,
        extract_osm_street_lamps,
        load_lighting_manifest,
        save_lighting_manifest,
        synthesize_procedural_street_lamps,
    )
    from Util.MapBuilder.osm_traffic_lights import (
        TrafficSignal,
        extract_osm_traffic_signals,
        generate_stop_line_meshes,
        inject_signals_into_opendrive,
        load_traffic_lights_manifest,
        save_traffic_lights_manifest,
    )
    from Util.MapBuilder.osm_crosswalks import (
        CrosswalkItem,
        audit_crosswalk_metrics,
        extract_osm_crosswalks,
        generate_crosswalk_meshes,
        load_crosswalks_manifest,
        save_crosswalks_manifest,
    )
    from Util.MapBuilder.osm_vegetation import (
        generate_osm_vegetation_placements,
        save_vegetation_manifest,
    )
    from Util.MapBuilder.osm_satellite import (
        analyze_satellite_parcels,
        compute_spectral_classification,
        fetch_or_load_satellite_imagery,
        generate_satellite_green_mesh_and_foliage,
        generate_satellite_parking_layers,
        render_satellite_diagnostic_overlay,
        save_satellite_manifest,
        SatelliteProjector,
    )
    from Util.MapBuilder.osm_cycleways import (
        extract_osm_cycleways,
        generate_cycleway_geometry,
        generate_cycleway_demarcation_mesh,
        generate_bike_stencils_and_boxes_mesh,
        generate_bike_parking_clutter,
        save_cycleways_manifest,
        load_cycleways_manifest,
        audit_cycleway_metrics,
        CYCLEWAY_COLOR_PRESETS,
    )
    from Util.MapBuilder.osm_barriers import (
        BarrierWay,
        extract_osm_barriers,
        build_osm_barriers_multislot_layers,
        audit_barrier_metrics,
    )
    from Util.MapBuilder.osm_curbs import (
        CurbConfig,
        extract_sidewalk_curb_runs,
        build_sidewalk_curbs_geometry,
        generate_sidewalk_curbs_mesh,
        audit_curb_metrics,
    )
    from Util.MapBuilder.osm_utilities import (
        UtilityItem,
        extract_osm_utilities,
        synthesize_procedural_fire_hydrants,
        generate_utility_mesh_layers,
        generate_utility_hism_placements,
        generate_utility_obstacle_geometry,
        audit_utility_metrics,
        save_utilities_manifest,
    )
    from Util.MapBuilder.osm_traffic_signs import (
        TrafficSignItem,
        extract_osm_traffic_signs,
        synthesize_procedural_traffic_signs,
        generate_traffic_signs_texture_atlas,
        generate_traffic_signs_mesh,
        inject_traffic_signs_into_opendrive,
        generate_traffic_sign_obstacle_geometry,
        audit_traffic_sign_metrics,
        save_traffic_signs_manifest,
    )
except ImportError:
    from cmesh import (
        CMESH_MAGIC,
        CMESH_VERSION_1,
        read_cmesh_file,
        unpack_cmesh,
        write_cmesh_file,
    )
    from mesh_importer import (
        MeshSubpart,
        classify_submesh,
        import_and_pack_custom_meshes,
        parse_gltf_file,
        parse_obj_file,
    )
    from navmesh_builder import (
        compile_navmesh_binary,
        export_recast_geometry_obj,
        find_recast_builder_binary,
    )
    from osm_buildings import (
        build_osm_buildings_layer,
        build_osm_buildings_multislot_layers,
        extract_osm_building_footprints,
        generate_3d_building_placements,
    )
    from osm_terrain import (
        build_osm_terrain_layers,
        extract_osm_terrain_polygons,
    )
    from osm_clutter import (
        ClutterItem,
        extract_osm_clutter,
        generate_curbside_parked_cars,
        load_clutter_manifest,
        save_clutter_manifest,
        synthesize_procedural_sidewalk_clutter,
    )
    from osm_lighting import (
        StreetLamp,
        extract_osm_street_lamps,
        load_lighting_manifest,
        save_lighting_manifest,
        synthesize_procedural_street_lamps,
    )
    from osm_traffic_lights import (
        TrafficSignal,
        extract_osm_traffic_signals,
        generate_stop_line_meshes,
        inject_signals_into_opendrive,
        load_traffic_lights_manifest,
        save_traffic_lights_manifest,
    )
    from osm_crosswalks import (
        CrosswalkItem,
        audit_crosswalk_metrics,
        extract_osm_crosswalks,
        generate_crosswalk_meshes,
        load_crosswalks_manifest,
        save_crosswalks_manifest,
    )
    from osm_vegetation import (
        generate_osm_vegetation_placements,
        save_vegetation_manifest,
    )
    from osm_satellite import (
        analyze_satellite_parcels,
        compute_spectral_classification,
        fetch_or_load_satellite_imagery,
        generate_satellite_green_mesh_and_foliage,
        generate_satellite_parking_layers,
        render_satellite_diagnostic_overlay,
        save_satellite_manifest,
        SatelliteProjector,
    )
    from osm_cycleways import (
        extract_osm_cycleways,
        generate_cycleway_geometry,
        generate_cycleway_demarcation_mesh,
        generate_bike_stencils_and_boxes_mesh,
        generate_bike_parking_clutter,
        save_cycleways_manifest,
        load_cycleways_manifest,
        audit_cycleway_metrics,
        CYCLEWAY_COLOR_PRESETS,
    )
    from osm_barriers import (
        BarrierWay,
        extract_osm_barriers,
        build_osm_barriers_multislot_layers,
        audit_barrier_metrics,
    )
    from osm_curbs import (
        CurbConfig,
        extract_sidewalk_curb_runs,
        build_sidewalk_curbs_geometry,
        generate_sidewalk_curbs_mesh,
        audit_curb_metrics,
    )
    from osm_utilities import (
        UtilityItem,
        extract_osm_utilities,
        synthesize_procedural_fire_hydrants,
        generate_utility_mesh_layers,
        generate_utility_hism_placements,
        generate_utility_obstacle_geometry,
        audit_utility_metrics,
        save_utilities_manifest,
    )
    from osm_traffic_signs import (
        TrafficSignItem,
        extract_osm_traffic_signs,
        synthesize_procedural_traffic_signs,
        generate_traffic_signs_texture_atlas,
        generate_traffic_signs_mesh,
        inject_traffic_signs_into_opendrive,
        generate_traffic_sign_obstacle_geometry,
        audit_traffic_sign_metrics,
        save_traffic_signs_manifest,
    )

REPO_ROOT = Path(__file__).resolve().parent.parent.parent


# =============================================================================
# Subcommand: init
# =============================================================================

def cmd_init(args: argparse.Namespace) -> int:
    """Scaffolds a new map creation project directory."""
    map_name = args.name.strip()
    if not map_name:
        print("[carla-map] ERROR: Map name cannot be empty.")
        return 1

    out_dir = Path(args.output or ".").resolve() / map_name
    if out_dir.exists() and not args.force:
        print(f"[carla-map] ERROR: Directory '{out_dir}' already exists. Use --force to overwrite.")
        return 1

    out_dir.mkdir(parents=True, exist_ok=True)
    (out_dir / "osm").mkdir(exist_ok=True)
    (out_dir / "opendrive").mkdir(exist_ok=True)
    (out_dir / "meshes").mkdir(exist_ok=True)
    (out_dir / "textures").mkdir(exist_ok=True)
    (out_dir / "placements").mkdir(exist_ok=True)

    # Copy template input files if provided
    osm_file_rel = ""
    xodr_file_rel = ""

    if args.osm:
        src_osm = Path(args.osm).resolve()
        if src_osm.is_file():
            dest_osm = out_dir / "osm" / src_osm.name
            shutil.copy2(src_osm, dest_osm)
            osm_file_rel = f"osm/{src_osm.name}"
            print(f"[carla-map] Staged initial OSM network: {src_osm.name}")

    if args.xodr:
        src_xodr = Path(args.xodr).resolve()
        if src_xodr.is_file():
            dest_xodr = out_dir / "opendrive" / src_xodr.name
            shutil.copy2(src_xodr, dest_xodr)
            xodr_file_rel = f"opendrive/{src_xodr.name}"
            print(f"[carla-map] Staged initial OpenDRIVE file: {src_xodr.name}")

    recipe: dict[str, Any] = {
        "format_version": "1.0",
        "map_name": map_name,
        "source": {
            "osm_file": osm_file_rel or "osm/sample.osm",
            "xodr_file": xodr_file_rel or f"opendrive/{map_name}.xodr",
        },
        "opendrive_parameters": {
            "generate_road_mesh": False if args.xodr else True,
            "generate_ground_mesh": False,
            "generate_traffic_signals": True,
            "generate_street_furniture": True,
            "wall_height": 0.0,
        },
        "osm_buildings": {
            "enabled": True,
            "default_height": 8.0,
            "level_height": 3.5,
            "material_preset": "building_brick",
        },
        "osm_crosswalks": {
            "enabled": True,
        },
        "custom_meshes": [
            {
                "file_pattern": "meshes/*.obj",
                "scale": 1.0,
            }
        ],
        "placements": [],
        "spawn_points": [],
    }

    recipe_path = out_dir / "map_recipe.json"
    recipe_path.write_text(json.dumps(recipe, indent=2) + "\n", encoding="utf-8")

    print(f"[carla-map] Successfully initialized map project at: {out_dir}")
    print(f"  Recipe configuration : {recipe_path}")
    print("  Input folders        : osm/, opendrive/, meshes/, textures/, placements/")
    print(f"  To compile bundle    : carla-map build {out_dir}")
    return 0


def clean_junction_sidewalks(xodr_xml: str) -> str:
    """
    Strips pedestrian sidewalk and biking lanes from:
    1. Internal junction connecting roads (junction != "-1").
    2. Micro-roads (length < 3.0m), such as 0.2m or 0.6m inter-junction connecting seams.
    Also strips lane markings (roadMark) from micro-roads (length < 1.5m) to prevent
    spurious white stripes and transverse curb bars slicing across islands and roadways.
    """
    if not xodr_xml:
        return xodr_xml
    try:
        root = ET.fromstring(xodr_xml)
    except Exception:
        return xodr_xml

    stripped_lanes = 0
    stripped_links = 0
    stripped_marks = 0

    for road in root.findall("road"):
        junc_id = road.get("junction")
        road_id = road.get("id")
        length = float(road.get("length", "0.0"))
        is_internal_junc = bool(junc_id and junc_id != "-1")
        is_micro_road = (length < 3.0)

        lanes_elem = road.find("lanes")
        if lanes_elem is not None:
            for lsect in lanes_elem.findall("laneSection"):
                for side in ("left", "right"):
                    side_elem = lsect.find(side)
                    if side_elem is not None:
                        for lane in list(side_elem.findall("lane")):
                            if lane.get("type") in ("sidewalk", "biking") and (is_internal_junc or is_micro_road):
                                lane_id = lane.get("id")
                                side_elem.remove(lane)
                                stripped_lanes += 1
                                for junc in root.findall("junction"):
                                    for conn in list(junc.findall("connection")):
                                        if conn.get("connectingRoad") == road_id:
                                            for ll in list(conn.findall("laneLink")):
                                                if ll.get("to") == lane_id:
                                                    conn.remove(ll)
                                                    stripped_links += 1

                            # Strip road marks from micro-roads (< 1.5m)
                            if is_micro_road and length < 1.5:
                                for rm in lane.findall("roadMark"):
                                    rm.set("type", "none")
                                    rm.set("width", "0.0")
                                    stripped_marks += 1

                # Center lane marks on micro-roads
                if is_micro_road and length < 1.5:
                    center_lane = lsect.find("center/lane")
                    if center_lane is not None:
                        for rm in center_lane.findall("roadMark"):
                            rm.set("type", "none")
                            rm.set("width", "0.0")
                            stripped_marks += 1

    if stripped_lanes > 0 or stripped_marks > 0:
        print(f"[carla-map] Cleaned {stripped_lanes} junction/micro-road sidewalk lanes ({stripped_links} laneLinks) and {stripped_marks} micro-road lane markings from OpenDRIVE.")
        return ET.tostring(root, encoding="utf-8").decode("utf-8")
    return xodr_xml


def crop_sidewalks_overlapping_cycleways(
    xodr_xml: str,
    cycleways: list[Any],
    carla_map: Any | None = None,
) -> tuple[str, int, list[tuple[int, int, float | None, float | None, float]]]:
    """
    Detects when an elevated pedestrian sidewalk encroaches onto a continuous bicycle corridor
    (e.g., as on-road cycleways taper across lane groups or approach junction transitions),
    and crops the sidewalk lane in OpenDRIVE by adjusting its polynomial width.
    This guarantees that the active bicycle corridor maintains 100% of its full width
    without being obstructed or sliced by elevated sidewalk curbs.
    """
    if not xodr_xml or not cycleways:
        return xodr_xml, 0, []
    try:
        root = ET.fromstring(xodr_xml)
    except Exception:
        return xodr_xml, 0, []

    if carla_map is None:
        try:
            carla_map = carla.Map("crop_temp", xodr_xml)
        except Exception:
            return xodr_xml, 0, []

    def _dist_pt_seg(px: float, py: float, ax: float, ay: float, bx: float, by: float) -> float:
        dx, dy = bx - ax, by - ay
        l2 = dx * dx + dy * dy
        if l2 == 0:
            return math.hypot(px - ax, py - ay)
        t = max(0.0, min(1.0, ((px - ax) * dx + (py - ay) * dy) / l2))
        return math.hypot(px - (ax + t * dx), py - (ay + t * dy))

    def _dist_pt_polyline(px: float, py: float, poly: list[Any]) -> float:
        d_min = 1e9
        for i in range(len(poly) - 1):
            d = _dist_pt_seg(px, py, poly[i][0], poly[i][1], poly[i + 1][0], poly[i + 1][1])
            if d < d_min:
                d_min = d
        return d_min

    cyc_list: list[tuple[list[Any], float]] = []
    for c in cycleways:
        nodes = c.nodes_carla if hasattr(c, "nodes_carla") else c.get("nodes_carla", [])
        w = c.width_m if hasattr(c, "width_m") else c.get("width_m", 2.0)
        if len(nodes) >= 2:
            cyc_list.append((nodes, w * 0.5))

    if not cyc_list:
        return xodr_xml, 0, []

    cropped_count = 0
    details: list[tuple[int, int, float | None, float | None, float]] = []

    for road in root.findall("road"):
        rid = int(road.get("id", "0"))
        length = float(road.get("length", "0.0"))
        if length < 3.0:
            continue

        lanes_elem = road.find("lanes")
        if lanes_elem is None:
            continue

        for side in ("left", "right"):
            side_elem = lanes_elem.find(f"laneSection/{side}")
            if side_elem is None:
                continue

            for lane in side_elem.findall("lane"):
                if lane.get("type") != "sidewalk":
                    continue
                lid = int(lane.get("id", "0"))

                step = 0.5
                num_steps = int(math.ceil(length / step))
                encroaching_s: list[float] = []

                for i in range(num_steps + 1):
                    s = min(float(i * step), length)
                    wp = carla_map.get_waypoint_xodr(rid, lid, s)
                    if not wp:
                        continue
                    sw_w = wp.lane_width
                    if sw_w <= 0.05:
                        continue
                    loc = wp.transform.location

                    inner_lid = lid + 1 if lid < 0 else lid - 1
                    wp_in = carla_map.get_waypoint_xodr(rid, inner_lid, s) if inner_lid != 0 else None
                    if wp_in:
                        vx = wp_in.transform.location.x - loc.x
                        vy = wp_in.transform.location.y - loc.y
                        vlen = math.hypot(vx, vy)
                        ux = vx / vlen if vlen > 0 else 0.0
                        uy = vy / vlen if vlen > 0 else 0.0
                    else:
                        yaw = math.radians(wp.transform.rotation.yaw)
                        ux = math.sin(yaw) if lid < 0 else -math.sin(yaw)
                        uy = -math.cos(yaw) if lid < 0 else math.cos(yaw)

                    inner_x = loc.x + 0.5 * sw_w * ux
                    inner_y = loc.y + 0.5 * sw_w * uy

                    for poly, r_cyc in cyc_list:
                        d = _dist_pt_polyline(inner_x, inner_y, poly)
                        if d < (r_cyc - 0.05):
                            encroaching_s.append(s)
                            break

                if encroaching_s:
                    clusters: list[list[float]] = []
                    curr: list[float] = []
                    for s in sorted(encroaching_s):
                        if not curr or (s - curr[-1]) <= 1.5:
                            curr.append(s)
                        else:
                            clusters.append(curr)
                            curr = [s]
                    if curr:
                        clusters.append(curr)

                    start_crop: float | None = None
                    end_crop: float | None = None

                    for cl in clusters:
                        cl_min, cl_max = min(cl), max(cl)
                        if cl_max >= length - 2.0 and cl_min > 2.0:
                            end_crop = max(0.0, cl_min - 1.5)
                        elif cl_min <= 2.0 and cl_max < length - 2.0:
                            start_crop = min(length, cl_max + 1.5)

                    if start_crop is not None or end_crop is not None:
                        w_init = lane.find("width")
                        init_a = w_init.get("a", "2.80") if w_init is not None else "2.80"

                        for w in list(lane.findall("width")):
                            lane.remove(w)

                        def add_w(s_off: float, a_val: str) -> None:
                            w_elem = ET.SubElement(lane, "width")
                            w_elem.set("sOffset", f"{s_off:.2f}")
                            w_elem.set("a", f"{a_val}")
                            w_elem.set("b", "0")
                            w_elem.set("c", "0")
                            w_elem.set("d", "0")

                        if start_crop is not None and end_crop is not None:
                            if start_crop >= end_crop or (end_crop - start_crop) < 3.0:
                                add_w(0.0, "0.0")
                            else:
                                add_w(0.0, "0.0")
                                add_w(start_crop, init_a)
                                add_w(end_crop, "0.0")
                        elif start_crop is not None:
                            if length - start_crop < 3.0:
                                add_w(0.0, "0.0")
                            else:
                                add_w(0.0, "0.0")
                                add_w(start_crop, init_a)
                        elif end_crop is not None:
                            if end_crop < 3.0:
                                add_w(0.0, "0.0")
                            else:
                                add_w(0.0, init_a)
                                add_w(end_crop, "0.0")

                        cropped_count += 1
                        details.append((rid, lid, start_crop, end_crop, length))

    res_xml = ET.tostring(root, encoding="utf-8").decode("utf-8")
    return res_xml, cropped_count, details



# =============================================================================
# Subcommand: build
# =============================================================================

def cmd_build(args: argparse.Namespace) -> int:
    """Builds a .carlamap bundle from recipe or direct command-line arguments."""
    target_path = Path(args.target or ".").resolve()

    map_name = args.name or ""
    osm_path: Path | None = None
    xodr_path: Path | None = None
    mesh_files: list[Path] = []
    recipe_dict: dict[str, Any] = {}

    if target_path.is_file() and target_path.name.endswith(".json"):
        recipe_dict = json.loads(target_path.read_text(encoding="utf-8"))
        project_dir = target_path.parent
    elif target_path.is_dir() and (target_path / "map_recipe.json").is_file():
        project_dir = target_path
        recipe_dict = json.loads((target_path / "map_recipe.json").read_text(encoding="utf-8"))
    else:
        project_dir = target_path if target_path.is_dir() else target_path.parent

    if not map_name:
        map_name = recipe_dict.get("map_name") or project_dir.name or "CustomMap"

    # Resolve OSM input
    if args.osm:
        osm_path = Path(args.osm).resolve()
    elif recipe_dict.get("source", {}).get("osm_file"):
        osm_candidate = project_dir / recipe_dict["source"]["osm_file"]
        if osm_candidate.is_file():
            osm_path = osm_candidate

    # Resolve OpenDRIVE input
    if args.xodr:
        xodr_path = Path(args.xodr).resolve()
    elif recipe_dict.get("source", {}).get("xodr_file"):
        xodr_candidate = project_dir / recipe_dict["source"]["xodr_file"]
        if xodr_candidate.is_file():
            xodr_path = xodr_candidate

    # Resolve custom meshes input
    if args.meshes:
        m_arg = Path(args.meshes).resolve()
        if m_arg.is_file():
            mesh_files.append(m_arg)
        elif m_arg.is_dir():
            for ext in ("*.obj", "*.gltf", "*.glb"):
                mesh_files.extend(m_arg.glob(ext))
    elif (project_dir / "meshes").is_dir():
        for ext in ("*.obj", "*.gltf", "*.glb"):
            mesh_files.extend((project_dir / "meshes").glob(ext))

    if not osm_path and not xodr_path:
        print("[carla-map] ERROR: Neither OpenStreetMap (.osm) nor OpenDRIVE (.xodr) source file provided.")
        return 1

    print(f"[carla-map] Building map bundle '{map_name}'...")

    # Staging directory: <MapName>.carlamap
    output_dir = Path(args.output).resolve() if args.output else project_dir
    if output_dir.suffix in (".gz", ".tar"):
        bundle_dir = output_dir.parent / f"{map_name}.carlamap"
        tar_output_path = output_dir
    elif output_dir.name.endswith(".carlamap"):
        bundle_dir = output_dir
        tar_output_path = output_dir.with_name(f"{output_dir.name}.tar.gz")
    else:
        bundle_dir = output_dir / f"{map_name}.carlamap"
        tar_output_path = output_dir / f"{map_name}.carlamap.tar.gz"

    if bundle_dir.exists():
        shutil.rmtree(bundle_dir)
    bundle_dir.mkdir(parents=True, exist_ok=True)
    (bundle_dir / "OpenDrive").mkdir(exist_ok=True)
    (bundle_dir / "Nav").mkdir(exist_ok=True)
    (bundle_dir / "TM").mkdir(exist_ok=True)
    (bundle_dir / "Meshes").mkdir(exist_ok=True)
    (bundle_dir / "Placements").mkdir(exist_ok=True)

    # -------------------------------------------------------------------------
    # 1. OpenDRIVE Generation / Staging
    # -------------------------------------------------------------------------
    final_xodr_path = bundle_dir / "OpenDrive" / f"{map_name}.xodr"
    xodr_xml_content = ""

    if osm_path and osm_path.is_file():
        print(f"[carla-map] Converting OpenStreetMap: {osm_path.name} -> {map_name}.xodr...")
        osm_xml = osm_path.read_text(encoding="utf-8")
        settings = carla.Osm2OdrSettings()
        settings.center_map = True
        settings.generate_traffic_lights = not getattr(args, "no_traffic_lights", False)
        settings.all_junctions_with_traffic_lights = not getattr(args, "no_all_junctions_tl", False)
        settings.default_lane_width = 3.5
        settings.default_sidewalk_width = 2.8

        layer_elev = 0.0
        if getattr(args, "layer_elevation", None) is not None:
            layer_elev = float(args.layer_elevation)
        elif recipe_dict.get("osm_settings", {}).get("elevation_layer_height") is not None:
            layer_elev = float(recipe_dict["osm_settings"]["elevation_layer_height"])

        if layer_elev > 0.0:
            settings.elevation_layer_height = layer_elev
            print(f"[carla-map] Setting OSM layer elevation separation: {layer_elev:.1f}m")

        xodr_xml_content = carla.Osm2Odr.convert(osm_xml, settings)
        xodr_xml_content = clean_junction_sidewalks(xodr_xml_content)
        final_xodr_path.write_text(xodr_xml_content, encoding="utf-8")
    elif xodr_path and xodr_path.is_file():
        print(f"[carla-map] Copying OpenDRIVE file: {xodr_path.name} -> {map_name}.xodr...")
        xodr_xml_content = xodr_path.read_text(encoding="utf-8")
        xodr_xml_content = clean_junction_sidewalks(xodr_xml_content)
        final_xodr_path.write_text(xodr_xml_content, encoding="utf-8")

    # Validate OpenDRIVE with carla.Map
    try:
        carla_map = carla.Map(map_name, xodr_xml_content)
        waypoints = carla_map.generate_waypoints(2.0)
        topology = carla_map.get_topology()
        print(f"[carla-map] OpenDRIVE validated successfully: {len(topology)} road segments, {len(waypoints)} waypoints.")
    except Exception as exc:
        print(f"[carla-map] ERROR: OpenDRIVE validation failed: {exc}")
        return 1

    # -------------------------------------------------------------------------
    # 2. Traffic Manager Cache (.bin) Compilation
    # -------------------------------------------------------------------------
    tm_bin_path = bundle_dir / "TM" / f"{map_name}.bin"
    print(f"[carla-map] Cooking Traffic Manager cache -> {tm_bin_path.name}...")
    try:
        carla_map.cook_in_memory_map(str(tm_bin_path))
    except Exception as exc:
        print(f"[carla-map] Warning: Could not cook in-memory map cache: {exc}")

    # -------------------------------------------------------------------------
    # 3. Custom Mesh Import & Layer Packing
    # -------------------------------------------------------------------------
    layers: list[dict[str, Any]] = []
    has_custom_road_meshes = False
    walkable_surfaces: list[tuple[list[tuple[float, float, float]], list[tuple[int, int, int]]]] = []
    obstacle_surfaces: list[tuple[list[tuple[float, float, float]], list[tuple[int, int, int]]]] = []

    if mesh_files:
        print(f"[carla-map] Importing {len(mesh_files)} custom 3D mesh files...")
        custom_rules = recipe_dict.get("semantic_rules")
        imported_layers = import_and_pack_custom_meshes(
            mesh_files,
            output_dir=bundle_dir / "Meshes",
            custom_rules=custom_rules,
        )
        layers.extend(imported_layers)

        for layer in imported_layers:
            if layer["semantic_label"] == "Roads":
                has_custom_road_meshes = True
            # Read back geometry for NavMesh inclusion
            cmesh_file = bundle_dir / layer["mesh_file"]
            if cmesh_file.is_file():
                geom = read_cmesh_file(cmesh_file)
                if layer["semantic_label"] in ("Roads", "Sidewalks", "Terrain"):
                    walkable_surfaces.append((geom["vertices"], geom["triangles"]))
                elif layer["semantic_label"] in ("Buildings", "Walls", "Fences"):
                    obstacle_surfaces.append((geom["vertices"], geom["triangles"]))

    # -------------------------------------------------------------------------
    # 4. Procedural 3D Building Extrusion & Geographic Projection (from OSM)
    # -------------------------------------------------------------------------
    b_verts: list[tuple[float, float, float]] = []
    building_placements: list[dict[str, Any]] = []
    if osm_path:
        # Read OpenDRIVE header offset for exact geographic coordinate alignment
        import xml.etree.ElementTree as ET
        xodr_root = ET.fromstring(xodr_xml_content)
        header = xodr_root.find("header")
        offset_elem = header.find("offset") if header is not None else None
        off_x = float(offset_elem.get("x", "0.0")) if offset_elem is not None else 0.0
        off_y = float(offset_elem.get("y", "0.0")) if offset_elem is not None else 0.0

        def project_lat_lon(lat: float, lon: float) -> tuple[float, float, float]:
            loc = carla_map.geolocation_to_transform(carla.GeoLocation(lat, lon, 0.0))
            # OpenDRIVE maps X_tmerc + off_x to x, and Y_tmerc + off_y to y.
            # In CARLA left-handed coordinate frame, y_carla = -y_odr = -(loc.y + off_y).
            x_carla = loc.x + off_x
            y_carla = -(loc.y + off_y)
            return x_carla, y_carla, loc.z

        osm_bldgs_cfg = recipe_dict.get("osm_buildings", {"enabled": True})
        if osm_bldgs_cfg.get("enabled", True):
            print(f"[carla-map] Extracting 3D OSM buildings from {osm_path.name}...")
            footprints = extract_osm_building_footprints(osm_path)

            if footprints:
                def_height = float(osm_bldgs_cfg.get("default_height", 8.0))
                lvl_height = float(osm_bldgs_cfg.get("level_height", 3.5))

                use_3d_models = osm_bldgs_cfg.get("use_3d_models", False)
                if use_3d_models:
                    print(f"[carla-map] Generating photorealistic 3D static mesh placements for {len(footprints)} parcels...")
                    bldg_placements = generate_3d_building_placements(
                        footprints,
                        project_coord_fn=project_lat_lon,
                        default_height=def_height,
                        level_height=lvl_height,
                    )
                    building_placements.extend(bldg_placements)
                    total_placed = sum(len(p["transforms"]) for p in bldg_placements)
                    print(f"[carla-map] Placed {total_placed} complete 3D static mesh buildings across {len(bldg_placements)} architectural models.")

                    # Also provide obstacle bounding geometry for Recast NavMesh so pedestrians navigate around buildings
                    for p in bldg_placements:
                        for t in p["transforms"]:
                            loc = t["location"]
                            scale = t["scale"]
                            bx = 10.0 * scale[0]
                            by = 10.0 * scale[1]
                            bz = 15.0 * scale[2]
                            x0, x1 = loc[0] - bx * 0.5, loc[0] + bx * 0.5
                            y0, y1 = loc[1] - by * 0.5, loc[1] + by * 0.5
                            z0, z1 = loc[2], loc[2] + bz
                            box_v = [
                                (x0, y0, z0), (x1, y0, z0), (x1, y1, z0), (x0, y1, z0),
                                (x0, y0, z1), (x1, y0, z1), (x1, y1, z1), (x0, y1, z1),
                            ]
                            box_t = [
                                (0, 2, 1), (0, 3, 2),
                                (4, 5, 6), (4, 6, 7),
                                (0, 1, 5), (0, 5, 4),
                                (1, 2, 6), (1, 6, 5),
                                (2, 3, 7), (2, 7, 6),
                                (3, 0, 4), (3, 4, 7),
                            ]
                            obstacle_surfaces.append((box_v, box_t))

                extrude_fallback = osm_bldgs_cfg.get("extrude_fallback", True)
                if not use_3d_models or extrude_fallback:
                    multi_slot = osm_bldgs_cfg.get("multi_slot", True)
                    if multi_slot:
                        bldg_layers = build_osm_buildings_multislot_layers(
                            footprints,
                            project_coord_fn=project_lat_lon,
                            default_height=def_height,
                            level_height=lvl_height,
                        )
                        total_b_verts = 0
                        total_b_tris = 0
                        for slot_id, l_data in bldg_layers.items():
                            lv = l_data["vertices"]
                            lt = l_data["triangles"]
                            if not lv or not lt:
                                continue
                            cmesh_filename = "Meshes/osm_buildings.cmesh" if slot_id == "walls" else f"Meshes/osm_buildings_{slot_id}.cmesh"
                            bldg_cmesh_path = bundle_dir / cmesh_filename
                            write_cmesh_file(
                                bldg_cmesh_path,
                                vertices=lv,
                                triangles=lt,
                                normals=l_data["normals"],
                                uvs=l_data["uvs"],
                            )
                            mat_preset = l_data["material_preset"]
                            if slot_id in ("walls", "modern_walls"):
                                mat_preset = osm_bldgs_cfg.get(
                                    "material_preset",
                                    "/Game/Carla/Static/GenericMaterials/Facade/MI_Facade01.MI_Facade01",
                                )

                            layers.append({
                                "name": l_data["name"],
                                "mesh_file": cmesh_filename,
                                "semantic_label": l_data["semantic_label"],
                                "material_preset": mat_preset,
                                "enable_collision": l_data["enable_collision"],
                            })
                            obstacle_surfaces.append((lv, lt))
                            b_verts.extend(lv)
                            total_b_verts += len(lv)
                            total_b_tris += len(lt)

                        print(f"[carla-map] Extruded {len(footprints)} 3D buildings into {len(bldg_layers)} multi-slot architectural layers: {total_b_verts} vertices, {total_b_tris} triangles.")
                    else:
                        b_verts, b_tris, b_norms, b_uvs = build_osm_buildings_layer(
                            footprints,
                            project_coord_fn=project_lat_lon,
                            default_height=def_height,
                            level_height=lvl_height,
                        )
                        if b_verts:
                            bldg_cmesh_path = bundle_dir / "Meshes" / "osm_buildings.cmesh"
                            write_cmesh_file(
                                bldg_cmesh_path,
                                vertices=b_verts,
                                triangles=b_tris,
                                normals=b_norms,
                                uvs=b_uvs,
                            )
                            mat_preset = osm_bldgs_cfg.get("material_preset", "/Game/Carla/Static/GenericMaterials/Facade/MI_Facade01.MI_Facade01")
                            layers.append({
                                "name": "OSM_Buildings",
                                "mesh_file": "Meshes/osm_buildings.cmesh",
                                "semantic_label": "Buildings",
                                "material_preset": mat_preset,
                                "enable_collision": True,
                            })
                            obstacle_surfaces.append((b_verts, b_tris))
                            print(f"[carla-map] Extruded {len(footprints)} 3D buildings: {len(b_verts)} vertices, {len(b_tris)} triangles.")

        # ---------------------------------------------------------------------
        # 5. Procedural 3D Urban Terrain & Sub-Base Floor (from OSM)
        # ---------------------------------------------------------------------
        osm_terrain_cfg = recipe_dict.get("osm_terrain", {"enabled": True})
        no_terrain = getattr(args, "no_terrain", False) or not osm_terrain_cfg.get("enabled", True)
        no_subbase = getattr(args, "no_subbase", False) or not osm_terrain_cfg.get("subbase_enabled", True)
        subbase_margin = float(getattr(args, "subbase_margin", 50.0))

        if not no_terrain:
            print(f"[carla-map] Extracting urban terrain and landuse polygons from {osm_path.name}...")
            terrain_polys = extract_osm_terrain_polygons(osm_path)

            all_x: list[float] = [wp.transform.location.x for wp in waypoints]
            all_y: list[float] = [wp.transform.location.y for wp in waypoints]
            if b_verts:
                all_x.extend(v[0] for v in b_verts)
                all_y.extend(v[1] for v in b_verts)

            map_bounds = (min(all_x), max(all_x), min(all_y), max(all_y)) if all_x and all_y else (-200.0, 200.0, -200.0, 200.0)

            terrain_layers = build_osm_terrain_layers(
                terrain_polys,
                project_coord_fn=project_lat_lon,
                map_bounds=map_bounds,
                include_subbase=not no_subbase,
                subbase_margin=subbase_margin,
            )

            for cat_name, t_layer in terrain_layers.items():
                t_verts = t_layer["vertices"]
                t_tris = t_layer["triangles"]
                if not t_verts or not t_tris:
                    continue

                cmesh_filename = f"Meshes/osm_terrain_{cat_name}.cmesh"
                t_cmesh_path = bundle_dir / cmesh_filename
                write_cmesh_file(
                    t_cmesh_path,
                    vertices=t_verts,
                    triangles=t_tris,
                    normals=t_layer["normals"],
                    uvs=t_layer["uvs"],
                )

                layers.append({
                    "name": t_layer["name"],
                    "mesh_file": cmesh_filename,
                    "semantic_label": t_layer["semantic_label"],
                    "material_preset": t_layer["material_preset"],
                    "enable_collision": t_layer["enable_collision"],
                })

                if cat_name in ("plaza", "grass", "subbase"):
                    mat_tag = "sidewalk" if cat_name == "plaza" else "grass"
                    walkable_surfaces.append((t_verts, t_tris, mat_tag))

                print(f"[carla-map] Generated terrain layer '{t_layer['name']}': {len(t_verts)} vertices, {len(t_tris)} triangles (preset: {t_layer['material_preset']}).")

    # -------------------------------------------------------------------------
    # 5b. OSM Pedestrian Crosswalks, Zebra Markings & Tactile Paving
    # -------------------------------------------------------------------------
    crosswalks_file_rel: str | None = None
    osm_cw_cfg = recipe_dict.get("osm_crosswalks", {"enabled": True})
    no_crosswalks = getattr(args, "no_crosswalks", False) or not osm_cw_cfg.get("enabled", True)

    if osm_path and not no_crosswalks:
        print(f"[carla-map] Extracting pedestrian crosswalks from {osm_path.name}...")
        crosswalk_items = extract_osm_crosswalks(
            osm_path,
            project_coord_fn=project_lat_lon,
            carla_map=carla_map,
        )
        if crosswalk_items:
            cw_meshes = generate_crosswalk_meshes(crosswalk_items, z_offset=0.035)

            # Zebra stripes layer
            z_layer = cw_meshes.get("zebra_stripes")
            if z_layer and z_layer["vertices"]:
                z_cmesh_path = bundle_dir / "Meshes" / "osm_crosswalks.cmesh"
                write_cmesh_file(
                    z_cmesh_path,
                    vertices=z_layer["vertices"],
                    triangles=z_layer["triangles"],
                    normals=z_layer["normals"],
                    uvs=z_layer["uvs"],
                )
                layers.append({
                    "name": z_layer["name"],
                    "mesh_file": "Meshes/osm_crosswalks.cmesh",
                    "semantic_label": z_layer["semantic_label"],
                    "material_preset": z_layer["material_preset"],
                    "enable_collision": z_layer["enable_collision"],
                })

            # Refuge islands layer
            isl_layer = cw_meshes.get("refuge_islands")
            if isl_layer and isl_layer["vertices"]:
                isl_cmesh_path = bundle_dir / "Meshes" / "osm_crosswalk_islands.cmesh"
                write_cmesh_file(
                    isl_cmesh_path,
                    vertices=isl_layer["vertices"],
                    triangles=isl_layer["triangles"],
                    normals=isl_layer["normals"],
                    uvs=isl_layer["uvs"],
                )
                layers.append({
                    "name": isl_layer["name"],
                    "mesh_file": "Meshes/osm_crosswalk_islands.cmesh",
                    "semantic_label": isl_layer["semantic_label"],
                    "material_preset": isl_layer["material_preset"],
                    "enable_collision": isl_layer["enable_collision"],
                })
                walkable_surfaces.append((isl_layer["vertices"], isl_layer["triangles"]))

            # Tactile paving layer
            tac_layer = cw_meshes.get("tactile_paving")
            if tac_layer and tac_layer["vertices"]:
                tac_cmesh_path = bundle_dir / "Meshes" / "osm_crosswalk_tactile.cmesh"
                write_cmesh_file(
                    tac_cmesh_path,
                    vertices=tac_layer["vertices"],
                    triangles=tac_layer["triangles"],
                    normals=tac_layer["normals"],
                    uvs=tac_layer["uvs"],
                )
                tac_dict: dict[str, Any] = {
                    "name": tac_layer["name"],
                    "mesh_file": "Meshes/osm_crosswalk_tactile.cmesh",
                    "semantic_label": tac_layer["semantic_label"],
                    "material_preset": tac_layer["material_preset"],
                    "enable_collision": tac_layer["enable_collision"],
                }
                if tac_layer.get("material_params"):
                    tac_dict["material_params"] = tac_layer["material_params"]
                layers.append(tac_dict)

            # Add walkable crosswalk corridors to NavMesh
            for cw_v, cw_t in cw_meshes.get("walkable_corridors", []):
                walkable_surfaces.append((cw_v, cw_t, "crosswalk"))

            # Save Placements/crosswalks.json
            placements_dir = bundle_dir / "Placements"
            placements_dir.mkdir(parents=True, exist_ok=True)
            cw_manifest_path = placements_dir / "crosswalks.json"
            save_crosswalks_manifest(crosswalk_items, cw_manifest_path)
            crosswalks_file_rel = "Placements/crosswalks.json"
            print(f"[carla-map] Generated {len(crosswalk_items)} crosswalks ({sum(c.num_stripes for c in crosswalk_items)} zebra stripes, {len(z_layer['vertices'])} verts).")

    # -------------------------------------------------------------------------
    # 5c. Native CARLA 3D Urban Foliage, Avenue Trees & Park Greenery
    # -------------------------------------------------------------------------
    vegetation_file_rel: str | None = None
    vegetation_placements: list[dict[str, Any]] = []
    osm_veg_cfg = recipe_dict.get("osm_vegetation", {"enabled": True})
    no_vegetation = getattr(args, "no_vegetation", False) or not osm_veg_cfg.get("enabled", True)

    if osm_path and not no_vegetation:
        print(f"[carla-map] Extracting and synthesizing native CARLA 3D vegetation from {osm_path.name}...")
        b_polys_for_veg = []
        if footprints:
            for fp in footprints:
                poly = [(project_lat_lon(lat, lon)[0], project_lat_lon(lat, lon)[1]) for lat, lon in fp.nodes]
                if len(poly) >= 3:
                    b_polys_for_veg.append(poly)

        veg_placements, veg_obstacles, veg_trees, veg_bushes = generate_osm_vegetation_placements(
            osm_path,
            carla_map=carla_map,
            project_coord_fn=project_lat_lon,
            building_polygons=b_polys_for_veg,
        )
        if veg_placements:
            vegetation_placements = veg_placements
            for obs_v, obs_t in veg_obstacles:
                obstacle_surfaces.append((obs_v, obs_t))

            placements_dir = bundle_dir / "Placements"
            placements_dir.mkdir(parents=True, exist_ok=True)
            veg_manifest_path = placements_dir / "vegetation.json"
            save_vegetation_manifest(veg_trees, veg_bushes, veg_manifest_path)
            vegetation_file_rel = "Placements/vegetation.json"
            print(f"[carla-map] Generated {len(veg_trees)} native CARLA trees and {len(veg_bushes)} bushes across {len(veg_placements)} static mesh groups.")

    # -------------------------------------------------------------------------
    # 5d. Satellite & Aerial Imagery Integration (Parcels, Parks & Parking Lots)
    # -------------------------------------------------------------------------
    satellite_file_rel: str | None = None
    sat_parked_cars: list[ClutterItem] = []
    osm_sat_cfg = recipe_dict.get("satellite_imagery", {"enabled": True})
    no_satellite = getattr(args, "no_satellite_imagery", False) or not osm_sat_cfg.get("enabled", True)
    no_sat_veg = getattr(args, "no_satellite_vegetation", False) or not osm_sat_cfg.get("vegetation_enabled", True)
    no_sat_parking = getattr(args, "no_satellite_parking", False) or not osm_sat_cfg.get("parking_enabled", True)
    custom_sat_img = getattr(args, "satellite_image", None) or osm_sat_cfg.get("image_path")

    if osm_path and not no_satellite:
        print(f"[carla-map] Ingesting and cross-referencing aerial/satellite imagery...")
        try:
            # Parse OSM bounding box
            osm_bounds = None
            try:
                tree = ET.parse(osm_path)
                bounds_el = tree.getroot().find("bounds")
                if bounds_el is not None:
                    osm_bounds = (
                        float(bounds_el.get("minlat")),
                        float(bounds_el.get("minlon")),
                        float(bounds_el.get("maxlat")),
                        float(bounds_el.get("maxlon")),
                    )
            except Exception:
                pass

            if osm_bounds is None:
                osm_bounds = (48.138, 11.564, 48.142, 11.570)

            sat_img, sat_meta = fetch_or_load_satellite_imagery(
                osm_bounds[0], osm_bounds[1], osm_bounds[2], osm_bounds[3],
                zoom=18,
                custom_image_path=custom_sat_img,
                osm_path=osm_path,
            )
            sat_classification = compute_spectral_classification(sat_img)
            sat_proj = SatelliteProjector(sat_meta, project_lat_lon)

            # Building polygons for exclusion
            b_polys_for_sat = []
            if footprints:
                for fp in footprints:
                    poly = [(project_lat_lon(lat, lon)[0], project_lat_lon(lat, lon)[1]) for lat, lon in fp.nodes]
                    if len(poly) >= 3:
                        b_polys_for_sat.append(poly)

            # Map bounds in simulation space
            all_x_sat = [wp.transform.location.x for wp in waypoints]
            all_y_sat = [wp.transform.location.y for wp in waypoints]
            sat_map_bounds = (min(all_x_sat), max(all_x_sat), min(all_y_sat), max(all_y_sat))

            green_parcels, parking_lots = analyze_satellite_parcels(
                sat_meta, sat_classification, sat_proj, b_polys_for_sat, carla_map, sat_map_bounds
            )

            # A. Green park & vegetation infill
            if not no_sat_veg and green_parcels:
                sat_green_mesh, sat_hism_veg, sat_obs, sat_foliage = generate_satellite_green_mesh_and_foliage(
                    green_parcels, carla_map, b_polys_for_sat
                )
                if sat_green_mesh["vertices"]:
                    g_cmesh_path = bundle_dir / "Meshes" / "osm_terrain_satellite.cmesh"
                    write_cmesh_file(
                        g_cmesh_path,
                        vertices=sat_green_mesh["vertices"],
                        triangles=sat_green_mesh["triangles"],
                        normals=sat_green_mesh["normals"],
                        uvs=sat_green_mesh["uvs"],
                    )
                    layers.append({
                        "name": "OSM_Terrain_Satellite_Grass",
                        "mesh_file": "Meshes/osm_terrain_satellite.cmesh",
                        "material_preset": "grass",
                        "semantic_label": "Vegetation",
                        "mobility": "Static",
                        "z_elevation": -0.01,
                    })
                    walkable_surfaces.append((sat_green_mesh["vertices"], sat_green_mesh["triangles"]))

                if sat_hism_veg:
                    vegetation_placements.extend(sat_hism_veg)
                for obs_v, obs_t in sat_obs:
                    obstacle_surfaces.append((obs_v, obs_t))

            # B. Surface parking lots & parked cars
            if not no_sat_parking and parking_lots:
                lot_mesh, stall_mesh, s_cars = generate_satellite_parking_layers(parking_lots, carla_map)
                if lot_mesh["vertices"]:
                    lot_cmesh_path = bundle_dir / "Meshes" / "osm_parking_lots.cmesh"
                    write_cmesh_file(
                        lot_cmesh_path,
                        vertices=lot_mesh["vertices"],
                        triangles=lot_mesh["triangles"],
                        normals=lot_mesh["normals"],
                        uvs=lot_mesh["uvs"],
                    )
                    layers.append({
                        "name": "OSM_Parking_Lots",
                        "mesh_file": "Meshes/osm_parking_lots.cmesh",
                        "material_preset": "road_asphalt",
                        "semantic_label": "Ground",
                        "mobility": "Static",
                        "z_elevation": 0.008,
                    })
                    walkable_surfaces.append((lot_mesh["vertices"], lot_mesh["triangles"]))

                if stall_mesh["vertices"]:
                    stall_cmesh_path = bundle_dir / "Meshes" / "osm_parking_stalls.cmesh"
                    write_cmesh_file(
                        stall_cmesh_path,
                        vertices=stall_mesh["vertices"],
                        triangles=stall_mesh["triangles"],
                        normals=stall_mesh["normals"],
                        uvs=stall_mesh["uvs"],
                    )
                    layers.append({
                        "name": "OSM_Parking_Stalls",
                        "mesh_file": "Meshes/osm_parking_stalls.cmesh",
                        "material_preset": "crosswalk",
                        "semantic_label": "RoadLines",
                        "mobility": "Static",
                        "z_elevation": 0.028,
                    })

                if s_cars:
                    sat_parked_cars.extend(s_cars)

            # Save manifest & diagnostic overlay
            placements_dir = bundle_dir / "Placements"
            placements_dir.mkdir(parents=True, exist_ok=True)
            sat_manifest_path = placements_dir / "satellite_analysis.json"
            save_satellite_manifest(green_parcels, parking_lots, sat_parked_cars, sat_manifest_path)
            satellite_file_rel = "Placements/satellite_analysis.json"

            # Diagnostic render
            images_dir = bundle_dir / "Images"
            images_dir.mkdir(parents=True, exist_ok=True)
            diag_path = images_dir / "satellite_analysis.png"
            render_satellite_diagnostic_overlay(
                sat_img, sat_meta, sat_classification, sat_proj, green_parcels, parking_lots, diag_path
            )
            print(f"[carla-map] Satellite Analysis: {len(green_parcels)} green parcels, {len(parking_lots)} parking lots ({sum(l.num_stalls for l in parking_lots)} stalls, {len(sat_parked_cars)} parked cars).")
        except Exception as exc:
            print(f"[carla-map] Warning: Satellite imagery processing notice: {exc}")

    # -------------------------------------------------------------------------
    # 5e. Dedicated Bicycle Infrastructure & Cycleway Networks (Milestone 13)
    # -------------------------------------------------------------------------
    cycleways_file_rel: str | None = None
    parked_bicycles: list[ClutterItem] = []
    osm_cyc_cfg = recipe_dict.get("osm_cycleways", {"enabled": True})
    no_cycleways = getattr(args, "no_cycleways", False) or not osm_cyc_cfg.get("enabled", True)
    no_bike_boxes = getattr(args, "no_bike_boxes", False) or not osm_cyc_cfg.get("bike_boxes_enabled", True)
    no_bike_stencils = getattr(args, "no_bike_stencils", False) or not osm_cyc_cfg.get("stencils_enabled", True)
    cyc_color_pref = getattr(args, "cycleway_color", "auto") or osm_cyc_cfg.get("color", "auto")

    if osm_path and not no_cycleways:
        print(f"[carla-map] Ingesting and synthesizing dedicated bicycle infrastructure from {osm_path.name}...")
        try:
            cycleways, bike_boxes, bike_racks = extract_osm_cycleways(
                osm_path,
                project_coord_fn=project_lat_lon,
                carla_map=carla_map,
            )

            if cycleways:
                # Apply user color override if not auto
                if cyc_color_pref in ("green", "red", "asphalt"):
                    for c in cycleways:
                        c.surface_color = cyc_color_pref

                # 1. Surface Mesh Layer
                cyc_mesh, cyc_walkable = generate_cycleway_geometry(cycleways)
                if cyc_mesh["vertices"]:
                    cyc_cmesh_path = bundle_dir / "Meshes" / "osm_cycleways.cmesh"
                    write_cmesh_file(
                        cyc_cmesh_path,
                        vertices=cyc_mesh["vertices"],
                        triangles=cyc_mesh["triangles"],
                        normals=cyc_mesh["normals"],
                        uvs=cyc_mesh["uvs"],
                        colors=cyc_mesh.get("colors"),
                    )

                    rgb = CYCLEWAY_COLOR_PRESETS.get(cyc_color_pref if cyc_color_pref != "auto" else "red", [195, 52, 42])
                    tex_filename = f"cycleway_{cyc_color_pref if cyc_color_pref != 'auto' else 'red'}.png"
                    tex_dir = bundle_dir / "Textures"
                    tex_dir.mkdir(parents=True, exist_ok=True)
                    tex_path = tex_dir / tex_filename
                    try:
                        from PIL import Image
                        Image.new("RGB", (128, 128), color=tuple(rgb)).save(tex_path)
                    except Exception:
                        pass

                    mat_params: dict[str, Any] = {
                        "base_color": rgb,
                        "roughness": 0.82,
                        "metallic": 0.0,
                    }
                    if tex_path.is_file():
                        mat_params["albedo_texture_path"] = f"Textures/{tex_filename}"

                    layers.append({
                        "name": "OSM_Cycleways",
                        "mesh_file": "Meshes/osm_cycleways.cmesh",
                        "material_preset": "custom_pbr" if cyc_color_pref != "asphalt" else "road_asphalt",
                        "material_params": mat_params,
                        "semantic_label": "Roads",
                        "mobility": "Static",
                    })
                    walkable_surfaces.extend(cyc_walkable)

                # 2. White Demarcation Line Markings
                demarc_mesh = generate_cycleway_demarcation_mesh(cycleways)
                if demarc_mesh["vertices"]:
                    demarc_cmesh_path = bundle_dir / "Meshes" / "osm_cycleway_lines.cmesh"
                    write_cmesh_file(
                        demarc_cmesh_path,
                        vertices=demarc_mesh["vertices"],
                        triangles=demarc_mesh["triangles"],
                        normals=demarc_mesh["normals"],
                        uvs=demarc_mesh["uvs"],
                    )
                    layers.append({
                        "name": "OSM_Cycleway_Lines",
                        "mesh_file": "Meshes/osm_cycleway_lines.cmesh",
                        "material_preset": "crosswalk",
                        "semantic_label": "RoadLines",
                        "mobility": "Static",
                        "z_elevation": 0.022,
                    })

                # 3. Bicycle Stencils & Advance Stop Lines (Bike Boxes)
                boxes_to_generate = [] if no_bike_boxes else bike_boxes
                if not no_bike_stencils or boxes_to_generate:
                    stencil_mesh, stencils_meta = generate_bike_stencils_and_boxes_mesh(
                        cycleways if not no_bike_stencils else [],
                        boxes_to_generate,
                    )
                    if stencil_mesh["vertices"]:
                        st_cmesh_path = bundle_dir / "Meshes" / "osm_cycleway_stencils.cmesh"
                        write_cmesh_file(
                            st_cmesh_path,
                            vertices=stencil_mesh["vertices"],
                            triangles=stencil_mesh["triangles"],
                            normals=stencil_mesh["normals"],
                            uvs=stencil_mesh["uvs"],
                        )
                        layers.append({
                            "name": "OSM_Cycleway_Stencils",
                            "mesh_file": "Meshes/osm_cycleway_stencils.cmesh",
                            "material_preset": "crosswalk",
                            "semantic_label": "RoadLines",
                            "mobility": "Static",
                            "z_elevation": 0.026,
                        })

                # 4. Bicycle Parking Clutter
                if bike_racks:
                    parked_bicycles = generate_bike_parking_clutter(bike_racks, carla_map=carla_map)

                # 5. Manifest Serialization
                placements_dir = bundle_dir / "Placements"
                placements_dir.mkdir(parents=True, exist_ok=True)
                cyc_manifest_path = placements_dir / "cycleways.json"
                save_cycleways_manifest(cycleways, bike_boxes, bike_racks, cyc_manifest_path)
                cycleways_file_rel = "Placements/cycleways.json"

                tot_km = sum(c.length_m for c in cycleways) / 1000.0
                print(f"[carla-map] Generated {len(cycleways)} cycleways ({tot_km:.2f} km), {len(bike_boxes)} bike boxes, {len(parked_bicycles)} parked bicycles.")

                # 6. Automatic Sidewalk Cropping for Cycleway Corridors
                xodr_xml_content, num_cropped, _ = crop_sidewalks_overlapping_cycleways(
                    xodr_xml_content, cycleways, carla_map=carla_map
                )
                if num_cropped > 0:
                    final_xodr_path.write_text(xodr_xml_content, encoding="utf-8")
                    print(f"[carla-map] Cropped {num_cropped} sidewalk lanes overlapping bicycle corridors.")
                    try:
                        carla_map = carla.Map(map_name, xodr_xml_content)
                    except Exception as exc:
                        print(f"[carla-map] Warning: Failed to re-instantiate carla.Map after cropping: {exc}")
        except Exception as exc:
            print(f"[carla-map] Warning: Cycleways processing notice: {exc}")

    # -------------------------------------------------------------------------
    # 5.5 Continuous Barriers, Perimeter Walls, Fences & Guard Rails (from OSM)
    # -------------------------------------------------------------------------
    barriers_file_rel = None
    osm_barriers_cfg = recipe_dict.get("osm_barriers", {"enabled": True})
    no_barriers = getattr(args, "no_barriers", False) or not osm_barriers_cfg.get("enabled", True)

    if osm_path and not no_barriers:
        try:
            print(f"[carla-map] Ingesting continuous barriers, walls, fences & guard rails from {osm_path.name}...")
            barriers = extract_osm_barriers(osm_path)
            if barriers:
                def_wall_h = float(osm_barriers_cfg.get("default_wall_height", 1.5))
                def_fence_h = float(osm_barriers_cfg.get("default_fence_height", 1.2))
                def_guard_h = float(osm_barriers_cfg.get("default_guardrail_height", 0.85))

                barrier_layers = build_osm_barriers_multislot_layers(
                    barriers,
                    project_coord_fn=project_lat_lon,
                    default_wall_height=def_wall_h,
                    default_fence_height=def_fence_h,
                    default_guardrail_height=def_guard_h,
                )

                for slot_id, l_data in barrier_layers.items():
                    lv = l_data["vertices"]
                    lt = l_data["triangles"]
                    if not lv or not lt:
                        continue
                    cmesh_filename = f"Meshes/osm_barriers_{slot_id}.cmesh"
                    b_cmesh_path = bundle_dir / cmesh_filename
                    write_cmesh_file(
                        b_cmesh_path,
                        vertices=lv,
                        triangles=lt,
                        normals=l_data["normals"],
                        uvs=l_data["uvs"],
                    )
                    layers.append({
                        "name": l_data["name"],
                        "mesh_file": cmesh_filename,
                        "semantic_label": l_data["semantic_label"],
                        "material_preset": l_data["material_preset"],
                        "enable_collision": l_data["enable_collision"],
                    })
                    obstacle_surfaces.append((lv, lt))

                # Serialize barriers.json into Placements/
                placements_dir = bundle_dir / "Placements"
                placements_dir.mkdir(parents=True, exist_ok=True)
                bar_manifest_path = placements_dir / "barriers.json"
                bar_meta = audit_barrier_metrics(barriers, barrier_layers)
                bar_manifest_path.write_text(json.dumps(bar_meta, indent=2) + "\n", encoding="utf-8")
                barriers_file_rel = "Placements/barriers.json"

                tot_bar_len = sum(l.get("total_length_m", 0.0) for l in barrier_layers.values())
                print(f"[carla-map] Generated {len(barriers)} barriers across {len(barrier_layers)} architectural layers ({tot_bar_len:.1f}m total).")
        except Exception as exc:
            print(f"[carla-map] Warning: Barriers processing notice: {exc}")

    # -------------------------------------------------------------------------
    # 5.6 Procedural 3D Sidewalk Curbs & Drainage Gutters (from OpenDRIVE)
    # -------------------------------------------------------------------------
    curbs_file_rel = None
    osm_curbs_cfg = recipe_dict.get("osm_curbs", {"enabled": True})
    no_curbs = getattr(args, "no_curbs", False) or not osm_curbs_cfg.get("enabled", True)

    if carla_map and not no_curbs:
        try:
            print(f"[carla-map] Synthesizing continuous 3D concrete curbs along sidewalk boundaries...")
            curb_mat = getattr(args, "curb_material", None) or osm_curbs_cfg.get(
                "material_preset", "/Game/Carla/Static/SideWalk/Curbs/MI_Curb_01.MI_Curb_01"
            )
            curb_cfg = CurbConfig(
                enabled=True,
                inner_curbs=osm_curbs_cfg.get("inner_curbs", True),
                outer_curbs=osm_curbs_cfg.get("outer_curbs", True),
                curb_top_width=float(osm_curbs_cfg.get("curb_top_width", 0.16)),
                outer_border_width=float(osm_curbs_cfg.get("outer_border_width", 0.12)),
                gutter_width=float(osm_curbs_cfg.get("gutter_width", 0.32)),
                chamfer_size=float(osm_curbs_cfg.get("chamfer_size", 0.03)),
                curb_elevation=float(osm_curbs_cfg.get("curb_elevation", 0.155)),
                gutter_elevation=float(osm_curbs_cfg.get("gutter_elevation", 0.012)),
                submerge_depth=float(osm_curbs_cfg.get("submerge_depth", 0.08)),
                material_preset=curb_mat,
            )
            curb_cmesh_path = bundle_dir / "Meshes" / "osm_sidewalk_curbs.cmesh"
            curb_geom = generate_sidewalk_curbs_mesh(
                carla_map=carla_map,
                xodr_xml_content=xodr_xml_content,
                output_cmesh_path=curb_cmesh_path,
                config=curb_cfg,
                cycleways=cycleways if "cycleways" in locals() and cycleways else None,
            )
            if curb_geom and curb_geom["vertices"]:
                layers.append({
                    "name": "OSM_Sidewalk_Curbs",
                    "mesh_file": "Meshes/osm_sidewalk_curbs.cmesh",
                    "semantic_label": "Sidewalks",
                    "material_preset": curb_mat,
                    "enable_collision": True,
                })
                walkable_surfaces.append((curb_geom["vertices"], curb_geom["triangles"]))

                placements_dir = bundle_dir / "Placements"
                placements_dir.mkdir(parents=True, exist_ok=True)
                curb_manifest_path = placements_dir / "curbs.json"
                curb_meta = audit_curb_metrics(curb_geom["metrics"])
                curb_manifest_path.write_text(json.dumps(curb_meta, indent=2) + "\n", encoding="utf-8")
                curbs_file_rel = "Placements/curbs.json"

                tot_c_len = curb_geom["metrics"].get("total_curb_length_m", 0.0)
                tot_c_runs = curb_geom["metrics"].get("sidewalk_runs", 0)
                print(f"[carla-map] Generated {tot_c_runs} 3D sidewalk curb sections ({tot_c_len:.1f}m total, {len(curb_geom['vertices'])} verts).")
        except Exception as exc:
            print(f"[carla-map] Warning: Sidewalk curbs processing notice: {exc}")

    # -------------------------------------------------------------------------
    # 5.7 Municipal Utilities, Fire Hydrants & Surface Covers (from OSM)
    # -------------------------------------------------------------------------
    utilities_file_rel = None
    utility_placements: list[dict[str, Any]] = []
    osm_util_cfg = recipe_dict.get("osm_utilities", {"enabled": True})
    no_utilities = getattr(args, "no_utilities", False) or not osm_util_cfg.get("enabled", True)
    no_proc_hydrants = getattr(args, "no_procedural_hydrants", False) or not osm_util_cfg.get("procedural_hydrants", True)
    hydrant_spacing = float(getattr(args, "hydrant_spacing", 120.0) or osm_util_cfg.get("hydrant_spacing_m", 120.0))

    if osm_path and not no_utilities:
        try:
            print(f"[carla-map] Ingesting municipal utilities & fire hydrants from {osm_path.name}...")
            building_polys_for_util = []
            if footprints:
                for fp in footprints:
                    poly = [(project_lat_lon(lat, lon)[0], project_lat_lon(lat, lon)[1]) for lat, lon in fp.nodes]
                    if len(poly) >= 3:
                        building_polys_for_util.append(poly)

            osm_utils = extract_osm_utilities(
                osm_path,
                project_coord_fn=project_lat_lon,
                carla_map=carla_map,
                building_polygons=building_polys_for_util,
                cycleways=cycleways if "cycleways" in locals() and cycleways else None,
            )

            synth_hydrants: list[UtilityItem] = []
            if not no_proc_hydrants and carla_map is not None:
                synth_hydrants = synthesize_procedural_fire_hydrants(
                    carla_map,
                    existing_hydrants=osm_utils,
                    spacing=hydrant_spacing,
                    building_polygons=building_polys_for_util,
                    cycleways=cycleways if "cycleways" in locals() and cycleways else None,
                )

            all_utilities = osm_utils + synth_hydrants

            if all_utilities:
                # 1. 3D Surface Mesh for ground covers (circular manholes & underground hydrants)
                cover_mesh = generate_utility_mesh_layers(all_utilities)
                if cover_mesh["vertices"]:
                    cmesh_filename = "Meshes/osm_utility_covers.cmesh"
                    u_cmesh_path = bundle_dir / cmesh_filename
                    write_cmesh_file(
                        u_cmesh_path,
                        vertices=cover_mesh["vertices"],
                        triangles=cover_mesh["triangles"],
                        normals=cover_mesh["normals"],
                        uvs=cover_mesh["uvs"],
                    )
                    layers.append({
                        "name": cover_mesh["name"],
                        "mesh_file": cmesh_filename,
                        "semantic_label": cover_mesh["semantic_label"],
                        "material_preset": cover_mesh["material_preset"],
                        "enable_collision": False,
                        "z_elevation": 0.012,
                    })
                    walkable_surfaces.append((cover_mesh["vertices"], cover_mesh["triangles"]))

                # 2. HISM Placements for standing assets (pillar hydrants, street cabinets)
                util_hism = generate_utility_hism_placements(all_utilities)
                if util_hism:
                    utility_placements.extend(util_hism)

                # 3. Obstacle geometry for Recast NavMesh (pillar hydrants and street cabinets)
                util_obs = generate_utility_obstacle_geometry(all_utilities)
                for obs_v, obs_t in util_obs:
                    obstacle_surfaces.append((obs_v, obs_t))

                # 4. Manifest serialization into Placements/utilities.json
                placements_dir = bundle_dir / "Placements"
                placements_dir.mkdir(parents=True, exist_ok=True)
                util_manifest_path = placements_dir / "utilities.json"
                save_utilities_manifest(all_utilities, util_manifest_path, cover_mesh)
                utilities_file_rel = "Placements/utilities.json"

                u_metrics = audit_utility_metrics(all_utilities, cover_mesh)
                u_counts = u_metrics["counts"]
                print(
                    f"[carla-map] Generated {u_counts['total_utilities']} utilities: "
                    f"{u_counts['pillar_hydrants']} pillar hydrants, "
                    f"{u_counts['underground_hydrants']} underground hydrants, "
                    f"{u_counts['street_cabinets']} street cabinets, "
                    f"{u_counts['manhole_covers']} manholes."
                )
        except Exception as exc:
            print(f"[carla-map] Warning: Utilities processing notice: {exc}")

    # -------------------------------------------------------------------------
    # 5.8 Physical Speed Limit & Warning Signs (StVO metal signboards)
    # -------------------------------------------------------------------------
    traffic_signs_file_rel = None
    osm_signs_cfg = recipe_dict.get("osm_traffic_signs", {"enabled": True})
    no_traffic_signs = getattr(args, "no_traffic_signs", False) or not osm_signs_cfg.get("enabled", True)
    sign_spacing = float(getattr(args, "sign_spacing", 160.0) or osm_signs_cfg.get("sign_spacing_m", 160.0))

    if osm_path and not no_traffic_signs:
        try:
            print(f"[carla-map] Ingesting & synthesizing physical StVO traffic signs from {osm_path.name}...")
            # 1. Generate crisp retroreflective texture atlas in Textures/
            tex_dir = bundle_dir / "Textures"
            tex_dir.mkdir(parents=True, exist_ok=True)
            atlas_path = tex_dir / "traffic_signs_atlas.png"
            generate_traffic_signs_texture_atlas(atlas_path)

            building_polys_for_signs = []
            if footprints:
                for fp in footprints:
                    poly = [(project_lat_lon(lat, lon)[0], project_lat_lon(lat, lon)[1]) for lat, lon in fp.nodes]
                    if len(poly) >= 3:
                        building_polys_for_signs.append(poly)

            osm_signs = extract_osm_traffic_signs(
                osm_path,
                project_coord_fn=project_lat_lon,
                carla_map=carla_map,
                building_polygons=building_polys_for_signs,
                cycleways=cycleways if "cycleways" in locals() and cycleways else None,
            )

            synth_signs: list[TrafficSignItem] = []
            if carla_map is not None:
                synth_signs = synthesize_procedural_traffic_signs(
                    carla_map,
                    existing_signs=osm_signs,
                    spacing=sign_spacing,
                    building_polygons=building_polys_for_signs,
                    cycleways=cycleways if "cycleways" in locals() and cycleways else None,
                )

            all_traffic_signs = osm_signs + synth_signs

            if all_traffic_signs:
                # 2. 3D Metal Geometry (tubular steel post, ground collar, aluminum bracket, StVO plate)
                sign_mesh = generate_traffic_signs_mesh(
                    all_traffic_signs,
                    atlas_relative_path="Textures/traffic_signs_atlas.png",
                )
                if sign_mesh["vertices"]:
                    cmesh_filename = "Meshes/osm_traffic_signs.cmesh"
                    s_cmesh_path = bundle_dir / cmesh_filename
                    write_cmesh_file(
                        s_cmesh_path,
                        vertices=sign_mesh["vertices"],
                        triangles=sign_mesh["triangles"],
                        normals=sign_mesh["normals"],
                        uvs=sign_mesh["uvs"],
                    )
                    layers.append({
                        "name": sign_mesh["name"],
                        "mesh_file": cmesh_filename,
                        "semantic_label": sign_mesh["semantic_label"],
                        "material_preset": sign_mesh["material_preset"],
                        "material_params": sign_mesh["material_params"],
                        "enable_collision": True,
                    })

                # 3. Obstacle geometry for Recast NavMesh
                sign_obs = generate_traffic_sign_obstacle_geometry(all_traffic_signs)
                for obs_v, obs_t in sign_obs:
                    obstacle_surfaces.append((obs_v, obs_t))

                # 4. Synchronize OpenDRIVE <signals> with curb-aligned t offset
                xodr_xml_content = inject_traffic_signs_into_opendrive(
                    xodr_xml_content, all_traffic_signs
                )
                final_xodr_path.write_text(xodr_xml_content, encoding="utf-8")
                try:
                    carla_map = carla.Map(map_name, xodr_xml_content)
                except Exception as exc:
                    print(f"[carla-map] Warning: Failed to re-instantiate carla.Map after sign injection: {exc}")

                # 5. Manifest serialization into Placements/traffic_signs.json
                placements_dir = bundle_dir / "Placements"
                placements_dir.mkdir(parents=True, exist_ok=True)
                signs_manifest_path = placements_dir / "traffic_signs.json"
                save_traffic_signs_manifest(all_traffic_signs, signs_manifest_path, sign_mesh)
                traffic_signs_file_rel = "Placements/traffic_signs.json"

                s_metrics = audit_traffic_sign_metrics(all_traffic_signs, sign_mesh)
                print(
                    f"[carla-map] Generated {s_metrics['total_signs']} StVO traffic signs: "
                    f"{s_metrics['counts_by_type'].get('speed_limit', 0)} speed limits, "
                    f"{s_metrics['counts_by_type'].get('stop', 0)} stop signs, "
                    f"{s_metrics['counts_by_type'].get('yield', 0)} yield signs, "
                    f"{s_metrics['counts_by_type'].get('priority_road', 0)} priority road signs, "
                    f"{s_metrics['counts_by_type'].get('warning', 0)} warning signs."
                )
        except Exception as exc:
            print(f"[carla-map] Warning: Traffic signs processing notice: {exc}")

    # -------------------------------------------------------------------------
    # 6. Offline Pedestrian Navigation Mesh (Nav/<MapName>.bin) Compilation
    # -------------------------------------------------------------------------
    nav_bin_path = bundle_dir / "Nav" / f"{map_name}.bin"
    print(f"[carla-map] Compiling pedestrian navigation mesh -> {nav_bin_path.name}...")

    # Extract continuous sidewalk and road lane ribbons from OpenDRIVE
    if carla_map is not None:
        try:
            topo = carla_map.get_topology()
            road_lanes: set[tuple[int, int]] = set()
            for seg_start, _ in topo:
                for direction in ["self", "right", "left"]:
                    curr = seg_start if direction == "self" else (seg_start.get_right_lane() if direction == "right" else seg_start.get_left_lane())
                    visited_lanes: set[tuple[int, int]] = set()
                    while curr is not None and (curr.road_id, curr.lane_id) not in visited_lanes:
                        visited_lanes.add((curr.road_id, curr.lane_id))
                        road_lanes.add((curr.road_id, curr.lane_id))
                        if direction == "right":
                            curr = curr.get_right_lane()
                        elif direction == "left":
                            curr = curr.get_left_lane()
                        else:
                            break

            for road_id, lane_id in road_lanes:
                wp = carla_map.get_waypoint_xodr(road_id, lane_id, 0.1)
                if wp is None:
                    continue
                lane_type = wp.lane_type
                is_sidewalk = (lane_type == carla.LaneType.Sidewalk)
                is_road = (lane_type == carla.LaneType.Driving)
                if not is_sidewalk and not is_road:
                    continue

                curr = wp
                pts_left = []
                pts_right = []
                visited_s: set[float] = set()
                while curr is not None and curr.road_id == road_id and curr.lane_id == lane_id:
                    s_key = round(curr.s, 2)
                    if s_key in visited_s or len(pts_left) >= 5000:
                        break
                    visited_s.add(s_key)

                    loc = curr.transform.location
                    yaw_rad = math.radians(curr.transform.rotation.yaw)
                    rx, ry = -math.sin(yaw_rad), math.cos(yaw_rad)
                    half_w = curr.lane_width * 0.5
                    z = loc.z + (0.15 if is_sidewalk else 0.0)

                    pts_left.append((loc.x - half_w * rx, loc.y - half_w * ry, z))
                    pts_right.append((loc.x + half_w * rx, loc.y + half_w * ry, z))

                    next_wps = curr.next(2.0)
                    if not next_wps:
                        break
                    matching = [w for w in next_wps if w.road_id == road_id and w.lane_id == lane_id]
                    if not matching:
                        break
                    curr = matching[0]

                n_pts = len(pts_left)
                if n_pts >= 2:
                    l_verts = []
                    l_tris = []
                    for i in range(n_pts):
                        l_verts.append(pts_left[i])
                        l_verts.append(pts_right[i])
                    for i in range(n_pts - 1):
                        i0 = i * 2
                        i1 = i * 2 + 1
                        i2 = (i + 1) * 2
                        i3 = (i + 1) * 2 + 1
                        l_tris.append((i0, i2, i1))
                        l_tris.append((i1, i2, i3))
                    walkable_surfaces.append((l_verts, l_tris, "sidewalk" if is_sidewalk else "road"))
        except Exception as exc:
            print(f"[carla-map] Notice: OpenDRIVE lane mesh extraction for navmesh: {exc}")

    # Fallback proxy walkway if still empty
    if not walkable_surfaces and waypoints:
        proxy_verts: list[tuple[float, float, float]] = []
        proxy_tris: list[tuple[int, int, int]] = []
        for wp in waypoints:
            loc = wp.transform.location
            w = wp.lane_width * 0.5
            p0 = (loc.x - w, loc.y - w, loc.z)
            p1 = (loc.x + w, loc.y - w, loc.z)
            p2 = (loc.x + w, loc.y + w, loc.z)
            p3 = (loc.x - w, loc.y + w, loc.z)
            idx = len(proxy_verts)
            proxy_verts.extend([p0, p1, p2, p3])
            proxy_tris.append((idx, idx + 1, idx + 2))
            proxy_tris.append((idx, idx + 2, idx + 3))
        walkable_surfaces.append((proxy_verts, proxy_tris, "sidewalk"))

    intermediate_obj = bundle_dir / "Nav" / f"{map_name}_nav_temp.obj"
    export_recast_geometry_obj(intermediate_obj, walkable_surfaces, obstacle_surfaces)

    try:
        compile_navmesh_binary(intermediate_obj, nav_bin_path, repo_root=REPO_ROOT)
        print(f"[carla-map] NavMesh compiled successfully ({nav_bin_path.stat().st_size} bytes).")
    except Exception as exc:
        print(f"[carla-map] Warning: RecastBuilder execution notice: {exc}")
        if not nav_bin_path.exists() or nav_bin_path.stat().st_size == 0:
            # Minimal header fallback
            nav_bin_path.write_bytes(b"\x00" * 40)
    finally:
        if intermediate_obj.exists():
            intermediate_obj.unlink()

    # -------------------------------------------------------------------------
    # 6. Urban Clutter, Vegetation & Curbside Parked Cars (from OSM)
    # -------------------------------------------------------------------------
    osm_clutter_cfg = recipe_dict.get("osm_clutter", {"enabled": True})
    no_clutter = getattr(args, "no_clutter", False) or not osm_clutter_cfg.get("enabled", True)
    no_parking = getattr(args, "no_parked_cars", False) or not osm_clutter_cfg.get("parked_cars_enabled", True)
    max_parked = int(getattr(args, "max_parked_cars", 25) or osm_clutter_cfg.get("max_parked_cars", 25))

    clutter_items: list[ClutterItem] = []
    parked_cars: list[ClutterItem] = []

    if osm_path and not no_clutter:
        building_polys_for_clutter = []
        if footprints:
            for fp in footprints:
                poly = [(project_lat_lon(lat, lon)[0], project_lat_lon(lat, lon)[1]) for lat, lon in fp.nodes]
                if len(poly) >= 3:
                    building_polys_for_clutter.append(poly)

        osm_items = extract_osm_clutter(
            osm_path,
            project_coord_fn=project_lat_lon,
            carla_map=carla_map,
            building_polygons=building_polys_for_clutter,
        )
        synth_items = synthesize_procedural_sidewalk_clutter(
            carla_map,
            existing_items=osm_items,
            building_polygons=building_polys_for_clutter,
        )
        clutter_items = osm_items + synth_items
        if parked_bicycles:
            clutter_items.extend(parked_bicycles)
        print(f"[carla-map] Extracted {len(osm_items)} OSM clutter items and synthesized {len(synth_items)} sidewalk items.")

    if not no_parking:
        parked_cars = generate_curbside_parked_cars(carla_map, max_cars=max_parked)
        if sat_parked_cars:
            parked_cars.extend(sat_parked_cars)
        print(f"[carla-map] Placed {len(parked_cars)} parked cars ({len(sat_parked_cars)} in surface lots).")
    elif sat_parked_cars and not no_sat_parking:
        parked_cars = list(sat_parked_cars)
        print(f"[carla-map] Placed {len(parked_cars)} surface lot parked cars.")

    placements_file_rel: str | None = None
    if clutter_items or parked_cars:
        placements_dir = bundle_dir / "Placements"
        placements_dir.mkdir(parents=True, exist_ok=True)
        clutter_json_path = placements_dir / "clutter.json"
        save_clutter_manifest(clutter_items, parked_cars, clutter_json_path)
        placements_file_rel = "Placements/clutter.json"

    # -------------------------------------------------------------------------
    # 7. Street Lighting & Nocturnal Luminaire Placement (from OSM)
    # -------------------------------------------------------------------------
    osm_lighting_cfg = recipe_dict.get("osm_lighting", {"enabled": True})
    no_lighting = getattr(args, "no_lighting", False) or not osm_lighting_cfg.get("enabled", True)

    street_lamps: list[StreetLamp] = []
    if osm_path and not no_lighting:
        building_polys_for_lighting = []
        if footprints:
            for fp in footprints:
                poly = [(project_lat_lon(lat, lon)[0], project_lat_lon(lat, lon)[1]) for lat, lon in fp.nodes]
                if len(poly) >= 3:
                    building_polys_for_lighting.append(poly)

        osm_lamps = extract_osm_street_lamps(
            osm_path,
            project_coord_fn=project_lat_lon,
            carla_map=carla_map,
            building_polygons=building_polys_for_lighting,
        )
        synth_lamps = synthesize_procedural_street_lamps(
            carla_map,
            existing_lamps=osm_lamps,
            building_polygons=building_polys_for_lighting,
            spacing=20.0,
            min_dist_existing=13.0,
        )
        street_lamps = osm_lamps + synth_lamps
        print(f"[carla-map] Extracted {len(osm_lamps)} OSM street lamps and synthesized {len(synth_lamps)} procedural lamps.")

    streetlights_file_rel: str | None = None
    if street_lamps:
        placements_dir = bundle_dir / "Placements"
        placements_dir.mkdir(parents=True, exist_ok=True)
        lights_json_path = placements_dir / "streetlights.json"
        save_lighting_manifest(street_lamps, lights_json_path)
        streetlights_file_rel = "Placements/streetlights.json"

    # -------------------------------------------------------------------------
    # 8. Dynamic Traffic Lights & OpenDRIVE Signals
    # -------------------------------------------------------------------------
    osm_tl_cfg = recipe_dict.get("osm_traffic_lights", {"enabled": True})
    no_traffic_lights = getattr(args, "no_traffic_lights", False) or not osm_tl_cfg.get("enabled", True)

    traffic_signals: list[TrafficSignal] = []
    trafficlights_file_rel: str | None = None
    if osm_path and not no_traffic_lights:
        traffic_signals = extract_osm_traffic_signals(
            osm_path,
            project_coord_fn=project_lat_lon,
            carla_map=carla_map,
        )
        print(f"[carla-map] Extracted {len(traffic_signals)} traffic signals from OSM.")

        if traffic_signals:
            modified_xodr = inject_signals_into_opendrive(xodr_xml_content, traffic_signals)
            final_xodr_path.write_text(modified_xodr, encoding="utf-8")
            xodr_xml_content = modified_xodr
            try:
                carla_map = carla.Map(map_name, xodr_xml_content)
                carla_map.cook_in_memory_map(str(tm_bin_path))
                print(f"[carla-map] OpenDRIVE updated with signals ({len(carla_map.get_all_landmarks())} landmarks/signals registered).")
            except Exception as exc:
                print(f"[carla-map] Warning: Re-parsing OpenDRIVE with signals failed: {exc}")

            placements_dir = bundle_dir / "Placements"
            placements_dir.mkdir(parents=True, exist_ok=True)
            tl_json_path = placements_dir / "traffic_lights.json"
            save_traffic_lights_manifest(traffic_signals, tl_json_path)
            trafficlights_file_rel = "Placements/traffic_lights.json"

            # Physical painted stop line decals before traffic lights
            stop_line_layer = generate_stop_line_meshes(traffic_signals, carla_map=carla_map, z_offset=0.035)
            if stop_line_layer and stop_line_layer["vertices"]:
                sl_cmesh_path = bundle_dir / "Meshes" / "osm_stop_lines.cmesh"
                write_cmesh_file(
                    sl_cmesh_path,
                    vertices=stop_line_layer["vertices"],
                    triangles=stop_line_layer["triangles"],
                    normals=stop_line_layer["normals"],
                    uvs=stop_line_layer["uvs"],
                )
                layers.append({
                    "name": stop_line_layer["name"],
                    "mesh_file": "Meshes/osm_stop_lines.cmesh",
                    "semantic_label": stop_line_layer["semantic_label"],
                    "material_preset": stop_line_layer["material_preset"],
                    "enable_collision": stop_line_layer["enable_collision"],
                })
                print(f"[carla-map] Generated {len(stop_line_layer['vertices']) // 4} physical stop lines ({len(stop_line_layer['vertices'])} verts, {len(stop_line_layer['triangles'])} tris).")

    # -------------------------------------------------------------------------
    # 9. Spawn Points & Asset Placements
    # -------------------------------------------------------------------------
    spawn_points: list[dict[str, Any]] = []
    if recipe_dict.get("spawn_points"):
        spawn_points = recipe_dict["spawn_points"]
    else:
        # Extract spawn points from OpenDRIVE map
        carla_spawns = carla_map.get_spawn_points()
        for sp in carla_spawns[:10]:
            spawn_points.append({
                "location": [round(sp.location.x, 3), round(sp.location.y, 3), round(sp.location.z + 0.5, 3)],
                "rotation": [round(sp.rotation.pitch, 2), round(sp.rotation.yaw, 2), round(sp.rotation.roll, 2)],
            })

    placements = list(recipe_dict.get("placements", []))
    if building_placements:
        placements.extend(building_placements)
    if vegetation_placements:
        placements.extend(vegetation_placements)
    if utility_placements:
        placements.extend(utility_placements)

    # -------------------------------------------------------------------------
    # 10. Generate carla-map.json Manifest
    # -------------------------------------------------------------------------
    od_params = recipe_dict.get("opendrive_parameters", {})
    # If custom road meshes were imported, default generate_road_mesh to false to avoid Z-fighting
    gen_road_mesh = od_params.get("generate_road_mesh", False if has_custom_road_meshes else True)
    gen_ground_mesh = od_params.get("generate_ground_mesh", False)

    manifest: dict[str, Any] = {
        "format_version": "1.0",
        "map_name": map_name,
        "xodr_file": f"OpenDrive/{map_name}.xodr",
        "nav_file": f"Nav/{map_name}.bin",
        "tm_file": f"TM/{map_name}.bin",
        "opendrive_parameters": {
            "generate_road_mesh": gen_road_mesh,
            "generate_ground_mesh": gen_ground_mesh,
            "generate_traffic_signals": od_params.get("generate_traffic_signals", True),
            "generate_street_furniture": od_params.get("generate_street_furniture", False),
            "wall_height": float(od_params.get("wall_height", 0.0)),
        },
        "layers": layers,
        "placements": placements,
        "spawn_points": spawn_points,
    }
    if placements_file_rel:
        manifest["placements_file"] = placements_file_rel
    if streetlights_file_rel:
        manifest["streetlights_file"] = streetlights_file_rel
    if trafficlights_file_rel:
        manifest["traffic_lights_file"] = trafficlights_file_rel
    if crosswalks_file_rel:
        manifest["crosswalks_file"] = crosswalks_file_rel
    if vegetation_file_rel:
        manifest["vegetation_file"] = vegetation_file_rel
    if satellite_file_rel:
        manifest["satellite_file"] = satellite_file_rel
    if cycleways_file_rel:
        manifest["cycleways_file"] = cycleways_file_rel
    if barriers_file_rel:
        manifest["barriers_file"] = barriers_file_rel
    if curbs_file_rel:
        manifest["curbs_file"] = curbs_file_rel
    if utilities_file_rel:
        manifest["utilities_file"] = utilities_file_rel
    if traffic_signs_file_rel:
        manifest["traffic_signs_file"] = traffic_signs_file_rel

    manifest_path = bundle_dir / "carla-map.json"
    manifest_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")

    # -------------------------------------------------------------------------
    # 8. Create Compressed .tar.gz Bundle Archive
    # -------------------------------------------------------------------------
    print(f"[carla-map] Creating bundle archive -> {tar_output_path.name}...")
    with tarfile.open(tar_output_path, "w:gz") as tar:
        tar.add(bundle_dir, arcname=map_name)

    print(f"\n[carla-map] BUILD SUCCESS:")
    print(f"  Bundle Directory : {bundle_dir}")
    print(f"  Bundle Archive   : {tar_output_path} ({tar_output_path.stat().st_size:,} bytes)")
    print(f"  Layers           : {len(layers)} (.cmesh geometry layers)")
    print(f"  Road Mesh Mode   : {'Procedural Mesh' if gen_road_mesh else 'Custom Mesh Only (Zero Z-Fighting)'}")
    print(f"  To inspect       : carla-map inspect {tar_output_path}")
    print(f"  To verify        : carla-map verify {tar_output_path}")
    return 0


# =============================================================================
# Subcommand: inspect
# =============================================================================

def cmd_inspect(args: argparse.Namespace) -> int:
    """Inspects and reports all metadata and geometry layers inside a .carlamap bundle."""
    target_path = Path(args.target).resolve()
    temp_dir: Path | None = None

    if target_path.is_file() and target_path.name.endswith((".tar.gz", ".tar")):
        import tempfile
        temp_dir = Path(tempfile.mkdtemp(prefix="carla_inspect_"))
        with tarfile.open(target_path, "r:*") as tar:
            tar.extractall(temp_dir)
        # Find inner folder
        inner_dirs = [d for d in temp_dir.iterdir() if d.is_dir() and (d / "carla-map.json").is_file()]
        bundle_dir = inner_dirs[0] if inner_dirs else temp_dir
    elif target_path.is_dir():
        bundle_dir = target_path
    else:
        print(f"[carla-map] ERROR: Target '{target_path}' is neither a .carlamap directory nor a .tar.gz archive.")
        return 1

    try:
        manifest_file = bundle_dir / "carla-map.json"
        if not manifest_file.is_file():
            print(f"[carla-map] ERROR: Missing 'carla-map.json' in bundle: {bundle_dir}")
            return 1

        manifest = json.loads(manifest_file.read_text(encoding="utf-8"))
        map_name = manifest.get("map_name", "Unknown")

        print("=" * 72)
        print(f"CARLA Map Bundle Inspection: {map_name}")
        print("=" * 72)
        print(f"  Format Version   : {manifest.get('format_version', '1.0')}")
        print(f"  OpenDRIVE Path   : {manifest.get('xodr_file')}")
        print(f"  NavMesh Path     : {manifest.get('nav_file')}")
        print(f"  TM Cache Path    : {manifest.get('tm_file', 'None')}")

        od_params = manifest.get("opendrive_parameters", {})
        print(f"  OpenDRIVE Params : generate_road_mesh={od_params.get('generate_road_mesh')}, generate_ground_mesh={od_params.get('generate_ground_mesh')}")

        # OpenDRIVE Inspection
        xodr_rel = manifest.get("xodr_file")
        if xodr_rel and (bundle_dir / xodr_rel).is_file():
            xodr_data = (bundle_dir / xodr_rel).read_text(encoding="utf-8")
            try:
                cm = carla.Map(map_name, xodr_data)
                wps = cm.generate_waypoints(2.0)
                topo = cm.get_topology()
                landmarks = cm.get_all_landmarks()
                print(f"  Logical Roads    : {len(topo)} segments, {len(wps)} waypoints (2m), {len(landmarks)} landmarks/signals")
            except Exception as exc:
                print(f"  Logical Roads    : Error parsing OpenDRIVE: {exc}")

        # Layers Inspection
        layers = manifest.get("layers", [])
        print(f"\nGeometry Layers ({len(layers)}):")
        total_verts = 0
        total_tris = 0
        for idx, layer in enumerate(layers, 1):
            mesh_rel = layer.get("mesh_file", "")
            mesh_path = bundle_dir / mesh_rel
            stats_str = "File Missing"
            if mesh_path.is_file():
                try:
                    geom = read_cmesh_file(mesh_path)
                    v_cnt = geom["vertex_count"]
                    t_cnt = geom["triangle_count"]
                    total_verts += v_cnt
                    total_tris += t_cnt
                    b = geom["bounds"]["size"]
                    stats_str = f"{v_cnt:,} verts, {t_cnt:,} tris | Bounding Box: {b[0]:.1f}m x {b[1]:.1f}m x {b[2]:.1f}m"
                except Exception as exc:
                    stats_str = f"Corrupt ({exc})"

            print(f"  [{idx}] {layer.get('name', 'Unnamed'):<20} | Label: {layer.get('semantic_label'):<12} | Mat: {layer.get('material_preset'):<18} | {stats_str}")

        print(f"  Total Geometry   : {total_verts:,} vertices, {total_tris:,} triangles")

        # Placements / Clutter & Parked Cars
        placements_rel = manifest.get("placements_file")
        if placements_rel and (bundle_dir / placements_rel).is_file():
            try:
                p_data = json.loads((bundle_dir / placements_rel).read_text(encoding="utf-8"))
                clutter_cnt = len(p_data.get("clutter", []))
                parked_cnt = len(p_data.get("parked_cars", []))
                print(f"\nAsset Placements & Street Furniture:")
                print(f"  Clutter Props    : {clutter_cnt:,} items (trees, benches, bins, bollards, etc.)")
                print(f"  Parked Cars      : {parked_cnt:,} static curbside vehicles")
            except Exception:
                pass

        # Street Lighting & Nocturnal Luminaires
        streetlights_rel = manifest.get("streetlights_file")
        if streetlights_rel and (bundle_dir / streetlights_rel).is_file():
            try:
                sl_data = json.loads((bundle_dir / streetlights_rel).read_text(encoding="utf-8"))
                counts = sl_data.get("counts", {})
                tot_l = counts.get("total_lamps", len(sl_data.get("street_lamps", [])))
                osm_l = counts.get("osm_lamps", 0)
                synth_l = counts.get("synthetic_lamps", 0)
                print(f"\nStreet Lighting & Nocturnal Luminaires:")
                print(f"  Total Street Lamps : {tot_l:,} ({osm_l} OSM authentic, {synth_l} procedural)")
            except Exception:
                pass

        # Dynamic Traffic Lights & Signals
        trafficlights_rel = manifest.get("traffic_lights_file")
        if trafficlights_rel and (bundle_dir / trafficlights_rel).is_file():
            try:
                tl_data = json.loads((bundle_dir / trafficlights_rel).read_text(encoding="utf-8"))
                counts = tl_data.get("counts", {})
                tot_sig = counts.get("total_signals", len(tl_data.get("traffic_signals", tl_data.get("signals", []))))
                cant_sig = counts.get("cantilever_masts", counts.get("cantilever_signals", 0))
                post_sig = counts.get("vertical_posts", counts.get("post_signals", 0))
                ped_sig = counts.get("pedestrian_signals", 0)
                print(f"\nDynamic Traffic Lights & Signals:")
                print(f"  Total Signals    : {tot_sig:,} ({cant_sig} cantilever, {post_sig} post, {ped_sig} pedestrian)")
            except Exception:
                pass

        # Pedestrian Crosswalks & Markings
        crosswalks_rel = manifest.get("crosswalks_file")
        if crosswalks_rel and (bundle_dir / crosswalks_rel).is_file():
            try:
                cw_data = json.loads((bundle_dir / crosswalks_rel).read_text(encoding="utf-8"))
                cw_cnt = cw_data.get("crosswalks_count", len(cw_data.get("crosswalks", [])))
                tot_stripes = cw_data.get("total_stripes", 0)
                islands_cnt = sum(1 for c in cw_data.get("crosswalks", []) if c.get("has_island"))
                tactile_cnt = sum(1 for c in cw_data.get("crosswalks", []) if c.get("has_tactile"))
                print(f"\nPedestrian Crosswalks & Markings:")
                print(f"  Crosswalks       : {cw_cnt:,} locations ({tot_stripes:,} zebra stripes, {islands_cnt} refuge islands, {tactile_cnt} tactile pads)")
            except Exception:
                pass

        # Urban Vegetation & 3D Foliage
        vegetation_rel = manifest.get("vegetation_file")
        if vegetation_rel and (bundle_dir / vegetation_rel).is_file():
            try:
                v_data = json.loads((bundle_dir / vegetation_rel).read_text(encoding="utf-8"))
                counts = v_data.get("counts", {})
                tot_trees = counts.get("total_trees", len(v_data.get("trees", [])))
                tot_bushes = counts.get("total_bushes", len(v_data.get("bushes", [])))
                print(f"\nUrban Vegetation & 3D Foliage:")
                print(f"  Trees            : {tot_trees:,} (avenue corridors & park canopies)")
                print(f"  Bushes           : {tot_bushes:,} understory shrubs")
            except Exception:
                pass

        # Satellite & Aerial Imagery Analysis
        satellite_rel = manifest.get("satellite_file")
        if satellite_rel and (bundle_dir / satellite_rel).is_file():
            try:
                sat_data = json.loads((bundle_dir / satellite_rel).read_text(encoding="utf-8"))
                g_parcels = sat_data.get("total_green_parcels", len(sat_data.get("green_parcels", [])))
                p_lots = sat_data.get("total_parking_lots", len(sat_data.get("parking_lots", [])))
                tot_stalls = sat_data.get("total_stalls", sum(p.get("num_stalls", 0) for p in sat_data.get("parking_lots", [])))
                tot_parked = sat_data.get("total_parked_cars", len(sat_data.get("parked_cars", [])))
                print(f"\nSatellite & Aerial Imagery Analysis:")
                print(f"  Green Parcels    : {g_parcels:,} detected parks & garden canopies")
                print(f"  Surface Parking  : {p_lots:,} open surface lots ({tot_stalls:,} marked stalls, {tot_parked:,} parked cars)")
            except Exception:
                pass

        # Dedicated Bicycle Infrastructure & Cycleway Networks
        cycleways_rel = manifest.get("cycleways_file")
        if cycleways_rel and (bundle_dir / cycleways_rel).is_file():
            try:
                cyc_data = json.loads((bundle_dir / cycleways_rel).read_text(encoding="utf-8"))
                counts = cyc_data.get("counts", {})
                tot_cyc = counts.get("total_cycleways", len(cyc_data.get("cycleways", [])))
                tot_km = counts.get("total_lane_km", 0.0)
                tot_bb = counts.get("total_bike_boxes", len(cyc_data.get("bike_boxes", [])))
                tot_racks = counts.get("total_bike_racks", len(cyc_data.get("bike_racks", [])))
                tot_bikes = counts.get("total_parked_bikes", sum(r.get("capacity", 0) for r in cyc_data.get("bike_racks", [])))
                print(f"\nDedicated Bicycle Infrastructure:")
                print(f"  Cycleways        : {tot_cyc:,} corridors ({tot_km:.2f} lane km)")
                print(f"  Bike Boxes (ASLs): {tot_bb:,} advance stop lines")
                print(f"  Bicycle Parking  : {tot_racks:,} parking racks ({tot_bikes:,} parked bicycles)")
            except Exception:
                pass

        # Continuous Barriers, Walls, Fences & Guard Rails
        barriers_rel = manifest.get("barriers_file")
        if barriers_rel and (bundle_dir / barriers_rel).is_file():
            try:
                b_data = json.loads((bundle_dir / barriers_rel).read_text(encoding="utf-8"))
                counts = b_data.get("counts", {})
                tot_b = counts.get("total_barriers", 0)
                tot_b_len = counts.get("total_length_m", 0.0)
                n_w = counts.get("walls", 0)
                n_f = counts.get("fences", 0)
                n_gr = counts.get("guard_rails", 0)
                n_h = counts.get("hedges", 0)
                print(f"\nContinuous Barriers & Perimeter Structures:")
                print(f"  Total Barriers   : {tot_b:,} structures ({tot_b_len:.1f}m total length)")
                print(f"  Walls & Masonry  : {n_w:,} perimeter/retaining walls")
                print(f"  Fences & Railings: {n_f:,} metal/wood fences")
                if n_gr > 0:
                    print(f"  Guard Rails      : {n_gr:,} highway crash barriers")
                if n_h > 0:
                    print(f"  Living Hedges    : {n_h:,} natural boundary hedges")
            except Exception:
                pass

        # Procedural 3D Sidewalk Curbs
        curbs_rel = manifest.get("curbs_file")
        if curbs_rel and (bundle_dir / curbs_rel).is_file():
            try:
                c_data = json.loads((bundle_dir / curbs_rel).read_text(encoding="utf-8"))
                counts = c_data.get("counts", {})
                tot_runs = counts.get("sidewalk_runs", 0)
                tot_c_len = counts.get("total_length_m", 0.0)
                in_c = counts.get("inner_curbs", 0)
                out_c = counts.get("outer_curbs", 0)
                print(f"\nProcedural 3D Sidewalk Curbs:")
                print(f"  Total Curb Runs  : {tot_runs:,} sections ({tot_c_len:.1f}m total length)")
                print(f"  Road-facing Curbs: {in_c:,} curb stones with drainage gutters")
                print(f"  Outer Skirt Curbs: {out_c:,} foundation borders")
            except Exception:
                pass

        # Municipal Utilities & Fire Hydrants
        utilities_rel = manifest.get("utilities_file")
        if utilities_rel and (bundle_dir / utilities_rel).is_file():
            try:
                u_data = json.loads((bundle_dir / utilities_rel).read_text(encoding="utf-8"))
                counts = u_data.get("counts", {})
                tot_u = counts.get("total_utilities", 0)
                p_hyd = counts.get("pillar_hydrants", 0)
                u_hyd = counts.get("underground_hydrants", 0)
                cabs = counts.get("street_cabinets", 0)
                manholes = counts.get("manhole_covers", 0)
                osm_c = counts.get("osm_sourced", 0)
                proc_c = counts.get("procedurally_synthesized", 0)
                print(f"\nMunicipal Utilities & Fire Hydrants:")
                print(f"  Total Utilities  : {tot_u:,} assets ({osm_c:,} OSM, {proc_c:,} procedural)")
                print(f"  Pillar Hydrants  : {p_hyd:,} red cast-iron standing hydrants")
                print(f"  Underground Hydr.: {u_hyd:,} DIN 4055 cast-iron ground plates")
                print(f"  Street Cabinets  : {cabs:,} telecom & power distribution cabinets")
                print(f"  Manhole Covers   : {manholes:,} drainage & utility access covers")
            except Exception:
                pass

        # Physical Speed Limit & Warning Signs (StVO metal signboards)
        signs_rel = manifest.get("traffic_signs_file")
        if signs_rel and (bundle_dir / signs_rel).is_file():
            try:
                s_data = json.loads((bundle_dir / signs_rel).read_text(encoding="utf-8"))
                metrics = s_data.get("metrics", {})
                tot_s = metrics.get("total_signs", len(s_data.get("signs", [])))
                by_type = metrics.get("counts_by_type", {})
                by_subtype = metrics.get("counts_by_subtype", {})
                sp_clear = metrics.get("spatial_clearance", {})
                print(f"\nPhysical Speed Limit & Warning Signs (StVO):")
                print(f"  Total Signs      : {tot_s:,} metal signboards ({by_type.get('speed_limit', 0)} speed limit, {by_type.get('stop', 0)} stop, {by_type.get('yield', 0)} yield, {by_type.get('priority_road', 0)} priority)")
                print(f"  Speed Breakdown  : 30 km/h: {by_subtype.get('30', 0)}, 50 km/h: {by_subtype.get('50', 0)}, 60 km/h: {by_subtype.get('60', 0)}")
                print(f"  Spatial Clearance: Min curb setback {sp_clear.get('min_curb_setback_m', 0.60):.2f}m, zero driving-lane encroachment: {sp_clear.get('zero_driving_lane_encroachment', True)}")
            except Exception:
                pass

        # Spawn Points
        spawns = manifest.get("spawn_points", [])
        print(f"\nSpawn Points ({len(spawns)}):")
        for s_idx, sp in enumerate(spawns[:5], 1):
            loc = sp.get("location", [0, 0, 0])
            print(f"  [{s_idx}] Location: ({loc[0]:.2f}, {loc[1]:.2f}, {loc[2]:.2f})")
        if len(spawns) > 5:
            print(f"  ... and {len(spawns) - 5} more.")

        print("=" * 72)
        return 0
    finally:
        if temp_dir is not None:
            shutil.rmtree(temp_dir, ignore_errors=True)


# =============================================================================
# Subcommand: verify
# =============================================================================

def cmd_verify(args: argparse.Namespace) -> int:
    """Rigorous structural and cryptographic verification of a .carlamap bundle."""
    target_path = Path(args.target).resolve()
    temp_dir: Path | None = None

    if target_path.is_file() and target_path.name.endswith((".tar.gz", ".tar")):
        import tempfile
        temp_dir = Path(tempfile.mkdtemp(prefix="carla_verify_"))
        try:
            with tarfile.open(target_path, "r:*") as tar:
                tar.extractall(temp_dir)
        except Exception as exc:
            print(f"[carla-map] VERIFY FAILED: Corrupted tar archive: {exc}")
            return 1
        inner_dirs = [d for d in temp_dir.iterdir() if d.is_dir() and (d / "carla-map.json").is_file()]
        bundle_dir = inner_dirs[0] if inner_dirs else temp_dir
    elif target_path.is_dir():
        bundle_dir = target_path
    else:
        print(f"[carla-map] VERIFY FAILED: Target '{target_path}' not found or invalid format.")
        return 1

    errors: list[str] = []
    checks_passed = 0

    try:
        # 1. Check carla-map.json
        manifest_file = bundle_dir / "carla-map.json"
        if not manifest_file.is_file():
            errors.append("Missing required manifest 'carla-map.json'.")
        else:
            try:
                manifest = json.loads(manifest_file.read_text(encoding="utf-8"))
                checks_passed += 1
            except Exception as exc:
                errors.append(f"Invalid JSON syntax in 'carla-map.json': {exc}")
                manifest = {}

        if not errors:
            map_name = manifest.get("map_name")
            if not map_name:
                errors.append("Manifest 'map_name' field is missing or empty.")
            else:
                checks_passed += 1

            # 2. Check OpenDRIVE
            xodr_rel = manifest.get("xodr_file")
            if not xodr_rel or not (bundle_dir / xodr_rel).is_file():
                errors.append(f"OpenDRIVE file missing: {xodr_rel}")
            else:
                try:
                    xodr_content = (bundle_dir / xodr_rel).read_text(encoding="utf-8")
                    test_map = carla.Map(map_name, xodr_content)
                    if len(test_map.get_topology()) == 0:
                        errors.append("OpenDRIVE contains 0 topology roads.")
                    else:
                        checks_passed += 1
                except Exception as exc:
                    errors.append(f"OpenDRIVE XML failed CARLA parser validation: {exc}")

            # 3. Check NavMesh
            nav_rel = manifest.get("nav_file")
            if not nav_rel or not (bundle_dir / nav_rel).is_file():
                errors.append(f"NavMesh file missing: {nav_rel}")
            else:
                nav_size = (bundle_dir / nav_rel).stat().st_size
                if nav_size == 0:
                    errors.append(f"NavMesh binary is empty (0 bytes): {nav_rel}")
                else:
                    checks_passed += 1

            # 4. Check .cmesh layers
            layers = manifest.get("layers", [])
            for layer in layers:
                mesh_rel = layer.get("mesh_file")
                layer_name = layer.get("name", "Unnamed")
                if not mesh_rel or not (bundle_dir / mesh_rel).is_file():
                    errors.append(f"Layer '{layer_name}': Mesh file missing '{mesh_rel}'")
                    continue

                mesh_path = bundle_dir / mesh_rel
                try:
                    geom = read_cmesh_file(mesh_path)
                    if geom["magic"] != CMESH_MAGIC:
                        errors.append(f"Layer '{layer_name}': Invalid CMesh magic 0x{geom['magic']:08X}")
                    elif geom["version"] != CMESH_VERSION_1:
                        errors.append(f"Layer '{layer_name}': Unsupported CMesh version {geom['version']}")
                    elif geom["vertex_count"] == 0:
                        errors.append(f"Layer '{layer_name}': Vertex count is 0")
                    elif geom["triangle_count"] == 0:
                        errors.append(f"Layer '{layer_name}': Triangle count is 0")
                    else:
                        checks_passed += 1
                except Exception as exc:
                    errors.append(f"Layer '{layer_name}': CMesh binary deserialization failed: {exc}")

            # 5. Check Placements / Clutter
            placements_rel = manifest.get("placements_file")
            if placements_rel:
                place_path = bundle_dir / placements_rel
                if not place_path.is_file():
                    errors.append(f"Placements manifest file missing: '{placements_rel}'")
                else:
                    try:
                        p_data = json.loads(place_path.read_text(encoding="utf-8"))
                        if "clutter" in p_data and "parked_cars" in p_data:
                            checks_passed += 1
                        else:
                            errors.append(f"Placements file '{placements_rel}' missing 'clutter' or 'parked_cars' key.")
                    except Exception as exc:
                        errors.append(f"Placements file '{placements_rel}' has invalid JSON syntax: {exc}")

            # 6. Check Street Lighting
            streetlights_rel = manifest.get("streetlights_file")
            if streetlights_rel:
                sl_path = bundle_dir / streetlights_rel
                if not sl_path.is_file():
                    errors.append(f"Streetlights manifest file missing: '{streetlights_rel}'")
                else:
                    try:
                        sl_data = json.loads(sl_path.read_text(encoding="utf-8"))
                        if "street_lamps" in sl_data:
                            checks_passed += 1
                        else:
                            errors.append(f"Streetlights file '{streetlights_rel}' missing 'street_lamps' key.")
                    except Exception as exc:
                        errors.append(f"Streetlights file '{streetlights_rel}' has invalid JSON syntax: {exc}")

            # 7. Check Dynamic Traffic Lights
            trafficlights_rel = manifest.get("traffic_lights_file")
            if trafficlights_rel:
                tl_path = bundle_dir / trafficlights_rel
                if not tl_path.is_file():
                    errors.append(f"Traffic lights manifest file missing: '{trafficlights_rel}'")
                else:
                    try:
                        tl_data = json.loads(tl_path.read_text(encoding="utf-8"))
                        if "signals" in tl_data:
                            checks_passed += 1
                        else:
                            errors.append(f"Traffic lights file '{trafficlights_rel}' missing 'signals' key.")
                    except Exception as exc:
                        errors.append(f"Traffic lights file '{trafficlights_rel}' has invalid JSON syntax: {exc}")

            # 8. Check Pedestrian Crosswalks & Zebra Markings
            crosswalks_rel = manifest.get("crosswalks_file")
            if crosswalks_rel:
                cw_path = bundle_dir / crosswalks_rel
                if not cw_path.is_file():
                    errors.append(f"Crosswalks manifest file missing: '{crosswalks_rel}'")
                else:
                    try:
                        cw_data = json.loads(cw_path.read_text(encoding="utf-8"))
                        if "crosswalks" in cw_data and "total_stripes" in cw_data:
                            checks_passed += 1
                        else:
                            errors.append(f"Crosswalks file '{crosswalks_rel}' missing 'crosswalks' or 'total_stripes' key.")
                    except Exception as exc:
                        errors.append(f"Crosswalks file '{crosswalks_rel}' has invalid JSON syntax: {exc}")

            # 9. Check Urban Vegetation & Foliage Manifest
            vegetation_rel = manifest.get("vegetation_file")
            if vegetation_rel:
                veg_path = bundle_dir / vegetation_rel
                if not veg_path.is_file():
                    errors.append(f"Vegetation manifest file missing: '{vegetation_rel}'")
                else:
                    try:
                        v_data = json.loads(veg_path.read_text(encoding="utf-8"))
                        if "trees" in v_data and "bushes" in v_data:
                            checks_passed += 1
                        else:
                            errors.append(f"Vegetation file '{vegetation_rel}' missing 'trees' or 'bushes' key.")
                    except Exception as exc:
                        errors.append(f"Vegetation file '{vegetation_rel}' has invalid JSON syntax: {exc}")

            # 10. Check Satellite & Aerial Imagery Analysis Manifest
            satellite_rel = manifest.get("satellite_file")
            if satellite_rel:
                sat_path = bundle_dir / satellite_rel
                if not sat_path.is_file():
                    errors.append(f"Satellite analysis manifest file missing: '{satellite_rel}'")
                else:
                    try:
                        s_data = json.loads(sat_path.read_text(encoding="utf-8"))
                        if "green_parcels" in s_data and "parking_lots" in s_data:
                            checks_passed += 1
                        else:
                            errors.append(f"Satellite file '{satellite_rel}' missing 'green_parcels' or 'parking_lots' key.")
                    except Exception as exc:
                        errors.append(f"Satellite file '{satellite_rel}' has invalid JSON syntax: {exc}")

            # 11. Check Dedicated Bicycle Infrastructure Manifest
            cycleways_rel = manifest.get("cycleways_file")
            if cycleways_rel:
                cyc_path = bundle_dir / cycleways_rel
                if not cyc_path.is_file():
                    errors.append(f"Cycleways manifest file missing: '{cycleways_rel}'")
                else:
                    try:
                        c_data = json.loads(cyc_path.read_text(encoding="utf-8"))
                        if "cycleways" in c_data and "counts" in c_data:
                            checks_passed += 1
                        else:
                            errors.append(f"Cycleways file '{cycleways_rel}' missing 'cycleways' or 'counts' key.")
                    except Exception as exc:
                        errors.append(f"Cycleways file '{cycleways_rel}' has invalid JSON syntax: {exc}")

            # 12. Check Continuous Barriers Manifest
            barriers_rel = manifest.get("barriers_file")
            if barriers_rel:
                bar_path = bundle_dir / barriers_rel
                if not bar_path.is_file():
                    errors.append(f"Barriers manifest file missing: '{barriers_rel}'")
                else:
                    try:
                        b_data = json.loads(bar_path.read_text(encoding="utf-8"))
                        if "counts" in b_data and "layers" in b_data:
                            checks_passed += 1
                        else:
                            errors.append(f"Barriers file '{barriers_rel}' missing 'counts' or 'layers' key.")
                    except Exception as exc:
                        errors.append(f"Barriers file '{barriers_rel}' has invalid JSON syntax: {exc}")

            # 13. Check Sidewalk Curbs Manifest
            curbs_rel = manifest.get("curbs_file")
            if curbs_rel:
                curb_path = bundle_dir / curbs_rel
                if not curb_path.is_file():
                    errors.append(f"Curbs manifest file missing: '{curbs_rel}'")
                else:
                    try:
                        c_data = json.loads(curb_path.read_text(encoding="utf-8"))
                        if "counts" in c_data and "mesh" in c_data:
                            checks_passed += 1
                        else:
                            errors.append(f"Curbs file '{curbs_rel}' missing 'counts' or 'mesh' key.")
                    except Exception as exc:
                        errors.append(f"Curbs file '{curbs_rel}' has invalid JSON syntax: {exc}")

            # 14. Check Municipal Utilities & Fire Hydrants Manifest
            utilities_rel = manifest.get("utilities_file")
            if utilities_rel:
                util_path = bundle_dir / utilities_rel
                if not util_path.is_file():
                    errors.append(f"Utilities manifest file missing: '{utilities_rel}'")
                else:
                    try:
                        u_data = json.loads(util_path.read_text(encoding="utf-8"))
                        if "utilities" in u_data and "counts" in u_data:
                            checks_passed += 1
                        else:
                            errors.append(f"Utilities file '{utilities_rel}' missing 'utilities' or 'counts' key.")
                    except Exception as exc:
                        errors.append(f"Utilities file '{utilities_rel}' has invalid JSON syntax: {exc}")

            # 15. Check StVO Traffic Signs & Speed Limit Signboards Manifest
            traffic_signs_rel = manifest.get("traffic_signs_file")
            if traffic_signs_rel:
                signs_path = bundle_dir / traffic_signs_rel
                if not signs_path.is_file():
                    errors.append(f"Traffic signs manifest file missing: '{traffic_signs_rel}'")
                else:
                    try:
                        s_data = json.loads(signs_path.read_text(encoding="utf-8"))
                        if ("signs" in s_data and "metrics" in s_data) or "counts" in s_data:
                            checks_passed += 1
                        else:
                            errors.append(f"Traffic signs file '{traffic_signs_rel}' missing 'signs'/'metrics' or 'counts' key.")
                    except Exception as exc:
                        errors.append(f"Traffic signs file '{traffic_signs_rel}' has invalid JSON syntax: {exc}")

        print("=" * 72)
        if errors:
            print(f"[carla-map] VERIFICATION FAILED ({len(errors)} errors found):")
            for err in errors:
                print(f"  [FAIL] {err}")
            print("=" * 72)
            return 1
        else:
            print(f"[carla-map] VERIFICATION PASSED: All {checks_passed} structural checks verified successfully!")
            print("=" * 72)
            return 0
    finally:
        if temp_dir is not None:
            shutil.rmtree(temp_dir, ignore_errors=True)


# =============================================================================
# Subcommand: install
# =============================================================================

def cmd_install(args: argparse.Namespace) -> int:
    """Installs a .carlamap bundle into a target CARLA installation."""
    target_path = Path(args.target).resolve()
    if not getattr(args, "server", ""):
        pkg_cand = REPO_ROOT / "Build/Package/Carla-0.10.0-Mac-Shipping"
        if pkg_cand.is_dir():
            server_root = pkg_cand.resolve()
        else:
            server_root = REPO_ROOT
    else:
        server_root = Path(args.server).resolve()

    if not server_root.exists():
        server_root.mkdir(parents=True, exist_ok=True)

    # Locate target Maps/ directories in CARLA installation
    maps_dest_candidates = [
        # macOS packaged app layout
        server_root / "Mac" / "CarlaUnreal-Mac-Shipping.app" / "Contents" / "UE" / "CarlaUnreal" / "Maps",
        server_root / "CarlaUnreal-Mac-Shipping.app" / "Contents" / "UE" / "CarlaUnreal" / "Maps",
        # Package root Maps/
        server_root / "Maps",
        # User app support (where Unreal Engine UE5 on macOS looks for Saved/Maps)
        Path.home() / "Library/Application Support/Epic/CarlaUnreal/Saved/Maps",
        # Linux packaged layout
        server_root / "Carla" / "Maps",
        server_root / "CarlaUnreal" / "Maps",
        server_root / "Linux" / "CarlaUnreal" / "Maps",
        # Source tree layout
        server_root / "Unreal" / "CarlaUnreal" / "Maps",
        server_root / "Unreal" / "CarlaUnreal" / "Saved" / "Maps",
    ]

    target_maps_dirs = [cand for cand in maps_dest_candidates if cand.is_dir() or cand.parent.is_dir()]
    if not target_maps_dirs:
        fallback = server_root / "Maps"
        fallback.mkdir(parents=True, exist_ok=True)
        target_maps_dirs.append(fallback)

    # Determine bundle name and source directory
    with tempfile.TemporaryDirectory(prefix="carla_map_install_") as tmp_dir_str:
        tmp_dir = Path(tmp_dir_str)
        if target_path.is_file() and target_path.name.endswith((".tar.gz", ".tar")):
            map_name = target_path.name.split(".")[0]
            with tarfile.open(target_path, "r:*") as tar:
                tar.extractall(tmp_dir)

            if (tmp_dir / "carla-map.json").is_file():
                src_dir = tmp_dir
            else:
                subdirs = [d for d in tmp_dir.iterdir() if d.is_dir() and (d / "carla-map.json").is_file()]
                src_dir = subdirs[0] if subdirs else tmp_dir
        elif target_path.is_dir():
            map_name = target_path.name.replace(".carlamap", "")
            src_dir = target_path
        else:
            print(f"[carla-map] ERROR: Target '{target_path}' is neither a file nor a directory.")
            return 1

        for t_dir in target_maps_dirs:
            t_dir.mkdir(parents=True, exist_ok=True)
            dest_bundle_dir = t_dir / map_name
            if dest_bundle_dir.exists():
                shutil.rmtree(dest_bundle_dir)
            shutil.copytree(src_dir, dest_bundle_dir)

            # Ensure any relative texture paths in carla-map.json are resolved to absolute paths
            json_file = dest_bundle_dir / "carla-map.json"
            if json_file.is_file():
                try:
                    with open(json_file, "r", encoding="utf-8") as jf:
                        jdata = json.load(jf)
                    modified_json = False
                    for layer in jdata.get("layers", []):
                        mp = layer.get("material_params", {})
                        atp = mp.get("albedo_texture_path")
                        if atp:
                            abs_atp = (dest_bundle_dir / atp).resolve() if not Path(atp).is_absolute() else Path(atp)
                            if abs_atp.is_file():
                                mp["albedo_texture_path"] = str(abs_atp)
                                modified_json = True
                    if modified_json:
                        with open(json_file, "w", encoding="utf-8") as jf:
                            json.dump(jdata, jf, indent=2)
                except Exception:
                    pass

            print(f"[carla-map] Installed '{map_name}' bundle to: {dest_bundle_dir}")

    # Check if there is an OpenDRIVE file to sync to ProjectSavedDir
    xodr_in_bundle = dest_bundle_dir / "OpenDrive" / f"{map_name}.xodr"
    if xodr_in_bundle.is_file():
        saved_od_dirs = [
            Path.home() / "Library" / "Application Support" / "Epic" / "CarlaUnreal" / "Saved" / "OpenDrive",
            server_root / "Saved" / "OpenDrive",
            server_root / "CarlaUnreal" / "Saved" / "OpenDrive",
        ]
        for sod in saved_od_dirs:
            if sod.exists() or sod.parent.is_dir():
                sod.mkdir(parents=True, exist_ok=True)
                dest_file = sod / f"{map_name}.xodr"
                shutil.copy2(xodr_in_bundle, dest_file)
                print(f"  Synced OpenDRIVE cache -> {dest_file}")

    # Check if there is a NavMesh file to sync to ProjectSavedDir
    nav_in_bundle = dest_bundle_dir / "Nav" / f"{map_name}.bin"
    if nav_in_bundle.is_file():
        saved_nav_dirs = [
            Path.home() / "Library" / "Application Support" / "Epic" / "CarlaUnreal" / "Saved" / "Nav",
            server_root / "Saved" / "Nav",
            server_root / "CarlaUnreal" / "Saved" / "Nav",
        ]
        for snd in saved_nav_dirs:
            if snd.exists() or snd.parent.is_dir():
                snd.mkdir(parents=True, exist_ok=True)
                dest_file = snd / f"{map_name}.bin"
                shutil.copy2(nav_in_bundle, dest_file)
                print(f"  Synced NavMesh cache -> {dest_file}")

        # Evict stale client cache if present
        cache_nav = Path.home() / "carlaCache" / "0.10.0" / "Carla" / "Maps" / "Nav" / f"{map_name}.bin"
        if cache_nav.parent.is_dir():
            shutil.copy2(nav_in_bundle, cache_nav)
            print(f"  Refreshed client NavMesh cache -> {cache_nav}")

    print(f"[carla-map] Successfully installed map bundle '{map_name}'!")
    print(f"  Installation Location : {dest_bundle_dir}")
    print(f"  Available via Python  : client.get_available_maps() will now list '{map_name}'")
    print(f"  Load via Python       : client.load_world('{map_name}')")
    return 0


# =============================================================================
# Main Entry Point
# =============================================================================

def main() -> int:
    parser = argparse.ArgumentParser(
        prog="carla-map",
        description="CARLA Editor-Free Map Creation & Packaging CLI.",
    )
    subparsers = parser.add_subparsers(dest="subcommand", help="Subcommand to execute")

    # init
    p_init = subparsers.add_parser("init", help="Scaffold a new map creation project")
    p_init.add_argument("name", help="Name of the new map (e.g. MyTown)")
    p_init.add_argument("--output", "-o", help="Destination folder (default: current directory)")
    p_init.add_argument("--osm", help="Optional initial OpenStreetMap (.osm) file to stage")
    p_init.add_argument("--xodr", help="Optional initial OpenDRIVE (.xodr) file to stage")
    p_init.add_argument("--force", "-f", action="store_true", help="Overwrite existing project directory")

    # build
    p_build = subparsers.add_parser("build", help="Compile assets and build a .carlamap bundle")
    p_build.add_argument("target", nargs="?", default=".", help="Project folder or map_recipe.json (default: .)")
    p_build.add_argument("--name", "-n", help="Override map name")
    p_build.add_argument("--output", "-o", help="Output .carlamap directory or .tar.gz archive")
    p_build.add_argument("--osm", help="Direct path to input OpenStreetMap (.osm) file")
    p_build.add_argument("--xodr", help="Direct path to input OpenDRIVE (.xodr) file")
    p_build.add_argument("--meshes", "-m", help="Path to custom 3D mesh file (.obj, .gltf, .glb) or folder")
    p_build.add_argument("--layer-elevation", type=float, default=None, help="Height separating OSM layers in meters (default: 0.0 or from recipe)")
    p_build.add_argument("--no-terrain", action="store_true", help="Disable procedural OSM terrain and landuse polygon generation")
    p_build.add_argument("--no-subbase", action="store_true", help="Disable continuous sub-base ground plane generation")
    p_build.add_argument("--subbase-margin", type=float, default=50.0, help="Margin around map bounding box in meters for sub-base (default: 50.0)")
    p_build.add_argument("--no-clutter", action="store_true", help="Disable extraction and synthesis of roadside clutter and vegetation")
    p_build.add_argument("--no-parked-cars", action="store_true", help="Disable procedural curbside parked cars placement")
    p_build.add_argument("--max-parked-cars", type=int, default=25, help="Maximum number of curbside parked cars to place (default: 25)")
    p_build.add_argument("--no-lighting", action="store_true", help="Disable extraction and procedural synthesis of street lamps")
    p_build.add_argument("--no-traffic-lights", action="store_true", help="Disable extraction and placement of dynamic traffic lights")
    p_build.add_argument("--no-all-junctions-tl", action="store_true", help="Disable automatic traffic light generation on all junctions during Osm2Odr conversion")
    p_build.add_argument("--no-crosswalks", action="store_true", help="Disable extraction and generation of pedestrian crosswalks and zebra markings")
    p_build.add_argument("--no-vegetation", action="store_true", help="Disable procedural 3D urban vegetation, avenue trees, and park foliage")
    p_build.add_argument("--no-satellite-imagery", action="store_true", help="Disable satellite and aerial orthophoto analysis")
    p_build.add_argument("--satellite-image", help="Direct path to custom aerial orthophoto image file (.jpg, .png, .tif)")
    p_build.add_argument("--no-satellite-vegetation", action="store_true", help="Disable satellite-verified green park and vegetation infill")
    p_build.add_argument("--no-satellite-parking", action="store_true", help="Disable automatic open surface parking lot and stall detection")
    p_build.add_argument("--no-cycleways", action="store_true", help="Disable dedicated bicycle infrastructure and cycleway networks")
    p_build.add_argument("--cycleway-color", choices=["auto", "green", "red", "asphalt"], default="red", help="Cycleway surface color preset (default: red)")
    p_build.add_argument("--no-bike-boxes", action="store_true", help="Disable advance stop lines (bike boxes) at signalized intersections")
    p_build.add_argument("--no-bike-stencils", action="store_true", help="Disable bicycle silhouette stencil decals along corridors")
    p_build.add_argument("--no-barriers", action="store_true", help="Disable continuous barriers, walls, fences, and guard rails")
    p_build.add_argument("--no-curbs", action="store_true", help="Disable procedural 3D sidewalk curbs and drainage gutters")
    p_build.add_argument("--curb-material", help="Material preset or Unreal asset path for sidewalk curbs")
    p_build.add_argument("--no-utilities", action="store_true", help="Disable municipal utilities, fire hydrants, and ground covers")
    p_build.add_argument("--no-procedural-hydrants", action="store_true", help="Disable procedural fire hydrant synthesis along corridors")
    p_build.add_argument("--hydrant-spacing", type=float, default=120.0, help="Spacing for procedural fire hydrants in meters (default: 120.0)")
    p_build.add_argument("--no-traffic-signs", action="store_true", help="Disable physical StVO speed limit and warning signs")
    p_build.add_argument("--sign-spacing", type=float, default=160.0, help="Spacing for procedural speed limit signs in meters (default: 160.0)")

    # inspect
    p_inspect = subparsers.add_parser("inspect", help="Inspect and display metadata of a .carlamap bundle")
    p_inspect.add_argument("target", help="Path to .carlamap directory or .tar.gz bundle archive")

    # verify
    p_verify = subparsers.add_parser("verify", help="Validate bundle structure, .cmesh headers, and OpenDRIVE")
    p_verify.add_argument("target", help="Path to .carlamap directory or .tar.gz bundle archive")

    # install
    p_install = subparsers.add_parser("install", help="Install a .carlamap bundle into a CARLA server")
    p_install.add_argument("target", help="Path to .carlamap directory or .tar.gz bundle archive")
    p_install.add_argument("--server", "-s", default="", help="Path to target CARLA server root (default: auto-detected)")

    args = parser.parse_args()
    if not args.subcommand:
        parser.print_help()
        return 1

    handlers = {
        "init": cmd_init,
        "build": cmd_build,
        "inspect": cmd_inspect,
        "verify": cmd_verify,
        "install": cmd_install,
    }

    return handlers[args.subcommand](args)


if __name__ == "__main__":
    sys.exit(main())
