#!/usr/bin/env python3
"""Read and change a running CARLA server's WorldSettings, then verify.

Commands:

    show                          print the current WorldSettings (+ derived fps)
    sync   --fps 20               enable synchronous mode at a fixed step
    sync   --delta 0.05           same, giving the step directly (seconds)
    async                         restore free-running (asynchronous) mode
    set    --no-rendering on      change individual fields without touching sync

`sync` and `async` also switch the **Traffic Manager** to the matching mode, so
the two never drift apart: a synchronous world with an asynchronous TM produces
non-deterministic, jittery traffic, and an async world with a sync TM leaves the
TM waiting for ticks that never come. Coupling is automatic here.

Every change is applied with world.apply_settings() and then read back, because
apply_settings returns a frame id, not confirmation of the resulting state.

Connection + TM port come from the environment (see env.sh): CARLA_HOST,
CARLA_PORT, CARLA_TIMEOUT, TM_PORT.
"""
from __future__ import annotations

import argparse
import math
import os
import sys

import carla  # provided by the active interpreter; check_env.sh verifies this


def _fresh(world):
    """A world handle that already holds a snapshot.

    In synchronous mode a freshly connected client has not seen a frame yet, so
    `get_actors()` comes back EMPTY and every --id/--filter lookup reports "no
    matching actor" while the scene is full of them. Observed against a live
    world holding 48 vehicles. One frame of waiting is the whole fix, and it
    must be a wait rather than a tick: another client owns that clock.
    """
    if world.get_settings().synchronous_mode:
        world.wait_for_tick()
    return world


def _client() -> carla.Client:
    client = carla.Client(os.environ.get("CARLA_HOST", "127.0.0.1"),
                          int(os.environ.get("CARLA_PORT", "2000")))
    client.set_timeout(float(os.environ.get("CARLA_TIMEOUT", "60.0")))
    return client


def _tm(client: carla.Client) -> carla.TrafficManager:
    return client.get_trafficmanager(int(os.environ.get("TM_PORT", "8000")))


def _report(world: carla.World, tm_mode: "bool | None", note: str) -> None:
    s = world.get_settings()
    fps = (1.0 / s.fixed_delta_seconds) if s.fixed_delta_seconds else None
    print(f"\nVERIFY {note}")
    print(f"  synchronous_mode      = {s.synchronous_mode}")
    print(f"  fixed_delta_seconds   = {s.fixed_delta_seconds}"
          + (f"  (~{fps:.1f} fps)" if fps else "  (variable step)"))
    print(f"  no_rendering_mode     = {s.no_rendering_mode}")
    print(f"  substepping           = {s.substepping}")
    print(f"  max_substep_delta_time= {s.max_substep_delta_time}")
    print(f"  max_substeps          = {s.max_substeps}")
    print(f"  max_culling_distance  = {s.max_culling_distance}")
    print(f"  deterministic_ragdolls= {s.deterministic_ragdolls}")
    print(f"  tile_stream_distance  = {s.tile_stream_distance}")
    print(f"  actor_active_distance = {s.actor_active_distance}")
    print(f"  spectator_as_ego      = {s.spectator_as_ego}")
    if tm_mode is not None:
        print(f"  traffic_manager sync  = {tm_mode}  (port {os.environ.get('TM_PORT','8000')})")


def cmd_show(_: argparse.Namespace) -> None:
    world = _fresh(_client().get_world())
    _report(world, None, "(current settings)")


def cmd_sync(args: argparse.Namespace) -> None:
    if args.fps is not None and args.delta is not None:
        sys.exit("give only one of --fps / --delta")
    delta = args.delta if args.delta is not None else (1.0 / args.fps if args.fps else 0.05)
    if delta <= 0:
        sys.exit("fixed step must be > 0")

    client = _client()
    world = _fresh(client.get_world())
    s = world.get_settings()
    s.synchronous_mode = True
    s.fixed_delta_seconds = delta
    if args.no_rendering:
        s.no_rendering_mode = True

    # Physics substepping must cover the whole frame: max_substep_delta_time *
    # max_substeps >= fixed_delta_seconds, or physics is unstable/inconsistent
    # (CARLA's documented rule; defaults 0.01 * 10 = 0.1s). If the requested step
    # exceeds the current budget, raise max_substeps to satisfy it and say so.
    if s.substepping and s.max_substep_delta_time * s.max_substeps < delta - 1e-9:
        needed = math.ceil(delta / s.max_substep_delta_time)
        print(f"note: raising max_substeps {s.max_substeps} -> {needed} so "
              f"{s.max_substep_delta_time}*{needed} >= {delta} (substep budget)")
        s.max_substeps = needed
    if delta > 0.1 + 1e-9:
        print(f"warn: fixed_delta_seconds {delta} > 0.1 — CARLA recommends <= 0.1 "
              f"for stable physics; consider a higher fps")

    world.apply_settings(s)
    _tm(client).set_synchronous_mode(True)   # keep TM in lockstep with the world
    _report(world, True, f"(sync on, step {delta}s)")
    print("  reminder: in sync mode the world only advances when you call world.tick()")


def cmd_async(args: argparse.Namespace) -> None:
    client = _client()
    world = _fresh(client.get_world())
    s = world.get_settings()
    s.synchronous_mode = False
    s.fixed_delta_seconds = None   # None = variable, server-driven time step
    if args.no_rendering:
        s.no_rendering_mode = True
    world.apply_settings(s)
    _tm(client).set_synchronous_mode(False)  # free the TM to self-tick
    _report(world, False, "(async restored)")
    print("  the server now self-ticks; safe to leave it here on exit")


def cmd_set(args: argparse.Namespace) -> None:
    world = _fresh(_client().get_world())
    s = world.get_settings()
    changed = []
    if args.no_rendering is not None:
        s.no_rendering_mode = (args.no_rendering == "on"); changed.append("no_rendering_mode")
    if args.substepping is not None:
        s.substepping = (args.substepping == "on"); changed.append("substepping")
    if args.substeps is not None:
        s.max_substeps = args.substeps; changed.append("max_substeps")
    if args.substep_dt is not None:
        s.max_substep_delta_time = args.substep_dt; changed.append("max_substep_delta_time")
    if args.fixed_delta is not None:
        s.fixed_delta_seconds = None if args.fixed_delta.lower() == "none" else float(args.fixed_delta)
        changed.append("fixed_delta_seconds")
    if args.max_culling_distance is not None:
        s.max_culling_distance = args.max_culling_distance; changed.append("max_culling_distance")
    if args.deterministic_ragdolls is not None:
        s.deterministic_ragdolls = (args.deterministic_ragdolls == "on"); changed.append("deterministic_ragdolls")
    if args.tile_stream_distance is not None:
        s.tile_stream_distance = args.tile_stream_distance; changed.append("tile_stream_distance")
    if args.actor_active_distance is not None:
        s.actor_active_distance = args.actor_active_distance; changed.append("actor_active_distance")
    if args.spectator_as_ego is not None:
        s.spectator_as_ego = (args.spectator_as_ego == "on"); changed.append("spectator_as_ego")
    if not changed:
        sys.exit("set needs at least one field flag (see --help)")
    world.apply_settings(s)
    _report(world, None, f"(set {', '.join(changed)}) — TM mode left unchanged")


def main() -> None:
    p = argparse.ArgumentParser(description="Read/change CARLA WorldSettings and verify.")
    sub = p.add_subparsers(dest="cmd", required=True)

    sub.add_parser("show", help="print current settings").set_defaults(func=cmd_show)

    ps = sub.add_parser("sync", help="enable synchronous mode (+ TM sync)")
    ps.add_argument("--fps", type=float, help="fixed rate in frames/sec (default 20)")
    ps.add_argument("--delta", type=float, help="fixed step in seconds (alternative to --fps)")
    ps.add_argument("--no-rendering", action="store_true", help="also disable rendering")
    ps.set_defaults(func=cmd_sync)

    pa = sub.add_parser("async", help="restore asynchronous mode (+ TM async)")
    pa.add_argument("--no-rendering", action="store_true", help="also disable rendering")
    pa.set_defaults(func=cmd_async)

    pt = sub.add_parser("set", help="change individual fields (does not touch sync/TM)")
    pt.add_argument("--no-rendering", choices=("on", "off"))
    pt.add_argument("--substepping", choices=("on", "off"))
    pt.add_argument("--substeps", type=int)
    pt.add_argument("--substep-dt", type=float)
    pt.add_argument("--fixed-delta", help="seconds, or 'none' for variable")
    pt.add_argument("--max-culling-distance", type=float, help="actor render-cull distance in m (0 = off)")
    pt.add_argument("--deterministic-ragdolls", choices=("on", "off"))
    pt.add_argument("--tile-stream-distance", type=float, help="large-map tile streaming distance in m")
    pt.add_argument("--actor-active-distance", type=float, help="large-map actor activation distance in m")
    pt.add_argument("--spectator-as-ego", choices=("on", "off"))
    pt.set_defaults(func=cmd_set)

    args = p.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
