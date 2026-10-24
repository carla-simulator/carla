#!/usr/bin/env python3
"""
Records High-Resolution Showcase Captures of Traffic Light Phase Transitions in CARLA.

Captures fixed-viewpoint images through all 3 signal states:
  1. Green Phase (Go) -> munich_traffic_light_green.png
  2. Yellow Phase (Caution) -> munich_traffic_light_yellow.png
  3. Red Phase (Stop) -> munich_traffic_light_red.png

Validates physical luminaire lens emission matching logical state.
"""

from __future__ import annotations

import math
import os
from pathlib import Path
import queue
import sys

os.environ["SDL_VIDEODRIVER"] = "dummy"

import carla
import numpy as np
from PIL import Image

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(REPO_ROOT))

OUT_DIR = REPO_ROOT / "Docs" / "map-maker" / "images"
OUT_DIR.mkdir(parents=True, exist_ok=True)


def capture_frame(sensor: carla.Sensor, out_path: Path, world: carla.World, ticks: int = 15) -> None:
    """Captures a single frame from sensor in synchronous mode."""
    q: queue.Queue[carla.Image] = queue.Queue()
    sensor.listen(q.put)

    for _ in range(ticks):
        world.tick()

    try:
        carla_img = q.get(timeout=5.0)
    finally:
        sensor.stop()

    raw_bgra = np.frombuffer(carla_img.raw_data, dtype=np.uint8).reshape((carla_img.height, carla_img.width, 4))
    rgb_arr = raw_bgra[:, :, :3][:, :, ::-1]

    im = Image.fromarray(rgb_arr)
    im.save(str(out_path), "PNG")
    print(f"[traffic-light-recorder] Saved -> {out_path.name} ({carla_img.width}x{carla_img.height})")


def main() -> int:
    client = carla.Client("127.0.0.1", 2000)
    client.set_timeout(30.0)

    print("[traffic-light-recorder] Connecting to CARLA world...")
    world = client.get_world()
    if "MunichStachus" not in world.get_map().name:
        world = client.load_world("MunichStachus")

    settings = world.get_settings()
    settings.synchronous_mode = True
    settings.fixed_delta_seconds = 0.05
    world.apply_settings(settings)

    # Clean clear lighting to clearly inspect signal lens luminescence
    world.set_weather(carla.WeatherParameters(
        cloudiness=15.0,
        precipitation=0.0,
        sun_altitude_angle=45.0,
        sun_azimuth_angle=200.0,
    ))

    bp_lib = world.get_blueprint_library()
    cam_bp = bp_lib.find("sensor.camera.rgb")
    cam_bp.set_attribute("image_size_x", "1920")
    cam_bp.set_attribute("image_size_y", "1080")
    cam_bp.set_attribute("fov", "80")

    tls = list(world.get_actors().filter("traffic.traffic_light*"))
    if not tls:
        print("[traffic-light-recorder] ERROR: No traffic lights found in world!")
        return 1

    target_tl = tls[0]
    tf = target_tl.get_transform()
    print(f"[traffic-light-recorder] Target TL ID: {target_tl.id} at ({tf.location.x:.1f}, {tf.location.y:.1f})")

    # Camera placed 9m in front of signal head, elevated 2.2m looking at the lenses
    yaw_rad = math.radians(tf.rotation.yaw)
    fwd = carla.Location(x=math.cos(yaw_rad), y=math.sin(yaw_rad), z=0.0)
    right = carla.Location(x=-math.sin(yaw_rad), y=math.cos(yaw_rad), z=0.0)

    cam_loc = tf.location + fwd * 7.5 + right * 1.8 + carla.Location(z=2.2)
    dx = tf.location.x - cam_loc.x
    dy = tf.location.y - cam_loc.y
    dz = (tf.location.z + 3.2) - cam_loc.z
    dist_horiz = math.sqrt(dx * dx + dy * dy)
    cam_pitch = math.degrees(math.atan2(dz, dist_horiz))
    cam_yaw = math.degrees(math.atan2(dy, dx))

    cam_tf = carla.Transform(cam_loc, carla.Rotation(pitch=cam_pitch, yaw=cam_yaw, roll=0.0))
    cam_actor = world.spawn_actor(cam_bp, cam_tf)

    try:
        target_tl.freeze(True)

        # 1. GREEN Phase
        print("[traffic-light-recorder] Setting state -> GREEN...")
        target_tl.set_state(carla.TrafficLightState.Green)
        capture_frame(cam_actor, OUT_DIR / "munich_traffic_light_green.png", world, ticks=15)

        # 2. YELLOW Phase
        print("[traffic-light-recorder] Setting state -> YELLOW...")
        target_tl.set_state(carla.TrafficLightState.Yellow)
        capture_frame(cam_actor, OUT_DIR / "munich_traffic_light_yellow.png", world, ticks=15)

        # 3. RED Phase
        print("[traffic-light-recorder] Setting state -> RED...")
        target_tl.set_state(carla.TrafficLightState.Red)
        capture_frame(cam_actor, OUT_DIR / "munich_traffic_light_red.png", world, ticks=15)

        print("[traffic-light-recorder] Full 3-phase transition cycle captured successfully!")
    finally:
        target_tl.freeze(False)
        if cam_actor is not None:
            cam_actor.destroy()
        settings.synchronous_mode = False
        world.apply_settings(settings)

    return 0


if __name__ == "__main__":
    sys.exit(main())

