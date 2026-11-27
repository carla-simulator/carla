#!/usr/bin/env python3
"""Tune the Traffic Manager: how autopilot vehicles drive, globally or per vehicle.

Commands:
    global   [--speed-diff -30] [--distance 2.5] [--seed 42]
             [--hybrid on|off] [--hybrid-radius 70] [--osm on|off]
             [--lane-offset M] [--wide-turn on|off]
                                                  TM-wide settings
    vehicle  <selector> [per-vehicle flags]        settings for one vehicle
    all      [--filter vehicle.*] [per-vehicle flags]  same, for every match
    plan     <selector> [--all]                    the vehicle's next road option(s) and waypoints
    sync     on|off                                TM synchronous mode

Per-vehicle flags (also valid on `all`):
    --speed-diff P        % slower than the speed limit; NEGATIVE = faster
    --distance M          metres to keep to the leading vehicle
    --ignore-lights P     % of traffic lights to run (0-100)
    --ignore-signs P      % of stop/yield signs to ignore
    --ignore-vehicles P   % of other vehicles to ignore (collisions!)
    --ignore-walkers P    % of pedestrians to ignore
    --auto-lane-change on|off
    --keep-slow-lane P    % adherence to the keep-right/slow-lane rule
    --lane-offset M       lateral offset from lane centre (m; +right)
    --lights on|off       let the TM manage this vehicle's lights
    --desired-speed KMH   exact target speed in km/h, ignoring the speed limit
    --wide-turn on|off    large vehicles take wider turns at junctions
    --route L,S,R         turns at the next junctions: Left, Right, Straight
    --path 'X,Y,Z;...'    locations to drive through

SPEED SIGN: --speed-diff is a percentage BELOW the limit, so -30 means 30% ABOVE
the limit (faster) and +30 means 30% slower. Use the same --tm-port everywhere.

Selector: --id N | --role hero | --filter '*prius*'. Connection + port from
env.sh: CARLA_HOST/PORT/TIMEOUT, TM_PORT.
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


def _client():
    c = carla.Client(os.environ.get("CARLA_HOST", "127.0.0.1"),
                     int(os.environ.get("CARLA_PORT", "2000")))
    c.set_timeout(float(os.environ.get("CARLA_TIMEOUT", "60.0")))
    return c


def _tm(client, args):
    return client.get_trafficmanager(args.tm_port)


def _on(v):  # "on"/"off" -> bool
    return v == "on"


def _apply_per_vehicle(tm, vehicles, args):
    """Apply whichever per-vehicle flags were given to each vehicle; report count."""
    applied = []
    for v in vehicles:
        if args.speed_diff is not None:   tm.vehicle_percentage_speed_difference(v, args.speed_diff)
        if args.distance is not None:     tm.distance_to_leading_vehicle(v, args.distance)
        if args.ignore_lights is not None: tm.ignore_lights_percentage(v, args.ignore_lights)
        if args.ignore_signs is not None:  tm.ignore_signs_percentage(v, args.ignore_signs)
        if args.ignore_vehicles is not None: tm.ignore_vehicles_percentage(v, args.ignore_vehicles)
        if args.ignore_walkers is not None:  tm.ignore_walkers_percentage(v, args.ignore_walkers)
        if args.auto_lane_change is not None: tm.auto_lane_change(v, _on(args.auto_lane_change))
        if args.keep_slow_lane is not None:  tm.keep_slow_lane_rule_percentage(v, args.keep_slow_lane)
        if args.lane_offset is not None:     tm.vehicle_lane_offset(v, args.lane_offset)
        if args.lights is not None:          tm.update_vehicle_lights(v, _on(args.lights))
        if args.desired_speed is not None:   tm.set_desired_speed(v, args.desired_speed)
        if args.wide_turn is not None:       tm.vehicle_large_vehicle_wide_turn(v, _on(args.wide_turn))
        if args.route:                       tm.set_route(v, args.route.split(","))
        if args.path:
            tm.set_path(v, [carla.Location(*(float(c) for c in p.split(","))) for p in args.path.split(";")])
        applied.append(v.id)
    return applied


def _resolve(world, args):
    actors = world.get_actors()
    if args.id is not None:
        v = actors.find(args.id)
        if v is None:
            raise SystemExit(f"no actor id {args.id}")
        return [v]
    if args.filter:
        return list(actors.filter(args.filter))
    role = args.role or "hero"
    return [a for a in actors.filter("vehicle.*") if a.attributes.get("role_name", "") == role]


def cmd_global(args):
    tm = _tm(_client(), args)
    done = []
    if args.speed_diff is not None:
        tm.global_percentage_speed_difference(args.speed_diff); done.append(f"speed_diff={args.speed_diff}%")
    if args.distance is not None:
        tm.set_global_distance_to_leading_vehicle(args.distance); done.append(f"distance={args.distance}m")
    if args.seed is not None:
        tm.set_random_device_seed(args.seed); done.append(f"seed={args.seed}")
    if args.hybrid is not None:
        tm.set_hybrid_physics_mode(_on(args.hybrid)); done.append(f"hybrid={args.hybrid}")
    if args.hybrid_radius is not None:
        tm.set_hybrid_physics_radius(args.hybrid_radius); done.append(f"hybrid_radius={args.hybrid_radius}")
    if args.osm is not None:
        tm.set_osm_mode(_on(args.osm)); done.append(f"osm={args.osm}")
    if args.lane_offset is not None:
        tm.global_lane_offset(args.lane_offset); done.append(f"lane_offset={args.lane_offset}m")
    if args.wide_turn is not None:
        tm.global_large_vehicle_wide_turn(_on(args.wide_turn)); done.append(f"wide_turn={args.wide_turn}")
    if not done:
        raise SystemExit("global needs at least one setting (see --help)")
    print(f"TM :{args.tm_port} global -> {', '.join(done)}")


def cmd_vehicle(args):
    world = _fresh(_client().get_world())
    vehicles = _resolve(world, args)
    if not vehicles:
        raise SystemExit("no matching vehicle (spawn one, or check --id/--role/--filter)")
    ids = _apply_per_vehicle(_tm(_client(), args), vehicles, args)
    print(f"TM :{args.tm_port} applied per-vehicle settings to {len(ids)} vehicle(s): {ids[:10]}"
          + (" ..." if len(ids) > 10 else ""))


def cmd_all(args):
    client = _client()
    # _fresh: in sync mode an unwaited handle lists no actors at all.
    vehicles = list(_fresh(client.get_world()).get_actors().filter(args.filter))
    if not vehicles:
        raise SystemExit(f"no vehicles match {args.filter!r}")
    ids = _apply_per_vehicle(_tm(client, args), vehicles, args)
    print(f"TM :{args.tm_port} applied settings to all {len(ids)} vehicle(s) matching {args.filter!r}")


def cmd_plan(args):
    client = _client()
    vehicles = _resolve(_fresh(client.get_world()), args)
    if len(vehicles) != 1:
        raise SystemExit(f"{len(vehicles)} vehicles match; plan reads one (pass --id)")
    v, tm = vehicles[0], _tm(client, args)
    actions = tm.get_all_actions(v) if args.all else [tm.get_next_action(v)]
    if not actions or not actions[0] or actions[0][1] is None:
        raise SystemExit(f"no plan for id={v.id}: is it on autopilot on TM port {args.tm_port}? "
                         "(before ue58-dev with the TM RPC fix, only the TM's own process could read it)")
    loc = v.get_location()
    print(f"TM :{args.tm_port} plan of id={v.id} ({len(actions)} action(s)):")
    for option, wp in actions[:args.limit]:
        l = wp.transform.location
        print(f"  {option:12s} road={wp.road_id} lane={wp.lane_id} ({l.x:.1f},{l.y:.1f})  {l.distance(loc):.0f} m away"
              + (" [junction]" if wp.is_junction else ""))
    if len(actions) > args.limit:
        print(f"  ... and {len(actions) - args.limit} more (raise --limit)")


def cmd_sync(args):
    _tm(_client(), args).set_synchronous_mode(args.mode == "on")
    print(f"TM :{args.tm_port} synchronous_mode = {args.mode == 'on'} "
          "(must match the world's sync mode — see set-world-settings)")


def _pv_args(sp):
    sp.add_argument("--speed-diff", type=float, help="%% below limit (negative = faster)")
    sp.add_argument("--distance", type=float, help="metres to leading vehicle")
    sp.add_argument("--ignore-lights", type=float)
    sp.add_argument("--ignore-signs", type=float)
    sp.add_argument("--ignore-vehicles", type=float)
    sp.add_argument("--ignore-walkers", type=float)
    sp.add_argument("--auto-lane-change", choices=("on", "off"))
    sp.add_argument("--keep-slow-lane", type=float)
    sp.add_argument("--lane-offset", type=float)
    sp.add_argument("--lights", choices=("on", "off"))
    sp.add_argument("--desired-speed", type=float, help="exact target speed, km/h (replaces --speed-diff)")
    sp.add_argument("--wide-turn", choices=("on", "off"), help="large vehicles take wider turns at junctions")
    sp.add_argument("--route", help="turns to take at the next junctions, e.g. Left,Straight,Right")
    sp.add_argument("--path", help="locations to drive through, 'X,Y,Z;X,Y,Z;...'")
    return sp


def main() -> None:
    p = argparse.ArgumentParser(description="Tune the CARLA Traffic Manager.")
    p.add_argument("--tm-port", type=int, default=int(os.environ.get("TM_PORT", "8000")))
    sub = p.add_subparsers(dest="cmd", required=True)

    pg = sub.add_parser("global", help="TM-wide settings")
    pg.add_argument("--speed-diff", type=float); pg.add_argument("--distance", type=float)
    pg.add_argument("--seed", type=int)
    pg.add_argument("--hybrid", choices=("on", "off")); pg.add_argument("--hybrid-radius", type=float)
    pg.add_argument("--osm", choices=("on", "off"))
    pg.add_argument("--lane-offset", type=float, help="lateral offset for every vehicle, m (+right)")
    pg.add_argument("--wide-turn", choices=("on", "off"), help="large vehicles take wider turns")
    pg.set_defaults(func=cmd_global)

    pv = _pv_args(sub.add_parser("vehicle", help="settings for one vehicle"))
    pv.add_argument("--id", type=int); pv.add_argument("--role"); pv.add_argument("--filter")
    pv.set_defaults(func=cmd_vehicle)

    pa = _pv_args(sub.add_parser("all", help="settings for every matching vehicle"))
    pa.add_argument("--filter", default="vehicle.*")
    pa.set_defaults(func=cmd_all)

    pp = sub.add_parser("plan", help="what one vehicle will do next: road option + waypoint")
    pp.add_argument("--id", type=int); pp.add_argument("--role"); pp.add_argument("--filter")
    pp.add_argument("--all", action="store_true", help="every planned action, not just the next")
    pp.add_argument("--limit", type=int, default=20)
    pp.set_defaults(func=cmd_plan)

    psy = sub.add_parser("sync", help="TM synchronous mode")
    psy.add_argument("mode", choices=("on", "off"))
    psy.set_defaults(func=cmd_sync)

    args = p.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
