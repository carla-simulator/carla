#!/usr/bin/env python3
"""
Renders a 4-phase Day/Night & Weather Sequence of Munich Stachus in CARLA.

Captures fixed-viewpoint high-resolution showcase images across:
  1. High Noon (ClearNoon, sun_altitude=70.0)
  2. Sunset / Golden Hour (ClearSunset, sun_altitude=12.0)
  3. Clear Night (ClearNight, sun_altitude=-90.0)
  4. Wet Midnight (WetNight, sun_altitude=-90.0, wetness=100%, specular reflections)

Showcases real OSM street lamps, curbside parked cars, urban clutter, and vehicle headlights.
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
from osm_lighting import (
    extract_osm_street_lamps,
    spawn_street_lamps_in_world,
    synthesize_procedural_street_lamps,
)

OUT_DIR = REPO_ROOT / "Docs" / "map-maker" / "images"
OUT_DIR.mkdir(parents=True, exist_ok=True)


def capture_sensor_frame(
    sensor: carla.Sensor,
    out_path: Path,
    world: carla.World,
    ticks: int = 15,
) -> None:
    """Captures a single frame from sensor in synchronous mode and writes to disk."""
    img_queue: queue.Queue[carla.Image] = queue.Queue()
    sensor.listen(img_queue.put)

    for _ in range(ticks):
        world.tick()

    try:
        carla_img = img_queue.get(timeout=8.0)
    finally:
        sensor.stop()

    raw_bgra = np.frombuffer(carla_img.raw_data, dtype=np.uint8).reshape((carla_img.height, carla_img.width, 4))
    rgb_arr = raw_bgra[:, :, :3][:, :, ::-1]  # BGRA to RGB

    im = Image.fromarray(rgb_arr)
    im.save(str(out_path), "PNG")
    print(f"[day-night-renderer] Saved showcase image -> {out_path.name} ({carla_img.width}x{carla_img.height})")


def main() -> int:
    client = carla.Client("127.0.0.1", 2000)
    client.set_timeout(40.0)

    print("[day-night-renderer] Loading MunichStachus into CARLA...")
    world = client.load_world("MunichStachus")

    settings = world.get_settings()
    settings.synchronous_mode = True
    settings.fixed_delta_seconds = 0.05
    world.apply_settings(settings)

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
    print("[day-night-renderer] Placing urban clutter...")
    osm_clutter = extract_osm_clutter(
        osm_path, project_coord, carla_map=carla_map, building_polygons=building_polys
    ) if osm_path.is_file() else []

    synth_clutter = synthesize_procedural_sidewalk_clutter(
        carla_map, osm_clutter, building_polygons=building_polys
    )
    all_clutter = osm_clutter + synth_clutter

    # 2. Curbside parked cars
    print("[day-night-renderer] Placing curbside parked cars...")
    parked_cars = generate_curbside_parked_cars(carla_map, max_cars=25)

    # 3. Street lamps (OSM authentic + procedural)
    print("[day-night-renderer] Extracting and synthesizing street lamps...")
    osm_lamps = extract_osm_street_lamps(
        osm_path, project_coord, carla_map=carla_map, building_polygons=building_polys
    ) if osm_path.is_file() else []

    synth_lamps = synthesize_procedural_street_lamps(
        carla_map,
        existing_lamps=osm_lamps,
        building_polygons=building_polys,
        spacing=20.0,
        min_dist_existing=13.0,
    )
    all_lamps = osm_lamps + synth_lamps
    print(f"[day-night-renderer] Total street lamps: {len(all_lamps)} ({len(osm_lamps)} OSM, {len(synth_lamps)} procedural).")

    # 4. Spawn actors into simulator
    spawn_clutter_res = spawn_clutter_in_world(world, all_clutter, parked_cars)
    spawned_props = spawn_clutter_res["props"]
    spawned_parked = spawn_clutter_res["parked_cars"]

    spawned_lamps = spawn_street_lamps_in_world(world, all_lamps)
    print(f"[day-night-renderer] Spawned {len(spawned_lamps)} street lamp poles, {len(spawned_props)} clutter props, {len(spawned_parked)} parked cars.")

    # 5. Spawn moving traffic
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

    print(f"[day-night-renderer] Spawned {len(moving_vehs)} moving autopilot vehicles.")

    # Camera Blueprint
    cam_bp = bp_lib.find("sensor.camera.rgb")
    cam_bp.set_attribute("image_size_x", "1920")
    cam_bp.set_attribute("image_size_y", "1080")
    cam_bp.set_attribute("fov", "90")

    # Camera framing overlooking the street corridor
    cam_actor: carla.Actor | None = None

    try:
        # Advance simulation 30 ticks to let traffic populate the road
        for _ in range(30):
            world.tick()

        # Find parked car for corridor framing
        if spawned_parked:
            target_car = spawned_parked[0]
            c_tf = target_car.get_transform()
            fwd = c_tf.get_forward_vector()
            right = c_tf.get_right_vector()

            # Elevated Streetscape framing overlooking the avenue with street lamps and parked cars
            cam_loc = c_tf.location - fwd * 14.0 + right * 5.0 + carla.Location(z=4.5)
            cam_rot = carla.Rotation(pitch=-14.0, yaw=c_tf.rotation.yaw + 15.0, roll=0.0)
            cam_actor = world.spawn_actor(cam_bp, carla.Transform(cam_loc, cam_rot))
        else:
            # Fallback transform near Stachus center
            cam_loc = carla.Location(x=-30.0, y=-50.0, z=14.0)
            cam_rot = carla.Rotation(pitch=-20.0, yaw=40.0, roll=0.0)
            cam_actor = world.spawn_actor(cam_bp, carla.Transform(cam_loc, cam_rot))

        # Weather definitions for the 4-phase sequence
        scenarios = [
            (
                "munich_day_noon.png",
                "High Noon (ClearNoon)",
                carla.WeatherParameters(
                    cloudiness=15.0,
                    precipitation=0.0,
                    precipitation_deposits=0.0,
                    wind_intensity=10.0,
                    sun_altitude_angle=70.0,
                    sun_azimuth_angle=200.0,
                    fog_density=0.0,
                    wetness=0.0,
                ),
                False,
            ),
            (
                "munich_day_sunset.png",
                "Sunset / Golden Hour (ClearSunset)",
                carla.WeatherParameters(
                    cloudiness=25.0,
                    precipitation=0.0,
                    precipitation_deposits=0.0,
                    wind_intensity=15.0,
                    sun_altitude_angle=12.0,
                    sun_azimuth_angle=275.0,
                    fog_density=2.0,
                    wetness=0.0,
                ),
                False,
            ),
            (
                "munich_night_clear.png",
                "Clear Night (ClearNight)",
                carla.WeatherParameters(
                    cloudiness=10.0,
                    precipitation=0.0,
                    precipitation_deposits=0.0,
                    wind_intensity=5.0,
                    sun_altitude_angle=-90.0,
                    sun_azimuth_angle=0.0,
                    fog_density=0.0,
                    wetness=0.0,
                ),
                True,
            ),
            (
                "munich_night_wet.png",
                "Wet Midnight (WetNight)",
                carla.WeatherParameters(
                    cloudiness=80.0,
                    precipitation=60.0,
                    precipitation_deposits=100.0,
                    wind_intensity=25.0,
                    sun_altitude_angle=-90.0,
                    sun_azimuth_angle=0.0,
                    fog_density=5.0,
                    wetness=100.0,
                ),
                True,
            ),
        ]

        for filename, label, weather_param, is_night in scenarios:
            print(f"[day-night-renderer] Setting weather: {label}...")
            world.set_weather(weather_param)

            # Turn on vehicle lights at night
            light_state = (
                carla.VehicleLightState(carla.VehicleLightState.Position | carla.VehicleLightState.LowBeam)
                if is_night
                else carla.VehicleLightState(carla.VehicleLightState.NONE)
            )
            for v in moving_vehs:
                try:
                    v.set_light_state(light_state)
                except Exception:
                    pass
            for p in spawned_parked:
                try:
                    p.set_light_state(light_state)
                except Exception:
                    pass

            capture_sensor_frame(cam_actor, OUT_DIR / filename, world, ticks=20)

    finally:
        if cam_actor is not None:
            try:
                cam_actor.destroy()
            except Exception:
                pass

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
        for sl in spawned_lamps:
            try:
                sl.destroy()
            except Exception:
                pass

        settings.synchronous_mode = False
        world.apply_settings(settings)
        print("[day-night-renderer] All day/night sequence captures completed successfully!")

    return 0


if __name__ == "__main__":
    sys.exit(main())

