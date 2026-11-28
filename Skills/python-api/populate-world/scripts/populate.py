#!/usr/bin/env python3
"""Fill the map with autopilot vehicles AND wandering pedestrians from ONE process.

Commands:

    spawn   [--vehicles 50] [--walkers 20] [--safe] [--filter vehicle.*]
            [--base-type car] [--seed 42] [--cross-factor 0.1] ...
            [--hold | --seconds 60] [--keep]
    destroy                         walkers (controllers first), then vehicles

Why one process: the Traffic Manager and the walker AI controllers both stop
when the client that created them exits, and only one client may tick a
synchronous world. Run as two skills that means two resident processes and a
clock hand-off between them (spawn-vehicles owns the clock, spawn-walkers
observes). Here a single client claims the clock, owns the TM, spawns both
populations, and holds them with one tick loop -- nothing to coordinate.

The spawning code is NOT duplicated: it is imported from the sibling skills
spawn-vehicles (vehicles.py) and spawn-walkers (walkers.py), so every fix and
measured gotcha there applies here too.
"""
from __future__ import annotations

import argparse
import os
import random
import sys

sys.dont_write_bytecode = True  # importing the siblings must not drop __pycache__ into the library

_HERE = os.path.dirname(os.path.abspath(__file__))
for _sibling in ("spawn-vehicles", "spawn-walkers"):
    sys.path.insert(0, os.path.join(_HERE, "..", "..", _sibling, "scripts"))

import vehicles as V  # noqa: E402  (spawn-vehicles/scripts/vehicles.py)
import walkers as W   # noqa: E402  (spawn-walkers/scripts/walkers.py)


def _share_clock(owns: bool) -> None:
    # Each sibling module tracks clock ownership in its own global; they must
    # agree, or the walker phases would wait_for_tick() on a world nobody ticks.
    V._OWNS_CLOCK = owns
    W._OWNS_CLOCK = owns


def _hold(world, seconds: float, what: str) -> None:
    print(f"holding {what}; " + (f"releasing after {seconds:g}s of simulated time, or "
                                 if seconds > 0 else "") + "Ctrl+C to release", flush=True)
    start = world.get_snapshot().timestamp.elapsed_seconds
    try:
        while True:
            V._pump(world)
            if seconds > 0 and \
                    world.get_snapshot().timestamp.elapsed_seconds - start >= seconds:
                print(f"released after {seconds:g}s of simulated time", flush=True)
                return
    except KeyboardInterrupt:
        print("\nreleased: TM and walker controllers stop with this process")


def cmd_spawn(args):
    if args.seconds > 0:
        args.hold = True
    if args.vehicles <= 0 and args.walkers <= 0:
        raise SystemExit("nothing to spawn: --vehicles and --walkers are both 0")

    client = V._client()
    world = V._fresh(client.get_world())
    previous = V._claim_clock(world, args.delta, allow_sync=not args.no_sync)
    _share_clock(V._OWNS_CLOCK)

    if args.seed is not None:
        random.seed(args.seed)
        world.set_pedestrians_seed(args.seed)
    tm = V._tm_and_sync(client, world, args.tm_port) if args.vehicles > 0 else None
    if tm is not None and args.seed is not None:
        tm.set_random_device_seed(args.seed)

    vehicle_ids, controller_ids, walker_ids = [], [], []
    try:
        # --- vehicles ---------------------------------------------------------
        if args.vehicles > 0:
            bps = V._vehicle_bps(world, args.filter, args.safe, args.base_type)
            points = world.get_map().get_spawn_points()
            random.shuffle(points)
            want = min(args.vehicles, len(points))
            if want < args.vehicles:
                print(f"note: only {len(points)} spawn points; capping "
                      f"{args.vehicles} -> {want} vehicles")
            vehicle_ids = V._spawn_at(client, points[:want], bps, True, args.tm_port)
            print(f"spawned {len(vehicle_ids)} vehicles on autopilot (TM :{args.tm_port})",
                  flush=True)

        # --- walkers ----------------------------------------------------------
        if args.walkers > 0:
            world.set_pedestrians_cross_factor(args.cross_factor)
            args.count = args.walkers          # the name walkers._spawn_crowd reads
            args.no_wander = False
            controller_ids, walker_ids = W._spawn_crowd(client, world, args)

        print(f"populated: {len(vehicle_ids)} vehicles + {len(walker_ids)} walkers",
              flush=True)

        if args.hold:
            _hold(world, args.seconds,
                  f"{len(vehicle_ids)} vehicles + {len(walker_ids)} walkers")
        else:
            print("  note: this process owns the TM and the walker controllers — "
                  "they stop on exit. Use --hold to keep the scene moving.")
    finally:
        if args.hold:
            try:
                W._cleanup(client, controller_ids, walker_ids, args.keep)
                V._cleanup(client, vehicle_ids, args.keep)
            except RuntimeError as error:
                print(f"  cleanup failed ({error}); actors may remain — "
                      "run `destroy` when the server is responsive again", flush=True)
        V._release_clock(world, previous, tm)
        _share_clock(False)


def cmd_destroy(args):
    W.cmd_destroy(args)                                   # controllers, then walkers
    V.cmd_destroy(argparse.Namespace(filter="vehicle.*"))


def main() -> None:
    p = argparse.ArgumentParser(
        description="Spawn vehicles + pedestrians together from one held client.")
    sub = p.add_subparsers(dest="cmd", required=True)

    ps = sub.add_parser("spawn", help="spawn autopilot vehicles and wandering walkers")
    ps.add_argument("--vehicles", type=int, default=30, help="vehicle count (0 = none)")
    ps.add_argument("--walkers", type=int, default=30, help="pedestrian count (0 = none)")
    # vehicle options (as spawn-vehicles)
    ps.add_argument("--filter", default="vehicle.*", help="vehicle blueprint filter")
    ps.add_argument("--safe", action="store_true", help="cars only (four wheels, base_type car)")
    ps.add_argument("--base-type", default="",
                    help="comma list of vehicle base_type to keep, e.g. car")
    ps.add_argument("--tm-port", type=int, default=int(os.environ.get("TM_PORT", "8000")))
    # walker options (as spawn-walkers)
    ps.add_argument("--speed-min", type=float, default=1.0)
    ps.add_argument("--speed-max", type=float, default=1.8)
    ps.add_argument("--cross-factor", type=float, default=0.1)
    ps.add_argument("--z-offset", type=float, default=2.0)
    ps.add_argument("--settle", type=int, default=10)
    ps.add_argument("--retries", type=int, default=3)
    ps.add_argument("--verify-frames", type=int, default=40)
    # shared
    ps.add_argument("--seed", type=int, help="reproducible placement and TM choices")
    ps.add_argument("--no-sync", action="store_true",
                    help="leave an asynchronous world asynchronous (TM misbehaves)")
    ps.add_argument("--delta", type=float, default=0.05, help="fixed timestep if we go sync")
    ps.add_argument("--hold", action="store_true",
                    help="stay alive ticking until Ctrl+C (else the scene stops on exit)")
    ps.add_argument("--seconds", type=float, default=0.0,
                    help="hold for N simulated seconds, then clean up and exit")
    ps.add_argument("--keep", action="store_true",
                    help="on release, leave the actors in the world")
    ps.set_defaults(func=cmd_spawn)

    sub.add_parser("destroy", help="remove all walkers (safe order) and vehicles") \
        .set_defaults(func=cmd_destroy)

    args = p.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
