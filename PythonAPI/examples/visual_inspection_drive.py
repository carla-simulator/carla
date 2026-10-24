#!/usr/bin/env python3
"""
Manual Visual Inspection Drive for CARLA Maps.

Spawns a Hero vehicle, multiple NPC vehicles, and a crowd of pedestrians on the map.
Enables autonomous driving on the Hero and smoothly locks the CARLA simulator's
main view (spectator) in third-person chase camera mode behind the self-driving
hero for live visual inspection of vehicles, pedestrians, sidewalks, curbs,
crosswalks, signs, and traffic lights.

Features:
  - Custom Hero spawn point selection by index (--hero-spawn-idx)
  - Interactive spawn point catalog & continuous route analyzer (--list-spawns)
  - Proximity-focused NPC spawning: places vehicles directly around the hero
    in adjacent lanes, oncoming lanes, and nearby intersections
  - Proximity-focused Pedestrian spawning: concentrates walkers on sidewalks
    and zebra crosswalks within visual range of the hero
  - Third-person chase camera follow with customizable distance/height/pitch
  - Clean lifecycle teardown and clock state restoration

Usage:
    # Standard 15s visual inspection with Hero at central boulevard (index 215)
    python3 PythonAPI/examples/visual_inspection_drive.py --seconds 15 -n 25 -w 50

    # Pick custom spawn point index (e.g. index 299)
    python3 PythonAPI/examples/visual_inspection_drive.py --hero-spawn-idx 299 --seconds 20

    # List all available spawn points and their continuous route lengths
    python3 PythonAPI/examples/visual_inspection_drive.py --list-spawns
"""

from __future__ import annotations

import argparse
import math
import random
import sys
import time
from typing import Sequence

try:
    import carla
except ImportError:
    print("Error: 'carla' module not found. Please install the CARLA PythonAPI.", file=sys.stderr)
    sys.exit(1)


def build_arg_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Manual visual inspection drive with autonomous hero, NPCs, and walkers."
    )
    parser.add_argument("--host", default="127.0.0.1", help="CARLA server host (default: 127.0.0.1)")
    parser.add_argument("--port", type=int, default=2000, help="CARLA server RPC port (default: 2000)")
    parser.add_argument("--tm-port", type=int, default=8000, help="Traffic Manager port (default: 8000)")
    parser.add_argument("-s", "--seconds", type=float, default=15.0, help="Duration of drive in seconds (default: 15.0)")
    parser.add_argument("-n", "--npcs", type=int, default=20, help="Number of NPC vehicles (default: 20)")
    parser.add_argument("-w", "--walkers", type=int, default=40, help="Number of pedestrians (default: 40)")
    parser.add_argument("--cross-factor", type=float, default=0.25, help="Walker road crossing factor [0.0 - 1.0]. 0.0 restricts walkers strictly to painted crosswalks, >0.0 permits road jaywalking (default: 0.25)")
    parser.add_argument("--hero-spawn-idx", type=int, default=215, help="Spawn point index for Hero vehicle (default: 215 - Central Boulevard)")
    parser.add_argument("--radius", type=float, default=160.0, help="Visual radius in meters to concentrate NPCs and pedestrians around Hero (default: 160.0)")
    parser.add_argument("--global-spawns", action="store_true", help="Scatter NPCs and walkers globally across the entire map instead of near Hero")
    parser.add_argument("--hero-filter", default="vehicle.dodge.charger", help="Hero blueprint filter (default: vehicle.dodge.charger)")
    parser.add_argument("--view", choices=["chase", "first", "top", "front"], default="chase", help="Camera follow view mode (default: chase)")
    parser.add_argument("--distance", type=float, default=8.0, help="Camera follow distance in meters (default: 8.0)")
    parser.add_argument("--height", type=float, default=3.5, help="Camera follow height in meters (default: 3.5)")
    parser.add_argument("--pitch", type=float, default=-12.0, help="Camera follow pitch angle in degrees (default: -12.0)")
    parser.add_argument("--speed-diff", type=float, default=-20.0, help="Hero percentage speed difference (default: -20.0)")
    parser.add_argument("--keep-alive", action="store_true", help="Keep simulation running until interrupted with Ctrl+C")
    parser.add_argument("--screenshot-path", default=None, help="Optional path to save an RGB camera screenshot during the drive")
    parser.add_argument("--list-spawns", action="store_true", help="Analyze and list all spawn points with continuous route lengths, then exit")
    return parser


def compute_camera_transform(
    target_tf: carla.Transform,
    view: str = "chase",
    distance: float = 8.0,
    height: float = 3.5,
    pitch: float = -12.0,
) -> carla.Transform:
    """Computes spectator camera transform relative to hero vehicle."""
    loc = target_tf.location
    rot = target_tf.rotation
    fwd = target_tf.get_forward_vector()

    if view == "top":
        return carla.Transform(
            carla.Location(loc.x, loc.y, loc.z + 28.0),
            carla.Rotation(pitch=-90.0, yaw=rot.yaw),
        )
    elif view == "first":
        return carla.Transform(
            carla.Location(loc.x + fwd.x * 0.4, loc.y + fwd.y * 0.4, loc.z + 1.25),
            carla.Rotation(pitch=0.0, yaw=rot.yaw),
        )
    elif view == "front":
        return carla.Transform(
            carla.Location(loc.x + fwd.x * distance, loc.y + fwd.y * distance, loc.z + height),
            carla.Rotation(pitch=pitch, yaw=rot.yaw + 180.0),
        )
    else:  # chase
        return carla.Transform(
            carla.Location(loc.x - fwd.x * distance, loc.y - fwd.y * distance, loc.z + height),
            carla.Rotation(pitch=pitch, yaw=rot.yaw),
        )


def evaluate_continuous_route_length(carla_map: carla.Map, spawn_transform: carla.Transform, max_dist: float = 1000.0) -> float:
    """Calculates how many meters a vehicle can drive continuously from this spawn point before a dead end."""
    curr = carla_map.get_waypoint(spawn_transform.location, project_to_road=True, lane_type=carla.LaneType.Driving)
    if curr is None:
        return 0.0

    dist = 0.0
    visited: set[tuple[int, int, float]] = set()
    while dist < max_dist:
        key = (curr.road_id, curr.lane_id, round(curr.s, 1))
        if key in visited:
            break
        visited.add(key)
        next_wps = curr.next(5.0)
        if not next_wps:
            break
        curr = next_wps[0]
        dist += 5.0
    return dist


def list_available_spawn_points(carla_map: carla.Map, top_n: int = 30) -> None:
    """Analyzes and displays the top spawn points sorted by continuous route length."""
    spawn_points = list(carla_map.get_spawn_points())
    print("\n" + "=" * 80)
    print(f"ANALYZING {len(spawn_points)} OPENDRIVE VEHICLE SPAWN POINTS")
    print("=" * 80)
    print(f"{'Idx':>4} | {'Road':>5} | {'Lane':>4} | {'Location (x, y, z)':>26} | {'Yaw':>6} | {'Route':>7} | Recommendation")
    print("-" * 80)

    analyzed = []
    for idx, sp in enumerate(spawn_points):
        route_len = evaluate_continuous_route_length(carla_map, sp)
        wp = carla_map.get_waypoint(sp.location, project_to_road=True, lane_type=carla.LaneType.Driving)
        road_id = wp.road_id if wp else -1
        lane_id = wp.lane_id if wp else 0
        analyzed.append((route_len, idx, sp, road_id, lane_id))

    # Sort descending by route length
    analyzed.sort(key=lambda item: (-item[0], item[1]))

    for route_len, idx, sp, road_id, lane_id in analyzed[:top_n]:
        loc_str = f"({sp.location.x:6.1f}, {sp.location.y:6.1f}, {sp.location.z:4.1f})"
        rec = ""
        if route_len >= 500.0:
            rec = "★★★ Excellent (Long Corridor / Boulevard)"
        elif route_len >= 200.0:
            rec = "★★☆ Great (Urban Avenue)"
        elif route_len >= 100.0:
            rec = "★☆☆ Medium (Street Segment)"
        else:
            rec = "⚠ Short / Near Boundary Dead-End"

        print(f"{idx:4d} | {road_id:5d} | {lane_id:4d} | {loc_str:>26} | {sp.rotation.yaw:6.1f}° | {route_len:6.0f}m | {rec}")

    print("=" * 80)
    print(f"To spawn at any of these, run: python3 PythonAPI/examples/visual_inspection_drive.py --hero-spawn-idx <INDEX>\n")


def run_visual_inspection_drive(
    client: carla.Client,
    seconds: float = 15.0,
    num_npcs: int = 20,
    num_walkers: int = 40,
    cross_factor: float = 0.25,
    hero_spawn_idx: int = 215,
    radius: float = 160.0,
    global_spawns: bool = False,
    hero_filter: str = "vehicle.dodge.charger",
    view: str = "chase",
    distance: float = 8.0,
    height: float = 3.5,
    pitch: float = -12.0,
    speed_diff: float = -20.0,
    tm_port: int = 8000,
    keep_alive: bool = False,
    screenshot_path: str | None = None,
) -> bool:
    world = client.get_world()
    carla_map = world.get_map()
    spectator = world.get_spectator()
    tm = client.get_trafficmanager(tm_port)

    orig_settings = world.get_settings()
    settings = world.get_settings()
    settings.synchronous_mode = True
    settings.fixed_delta_seconds = 0.05
    world.apply_settings(settings)
    tm.set_synchronous_mode(True)

    bp_lib = world.get_blueprint_library()
    spawn_points = list(carla_map.get_spawn_points())
    if not spawn_points:
        print("[error] No spawn points available on the map!", file=sys.stderr)
        world.apply_settings(orig_settings)
        return False

    # Filter out blueprints known to lack assets in standalone builds
    invalid_models = {"vehicle.tesla.model3", "vehicle.kia.niro"}
    valid_4w_bps = [
        bp for bp in bp_lib.filter("vehicle.*")
        if bp.id not in invalid_models and int(bp.get_attribute("number_of_wheels")) == 4
    ]

    hero_actor: carla.Vehicle | None = None
    cam_sensor: carla.Sensor | None = None
    saved_screenshot = False
    tick_idx = 0
    npc_ids: list[int] = []
    walker_ids: list[int] = []
    controller_ids: list[int] = []

    try:
        # -------------------------------------------------------------
        # 1. Spawn Hero Ego Vehicle at Selected Spawn Point Index
        # -------------------------------------------------------------
        if hero_spawn_idx < 0 or hero_spawn_idx >= len(spawn_points):
            print(f"[warning] Invalid --hero-spawn-idx {hero_spawn_idx}, falling back to index 215.")
            hero_spawn_idx = 215

        hero_bps = [bp for bp in bp_lib.filter(hero_filter) if bp.id not in invalid_models]
        if not hero_bps:
            hero_bps = valid_4w_bps
        hero_bp = hero_bps[0]
        hero_bp.set_attribute("role_name", "hero")
        if hero_bp.has_attribute("color"):
            hero_bp.set_attribute("color", "220,10,10")  # Vibrant red

        hero_sp = spawn_points[hero_spawn_idx]
        hero_tf = carla.Transform(
            carla.Location(hero_sp.location.x, hero_sp.location.y, hero_sp.location.z + 0.35),
            hero_sp.rotation,
        )
        hero_actor = world.try_spawn_actor(hero_bp, hero_tf)

        if hero_actor is None:
            # Fallback search for nearby valid spawn point
            print(f"[warning] Direct spawn at index {hero_spawn_idx} was obstructed, trying adjacent spawn points...")
            for idx in [215, 299, 298, 289, 279, 264, 261, 256] + list(range(len(spawn_points))):
                test_sp = carla.Transform(
                    carla.Location(spawn_points[idx].location.x, spawn_points[idx].location.y, spawn_points[idx].location.z + 0.35),
                    spawn_points[idx].rotation,
                )
                hero_actor = world.try_spawn_actor(hero_bp, test_sp)
                if hero_actor is not None:
                    hero_spawn_idx = idx
                    hero_sp = spawn_points[idx]
                    break

        if hero_actor is None:
            print("[error] Failed to spawn hero vehicle across all available spawn points!", file=sys.stderr)
            return False

        route_len = evaluate_continuous_route_length(carla_map, hero_sp)
        wp_hero = carla_map.get_waypoint(hero_sp.location)
        road_info = f"Road {wp_hero.road_id}, Lane {wp_hero.lane_id}" if wp_hero else "Road Unknown"
        print(f"[inspect] Spawned Hero EGO: {hero_actor.type_id} (ID: {hero_actor.id})")
        print(f"          Spawn Index   : {hero_spawn_idx} ({road_info})")
        print(f"          Location      : ({hero_sp.location.x:.1f}, {hero_sp.location.y:.1f}, {hero_sp.location.z:.1f}) | Yaw: {hero_sp.rotation.yaw:.1f}°")
        print(f"          Continuous Run: {route_len:.0f}m before any dead end")

        hero_actor.set_autopilot(True, tm_port)
        tm.vehicle_percentage_speed_difference(hero_actor, speed_diff)
        tm.ignore_lights_percentage(hero_actor, 0.0)
        tm.ignore_signs_percentage(hero_actor, 0.0)

        # Optional chase camera sensor for recording screenshots
        if screenshot_path:
            from pathlib import Path
            Path(screenshot_path).parent.mkdir(parents=True, exist_ok=True)
            cam_bp = bp_lib.find("sensor.camera.rgb")
            cam_bp.set_attribute("image_size_x", "1920")
            cam_bp.set_attribute("image_size_y", "1080")
            cam_bp.set_attribute("fov", "90")
            cam_rel_tf = carla.Transform(carla.Location(x=-distance, z=height), carla.Rotation(pitch=pitch))
            cam_sensor = world.spawn_actor(cam_bp, cam_rel_tf, attach_to=hero_actor)

            def _save_cb(img):
                nonlocal saved_screenshot
                if not saved_screenshot and tick_idx >= 60:
                    img.save_to_disk(str(screenshot_path))
                    saved_screenshot = True
                    print(f"[inspect] Captured visual inspection screenshot -> {screenshot_path}")

            cam_sensor.listen(_save_cb)

        # -------------------------------------------------------------
        # 2. Spawn NPC Vehicles (Concentrated near Hero for Visual Flow)
        # -------------------------------------------------------------
        hero_loc = hero_sp.location
        available_sps = []
        for idx, sp in enumerate(spawn_points):
            if idx == hero_spawn_idx:
                continue
            dist_to_hero = math.hypot(sp.location.x - hero_loc.x, sp.location.y - hero_loc.y)
            available_sps.append((dist_to_hero, idx, sp))

        if not global_spawns:
            # Sort by proximity to Hero so all requested NPCs fill the surrounding streets
            available_sps.sort(key=lambda item: item[0])
            print(f"[inspect] Concentrating NPCs within visual range (~{radius:.0f}m) around Hero...")
        else:
            random.seed(42)
            random.shuffle(available_sps)
            print("[inspect] Scattering NPCs globally across the entire map...")

        veh_batch = []
        for dist_to_hero, idx, sp in available_sps:
            if len(veh_batch) >= num_npcs:
                break
            # Skip if ridiculously close (< 8m) to avoid bumper-to-bumper spawn blockages
            if dist_to_hero < 8.0:
                continue

            sp_shifted = carla.Transform(
                carla.Location(sp.location.x, sp.location.y, sp.location.z + 0.35),
                sp.rotation,
            )
            bp = random.choice(valid_4w_bps)
            bp.set_attribute("role_name", "autopilot")
            if bp.has_attribute("color"):
                bp.set_attribute("color", random.choice(bp.get_attribute("color").recommended_values))

            veh_batch.append(
                carla.command.SpawnActor(bp, sp_shifted).then(
                    carla.command.SetAutopilot(carla.command.FutureActor, True, tm_port)
                )
            )

        for res in client.apply_batch_sync(veh_batch, True):
            if not res.error:
                npc_ids.append(res.actor_id)

        print(f"[inspect] Spawned {len(npc_ids)} / {num_npcs} NPC vehicles on Traffic Manager autopilot.")

        # -------------------------------------------------------------
        # 3. Spawn Pedestrians (Concentrated on Sidewalks & Crosswalks)
        # -------------------------------------------------------------
        walker_bps = list(bp_lib.filter("walker.pedestrian.*"))
        ai_bp = bp_lib.find("controller.ai.walker")

        walker_locs: list[carla.Location] = []
        max_samples = 4000
        for _ in range(max_samples):
            if len(walker_locs) >= num_walkers:
                break
            loc = world.get_random_location_from_navigation()
            if loc is not None:
                if not global_spawns:
                    d = math.hypot(loc.x - hero_loc.x, loc.y - hero_loc.y)
                    if d <= radius:
                        walker_locs.append(loc)
                else:
                    walker_locs.append(loc)

        walker_batch = []
        for loc in walker_locs:
            tf = carla.Transform(carla.Location(loc.x, loc.y, loc.z + 0.8))
            w_bp = random.choice(walker_bps)
            if w_bp.has_attribute("is_invincible"):
                w_bp.set_attribute("is_invincible", "false")
            walker_batch.append(carla.command.SpawnActor(w_bp, tf))

        for res in client.apply_batch_sync(walker_batch, True):
            if not res.error:
                walker_ids.append(res.actor_id)

        # Tick once to materialize walkers
        world.tick()

        # Attach Walker AI Controllers
        con_batch = [carla.command.SpawnActor(ai_bp, carla.Transform(), w_id) for w_id in walker_ids]
        for res in client.apply_batch_sync(con_batch, True):
            if not res.error:
                controller_ids.append(res.actor_id)

        # Settle ticks
        for _ in range(3):
            world.tick()

        # Set crosswalk crossing factor (0.0 = only zebra crosswalks, >0.0 = permits road jaywalking)
        world.set_pedestrians_cross_factor(cross_factor)

        controllers = [world.get_actor(c_id) for c_id in controller_ids]
        for c in controllers:
            if c is not None and c.is_alive:
                c.start()
                # Pick destination within neighborhood
                dest = None
                for _ in range(50):
                    cand = world.get_random_location_from_navigation()
                    if cand is not None:
                        if global_spawns or math.hypot(cand.x - hero_loc.x, cand.y - hero_loc.y) <= radius * 1.2:
                            dest = cand
                            break
                if dest is not None:
                    c.go_to_location(dest)
                c.set_max_speed(1.2 + random.uniform(0.0, 0.5))

        print(f"[inspect] Spawned {len(walker_ids)} / {num_walkers} wandering pedestrians on NavMesh.")

        # Position spectator immediately before starting drive
        initial_cam = compute_camera_transform(hero_actor.get_transform(), view, distance, height, pitch)
        spectator.set_transform(initial_cam)

        # -------------------------------------------------------------
        # 4. Follow Hero Vehicle with Spectator in Chase Mode
        # -------------------------------------------------------------
        target_ticks = int(seconds / 0.05) if not keep_alive else 999999
        print(f"\n{'=' * 75}")
        print(f"[inspect] Visual inspection drive active: {seconds:.1f}s simulated time.")
        print(f"[inspect] Main Simulator View: FOLLOWING HERO in {view.upper()} chase cam!")
        print(f"[inspect] Watch the running CARLA window live!")
        print(f"{'=' * 75}\n")

        t_start = time.monotonic()
        while tick_idx < target_ticks:
            if not hero_actor.is_alive:
                print("\n[inspect] Hero vehicle actor was destroyed or reached route end.")
                break

            hero_tf = hero_actor.get_transform()
            cam_tf = compute_camera_transform(hero_tf, view, distance, height, pitch)
            spectator.set_transform(cam_tf)

            world.tick()
            tick_idx += 1

            # Print telemetry status every 20 ticks (1.0 second)
            if tick_idx % 20 == 0:
                vel = hero_actor.get_velocity()
                speed_kmh = 3.6 * math.sqrt(vel.x ** 2 + vel.y ** 2 + vel.z ** 2)
                elapsed_sim = tick_idx * 0.05
                loc = hero_tf.location
                print(
                    f"  [{elapsed_sim:4.1f}s / {seconds:4.1f}s] "
                    f"Hero Speed: {speed_kmh:5.1f} km/h | "
                    f"Pos: ({loc.x:6.1f}, {loc.y:6.1f}, {loc.z:4.1f}) | "
                    f"Active NPCs: {len(npc_ids)} | Active Walkers: {len(walker_ids)}"
                )

        t_elapsed = time.monotonic() - t_start
        print(f"\n[inspect] Visual inspection drive completed ({t_elapsed:.1f}s wall clock, {tick_idx * 0.05:.1f}s sim).")
        return True

    except KeyboardInterrupt:
        print("\n[inspect] Interrupted by user (Ctrl+C). Cleaning up...")
        return True

    finally:
        print("[inspect] Tearing down test actors and restoring world clock...")
        # 1. Stop and destroy pedestrian controllers
        for c_id in controller_ids:
            try:
                c = world.get_actor(c_id)
                if c is not None and c.is_alive:
                    c.stop()
            except Exception:
                pass

        if controller_ids:
            client.apply_batch_sync([carla.command.DestroyActor(x) for x in controller_ids], True)

        # 2. Destroy walkers
        if walker_ids:
            client.apply_batch_sync([carla.command.DestroyActor(x) for x in walker_ids], True)

        # 3. Destroy NPCs
        if npc_ids:
            client.apply_batch_sync([carla.command.DestroyActor(x) for x in npc_ids], True)

        # 4. Destroy Camera and Hero
        if cam_sensor is not None and cam_sensor.is_alive:
            try:
                cam_sensor.stop()
            except Exception:
                pass
            cam_sensor.destroy()

        if hero_actor is not None and hero_actor.is_alive:
            hero_actor.destroy()

        # 5. Restore original settings
        world.apply_settings(orig_settings)
        tm.set_synchronous_mode(False)
        print("[inspect] Cleanup complete. Simulator state fully restored.\n")


def main() -> int:
    parser = build_arg_parser()
    args = parser.parse_args()

    client = carla.Client(args.host, args.port)
    client.set_timeout(20.0)
    world = client.get_world()

    if args.list_spawns:
        list_available_spawn_points(world.get_map())
        return 0

    success = run_visual_inspection_drive(
        client=client,
        seconds=args.seconds,
        num_npcs=args.npcs,
        num_walkers=args.walkers,
        cross_factor=args.cross_factor,
        hero_spawn_idx=args.hero_spawn_idx,
        radius=args.radius,
        global_spawns=args.global_spawns,
        hero_filter=args.hero_filter,
        view=args.view,
        distance=args.distance,
        height=args.height,
        pitch=args.pitch,
        speed_diff=args.speed_diff,
        tm_port=args.tm_port,
        keep_alive=args.keep_alive,
        screenshot_path=args.screenshot_path,
    )

    return 0 if success else 1


if __name__ == "__main__":
    sys.exit(main())
