#!/usr/bin/env python3
"""Control traffic-light actors: state, freeze, timing — all, by junction, or one.

Commands:
    list                                   every traffic light (id, state, location)
    set   --state green|red|yellow|off <selector>       set the light state
    timing [--green S --yellow S --red S] <selector>    set phase durations
    freeze on|off [--state green]          freeze/unfreeze ALL lights (optionally
                                           forcing a state first, e.g. all green)
    reset                                  reset all lights to their normal cycle

Selector (for set/timing): --all | --id N | --junction J (lights in that junction)
| --near X,Y,Z (the nearest one). Junction ids come from the map-waypoints skill.

This controls the actual `traffic.traffic_light` actors — NOT the Traffic Manager
(that is control-traffic, which governs how autopilot *vehicles* behave).
Connection from env.sh: CARLA_HOST/PORT/TIMEOUT.
"""
from __future__ import annotations

import argparse
import os

import carla  # provided by the active interpreter; check_env.sh verifies this

STATES = {"green": carla.TrafficLightState.Green, "red": carla.TrafficLightState.Red,
          "yellow": carla.TrafficLightState.Yellow, "off": carla.TrafficLightState.Off}


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


def _world():
    c = carla.Client(os.environ.get("CARLA_HOST", "127.0.0.1"),
                     int(os.environ.get("CARLA_PORT", "2000")))
    c.set_timeout(float(os.environ.get("CARLA_TIMEOUT", "60.0")))
    return _fresh(c.get_world())


def _select(world, args):
    lights = list(world.get_actors().filter("traffic.traffic_light"))
    if getattr(args, "id", None) is not None:
        lights = [l for l in lights if l.id == args.id]
    elif getattr(args, "junction", None) is not None:
        lights = list(world.get_traffic_lights_in_junction(args.junction))
    elif getattr(args, "near", None):
        x, y, z = (float(v) for v in args.near.split(","))
        ref = carla.Location(x, y, z)
        lights = [min(lights, key=lambda l: l.get_transform().location.distance(ref))] if lights else []
    elif not getattr(args, "all", False):
        raise SystemExit("need a selector: --all | --id N | --junction J | --near X,Y,Z")
    return lights


def cmd_list(_):
    lights = list(_world().get_actors().filter("traffic.traffic_light"))
    print(f"{len(lights)} traffic light(s):")
    for l in lights:
        loc = l.get_transform().location
        print(f"  id={l.id:6d}  state={str(l.get_state()).split('.')[-1]:7s} "
              f"({loc.x:.0f},{loc.y:.0f})  green={l.get_green_time():.0f}s "
              f"yellow={l.get_yellow_time():.0f}s red={l.get_red_time():.0f}s")


def cmd_set(args):
    world = _world()
    lights = _select(world, args)
    st = STATES[args.state]
    for l in lights:
        l.set_state(st)
    print(f"set {len(lights)} light(s) to {args.state}")


def cmd_timing(args):
    world = _world()
    lights = _select(world, args)
    if args.green is None and args.yellow is None and args.red is None:
        raise SystemExit("timing needs at least one of --green/--yellow/--red")
    for l in lights:
        if args.green is not None:
            l.set_green_time(args.green)
        if args.yellow is not None:
            l.set_yellow_time(args.yellow)
        if args.red is not None:
            l.set_red_time(args.red)
    print(f"set timing on {len(lights)} light(s): "
          f"green={args.green} yellow={args.yellow} red={args.red}")


def cmd_freeze(args):
    world = _world()
    if args.mode == "on" and args.state:
        for l in world.get_actors().filter("traffic.traffic_light"):
            l.set_state(STATES[args.state])
    world.freeze_all_traffic_lights(args.mode == "on")
    extra = f" (all forced {args.state} first)" if (args.mode == "on" and args.state) else ""
    print(f"traffic lights frozen = {args.mode == 'on'}{extra}")


def cmd_reset(_):
    _world().reset_all_traffic_lights()
    print("reset all traffic lights to their normal cycle")


def _sel(sp):
    sp.add_argument("--all", action="store_true"); sp.add_argument("--id", type=int)
    sp.add_argument("--junction", type=int); sp.add_argument("--near")
    return sp


def main() -> None:
    p = argparse.ArgumentParser(description="Control CARLA traffic lights.")
    sub = p.add_subparsers(dest="cmd", required=True)

    sub.add_parser("list", help="list all traffic lights").set_defaults(func=cmd_list)

    ps = _sel(sub.add_parser("set", help="set light state"))
    ps.add_argument("--state", required=True, choices=tuple(STATES))
    ps.set_defaults(func=cmd_set)

    pt = _sel(sub.add_parser("timing", help="set phase durations"))
    pt.add_argument("--green", type=float); pt.add_argument("--yellow", type=float)
    pt.add_argument("--red", type=float)
    pt.set_defaults(func=cmd_timing)

    pf = sub.add_parser("freeze", help="freeze/unfreeze all lights")
    pf.add_argument("mode", choices=("on", "off"))
    pf.add_argument("--state", choices=tuple(STATES), help="force all to this state first")
    pf.set_defaults(func=cmd_freeze)

    sub.add_parser("reset", help="reset all lights to normal cycle").set_defaults(func=cmd_reset)

    args = p.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
