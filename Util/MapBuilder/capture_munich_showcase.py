#!/usr/bin/env python3
"""
Captures high-resolution RGB and Semantic Segmentation showcase screenshots of Munich in CARLA.
"""

from __future__ import annotations

import os
from pathlib import Path
import queue
import random
import sys
import time

os.environ["SDL_VIDEODRIVER"] = "dummy"

import carla
import numpy as np
from PIL import Image

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
OUT_DIR = REPO_ROOT / "Docs" / "map-maker" / "images"
OUT_DIR.mkdir(parents=True, exist_ok=True)


def capture_sensor_image(sensor: carla.Sensor, out_path: Path, world: carla.World, ticks: int = 25, is_semantic: bool = False) -> None:
    """Captures a frame from a sensor in synchronous mode and saves to disk."""
    image_queue: queue.Queue[carla.Image] = queue.Queue()
    sensor.listen(image_queue.put)

    for _ in range(ticks):
        world.tick()

    try:
        carla_img = image_queue.get(timeout=5.0)
    finally:
        sensor.stop()
        sensor.destroy()

    if is_semantic:
        carla_img.convert(carla.ColorConverter.CityScapesPalette)

    raw_bgra = np.frombuffer(carla_img.raw_data, dtype=np.uint8).reshape((carla_img.height, carla_img.width, 4))
    rgb_arr = raw_bgra[:, :, :3][:, :, ::-1]  # BGRA to RGB

    im = Image.fromarray(rgb_arr)
    im.save(str(out_path), "PNG")
    print(f"[capture] Saved screenshot -> {out_path} ({carla_img.width}x{carla_img.height})")


def main() -> int:
    client = carla.Client("127.0.0.1", 2000)
    client.set_timeout(40.0)

    print("[capture] Loading MunichStachus into CARLA...")
    world = client.load_world("MunichStachus")

    settings = world.get_settings()
    settings.synchronous_mode = True
    settings.fixed_delta_seconds = 0.05
    world.apply_settings(settings)

    # Soft natural overcast daylight (eliminating harsh midday glare)
    weather = carla.WeatherParameters(
        cloudiness=50.0,
        precipitation=0.0,
        precipitation_deposits=0.0,
        wind_intensity=10.0,
        sun_azimuth_angle=180.0,
        sun_altitude_angle=32.0,
        fog_density=0.5,
        fog_distance=50.0,
        fog_falloff=0.2,
        wetness=5.0,
        scattering_intensity=0.8,
    )
    world.set_weather(weather)

    bp_lib = world.get_blueprint_library()
    cam_rgb_bp = bp_lib.find("sensor.camera.rgb")
    cam_rgb_bp.set_attribute("image_size_x", "1920")
    cam_rgb_bp.set_attribute("image_size_y", "1080")
    cam_rgb_bp.set_attribute("fov", "90")

    cam_sem_bp = bp_lib.find("sensor.camera.semantic_segmentation")
    cam_sem_bp.set_attribute("image_size_x", "1920")
    cam_sem_bp.set_attribute("image_size_y", "1080")
    cam_sem_bp.set_attribute("fov", "90")

    # 1. Aerial Overhead View (High altitude looking down on Stachus plaza & roads)
    aerial_tf = carla.Transform(
        carla.Location(x=300.0, y=-280.0, z=380.0),
        carla.Rotation(pitch=-75.0, yaw=90.0, roll=0.0),
    )
    cam_aerial = world.spawn_actor(cam_rgb_bp, aerial_tf)
    capture_sensor_image(cam_aerial, OUT_DIR / "munich_carla_aerial.png", world, ticks=20)

    # 2. Semantic Segmentation Overhead View
    cam_sem_aerial = world.spawn_actor(cam_sem_bp, aerial_tf)
    capture_sensor_image(cam_sem_aerial, OUT_DIR / "munich_carla_semantic.png", world, ticks=10, is_semantic=True)

    # Spawn traffic for dynamic views
    spawn_points = world.get_map().get_spawn_points()
    veh_bps = bp_lib.filter("vehicle.*")
    spawned_vehs: list[carla.Actor] = []

    tm = client.get_trafficmanager(8000)
    tm.set_synchronous_mode(True)

    # Pick a high-detail sports coupe for Ego (Audi TT)
    ego_bp = bp_lib.find("vehicle.ue4.audi.tt")
    ego_sp = spawn_points[102]
    ego = world.try_spawn_actor(ego_bp, ego_sp)
    if ego:
        ego.set_autopilot(True, 8000)
        spawned_vehs.append(ego)
        print(f"[capture] Spawned Ego Audi TT at ({ego_sp.location.x:.1f}, {ego_sp.location.y:.1f})")

    # Spawn surrounding 4-wheel traffic
    car_bps = [bp for bp in veh_bps if int(bp.get_attribute("number_of_wheels")) == 4]
    random.seed(42)
    sample_spawns = random.sample(spawn_points, min(35, len(spawn_points)))
    for sp in sample_spawns:
        if ego and sp.location.distance(ego_sp.location) < 12.0:
            continue
        bp = random.choice(car_bps)
        veh = world.try_spawn_actor(bp, sp)
        if veh:
            veh.set_autopilot(True, 8000)
            spawned_vehs.append(veh)

    print(f"[capture] Spawned {len(spawned_vehs)} total vehicles with Traffic Manager.")

    try:
        # Advance simulation 45 ticks to let traffic move naturally
        for _ in range(45):
            world.tick()

        if ego:
            # 3. Third-Person Chase View
            chase_tf = carla.Transform(
                carla.Location(x=-5.5, y=0.0, z=2.4),
                carla.Rotation(pitch=-12.0, yaw=0.0, roll=0.0),
            )
            cam_chase = world.spawn_actor(cam_rgb_bp, chase_tf, attach_to=ego)
            capture_sensor_image(cam_chase, OUT_DIR / "munich_carla_traffic.png", world, ticks=15)

            # 4. Driver Cockpit / Hood View
            driver_tf = carla.Transform(
                carla.Location(x=1.6, y=0.0, z=1.0),
                carla.Rotation(pitch=-4.0, yaw=0.0, roll=0.0),
            )
            cam_driver = world.spawn_actor(cam_rgb_bp, driver_tf, attach_to=ego)
            capture_sensor_image(cam_driver, OUT_DIR / "munich_carla_driver.png", world, ticks=15)

            # 5. Street / Sidewalk Perspective framing ego & street
            ego_tf = ego.get_transform()
            yaw_rad = np.radians(ego_tf.rotation.yaw)
            # Sidewalk offset to the right/front of ego
            side_x = -np.sin(yaw_rad) * 9.0 + np.cos(yaw_rad) * 16.0
            side_y = np.cos(yaw_rad) * 9.0 + np.sin(yaw_rad) * 16.0
            street_loc = ego_tf.location + carla.Location(x=side_x, y=side_y, z=1.8)
            street_rot = carla.Rotation(pitch=-6.0, yaw=ego_tf.rotation.yaw - 150.0, roll=0.0)
            street_tf = carla.Transform(street_loc, street_rot)
            cam_street = world.spawn_actor(cam_rgb_bp, street_tf)
            capture_sensor_image(cam_street, OUT_DIR / "munich_carla_street.png", world, ticks=15)

    finally:
        for v in spawned_vehs:
            v.destroy()

        # Restore asynchronous mode
        settings.synchronous_mode = False
        world.apply_settings(settings)
        print("[capture] All captures completed successfully!")

    return 0


if __name__ == "__main__":
    sys.exit(main())
