#!/usr/bin/env python3
"""
Renders high-resolution street corridor showcase images of Munich in CARLA
populated with real OSM vegetation, sidewalk furniture, and curbside parked cars.
"""

from __future__ import annotations

import os
from pathlib import Path
import queue
import random
import sys
import xml.etree.ElementTree as ET

os.environ["SDL_VIDEODRIVER"] = "dummy"

import carla
import numpy as np
from PIL import Image

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
LOCAL_TMP_DIR = REPO_ROOT / ".tmp"
LOCAL_TMP_DIR.mkdir(parents=True, exist_ok=True)
os.environ["TMPDIR"] = str(LOCAL_TMP_DIR)

sys.path.insert(0, str(REPO_ROOT))
sys.path.insert(0, str(REPO_ROOT / "Util" / "MapBuilder"))

from osm_buildings import extract_osm_building_footprints
from osm_clutter import (
    extract_osm_clutter,
    generate_curbside_parked_cars,
    spawn_clutter_in_world,
    synthesize_procedural_sidewalk_clutter,
)

OUT_DIR = REPO_ROOT / "Docs" / "map-maker" / "images"
OUT_DIR.mkdir(parents=True, exist_ok=True)


def capture_sensor_frame(sensor: carla.Sensor, out_path: Path, world: carla.World, ticks: int = 15) -> None:
    """Captures a single frame from sensor in synchronous mode and writes to disk."""
    img_queue: queue.Queue[carla.Image] = queue.Queue()
    sensor.listen(img_queue.put)

    for _ in range(ticks):
        world.tick()

    try:
        carla_img = img_queue.get(timeout=5.0)
    finally:
        sensor.stop()
        sensor.destroy()

    raw_bgra = np.frombuffer(carla_img.raw_data, dtype=np.uint8).reshape((carla_img.height, carla_img.width, 4))
    rgb_arr = raw_bgra[:, :, :3][:, :, ::-1]  # BGRA to RGB

    im = Image.fromarray(rgb_arr)
    im.save(str(out_path), "PNG")
    print(f"[corridor-renderer] Saved showcase image -> {out_path} ({carla_img.width}x{carla_img.height})")


def main() -> int:
    client = carla.Client("127.0.0.1", 2000)
    client.set_timeout(40.0)

    print("[corridor-renderer] Loading MunichStachus into CARLA...")
    world = client.load_world("MunichStachus")

    settings = world.get_settings()
    settings.synchronous_mode = True
    settings.fixed_delta_seconds = 0.05
    world.apply_settings(settings)

    # Clean sunny weather with realistic directional shadows
    weather = carla.WeatherParameters(
        cloudiness=15.0,
        precipitation=0.0,
        sun_altitude_angle=65.0,
        sun_azimuth_angle=210.0,
        fog_density=0.0,
    )
    world.set_weather(weather)

    carla_map = world.get_map()
    bp_lib = world.get_blueprint_library()

    # Load xodr offset for OSM coordinate projection
    bundle_dir = REPO_ROOT / "Docs" / "map-maker" / "output" / "MunichStachus.carlamap"
    if not bundle_dir.is_dir():
        bundle_dir = REPO_ROOT / "MunichStachus.carlamap"
    xodr_path = bundle_dir / "OpenDrive" / "MunichStachus.xodr"
    off_x, off_y = 0.0, 0.0
    if xodr_path.is_file():
        tree = ET.parse(str(xodr_path))
        header = tree.getroot().find("header")
        if header is not None:
            off_elem = header.find("offset")
            if off_elem is not None:
                off_x = float(off_elem.get("x", "0.0"))
                off_y = float(off_elem.get("y", "0.0"))

    def project_coord(lat: float, lon: float) -> tuple[float, float, float]:
        loc = carla_map.geolocation_to_transform(carla.GeoLocation(lat, lon, 0.0))
        return loc.x + off_x, -(loc.y + off_y), loc.z

    osm_path = REPO_ROOT / "Docs" / "map-maker" / "output" / "munich_stachus.osm"
    if not osm_path.is_file():
        osm_path = REPO_ROOT / "munich_stachus.osm"
    building_polys: list[list[tuple[float, float]]] = []
    if osm_path.is_file():
        footprints = extract_osm_building_footprints(osm_path)
        for fp in footprints:
            poly = [(project_coord(lat, lon)[0], project_coord(lat, lon)[1]) for lat, lon in fp.nodes]
            if len(poly) >= 3:
                building_polys.append(poly)

    # 1. Extract and synthesize clutter
    print("[corridor-renderer] Extracting and placing urban clutter...")
    osm_items = extract_osm_clutter(
        osm_path, project_coord, carla_map=carla_map, building_polygons=building_polys
    ) if osm_path.is_file() else []

    synth_items = synthesize_procedural_sidewalk_clutter(
        carla_map, osm_items, building_polygons=building_polys
    )
    all_clutter = osm_items + synth_items
    print(f"[corridor-renderer] Placed {len(all_clutter)} clutter items ({len(osm_items)} OSM, {len(synth_items)} procedural).")

    # 2. Generate curbside parked cars
    print("[corridor-renderer] Generating curbside parked cars...")
    parked_cars = generate_curbside_parked_cars(carla_map, max_cars=25)
    print(f"[corridor-renderer] Placed {len(parked_cars)} curbside parked cars.")

    # 3. Spawn clutter props and parked vehicles in world
    spawn_res = spawn_clutter_in_world(world, all_clutter, parked_cars)
    spawned_props = spawn_res["props"]
    spawned_parked = spawn_res["parked_cars"]
    print(f"[corridor-renderer] Successfully instantiated {len(spawned_props)} props and {len(spawned_parked)} parked cars.")

    # 4. Spawn moving traffic for background life
    spawn_points = carla_map.get_spawn_points()
    veh_bps = [bp for bp in bp_lib.filter("vehicle.*") if int(bp.get_attribute("number_of_wheels")) == 4]
    tm = client.get_trafficmanager(8000)
    tm.set_synchronous_mode(True)

    parked_locs = [p.get_location() for p in spawned_parked]
    traffic_spawns = [sp for sp in spawn_points if all(sp.location.distance(pl) > 10.0 for pl in parked_locs)]

    moving_vehs: list[carla.Actor] = []
    random.seed(1337)
    for sp in random.sample(traffic_spawns, min(15, len(traffic_spawns))):
        bp = random.choice(veh_bps)
        veh = world.try_spawn_actor(bp, sp)
        if veh:
            veh.set_autopilot(True, 8000)
            moving_vehs.append(veh)

    print(f"[corridor-renderer] Spawned {len(moving_vehs)} moving autopilot vehicles.")

    # Camera Blueprint
    cam_bp = bp_lib.find("sensor.camera.rgb")
    cam_bp.set_attribute("image_size_x", "1920")
    cam_bp.set_attribute("image_size_y", "1080")
    cam_bp.set_attribute("fov", "90")

    try:
        # Advance simulation 40 ticks to let moving traffic circulate
        for _ in range(40):
            world.tick()

        # Find parked car for corridor framing
        if spawned_parked:
            target_car = spawned_parked[0]
            c_tf = target_car.get_transform()
            fwd = c_tf.get_forward_vector()
            right = c_tf.get_right_vector()

            # View 1: Curbside Parked Cars Avenue (Eye Level looking down street)
            driver_cam_loc = c_tf.location - fwd * 7.5 + carla.Location(z=1.3) - right * 1.2
            driver_cam_rot = carla.Rotation(pitch=-3.0, yaw=c_tf.rotation.yaw + 8.0, roll=0.0)
            cam1 = world.spawn_actor(cam_bp, carla.Transform(driver_cam_loc, driver_cam_rot))
            capture_sensor_frame(cam1, OUT_DIR / "munich_corridor_parked_cars.png", world, ticks=15)

            # View 2: Pedestrian Sidewalk Eye-Level (1.7m) with trees, benches, and street life
            ped_cam_loc = c_tf.location - fwd * 9.0 + right * 2.2 + carla.Location(z=1.7)
            ped_cam_rot = carla.Rotation(pitch=-4.0, yaw=c_tf.rotation.yaw + 12.0, roll=0.0)
            cam2 = world.spawn_actor(cam_bp, carla.Transform(ped_cam_loc, ped_cam_rot))
            capture_sensor_frame(cam2, OUT_DIR / "munich_corridor_pedestrian.png", world, ticks=15)

            # View 3: Elevated Streetscape Framing
            elev_cam_loc = c_tf.location - fwd * 14.0 + right * 5.0 + carla.Location(z=4.5)
            elev_cam_rot = carla.Rotation(pitch=-14.0, yaw=c_tf.rotation.yaw + 15.0, roll=0.0)
            cam3 = world.spawn_actor(cam_bp, carla.Transform(elev_cam_loc, elev_cam_rot))
            capture_sensor_frame(cam3, OUT_DIR / "munich_corridor_street.png", world, ticks=15)

    finally:
        for v in moving_vehs:
            try:
                v.destroy()
            except Exception:
                pass
        for p in spawned_parked:
            try:
                p.destroy()
            except Exception:
                pass
        for pr in spawned_props:
            try:
                pr.destroy()
            except Exception:
                pass

        settings.synchronous_mode = False
        world.apply_settings(settings)
        print("[corridor-renderer] All corridor showcases rendered successfully!")

    return 0


if __name__ == "__main__":
    sys.exit(main())

