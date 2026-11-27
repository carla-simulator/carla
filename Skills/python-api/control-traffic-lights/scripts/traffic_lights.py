#!/usr/bin/env python3
"""Control traffic-light actors: state, freeze, timing — all, by junction, or one.

Commands:
    list                                   every traffic light (id, state, location)
    set   --state green|red|yellow|off <selector>       set the light state
    timing [--green S --yellow S --red S] <selector>    set phase durations
    freeze on|off [--state green]          freeze/unfreeze ALL lights (optionally
                                           forcing a state first, e.g. all green)
    reset                                  reset all lights to their normal cycle
    draw  [<selector>] [--seconds 120]     overlay each light's live data in the world
                                           (state, time in state, phase times, frozen);
                                           default selector --all

Selector (for set/timing): --all | --id N | --opendrive-id S (the .xodr signal id)
| --junction J (lights in that junction) | --near X,Y,Z (the nearest one).
Junction ids come from the map-waypoints skill.

This controls the actual `traffic.traffic_light` actors — NOT the Traffic Manager
(that is control-traffic, which governs how autopilot *vehicles* behave).
Connection from env.sh: CARLA_HOST/PORT/TIMEOUT.
"""
from __future__ import annotations

import argparse
import os

import carla  # provided by the active interpreter; check_env.sh verifies this


def _rgb(s: str) -> carla.Color:
    """--color r,g,b (0-255 each)."""
    try:
        r, g, b = (int(v) for v in s.split(","))
    except ValueError:
        raise argparse.ArgumentTypeError(f"expected r,g,b but got {s!r}")
    return carla.Color(r, g, b)

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
    elif getattr(args, "opendrive_id", None):
        tl = world.get_traffic_light_from_opendrive_id(args.opendrive_id)
        lights = [tl] if tl is not None else []
        if not lights:
            raise SystemExit(f"no traffic light with OpenDRIVE signal id {args.opendrive_id} (see list)")
    elif getattr(args, "junction", None) is not None:
        lights = list(world.get_traffic_lights_in_junction(args.junction))
    elif getattr(args, "near", None):
        x, y, z = (float(v) for v in args.near.split(","))
        ref = carla.Location(x, y, z)
        lights = [min(lights, key=lambda l: l.get_transform().location.distance(ref))] if lights else []
    elif not getattr(args, "all", False):
        raise SystemExit("need a selector: --all | --id N | --opendrive-id S | --junction J | --near X,Y,Z")
    return lights


def cmd_list(_):
    lights = list(_world().get_actors().filter("traffic.traffic_light"))
    print(f"{len(lights)} traffic light(s):")
    for l in lights:
        loc = l.get_transform().location
        frozen = " frozen" if l.is_frozen() else ""
        print(f"  id={l.id:6d}  odr={l.get_opendrive_id():>5s}  state={str(l.get_state()).split('.')[-1]:7s} "
              f"({loc.x:.0f},{loc.y:.0f})  green={l.get_green_time():.0f}s "
              f"yellow={l.get_yellow_time():.0f}s red={l.get_red_time():.0f}s{frozen}")


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


_STATE_COLOR = {"Green": carla.Color(0, 255, 0), "Yellow": carla.Color(255, 200, 0),
                "Red": carla.Color(255, 0, 0), "Off": carla.Color(128, 128, 128),
                "Unknown": carla.Color(128, 128, 128)}


def _for_seconds(world, seconds):
    """Yield one snapshot per frame until `seconds` of SIMULATED time pass (0 = forever).

    An observer: it waits for whoever owns the clock and never ticks, so it is
    safe next to a traffic holder. The budget is read off the world clock, not
    summed from deltas, so a slow client does not stretch it.
    """
    start = None
    while True:
        snap = world.wait_for_tick()
        now = snap.timestamp.elapsed_seconds
        if start is None:
            start = now
        elif seconds > 0 and now - start >= seconds:
            return
        yield snap


def cmd_draw(args):
    """Overlay every selected light's data, redrawn each frame so it follows the cycle.

    Per light: a bar across each lane it controls, on the stop line, in the
    light's colour, and a label above it -- id, OpenDRIVE id, state, seconds in
    the current state out of that state's duration, the green/yellow/red times
    and whether it is frozen. Each stamp lives about two frames, so the overlay
    tracks state changes and disappears as soon as this command stops.
    """
    if not any(getattr(args, k, None) not in (None, False)
               for k in ("all", "id", "opendrive_id", "junction", "near")):
        args.all = True
    world = _world()
    lights = _select(world, args)
    if not lights:
        raise SystemExit("no traffic lights matched")
    # Stop lines are static: query once. A light with no stop waypoint is
    # labelled at its pole instead.
    stops = {}
    for l in lights:
        wps = list(l.get_stop_waypoints())
        stops[l.id] = wps
    dbg = world.debug
    print(f"drawing {len(lights)} traffic light(s) for "
          f"{'ever (Ctrl+C)' if args.seconds <= 0 else f'{args.seconds:g}s'} "
          "— view on a rendered server; the overlay vanishes when this stops", flush=True)
    durations = {"Green": "get_green_time", "Yellow": "get_yellow_time"}
    try:
        for snap in _for_seconds(world, args.seconds):
            life = max(snap.timestamp.delta_seconds * 2.0, 0.05)
            for l in lights:
                st = str(l.get_state()).split(".")[-1]
                col = args.color or _STATE_COLOR.get(st, _STATE_COLOR["Unknown"])
                wps = stops[l.id]
                for wp in wps:
                    tf = wp.transform
                    right = tf.get_right_vector()
                    half = wp.lane_width / 2.0
                    base = tf.location + carla.Location(z=0.3)
                    dbg.draw_line(base - right * half, base + right * half, 0.3, col, life)
                if wps:
                    xs = [w.transform.location for w in wps]
                    at = carla.Location(sum(p.x for p in xs) / len(xs), sum(p.y for p in xs) / len(xs),
                                        sum(p.z for p in xs) / len(xs) + 2.5)
                else:
                    at = l.get_transform().location + carla.Location(z=4.0)
                # The timer only means something for the light whose turn it is.
                # A red light waiting in its junction group reads elapsed 0.0 the
                # whole time (measured on Town10HD_Opt), and red_time is just the
                # all-red clearance after yellow, not how long it stays red.
                dur = getattr(l, durations[st])() if st in ("Green", "Yellow") else 0.0
                timing = f"{l.get_elapsed_time():.1f}/{dur:.0f}s" if dur else ""
                label = (f"TL {l.id} [{l.get_opendrive_id()}] {st.upper()} {timing}"
                         f"  G{l.get_green_time():.0f} Y{l.get_yellow_time():.0f} R{l.get_red_time():.0f}"
                         + (" FROZEN" if l.is_frozen() else ""))
                dbg.draw_string(at, label, False, col, life)
    except KeyboardInterrupt:
        pass
    print(f"stopped drawing {len(lights)} traffic light(s)")


def _sel(sp):
    sp.add_argument("--all", action="store_true"); sp.add_argument("--id", type=int)
    sp.add_argument("--junction", type=int); sp.add_argument("--near")
    sp.add_argument("--opendrive-id", help="the light's OpenDRIVE signal id, as the .xodr and list show it")
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

    pd = _sel(sub.add_parser("draw", help="overlay each light's live state/timing in the world"))
    pd.add_argument("--seconds", type=float, default=120.0,
                    help="simulated seconds to keep drawing (0 = until Ctrl+C)")
    pd.add_argument("--color", type=_rgb, help="r,g,b for every light (default: its state colour)")
    pd.set_defaults(func=cmd_draw)

    args = p.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
