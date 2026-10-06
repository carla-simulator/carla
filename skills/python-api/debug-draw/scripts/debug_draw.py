#!/usr/bin/env python3
"""Draw debug primitives in the CARLA world: points, lines, arrows, boxes, text.

Commands (all take a --life in seconds and an optional --color r,g,b):

    point  --at X,Y,Z [--size 0.1]
    line   --from X,Y,Z --to X,Y,Z [--thickness 0.1]
    arrow  --from X,Y,Z --to X,Y,Z [--thickness 0.1 --arrow-size 0.2]
    box    --center X,Y,Z --extent EX,EY,EZ [--yaw 0 --thickness 0.1]
    text   --at X,Y,Z --text "hello"

Debug shapes are an overlay drawn by the server; they are not actors and cannot
be queried or individually removed — they simply expire after --life seconds.

Lifetime & sync mode (see references/debug.md): shapes are rendered on a world
tick. In asynchronous mode the server ticks itself, so a shape appears right away
and lasts --life seconds. In synchronous mode nothing renders until the client
ticks, and --life is counted in simulation time — so a persistent overlay means
redrawing each frame (or using a --life longer than your tick cadence).

Connection comes from the environment (see env.sh): CARLA_HOST/PORT/TIMEOUT.
"""
from __future__ import annotations

import argparse
import os

import carla  # provided by the active interpreter; check_env.sh verifies this


def _client() -> carla.Client:
    client = carla.Client(os.environ.get("CARLA_HOST", "127.0.0.1"),
                          int(os.environ.get("CARLA_PORT", "2000")))
    client.set_timeout(float(os.environ.get("CARLA_TIMEOUT", "60.0")))
    return client


def _xyz(s: str) -> "tuple[float, float, float]":
    try:
        x, y, z = (float(v) for v in s.split(","))
    except ValueError:
        raise SystemExit(f"expected X,Y,Z but got {s!r}")
    return x, y, z


def _loc(s: str) -> carla.Location:
    x, y, z = _xyz(s)
    return carla.Location(x=x, y=y, z=z)


def _color(s: str) -> carla.Color:
    r, g, b = (int(v) for v in s.split(","))
    return carla.Color(r, g, b)


def _debug(args):
    return _client().get_world().debug


def cmd_point(args):
    _debug(args).draw_point(_loc(args.at), args.size, args.color, args.life)
    print(f"point at {args.at} size={args.size} life={args.life}s")


def cmd_line(args):
    _debug(args).draw_line(_loc(getattr(args, "from")), _loc(args.to), args.thickness, args.color, args.life)
    print(f"line {getattr(args,'from')} -> {args.to} life={args.life}s")


def cmd_arrow(args):
    _debug(args).draw_arrow(_loc(getattr(args, "from")), _loc(args.to),
                            args.thickness, args.arrow_size, args.color, args.life)
    print(f"arrow {getattr(args,'from')} -> {args.to} life={args.life}s")


def cmd_box(args):
    cx, cy, cz = _xyz(args.center)
    ex, ey, ez = _xyz(args.extent)
    box = carla.BoundingBox(carla.Location(cx, cy, cz), carla.Vector3D(ex, ey, ez))
    _debug(args).draw_box(box, carla.Rotation(yaw=args.yaw), args.thickness, args.color, args.life)
    print(f"box center={args.center} extent={args.extent} yaw={args.yaw} life={args.life}s")


def cmd_text(args):
    _debug(args).draw_string(_loc(args.at), args.text, False, args.color, args.life)
    print(f"text {args.text!r} at {args.at} life={args.life}s")


def main() -> None:
    p = argparse.ArgumentParser(description="Draw debug primitives in CARLA.")
    # Shared options on every subcommand.
    def common(sp):
        sp.add_argument("--life", type=float, default=30.0, help="seconds the shape lasts (default 30)")
        sp.add_argument("--color", type=_color, default=carla.Color(255, 0, 0), help="r,g,b (default 255,0,0)")
        return sp
    sub = p.add_subparsers(dest="cmd", required=True)

    pp = common(sub.add_parser("point")); pp.add_argument("--at", required=True); pp.add_argument("--size", type=float, default=0.1); pp.set_defaults(func=cmd_point)
    pl = common(sub.add_parser("line")); pl.add_argument("--from", required=True); pl.add_argument("--to", required=True); pl.add_argument("--thickness", type=float, default=0.1); pl.set_defaults(func=cmd_line)
    pa = common(sub.add_parser("arrow")); pa.add_argument("--from", required=True); pa.add_argument("--to", required=True); pa.add_argument("--thickness", type=float, default=0.1); pa.add_argument("--arrow-size", type=float, default=0.2); pa.set_defaults(func=cmd_arrow)
    pb = common(sub.add_parser("box")); pb.add_argument("--center", required=True); pb.add_argument("--extent", required=True); pb.add_argument("--yaw", type=float, default=0.0); pb.add_argument("--thickness", type=float, default=0.1); pb.set_defaults(func=cmd_box)
    pt = common(sub.add_parser("text")); pt.add_argument("--at", required=True); pt.add_argument("--text", required=True); pt.set_defaults(func=cmd_text)

    args = p.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
