#!/usr/bin/env python3
"""Interrogate a recorded CARLA .log without replaying it.

Commands:

    info      --file run.log [--all]           header, actors, per-frame if --all
    collisions --file run.log --type1 v --type2 a   collisions between categories
    blocked   --file run.log --min-time 30 --min-distance 100   stuck actors

All three ask the SERVER to parse the log and return a human-readable report;
they do not change the world. Category codes for `collisions` (type1/type2):

    h = hero   v = vehicle   w = walker   t = traffic light   o = other   a = any

`blocked` finds actors that moved less than --min-distance (CENTIMETRES) over at
least --min-time (seconds) — i.e. vehicles stuck at a junction or wedged.

Connection comes from the environment (see env.sh): CARLA_HOST/PORT/TIMEOUT.
"""
from __future__ import annotations

import argparse
import os

import carla  # provided by the active interpreter; check_env.sh verifies this

CATEGORIES = {"h", "v", "w", "t", "o", "a"}


def _client() -> carla.Client:
    client = carla.Client(os.environ.get("CARLA_HOST", "127.0.0.1"),
                          int(os.environ.get("CARLA_PORT", "2000")))
    client.set_timeout(float(os.environ.get("CARLA_TIMEOUT", "60.0")))
    return client


def cmd_info(args: argparse.Namespace) -> None:
    # show_all=True prints every frame — huge on long logs, so it is opt-in.
    print(_client().show_recorder_file_info(args.file, args.all))


def cmd_collisions(args: argparse.Namespace) -> None:
    for t in (args.type1, args.type2):
        if t not in CATEGORIES:
            raise SystemExit(f"category {t!r} invalid; use one of {sorted(CATEGORIES)} "
                             "(h hero, v vehicle, w walker, t traffic-light, o other, a any)")
    print(_client().show_recorder_collisions(args.file, args.type1, args.type2))


def cmd_blocked(args: argparse.Namespace) -> None:
    # min_distance is in centimetres (CARLA's unit for this query), min_time in s.
    print(_client().show_recorder_actors_blocked(args.file, args.min_time, args.min_distance))


def main() -> None:
    p = argparse.ArgumentParser(description="Query a CARLA recording (no replay).")
    sub = p.add_subparsers(dest="cmd", required=True)

    pi = sub.add_parser("info", help="show recorder file info")
    pi.add_argument("--file", required=True, help="server-side .log path")
    pi.add_argument("--all", action="store_true", help="include every frame (large output)")
    pi.set_defaults(func=cmd_info)

    pc = sub.add_parser("collisions", help="list recorded collisions between two categories")
    pc.add_argument("--file", required=True, help="server-side .log path")
    pc.add_argument("--type1", required=True, help="first category (h/v/w/t/o/a)")
    pc.add_argument("--type2", required=True, help="second category (h/v/w/t/o/a)")
    pc.set_defaults(func=cmd_collisions)

    pb = sub.add_parser("blocked", help="list actors that stayed still")
    pb.add_argument("--file", required=True, help="server-side .log path")
    pb.add_argument("--min-time", type=float, default=30.0, help="seconds considered blocked (default 30)")
    pb.add_argument("--min-distance", type=float, default=100.0,
                    help="centimetres it must move to count as moving (default 100 = 1 m)")
    pb.set_defaults(func=cmd_blocked)

    args = p.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
