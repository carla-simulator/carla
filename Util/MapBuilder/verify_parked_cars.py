#!/usr/bin/env python3
"""
Dynamic Traffic Navigation & Parked Vehicle Flow Verifier for CARLA.

Spawns static curbside parked cars along road corridors, launches CARLA Traffic Manager
autopilot vehicles navigating through the map, and monitors physical collision events
over 200 simulation ticks to assert 0 collisions between moving vehicles and parked cars.
"""

from __future__ import annotations

import argparse
from pathlib import Path
import random
import sys
import time
from typing import Any

import carla

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
sys.path.insert(0, str(REPO_ROOT))
sys.path.insert(0, str(REPO_ROOT / "Util" / "MapBuilder"))

from osm_clutter import generate_curbside_parked_cars, spawn_clutter_in_world


def main() -> int:
    parser = argparse.ArgumentParser(description="CARLA Parked Car Traffic Flow & Collision Verifier")
    parser.add_argument("--host", default="127.0.0.1", help="CARLA host")
    parser.add_argument("--port", type=int, default=2000, help="CARLA RPC port")
    parser.add_argument("--map-name", default="MunichStachus", help="Map name to test")
    parser.add_argument("--num-parked", type=int, default=20, help="Number of parked cars to spawn")
    parser.add_argument("--num-traffic", type=int, default=12, help="Number of moving autopilot cars to spawn")
    parser.add_argument("--ticks", type=int, default=200, help="Number of simulation ticks to run")
    args = parser.parse_args()

    client = carla.Client(args.host, args.port)
    client.set_timeout(30.0)

    print(f"[traffic-verifier] Loading map '{args.map_name}'...")
    world = client.load_world(args.map_name)

    settings = world.get_settings()
    settings.synchronous_mode = True
    settings.fixed_delta_seconds = 0.05
    world.apply_settings(settings)

    carla_map = world.get_map()
    bp_lib = world.get_blueprint_library()

    # 1. Generate and spawn curbside parked cars
    print(f"[traffic-verifier] Generating {args.num_parked} curbside parked cars...")
    parked_items = generate_curbside_parked_cars(carla_map, max_cars=args.num_parked)
    spawn_res = spawn_clutter_in_world(world, clutter_items=[], parked_cars=parked_items)
    parked_actors = spawn_res["parked_cars"]
    print(f"[traffic-verifier] Successfully spawned {len(parked_actors)} static parked vehicles.")

    # 2. Setup Traffic Manager
    tm = client.get_trafficmanager(8000)
    tm.set_synchronous_mode(True)
    tm.set_global_distance_to_leading_vehicle(2.5)

    # 3. Spawn moving autopilot traffic
    spawn_points = carla_map.get_spawn_points()
    veh_bps = [bp for bp in bp_lib.filter("vehicle.*") if int(bp.get_attribute("number_of_wheels")) == 4]

    moving_vehs: list[carla.Actor] = []
    collision_sensors: list[carla.Sensor] = []
    collision_events: list[dict[str, Any]] = []

    collision_bp = bp_lib.find("sensor.other.collision")

    # Filter spawn points far from parked cars to avoid immediate collision on spawn
    parked_locs = [p.get_location() for p in parked_actors]
    valid_spawns: list[carla.Transform] = []
    for sp in spawn_points:
        if all(sp.location.distance(ploc) > 10.0 for ploc in parked_locs):
            valid_spawns.append(sp)

    random.seed(42)
    sample_spawns = random.sample(valid_spawns, min(args.num_traffic, len(valid_spawns)))
    for sp in sample_spawns:
        bp = random.choice(veh_bps)
        veh = world.try_spawn_actor(bp, sp)
        if veh:
            veh.set_autopilot(True, 8000)
            moving_vehs.append(veh)

            # Attach collision sensor
            col_sensor = world.spawn_actor(collision_bp, carla.Transform(), attach_to=veh)

            def on_collision(event: carla.CollisionEvent, actor_id: int = veh.id) -> None:
                other_id = event.other_actor.id if event.other_actor else -1
                other_type = event.other_actor.type_id if event.other_actor else "unknown"
                collision_events.append({
                    "vehicle_id": actor_id,
                    "other_id": other_id,
                    "other_type": other_type,
                    "intensity": event.normal_impulse.length(),
                })

            col_sensor.listen(on_collision)
            collision_sensors.append(col_sensor)

    print(f"[traffic-verifier] Spawned {len(moving_vehs)} moving autopilot vehicles with collision sensors.")

    try:
        print(f"[traffic-verifier] Simulating {args.ticks} ticks (synchronous delta = {settings.fixed_delta_seconds}s)...")
        for tick_idx in range(args.ticks):
            world.tick()
            if (tick_idx + 1) % 50 == 0:
                print(f"[traffic-verifier] Tick {tick_idx + 1}/{args.ticks} complete (Collisions logged: {len(collision_events)})...")

        # Analyze collisions with parked cars
        parked_ids = {p.id for p in parked_actors}
        parked_car_collisions = [c for c in collision_events if c["other_id"] in parked_ids]

        print("=" * 76)
        print("CARLA Traffic Navigation & Parked Vehicle Flow Audit")
        print("=" * 76)
        print(f"  Total Simulation Ticks   : {args.ticks}")
        print(f"  Active Parked Vehicles   : {len(parked_actors)}")
        print(f"  Active Autopilot Traffic : {len(moving_vehs)}")
        print(f"  Total Collisions Logged  : {len(collision_events)}")
        print(f"  Collisions w/ Parked Cars: {len(parked_car_collisions)} (must be 0)")
        print("=" * 76)

        if parked_car_collisions:
            print("[traffic-verifier] FAIL: Moving vehicles collided with parked cars:")
            for c in parked_car_collisions:
                print(f"  - Vehicle {c['vehicle_id']} collided with Parked Car {c['other_id']} ({c['other_type']}) intensity={c['intensity']:.1f}")
            return 1

        print("[traffic-verifier] SUCCESS: Traffic Manager vehicles navigated all corridors with 0 parked car collisions!")
        print("=" * 76)
        return 0

    finally:
        for s in collision_sensors:
            try:
                s.stop()
                s.destroy()
            except Exception:
                pass
        for v in moving_vehs:
            try:
                v.destroy()
            except Exception:
                pass
        for p in parked_actors:
            try:
                p.destroy()
            except Exception:
                pass

        settings.synchronous_mode = False
        world.apply_settings(settings)


if __name__ == "__main__":
    sys.exit(main())

