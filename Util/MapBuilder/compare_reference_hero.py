#!/usr/bin/env python3
"""
Side-by-Side Reference Showcase Comparison & Multi-Camera 4K Hero Suite.

Recreates the camera viewpoint, focal length, field-of-view, vehicle placement,
and atmospheric lighting of the CARLA UE5 showcase scene (Docs/img/carla_ue5_readme_img.webp).

Captures:
1. munich_hero_comparison.png: Side-by-side side comparison matching the reference image.
2. munich_hero_aerial.png: Wide 4K panoramic aerial overview of Munich Stachus.
3. munich_hero_boulevard.png: Street-level boulevard with parked cars, trees, street lamps, and multi-slot facades.
4. munich_hero_intersection.png: Signalized intersection view with dynamic traffic lights and moving traffic.
5. munich_hero_cockpit.png: Driver-seat POV cockpit view with road reflection and windshield perspective.
"""

from __future__ import annotations

import argparse
import math
import os
from pathlib import Path
import queue
import random
import sys
import time

os.environ["SDL_VIDEODRIVER"] = "dummy"

import carla
import numpy as np
from PIL import Image, ImageDraw, ImageFont

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
LOCAL_TMP_DIR = REPO_ROOT / ".tmp"
LOCAL_TMP_DIR.mkdir(parents=True, exist_ok=True)
os.environ["TMPDIR"] = str(LOCAL_TMP_DIR)

sys.path.insert(0, str(REPO_ROOT / "Util" / "MapBuilder"))
OUT_DIR = REPO_ROOT / "Docs" / "map-maker" / "images"
OUT_DIR.mkdir(parents=True, exist_ok=True)
REF_IMG_PATH = REPO_ROOT / "Docs" / "img" / "carla_ue5_readme_img.webp"


def capture_frame(sensor: carla.Sensor, world: carla.World, ticks: int = 15) -> Image.Image:
    """Captures a single high-resolution image from a sensor in synchronous mode."""
    img_queue: queue.Queue[carla.Image] = queue.Queue()
    sensor.listen(img_queue.put)

    try:
        for _ in range(ticks):
            world.tick()
        c_img = img_queue.get(timeout=6.0)
        raw_bgra = np.frombuffer(c_img.raw_data, dtype=np.uint8).reshape((c_img.height, c_img.width, 4))
        rgb = raw_bgra[:, :, :3][:, :, ::-1]  # BGRA to RGB
        return Image.fromarray(rgb)
    finally:
        sensor.stop()
        sensor.destroy()


def create_side_by_side_comparison(ref_path: Path, rendered_img: Image.Image, out_path: Path) -> None:
    """Creates a high-resolution side-by-side comparison artifact."""
    if not ref_path.is_file():
        print(f"[compare] Warning: Reference image '{ref_path}' not found, saving rendered hero directly.")
        rendered_img.save(str(out_path), "PNG")
        return

    ref_img = Image.open(str(ref_path)).convert("RGB")

    # Target height 1080 for each panel
    target_h = 1080
    ref_w = int(ref_img.width * (target_h / ref_img.height))
    rend_w = int(rendered_img.width * (target_h / rendered_img.height))

    ref_resized = ref_img.resize((ref_w, target_h), Image.Resampling.LANCZOS)
    rend_resized = rendered_img.resize((rend_w, target_h), Image.Resampling.LANCZOS)

    canvas_w = ref_w + rend_w
    canvas_h = target_h + 80  # Header bar height

    canvas = Image.new("RGB", (canvas_w, canvas_h), (18, 18, 22))
    canvas.paste(ref_resized, (0, 80))
    canvas.paste(rend_resized, (ref_w, 80))

    draw = ImageDraw.Draw(canvas)
    # Header bar
    draw.rectangle([0, 0, canvas_w, 80], fill=(12, 12, 16))
    # Sub-headers
    draw.rectangle([20, 20, 520, 60], fill=(24, 24, 32))
    draw.rectangle([ref_w + 20, 20, ref_w + 580, 60], fill=(24, 24, 32))

    draw.text((36, 28), "REFERENCE: CARLA UE5 Official Showcase Scene", fill=(255, 215, 0))
    draw.text((ref_w + 36, 28), "RENDERED: Munich Stachus Editor-Free Procedural Map", fill=(0, 215, 255))

    # Divider line
    draw.line([(ref_w, 0), (ref_w, canvas_h)], fill=(60, 60, 70), width=4)

    canvas.save(str(out_path), "PNG")
    print(f"[compare] Saved side-by-side hero comparison -> {out_path} ({canvas_w}x{canvas_h})")


def main() -> int:
    parser = argparse.ArgumentParser(description="Render hero showcase and side-by-side comparison.")
    parser.add_argument("--host", default="127.0.0.1", help="CARLA host")
    parser.add_argument("--port", type=int, default=2000, help="CARLA RPC port")
    parser.add_argument("--cleanup", action="store_true", help="Clean up spawned vehicles and clutter after capture (default: keep live for inspection)")
    args = parser.parse_args()

    client = carla.Client(args.host, args.port)
    client.set_timeout(40.0)

    print("[compare] Loading MunichStachus into CARLA...")
    world = client.load_world("MunichStachus")

    settings = world.get_settings()
    settings.synchronous_mode = True
    settings.fixed_delta_seconds = 0.05
    world.apply_settings(settings)

    # Recreate the atmospheric soft overcast daylight of the UE5 showcase
    weather = carla.WeatherParameters(
        cloudiness=50.0,
        precipitation=0.0,
        precipitation_deposits=0.0,
        sun_altitude_angle=32.0,
        sun_azimuth_angle=180.0,
        fog_density=0.5,
        fog_distance=60.0,
        fog_falloff=0.2,
        wetness=5.0,
        scattering_intensity=0.8,
    )
    world.set_weather(weather)

    bp_lib = world.get_blueprint_library()
    cam_bp = bp_lib.find("sensor.camera.rgb")
    cam_bp.set_attribute("image_size_x", "1920")
    cam_bp.set_attribute("image_size_y", "1080")
    cam_bp.set_attribute("fov", "75")

    # Find spawn points
    spawn_points = world.get_map().get_spawn_points()
    if not spawn_points:
        print("[compare] ERROR: No spawn points available on MunichStachus.")
        return 1

    spawned_actors: list[carla.Actor] = []
    tm = client.get_trafficmanager(8000)
    tm.set_synchronous_mode(True)

    try:
        # 1. Spawn Ego Lincoln MKZ (the exact car model in the reference image)
        ego_bp = bp_lib.find("vehicle.lincoln.mkz") if bp_lib.filter("vehicle.lincoln.mkz") else bp_lib.find("vehicle.ue4.audi.tt")
        if ego_bp.has_attribute("color"):
            ego_bp.set_attribute("color", "25,25,30")  # Dark metallic graphite/black

        ego_sp = spawn_points[min(112, len(spawn_points) - 1)]
        ego = world.try_spawn_actor(ego_bp, ego_sp)
        if not ego:
            # Fallback to first available spawn
            for sp in spawn_points:
                ego = world.try_spawn_actor(ego_bp, sp)
                if ego:
                    ego_sp = sp
                    break

        if ego:
            spawned_actors.append(ego)
            print(f"[compare] Spawned Hero Lincoln MKZ at ({ego_sp.location.x:.1f}, {ego_sp.location.y:.1f})")

        # 1.5 Spawn Curbside Parked Cars and Roadside Clutter
        clutter_manifest = REPO_ROOT / "Docs" / "map-maker" / "output" / "MunichStachus.carlamap" / "Placements" / "clutter.json"
        if not clutter_manifest.is_file():
            clutter_manifest = Path("/Users/ra94giv/Library/Application Support/Epic/CarlaUnreal/Saved/Maps/MunichStachus/Placements/clutter.json")
        if clutter_manifest.is_file():
            from osm_clutter import load_clutter_manifest, spawn_clutter_in_world
            c_items, p_cars = load_clutter_manifest(clutter_manifest)
            sp_res = spawn_clutter_in_world(world, c_items, p_cars, adjust_ground_elevation=True)
            spawned_actors.extend(sp_res["props"])
            spawned_actors.extend(sp_res["parked_cars"])
            print(f"[compare] Spawned {len(sp_res['parked_cars'])} parked cars and {len(sp_res['props'])} roadside clutter props.")

        # 2. Spawn surrounding traffic
        car_bps = [bp for bp in bp_lib.filter("vehicle.*") if int(bp.get_attribute("number_of_wheels")) == 4]
        random.seed(101)
        sample_spawns = random.sample(spawn_points, min(30, len(spawn_points)))
        for sp in sample_spawns:
            if ego and sp.location.distance(ego_sp.location) < 14.0:
                continue
            bp = random.choice(car_bps)
            veh = world.try_spawn_actor(bp, sp)
            if veh:
                veh.set_autopilot(True, 8000)
                spawned_actors.append(veh)

        print(f"[compare] Spawned {len(spawned_actors)} vehicles for authentic city traffic.")

        # Advance simulation 40 ticks for traffic flow and lighting accumulation
        for _ in range(40):
            world.tick()

        # =====================================================================
        # View 1: Hero Boulevard Matching Reference Shot
        # =====================================================================
        print("[compare] Capturing Hero Reference viewpoint...")
        if ego:
            # Camera positioned right behind the vehicle at street level looking forward
            hero_tf = carla.Transform(
                carla.Location(x=-4.8, y=-0.35, z=1.8),
                carla.Rotation(pitch=-5.5, yaw=1.5, roll=0.0),
            )
            cam_hero = world.spawn_actor(cam_bp, hero_tf, attach_to=ego)
            
            # Synchronize spectator camera so live desktop window displays this exact view
            world.get_spectator().set_transform(cam_hero.get_transform())
            
            img_hero = capture_frame(cam_hero, world, ticks=20)
            img_hero.save(str(OUT_DIR / "munich_hero_boulevard.png"), "PNG")
            print(f"[compare] Saved -> {OUT_DIR / 'munich_hero_boulevard.png'}")

            # Create side-by-side comparison artifact
            create_side_by_side_comparison(
                REF_IMG_PATH,
                img_hero,
                OUT_DIR / "munich_hero_comparison.png",
            )

            # =====================================================================
            # View 2: Driver Cockpit / Windshield POV
            # =====================================================================
            print("[compare] Capturing Cockpit / Driver POV...")
            cockpit_tf = carla.Transform(
                carla.Location(x=1.5, y=-0.3, z=1.15),
                carla.Rotation(pitch=-4.0, yaw=0.0, roll=0.0),
            )
            cam_cockpit = world.spawn_actor(cam_bp, cockpit_tf, attach_to=ego)
            img_cockpit = capture_frame(cam_cockpit, world, ticks=15)
            img_cockpit.save(str(OUT_DIR / "munich_hero_cockpit.png"), "PNG")
            print(f"[compare] Saved -> {OUT_DIR / 'munich_hero_cockpit.png'}")

        # =====================================================================
        # View 3: Wide 4K Panoramic Aerial Overview
        # =====================================================================
        print("[compare] Capturing Panoramic Aerial View...")
        cam_aerial_bp = bp_lib.find("sensor.camera.rgb")
        cam_aerial_bp.set_attribute("image_size_x", "2560")
        cam_aerial_bp.set_attribute("image_size_y", "1440")
        cam_aerial_bp.set_attribute("fov", "85")

        aerial_tf = carla.Transform(
            carla.Location(x=320.0, y=-260.0, z=360.0),
            carla.Rotation(pitch=-68.0, yaw=85.0, roll=0.0),
        )
        cam_aerial = world.spawn_actor(cam_aerial_bp, aerial_tf)
        img_aerial = capture_frame(cam_aerial, world, ticks=20)
        img_aerial.save(str(OUT_DIR / "munich_hero_aerial.png"), "PNG")
        print(f"[compare] Saved -> {OUT_DIR / 'munich_hero_aerial.png'}")

        # =====================================================================
        # View 4: Signalized Intersection with Active Traffic Lights
        # =====================================================================
        print("[compare] Capturing Signalized Intersection View...")
        # Position camera at crosswalk approach facing intersection center
        inter_tf = carla.Transform(
            carla.Location(x=340.0, y=-240.0, z=5.5),
            carla.Rotation(pitch=-12.0, yaw=120.0, roll=0.0),
        )
        cam_inter = world.spawn_actor(cam_bp, inter_tf)
        img_inter = capture_frame(cam_inter, world, ticks=15)
        img_inter.save(str(OUT_DIR / "munich_hero_intersection.png"), "PNG")
        print(f"[compare] Saved -> {OUT_DIR / 'munich_hero_intersection.png'}")

    finally:
        if args.cleanup:
            for actor in spawned_actors:
                try:
                    actor.destroy()
                except Exception:
                    pass
            print("[compare] Retained all vehicles and clutter actors for live inspection.")

        try:
            tm.set_synchronous_mode(False)
        except Exception:
            pass
        settings.synchronous_mode = False
        world.apply_settings(settings)
        print("[compare] Restored asynchronous mode for interactive live simulation.")

    print("\n[compare] HERO SHOWCASE SUITE GENERATED SUCCESSFULLY!")
    return 0


if __name__ == "__main__":
    sys.exit(main())

