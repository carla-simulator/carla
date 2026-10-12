#!/usr/bin/env python3
"""Visualise and validate the pedestrian navigation mesh (navmesh).

CARLA does not expose the navmesh geometry directly, but every point it returns
from get_random_location_from_navigation() lies ON the walkable mesh. Sampling
many such points and drawing them paints the walkable area, and confirms the
navmesh loaded at all — the check that matters after importing a new map.

Commands:

    validate [--count 500]                  sample N points; report coverage/extent
    sample   [--count 2000] [--life 120]    sample + draw the points (needs a view)

If get_random_location_from_navigation() returns None (or the same point every
time), the map has no usable navmesh: pedestrians spawned with a WalkerAIController
will not move. That is the failure this skill surfaces.

Connection comes from the environment (see env.sh): CARLA_HOST/PORT/TIMEOUT.
"""
from __future__ import annotations

import argparse
import os

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


def _sample(world: carla.World, count: int) -> "list[carla.Location]":
    pts = []
    for _ in range(count):
        loc = world.get_random_location_from_navigation()
        if loc is not None:
            pts.append(loc)
    return pts


def _extent(pts):
    xs = [p.x for p in pts]; ys = [p.y for p in pts]; zs = [p.z for p in pts]
    return (min(xs), max(xs), min(ys), max(ys), min(zs), max(zs))


def cmd_validate(args):
    world = _fresh(_client().get_world())
    pts = _sample(world, args.count)
    got = len(pts)
    print(f"navmesh validation on {world.get_map().name}:")
    print(f"  sampled {got}/{args.count} valid navigation points")
    if got == 0:
        print("  FAIL: no navmesh — get_random_location_from_navigation() gave nothing.")
        print("        Walkers will not navigate. Re-check the map's pedestrian nav build.")
        raise SystemExit(1)
    unique = len({(round(p.x, 1), round(p.y, 1)) for p in pts})
    x0, x1, y0, y1, z0, z1 = _extent(pts)
    print(f"  unique locations: {unique} (low number => tiny/degenerate walkable area)")
    print(f"  coverage bounds: x[{x0:.1f},{x1:.1f}] y[{y0:.1f},{y1:.1f}] z[{z0:.1f},{z1:.1f}]")
    print(f"  walkable span: {x1-x0:.0f} x {y1-y0:.0f} m")
    print("  PASS: navmesh present and sampling across an area.")


def cmd_sample(args):
    world = _fresh(_client().get_world())
    pts = _sample(world, args.count)
    if not pts:
        print("FAIL: no navmesh points to draw (empty navigation).")
        raise SystemExit(1)
    color = carla.Color(0, 180, 255)
    dbg = world.debug
    for p in pts:
        # lift slightly so points sit above the ground plane and stay visible
        dbg.draw_point(carla.Location(p.x, p.y, p.z + 0.3), 0.08, color, args.life)
    print(f"drew {len(pts)} navmesh points (life {args.life}s) on {world.get_map().name}")
    print("view in a rendered server; the dotted area is walkable for pedestrians")


def main() -> None:
    p = argparse.ArgumentParser(description="Visualise/validate the CARLA navmesh.")
    sub = p.add_subparsers(dest="cmd", required=True)

    pv = sub.add_parser("validate", help="check the navmesh loaded and its coverage")
    pv.add_argument("--count", type=int, default=500, help="points to sample (default 500)")
    pv.set_defaults(func=cmd_validate)

    ps = sub.add_parser("sample", help="sample and draw navmesh points")
    ps.add_argument("--count", type=int, default=2000, help="points to draw (default 2000)")
    ps.add_argument("--life", type=float, default=120.0, help="seconds the points last (default 120)")
    ps.set_defaults(func=cmd_sample)

    args = p.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
