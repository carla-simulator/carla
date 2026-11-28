#!/usr/bin/env python3
"""List and draw the traffic signs of the loaded map, with their trigger volumes.

Commands:

    list  [--type stop,yield,speed_limit,unknown] [--lights] [--near X,Y,Z --radius 50]
    draw  [same filters] [--life 120] [--no-labels]

Every `traffic.*` actor except traffic lights is a sign (speed limits, stops,
yields, and `traffic.unknown`); `--lights` adds the traffic lights, whose
trigger volumes work the same way. `draw` outlines each trigger volume (the box
a vehicle must be inside to be affected by the sign), draws a line from the box
to the sign itself, and labels the sign. Signs do not move, so it draws once
with --life; debug-draw `clear` removes the drawing early.

Trigger volume frame: `trigger_volume` is in the ACTOR's frame (location
relative to the sign, extent along the sign's axes), so it is drawn at
`transform.transform(location)` with the actor's rotation. Mind that
`carla.Transform.transform()` rewrites the location passed to it in place as
well as returning it, so transform a copy: transforming the live attribute twice
puts the box tens of metres away (measured: a yield trigger landed 58 m off).

Connection comes from the environment (see env.sh): CARLA_HOST/PORT/TIMEOUT.
"""
from __future__ import annotations

import argparse
import math
import os

import carla  # provided by the active interpreter; check_env.sh verifies this


def _rgb(s: str) -> carla.Color:
    """--color r,g,b (0-255 each)."""
    try:
        r, g, b = (int(v) for v in s.split(","))
    except ValueError:
        raise argparse.ArgumentTypeError(f"expected r,g,b but got {s!r}")
    return carla.Color(r, g, b)

_COLORS = {
    "speed_limit": carla.Color(255, 140, 0),
    "stop": carla.Color(255, 0, 0),
    "yield": carla.Color(255, 220, 0),
    "traffic_light": carla.Color(0, 200, 255),
    "unknown": carla.Color(170, 170, 170),
}
_SIGN_HEIGHT = 2.0   # the plate sits about this high above the actor's origin (its base)


def _world():
    c = carla.Client(os.environ.get("CARLA_HOST", "127.0.0.1"),
                     int(os.environ.get("CARLA_PORT", "2000")))
    c.set_timeout(float(os.environ.get("CARLA_TIMEOUT", "60.0")))
    world = c.get_world()
    if world.get_settings().synchronous_mode:
        world.wait_for_tick()   # a fresh client in sync mode has no snapshot yet
    return world


def _kind(type_id: str) -> str:
    rest = type_id.split(".", 1)[1] if "." in type_id else type_id   # traffic.<rest>
    return "speed_limit" if rest.startswith("speed_limit") else rest


def _label(actor) -> str:
    kind = _kind(actor.type_id)
    if kind == "speed_limit":
        return f"SPEED {actor.type_id.rsplit('.', 1)[-1]} #{actor.id}"
    return f"{kind.replace('_', ' ').upper()} #{actor.id}"


def _trigger(actor):
    """(centre, extent, rotation) of the trigger volume in world coordinates."""
    tf = actor.get_transform()
    tv = actor.trigger_volume
    local = carla.Location(tv.location.x, tv.location.y, tv.location.z)  # a copy: transform() mutates
    centre = tf.transform(local)
    rot = carla.Rotation(pitch=tf.rotation.pitch + tv.rotation.pitch,
                         yaw=tf.rotation.yaw + tv.rotation.yaw,
                         roll=tf.rotation.roll + tv.rotation.roll)
    return carla.Location(centre.x, centre.y, centre.z), tv.extent, rot


def _select(world, args):
    actors = [a for a in world.get_actors().filter("traffic.*")
              if args.lights or a.type_id != "traffic.traffic_light"]
    if args.type:
        want = {t.strip() for t in args.type.split(",") if t.strip()}
        actors = [a for a in actors if _kind(a.type_id) in want]
    if args.near:
        x, y, _z = (float(v) for v in args.near.split(","))
        actors = [a for a in actors
                  if math.hypot(a.get_location().x - x, a.get_location().y - y) <= args.radius]
        actors.sort(key=lambda a: math.hypot(a.get_location().x - x, a.get_location().y - y))
    return actors


def cmd_list(args):
    world = _world()
    actors = _select(world, args)
    print(f"{len(actors)} sign(s) on {world.get_map().name}:")
    for a in actors:
        loc = a.get_location()
        c, e, _ = _trigger(a)
        print(f"  {_label(a):22s} {a.type_id:28s} sign=({loc.x:.0f},{loc.y:.0f},{loc.z:.1f})  "
              f"trigger centre=({c.x:.0f},{c.y:.0f})  size={e.x*2:.1f}x{e.y*2:.1f}x{e.z*2:.1f}m  "
              f"{c.distance(loc):.1f}m from the sign")


def cmd_draw(args):
    world = _world()
    actors = _select(world, args)
    if not actors:
        raise SystemExit("no signs matched (see: list)")
    dbg = world.debug
    for a in actors:
        col = args.color or _COLORS.get(_kind(a.type_id), _COLORS["unknown"])
        centre, extent, rot = _trigger(a)
        dbg.draw_box(carla.BoundingBox(centre, extent), rot, 0.1, col, args.life)
        sign = a.get_location() + carla.Location(z=_SIGN_HEIGHT)
        top = centre + carla.Location(z=extent.z)
        if top.distance(sign) > 0.5:
            dbg.draw_line(top, sign, 0.05, col, args.life)
        dbg.draw_point(sign, 0.15, col, args.life)
        if not args.no_labels:
            dbg.draw_string(sign + carla.Location(z=0.6), _label(a), False, col, args.life)
    print(f"drew {len(actors)} trigger volume(s), each linked to its sign (life {args.life:g}s; "
          "view on a rendered server; debug-draw clear removes them early)")


def main() -> None:
    p = argparse.ArgumentParser(description="List/draw traffic signs and their trigger volumes.")
    sub = p.add_subparsers(dest="cmd", required=True)

    def filters(sp):
        sp.add_argument("--type", help="comma list of kinds: speed_limit, stop, yield, unknown"
                                       " (traffic_light with --lights); default all")
        sp.add_argument("--lights", action="store_true", help="include traffic lights")
        sp.add_argument("--near", help="X,Y,Z — only signs within --radius of it, nearest first")
        sp.add_argument("--radius", type=float, default=50.0)
        return sp

    filters(sub.add_parser("list", help="every sign: kind, position, trigger box")).set_defaults(func=cmd_list)
    pd = filters(sub.add_parser("draw", help="outline trigger volumes and link them to their signs"))
    pd.add_argument("--life", type=float, default=120.0, help="seconds the drawing stays (default 120)")
    pd.add_argument("--no-labels", action="store_true")
    pd.add_argument("--color", type=_rgb, help="r,g,b for every sign (default: one colour per kind)")
    pd.set_defaults(func=cmd_draw)

    args = p.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
