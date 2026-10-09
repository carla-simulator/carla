#!/usr/bin/env python3
"""Drive a vehicle to a destination with CARLA's navigation agents, or plan a route.

Commands:
    route --from X,Y,Z --to X,Y,Z [--draw] [--resolution 2]
          plan a route (waypoints + turn actions) with GlobalRoutePlanner; no driving
    go    --to X,Y,Z [selector] [--agent basic|behavior|constant] [--speed 20]
          [--behavior normal|cautious|aggressive] [--seconds 60]
          [--ignore-lights] [--ignore-vehicles] [--ignore-signs]
          drive the ego to the destination, ticking until it arrives or times out

`go` runs an agent loop: each frame it calls `agent.run_step()` and applies the
returned control, until `agent.done()` (arrived) or --seconds elapses. It turns
autopilot OFF (the agent is the driver). Agents: `basic` (BasicAgent, obeys lights
+ avoids obstacles, follows the planned route), `behavior` (BehaviorAgent, with a
driving style), `constant` (ConstantVelocityAgent, holds a speed along the route).

Selector for the ego: --id N | --role hero (default) | --filter '*prius*'.
Needs the agents package on PYTHONPATH (env.sh sets it from CARLA_ROOT).
Connection from env.sh: CARLA_HOST/PORT/TIMEOUT.
"""
from __future__ import annotations

import argparse
import os

import carla  # provided by the active interpreter; check_env.sh verifies this
from agents.navigation.basic_agent import BasicAgent
from agents.navigation.behavior_agent import BehaviorAgent
from agents.navigation.constant_velocity_agent import ConstantVelocityAgent
from agents.navigation.global_route_planner import GlobalRoutePlanner


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


def _loc(s):
    x, y, z = (float(v) for v in s.split(","))
    return carla.Location(x, y, z)


def _resolve_ego(world, args) -> carla.Vehicle:
    actors = world.get_actors()
    if args.id is not None:
        v = actors.find(args.id)
        if v is None:
            raise SystemExit(f"no actor id {args.id}")
        return v
    matches = list(actors.filter(args.filter)) if args.filter else \
        [a for a in actors.filter("vehicle.*") if a.attributes.get("role_name", "") == (args.role or "hero")]
    if not matches:
        raise SystemExit("no target vehicle — spawn an ego (spawn-vehicles) or pass --id/--filter")
    if len(matches) > 1:
        raise SystemExit(f"{len(matches)} match — narrow with --id (see world-data to resolve)")
    return matches[0]


def cmd_route(args):
    world = _fresh(_client().get_world())
    grp = GlobalRoutePlanner(world.get_map(), args.resolution)
    route = grp.trace_route(_loc(getattr(args, "from")), _loc(args.to))
    print(f"route: {len(route)} waypoints, ~{len(route) * args.resolution:.0f} m")
    # Summarise the turn sequence rather than dumping every waypoint.
    actions, last = [], None
    for wp, opt in route:
        name = str(opt).split(".")[-1]
        if name != last:
            actions.append(name); last = name
    print("  maneuvers: " + " -> ".join(actions))
    if args.draw:
        dbg = world.debug
        for wp, _ in route:
            l = wp.transform.location
            dbg.draw_point(carla.Location(l.x, l.y, l.z + 0.3), 0.1, carla.Color(0, 255, 0), args.life)
        print(f"  drew the route ({args.life}s) — view on a rendered server")


def _make_agent(vehicle, args):
    if args.agent == "behavior":
        return BehaviorAgent(vehicle, behavior=args.behavior)
    if args.agent == "constant":
        return ConstantVelocityAgent(vehicle, args.speed)
    return BasicAgent(vehicle, args.speed)


def cmd_go(args):
    client = _client()
    world = _fresh(client.get_world())
    ego = _resolve_ego(world, args)
    ego.set_autopilot(False)  # the agent drives, not the TM

    agent = _make_agent(ego, args)
    if args.ignore_lights:
        agent.ignore_traffic_lights(True)
    if args.ignore_vehicles:
        agent.ignore_vehicles(True)
    if args.ignore_signs:
        agent.ignore_stop_signs(True)
    agent.set_destination(_loc(args.to))

    print(f"driving id={ego.id} ({ego.type_id}) to {args.to} with {args.agent} agent "
          f"(speed {args.speed} km/h, up to {args.seconds}s)")
    # Budget in SIMULATED seconds, and never tick a clock this client does not
    # own: if the world is already synchronous another client is driving it, so
    # waiting is the only correct move (ticking it drops commands silently).
    elapsed = 0.0
    while elapsed < args.seconds:
        elapsed += world.wait_for_tick().timestamp.delta_seconds
        if agent.done():
            print("  arrived at destination"); return
        ego.apply_control(agent.run_step())
    ego.apply_control(carla.VehicleControl(brake=1.0))
    print(f"  stopped after {args.seconds}s (not yet arrived — raise --seconds or check the route)")


def main() -> None:
    p = argparse.ArgumentParser(description="Navigate a vehicle with CARLA agents.")
    sub = p.add_subparsers(dest="cmd", required=True)

    pr = sub.add_parser("route", help="plan a route (no driving)")
    pr.add_argument("--from", required=True); pr.add_argument("--to", required=True)
    pr.add_argument("--resolution", type=float, default=2.0, help="waypoint spacing (m)")
    pr.add_argument("--draw", action="store_true"); pr.add_argument("--life", type=float, default=60.0)
    pr.set_defaults(func=cmd_route)

    pg = sub.add_parser("go", help="drive the ego to a destination")
    pg.add_argument("--to", required=True, help="destination X,Y,Z")
    pg.add_argument("--id", type=int); pg.add_argument("--role"); pg.add_argument("--filter")
    pg.add_argument("--agent", choices=("basic", "behavior", "constant"), default="basic")
    pg.add_argument("--speed", type=float, default=20.0, help="target speed km/h (basic/constant)")
    pg.add_argument("--behavior", choices=("normal", "cautious", "aggressive"), default="normal")
    pg.add_argument("--seconds", type=float, default=60.0, help="max drive time")
    pg.add_argument("--ignore-lights", action="store_true")
    pg.add_argument("--ignore-vehicles", action="store_true")
    pg.add_argument("--ignore-signs", action="store_true")
    pg.set_defaults(func=cmd_go)

    args = p.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
