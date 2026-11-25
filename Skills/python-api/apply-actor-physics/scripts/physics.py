#!/usr/bin/env python3
"""Push, spin, launch, freeze or teleport an actor through its physics body.

Select the actor: --id N | --role hero | --filter 'vehicle.*' (must match exactly one).

Commands:
    force    --vec X,Y,Z [--at X,Y,Z] [--seconds S]   add_force (N), reapplied every frame for S seconds
    impulse  --vec X,Y,Z [--at X,Y,Z]                 add_impulse (N*s), once
    torque   --vec X,Y,Z [--seconds S]                add_torque, reapplied every frame
    spin     --vec X,Y,Z                              add_angular_impulse, once
    velocity [--vec X,Y,Z] [--angular X,Y,Z]          set_target_velocity / set_target_angular_velocity
    constant --vec X,Y,Z | --off                      enable/disable_constant_velocity, m/s in the vehicle's frame
    gravity  on|off                                   set_enable_gravity
    physics  on|off                                   set_simulate_physics
    teleport --to X,Y,Z [--yaw DEG]                   set_location, or set_transform with --yaw

--local reads --vec in the actor's frame (x forward, y right, z up) instead of the world's.
--at is an offset from the actor's origin in its own frame, metres; the force or impulse is applied there.
Every command then watches the actor for --observe seconds and prints the change in speed, position and height.
In synchronous mode pass --tick to advance the world yourself; otherwise the script waits for another client's ticks.
"""
from __future__ import annotations

import argparse
import math
import os

import carla  # provided by the active interpreter; check_env.sh verifies this


def _vec(text):
    x, y, z = (float(v) for v in text.split(","))
    return carla.Vector3D(x, y, z)


def _loc(text):
    x, y, z = (float(v) for v in text.split(","))
    return carla.Location(x, y, z)


class Clock:
    """Advances or follows the simulation, so every wait is counted in frames, not wall time."""

    def __init__(self, world, tick):
        s = world.get_settings()
        self.world, self.sync, self.own = world, s.synchronous_mode, tick and s.synchronous_mode
        if self.sync and not tick:
            print("[physics] synchronous mode: following another client's ticks (pass --tick if none is running)")

    def step(self):
        if self.own:
            self.world.tick()
            return self.world.get_snapshot()
        return self.world.wait_for_tick()


def _resolve(world, args):
    actors = world.get_actors()
    if args.id is not None:
        a = actors.find(args.id)
        if a is None:
            raise SystemExit(f"no actor id {args.id}")
        return a
    matches = list(actors.filter(args.filter or "*"))
    matches = [a for a in matches if not a.type_id.startswith(("spectator", "traffic.", "sensor.", "controller."))]
    if args.role:
        matches = [a for a in matches if a.attributes.get("role_name", "") == args.role]
    if len(matches) != 1:
        raise SystemExit(f"{len(matches)} actors match; pass --id (the world-data skill resolves one)")
    return matches[0]


def _state(snap, actor):
    s = snap.find(actor.id)
    t, v = s.get_transform(), s.get_velocity()
    return t.location, math.sqrt(v.x ** 2 + v.y ** 2 + v.z ** 2)


def _apply_vec(actor, args):
    v = _vec(args.vec)
    return actor.get_transform().transform_vector(v) if args.local else v


def main() -> None:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("cmd", choices=["force", "impulse", "torque", "spin", "velocity", "constant", "gravity", "physics", "teleport"])
    p.add_argument("state", nargs="?", choices=["on", "off"], help="for gravity / physics")
    sel = p.add_argument_group("actor")
    sel.add_argument("--id", type=int)
    sel.add_argument("--role")
    sel.add_argument("--filter")
    p.add_argument("--vec", help="X,Y,Z vector for the command")
    p.add_argument("--angular", help="X,Y,Z angular velocity, deg/s (velocity)")
    p.add_argument("--at", help="X,Y,Z offset in the actor's frame to apply a force or impulse at, instead of the centre of mass")
    p.add_argument("--to", help="X,Y,Z teleport target")
    p.add_argument("--yaw", type=float, help="teleport heading, degrees")
    p.add_argument("--seconds", type=float, default=1.0, help="how long force/torque is reapplied")
    p.add_argument("--off", action="store_true", help="constant: disable it")
    p.add_argument("--local", action="store_true", help="--vec is in the actor's frame")
    p.add_argument("--observe", type=float, default=2.0, help="seconds to watch the result")
    p.add_argument("--tick", action="store_true", help="tick the world yourself in synchronous mode")
    args = p.parse_args()

    c = carla.Client(os.environ.get("CARLA_HOST", "127.0.0.1"), int(os.environ.get("CARLA_PORT", "2000")))
    c.set_timeout(float(os.environ.get("CARLA_TIMEOUT", "60.0")))
    world = c.get_world()
    clock = Clock(world, args.tick)
    snap = clock.step()
    actor = _resolve(world, args)
    loc0, v0 = _state(snap, actor)
    t0 = snap.timestamp.elapsed_seconds
    print(f"[physics] {actor.type_id} id={actor.id} at ({loc0.x:.2f},{loc0.y:.2f},{loc0.z:.2f}) speed {v0:.2f} m/s")

    needs_vec = {"force", "impulse", "torque", "spin", "constant"}
    if args.cmd in needs_vec and not (args.vec or (args.cmd == "constant" and args.off)):
        raise SystemExit(f"{args.cmd} needs --vec X,Y,Z")
    if args.cmd in ("gravity", "physics") and not args.state:
        raise SystemExit(f"{args.cmd} needs on or off")

    if args.cmd in ("force", "torque"):
        # Unreal clears accumulated forces after each physics step, so one call moves almost nothing.
        while True:
            vec = _apply_vec(actor, args)
            if args.cmd == "torque":
                actor.add_torque(vec)
            elif args.at:
                actor.add_force_at_location(vec, actor.get_transform().transform(_loc(args.at)))
            else:
                actor.add_force(vec)
            snap = clock.step()
            if snap.timestamp.elapsed_seconds - t0 >= args.seconds:
                break
    elif args.cmd == "impulse":
        vec = _apply_vec(actor, args)
        if args.at:
            actor.add_impulse_at_location(vec, actor.get_transform().transform(_loc(args.at)))
        else:
            actor.add_impulse(vec)
    elif args.cmd == "spin":
        actor.add_angular_impulse(_apply_vec(actor, args))
    elif args.cmd == "velocity":
        if not (args.vec or args.angular):
            raise SystemExit("velocity needs --vec and/or --angular")
        if args.vec:
            actor.set_target_velocity(_apply_vec(actor, args))
        if args.angular:
            actor.set_target_angular_velocity(_vec(args.angular))
    elif args.cmd == "constant":
        if not isinstance(actor, carla.Vehicle):
            raise SystemExit("constant velocity works on vehicles only")
        actor.disable_constant_velocity() if args.off else actor.enable_constant_velocity(_vec(args.vec))
    elif args.cmd == "gravity":
        actor.set_enable_gravity(args.state == "on")
    elif args.cmd == "physics":
        actor.set_simulate_physics(args.state == "on")
    elif args.cmd == "teleport":
        if not args.to:
            raise SystemExit("teleport needs --to X,Y,Z")
        if args.yaw is None:
            actor.set_location(_loc(args.to))
        else:
            actor.set_transform(carla.Transform(_loc(args.to), carla.Rotation(yaw=args.yaw)))

    peak, start = 0.0, None
    while True:
        snap = clock.step()
        loc, v = _state(snap, actor)
        peak = max(peak, v)
        start = start if start is not None else snap.timestamp.elapsed_seconds
        if snap.timestamp.elapsed_seconds - start >= args.observe:
            break
    av = snap.find(actor.id).get_angular_velocity()  # deg/s, world axes
    print(f"[physics] after {args.observe:.1f}s: at ({loc.x:.2f},{loc.y:.2f},{loc.z:.2f}) moved {loc.distance(loc0):.2f} m, "
          f"dz {loc.z - loc0.z:+.2f} m, speed {v:.2f} m/s (peak {peak:.2f}), angular ({av.x:.1f},{av.y:.1f},{av.z:.1f}) deg/s")


if __name__ == "__main__":
    main()
