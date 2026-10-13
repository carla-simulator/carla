#!/usr/bin/env python3
"""Control the map's light sources via the LightManager: street/building/vehicle.

Commands:
    list  [--group all|street|building|vehicle|other]   count / show lights
    on    --group street [--color 255,220,150] [--intensity 2000]   turn on (+set)
    off   --group street                                            turn off
    set   --group building [--color ...] [--intensity ...]  change without toggling
    day-night on|off                        auto lights-at-night on/off

Light groups: `street` (street lamps), `building` (window/facade lights),
`vehicle` (car lights), `other`; `all` covers every group. These are the world's
light SOURCES (illumination) — distinct from toggle-env-objects (which hides the
lamp *meshes*) and from set-weather (the sun). Colour is r,g,b (0-255);
intensity is in lumens-ish units (street lamps ~1000-3000).

Connection from env.sh: CARLA_HOST/PORT/TIMEOUT.
"""
from __future__ import annotations

import argparse
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


def _done(msg):
    """Print, flush, and hard-exit.

    LightManager write ops (turn_on/off/set_*) succeed, but destroying the `Light`
    python objects after the RPC episode tears down aborts the process with a C++
    "operate on a destroyed actor" (a CARLA teardown-order bug; read-only paths are
    unaffected). We flush the result and os._exit past those destructors.
    """
    print(msg)
    sys.stdout.flush()
    os._exit(0)

GROUPS = {"all": carla.LightGroup.NONE, "street": carla.LightGroup.Street,
          "building": carla.LightGroup.Building, "vehicle": carla.LightGroup.Vehicle,
          "other": carla.LightGroup.Other}


def _client():
    c = carla.Client(os.environ.get("CARLA_HOST", "127.0.0.1"),
                     int(os.environ.get("CARLA_PORT", "2000")))
    c.set_timeout(float(os.environ.get("CARLA_TIMEOUT", "60.0")))
    return c


# NOTE: the LightManager is only valid while its Client is alive — if the client
# is garbage-collected the manager operates on a destroyed episode. So every
# command keeps `client` in local scope for the whole call (do not factor the
# client away and return only the manager).
def _lights(lm, group):
    return lm.get_all_lights(GROUPS[group])


def cmd_list(args):
    client = _client()
    lm = _fresh(client.get_world()).get_lightmanager()
    lights = _lights(lm, args.group)
    on = sum(1 for l in lights if l.is_on)
    print(f"{len(lights)} '{args.group}' light(s): {on} on, {len(lights)-on} off")
    for l in lights[:15]:
        print(f"  id={l.id:5d} on={l.is_on!s:5s} intensity={l.intensity:.0f} "
              f"color=({l.color.r},{l.color.g},{l.color.b}) at ({l.location.x:.0f},{l.location.y:.0f})")
    if len(lights) > 15:
        print(f"  ... and {len(lights)-15} more")


def _color(s):
    r, g, b = (int(v) for v in s.split(","))
    return carla.Color(r, g, b)


def cmd_on(args):
    client = _client()
    lm = _fresh(client.get_world()).get_lightmanager()
    lights = _lights(lm, args.group)
    lm.turn_on(lights)
    if args.color:
        lm.set_color(lights, _color(args.color))
    if args.intensity is not None:
        lm.set_intensity(lights, args.intensity)
    _done(f"turned ON {len(lights)} '{args.group}' lights"
          + (f", color={args.color}" if args.color else "")
          + (f", intensity={args.intensity}" if args.intensity is not None else ""))


def cmd_off(args):
    client = _client()
    lm = _fresh(client.get_world()).get_lightmanager()
    lights = _lights(lm, args.group)
    lm.turn_off(lights)
    _done(f"turned OFF {len(lights)} '{args.group}' lights")


def cmd_set(args):
    client = _client()
    lm = _fresh(client.get_world()).get_lightmanager()
    lights = _lights(lm, args.group)
    if not args.color and args.intensity is None:
        raise SystemExit("set needs --color and/or --intensity")
    if args.color:
        lm.set_color(lights, _color(args.color))
    if args.intensity is not None:
        lm.set_intensity(lights, args.intensity)
    _done(f"set {len(lights)} '{args.group}' lights"
          + (f", color={args.color}" if args.color else "")
          + (f", intensity={args.intensity}" if args.intensity is not None else ""))


def cmd_day_night(args):
    client = _client()
    client.get_world().get_lightmanager().set_day_night_cycle(args.mode == "on")
    print(f"day-night cycle = {args.mode == 'on'} (lights auto-switch with the sun when on)")


def main() -> None:
    p = argparse.ArgumentParser(description="Control CARLA light sources.")
    sub = p.add_subparsers(dest="cmd", required=True)

    def grp(sp, required=False):
        sp.add_argument("--group", choices=tuple(GROUPS), default="all")
        return sp

    grp(sub.add_parser("list", help="list lights in a group")).set_defaults(func=cmd_list)

    pon = grp(sub.add_parser("on", help="turn on (and optionally colour/dim)"))
    pon.add_argument("--color"); pon.add_argument("--intensity", type=float)
    pon.set_defaults(func=cmd_on)

    grp(sub.add_parser("off", help="turn off")).set_defaults(func=cmd_off)

    pset = grp(sub.add_parser("set", help="change colour/intensity without toggling"))
    pset.add_argument("--color"); pset.add_argument("--intensity", type=float)
    pset.set_defaults(func=cmd_set)

    pdn = sub.add_parser("day-night", help="auto lights-at-night on/off")
    pdn.add_argument("mode", choices=("on", "off"))
    pdn.set_defaults(func=cmd_day_night)

    args = p.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
