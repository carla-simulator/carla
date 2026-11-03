#!/usr/bin/env python3
"""Drive a vehicle directly: control input, Ackermann, lights, doors, physics.

Targets one vehicle (default the ego, role_name 'hero'):
    --id N | --role hero | --filter '*prius*'

Commands:
    control  [--throttle 0.5 --steer 0 --brake 0 --reverse --hand-brake]
             [--hold 3]                     apply raw VehicleControl (autopilot off)
    ackermann --speed 8 [--steer 0 --accel 3 --jerk 0]   drive to a target speed
    stop                                     full brake + hand-brake
    constant-velocity --speed 8 [--off]      hold a fixed forward speed
    lights   [--on Brake,LowBeam] [--off Position]       toggle light flags
    door     [--open FL,FR] [--close All]                open/close doors
    physics  [--show] [--mass 1600 --drag 0.3 --max-rpm 6000]   read/tune physics
    telemetry [--off]                        on-screen physics telemetry (rendered)
    ros-info                                 the ROS 2 command topics for this
                                             vehicle, with a ready-to-run
                                             `ros2 topic pub` line

`control` and `ackermann` first turn OFF autopilot (manual and TM control are
mutually exclusive). A VehicleControl PERSISTS until changed, so `--throttle 0.5`
keeps the car accelerating; `--hold S` applies it for S seconds then brakes.

Light flags: Position, LowBeam, HighBeam, Brake, LeftBlinker, RightBlinker,
Reverse, Fog, Interior, Special1, Special2 (comma-separated). Doors (0.9.16):
FL, FR, RL, RR, All.

Connection + TM port from env.sh: CARLA_HOST/PORT/TIMEOUT, TM_PORT.
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


def _resolve(world, args) -> carla.Vehicle:
    """Pick one vehicle: --id / --role / --filter, defaulting to the ego (hero)."""
    actors = world.get_actors()
    if args.id is not None:
        v = actors.find(args.id)
        if v is None or not v.type_id.startswith("vehicle."):
            raise SystemExit(f"no vehicle with id {args.id}")
        return v
    if args.filter:
        matches = list(actors.filter(args.filter))
    else:
        role = args.role or "hero"   # default target is the ego
        matches = [a for a in actors.filter("vehicle.*")
                   if a.attributes.get("role_name", "") == role]
        if not matches:
            raise SystemExit(f"no vehicle with role_name={role!r} "
                             "(spawn one with spawn-vehicles `ego`, or pass --id/--filter)")
    if not matches:
        raise SystemExit(f"no vehicle matching {args.filter!r}")
    if len(matches) > 1:
        print(f"note: {len(matches)} vehicles matched; using id={matches[0].id}")
    return matches[0]


def _flags(spec, enum):
    """Comma-separated flag names -> OR-combined bitmask value of `enum`."""
    valid = {n.lower(): getattr(enum, n) for n in dir(enum) if n[:1].isupper()}
    out = None
    for raw in spec.split(","):
        k = raw.strip().lower()
        if not k:
            continue
        if k not in valid:
            raise SystemExit(f"unknown value {raw!r}; valid: {', '.join(sorted(valid))}")
        out = valid[k] if out is None else out | valid[k]
    return out


# --- waiting on the simulation, not on the wall clock ----------------------
# These commands are observers: they never own the world's clock, so they
# advance by waiting for whoever does (the server itself in async, the holding
# client in sync). A fixed time.sleep() is wrong for both -- it measures wall
# time while the thing being waited for is measured in frames, so it overruns
# a fast server and cuts a slow one short, and in a synchronous world with a
# stalled ticker it "succeeds" having observed nothing at all.

def _for_seconds(world, seconds):
    """Yield one snapshot per frame until `seconds` of SIMULATED time pass.

    The budget is read off the world clock (`elapsed_seconds`), not summed from
    per-frame deltas. Summing deltas counts *observed* frames, so a consumer
    slower than the server -- a client encoding PNGs at 4 Hz against a 30 Hz
    server, say -- accumulates 0.03 s per poll and takes minutes to "reach"
    20 seconds. Reading the clock is correct whether or not frames are missed.
    """
    start = None
    while True:
        snapshot = world.wait_for_tick()
        now = snapshot.timestamp.elapsed_seconds
        if start is None:
            start = now
        elif now - start >= seconds:
            return
        yield snapshot


def _until(world, predicate, seconds):
    """Advance frames until predicate() is true; returns whether it became true.

    The budget is simulated seconds, so a stalled clock ends this by timing out
    in wait_for_tick() rather than by silently burning the budget.
    """
    if predicate():
        return True
    for _ in _for_seconds(world, seconds):
        if predicate():
            return True
    return False


def cmd_control(args):
    world = _fresh(_client().get_world())
    v = _resolve(world, args)
    v.set_autopilot(False)   # manual and TM control are mutually exclusive
    ctrl = carla.VehicleControl(throttle=args.throttle, steer=args.steer, brake=args.brake,
                                hand_brake=args.hand_brake, reverse=args.reverse)
    v.apply_control(ctrl)
    print(f"id={v.id}: throttle={args.throttle} steer={args.steer} brake={args.brake} "
          f"reverse={args.reverse} hand_brake={args.hand_brake}")
    if args.hold > 0:
        for _ in _for_seconds(world, args.hold):
            pass
        v.apply_control(carla.VehicleControl(throttle=0.0, brake=1.0))
        print(f"  held {args.hold}s, then braked to a stop")


def cmd_ackermann(args):
    world = _fresh(_client().get_world())
    v = _resolve(world, args)
    v.set_autopilot(False)
    v.apply_ackermann_control(carla.VehicleAckermannControl(
        steer=args.steer, steer_speed=args.steer_speed,
        speed=args.speed, acceleration=args.accel, jerk=args.jerk))
    print(f"id={v.id}: ackermann target speed={args.speed} m/s steer={args.steer} "
          f"accel={args.accel} (the built-in controller drives to this speed)")


def cmd_stop(args):
    world = _fresh(_client().get_world())
    v = _resolve(world, args)
    v.set_autopilot(False)
    v.apply_control(carla.VehicleControl(throttle=0.0, brake=1.0, hand_brake=True))
    print(f"id={v.id}: full brake + hand-brake")


def cmd_constant_velocity(args):
    world = _fresh(_client().get_world())
    v = _resolve(world, args)
    v.set_autopilot(False)
    if args.off:
        v.disable_constant_velocity()
        print(f"id={v.id}: constant velocity disabled (normal physics)")
    else:
        # Forward is the vehicle's local +x; holds this speed ignoring physics.
        v.enable_constant_velocity(carla.Vector3D(args.speed, 0.0, 0.0))
        print(f"id={v.id}: holding constant {args.speed} m/s forward "
              "(use --off to release; great for reproducible scenarios)")


def cmd_lights(args):
    world = _fresh(_client().get_world())
    v = _resolve(world, args)
    state = int(v.get_light_state())
    if args.on:
        state |= int(_flags(args.on, carla.VehicleLightState))
    if args.off:
        state &= ~int(_flags(args.off, carla.VehicleLightState))
    v.set_light_state(carla.VehicleLightState(state))
    print(f"id={v.id}: light_state -> {carla.VehicleLightState(state)}")


def _doors(spec):
    """Comma-separated door names -> list of VehicleDoor values (one per call).

    Unlike VehicleLightState, VehicleDoor values do NOT OR into a combined door;
    open_door/close_door take a single door, so we return a list and loop.
    """
    valid = {n.lower(): getattr(carla.VehicleDoor, n) for n in dir(carla.VehicleDoor) if n[:1].isupper()}
    out = []
    for raw in spec.split(","):
        k = raw.strip().lower()
        if not k:
            continue
        if k not in valid:
            raise SystemExit(f"unknown door {raw!r}; valid: {', '.join(sorted(valid))}")
        out.append(valid[k])
    return out


def cmd_door(args):
    world = _fresh(_client().get_world())
    v = _resolve(world, args)
    if not args.open and not args.close:
        raise SystemExit("door needs --open and/or --close")
    if args.open:
        for d in _doors(args.open):
            v.open_door(d)
        print(f"id={v.id}: opened {args.open}")
    if args.close:
        for d in _doors(args.close):
            v.close_door(d)
        print(f"id={v.id}: closed {args.close}")


def cmd_physics(args):
    world = _fresh(_client().get_world())
    v = _resolve(world, args)
    pc = v.get_physics_control()
    if args.show or (args.mass is None and args.drag is None and args.max_rpm is None):
        # The gearbox fields differ by engine, so read whichever this build has:
        # 0.9.x (PhysX) exposes forward_gears as a list of GearPhysicsControl plus
        # clutch_strength; 0.10.0 (Chaos) deletes both and exposes
        # forward_gear_ratios / reverse_gear_ratios as plain float lists.
        # 0.10.0's forward_gear_ratios/reverse_gear_ratios are declared but have
        # NO boost::python converter for std::vector<float>: reading either
        # raises TypeError("No to_python (by-value) converter found"). Everything
        # else on the struct reads fine, so degrade to a count of None.
        try:
            gears = pc.forward_gears
        except AttributeError:
            try:
                gears = list(pc.forward_gear_ratios)
            except (AttributeError, TypeError):
                gears = None
        extra = ""
        if hasattr(pc, "clutch_strength"):
            extra = f" clutch={pc.clutch_strength:.0f}"
        elif hasattr(pc, "transmission_efficiency"):
            extra = (f" transmission_efficiency={pc.transmission_efficiency:.2f}"
                     f" differential={pc.differential_type}")
        gear_text = "unreadable (0.10.0 converter gap)" if gears is None else len(gears)
        print(f"id={v.id} physics: mass={pc.mass:.0f}kg drag={pc.drag_coefficient:.2f} "
              f"max_rpm={pc.max_rpm:.0f} gears={gear_text} wheels={len(pc.wheels)}"
              f"{extra}")
        return
    if args.mass is not None:    pc.mass = args.mass
    if args.drag is not None:    pc.drag_coefficient = args.drag
    if args.max_rpm is not None: pc.max_rpm = args.max_rpm
    v.apply_physics_control(pc)
    print(f"id={v.id}: applied physics (mass={pc.mass:.0f} drag={pc.drag_coefficient:.2f} "
          f"max_rpm={pc.max_rpm:.0f})")


def cmd_telemetry(args):
    world = _fresh(_client().get_world())
    v = _resolve(world, args)
    v.show_debug_telemetry(not args.off)
    print(f"id={v.id}: debug telemetry {'off' if args.off else 'on'} (visible on a rendered server)")


def cmd_ros_info(args):
    """Report how (and whether) this vehicle can be driven from ROS 2.

    Read-only. The subscribers live in the server: ROS2::RegisterVehicle creates
    them, and ActorDispatcher only calls it for a vehicle whose role_name is
    exactly "hero" — so a non-hero vehicle has no ROS control path at all.
    """
    world = _fresh(_client().get_world())
    v = _resolve(world, args)
    role = v.attributes.get("role_name", "")
    ros_name = v.attributes.get("ros_name", "") or f"actor{v.id}"
    base = f"rt/carla/{ros_name}"

    print(f"id={v.id} ({v.type_id}) role_name={role!r} ros_name={ros_name!r}")
    if role != "hero":
        print("  ros: NOT registered — only role_name 'hero' gets ROS 2 subscribers.")
        print("  ros: re-spawn it as the ego (spawn-vehicles ego --ros-name ...) to drive it from ROS.")
        return
    print(f"  ros: {base}/vehicle_control_cmd     [carla_msgs/CarlaEgoVehicleControl]")
    print("       fields: header, throttle, steer, brake, hand_brake, reverse, gear,")
    print("               manual_gear_shift  (same semantics as carla.VehicleControl)")
    print(f"  ros: {base}/ackermann_control_cmd   [ackermann_msgs/AckermannDriveStamped]")
    print("       fields: header, drive.{steering_angle, steering_angle_velocity,")
    print("               speed, acceleration, jerk}")
    # A ROS 2 node sees the DDS name "rt/<x>" as the topic "/<x>".
    print(f"\n  from a ROS 2 environment on the same domain (topic = {base[3:]}):")
    print(f"    ros2 topic pub --once /carla/{ros_name}/vehicle_control_cmd \\")
    print("      carla_msgs/msg/CarlaEgoVehicleControl '{throttle: 0.5, steer: 0.0}'")
    print("\n  NOTE: ROS commands and this skill's `control` both write VehicleControl —"
          "\n        the last writer wins, and a held ROS command overrides local input.")


def _sel(sp):
    sp.add_argument("--id", type=int); sp.add_argument("--role"); sp.add_argument("--filter")
    return sp


def main() -> None:
    p = argparse.ArgumentParser(description="Directly control a CARLA vehicle.")
    sub = p.add_subparsers(dest="cmd", required=True)

    pc = _sel(sub.add_parser("control", help="apply raw VehicleControl"))
    pc.add_argument("--throttle", type=float, default=0.0)
    pc.add_argument("--steer", type=float, default=0.0)
    pc.add_argument("--brake", type=float, default=0.0)
    pc.add_argument("--reverse", action="store_true")
    pc.add_argument("--hand-brake", action="store_true")
    pc.add_argument("--hold", type=float, default=0.0, help="apply for N s then brake")
    pc.set_defaults(func=cmd_control)

    pa = _sel(sub.add_parser("ackermann", help="drive to a target speed"))
    pa.add_argument("--speed", type=float, required=True, help="target m/s")
    pa.add_argument("--steer", type=float, default=0.0)
    pa.add_argument("--steer-speed", type=float, default=0.0)
    pa.add_argument("--accel", type=float, default=3.0)
    pa.add_argument("--jerk", type=float, default=0.0)
    pa.set_defaults(func=cmd_ackermann)

    _sel(sub.add_parser("stop", help="full brake + hand-brake")).set_defaults(func=cmd_stop)

    pcv = _sel(sub.add_parser("constant-velocity", help="hold a fixed forward speed"))
    pcv.add_argument("--speed", type=float, default=8.0, help="m/s forward")
    pcv.add_argument("--off", action="store_true", help="release, back to normal physics")
    pcv.set_defaults(func=cmd_constant_velocity)

    pl = _sel(sub.add_parser("lights", help="toggle light flags"))
    pl.add_argument("--on"); pl.add_argument("--off"); pl.set_defaults(func=cmd_lights)

    pd = _sel(sub.add_parser("door", help="open/close doors"))
    pd.add_argument("--open"); pd.add_argument("--close"); pd.set_defaults(func=cmd_door)

    pp = _sel(sub.add_parser("physics", help="read/tune physics control"))
    pp.add_argument("--show", action="store_true")
    pp.add_argument("--mass", type=float); pp.add_argument("--drag", type=float)
    pp.add_argument("--max-rpm", type=float); pp.set_defaults(func=cmd_physics)

    pt = _sel(sub.add_parser("telemetry", help="on-screen physics telemetry"))
    pt.add_argument("--off", action="store_true"); pt.set_defaults(func=cmd_telemetry)

    _sel(sub.add_parser("ros-info", help="ROS 2 command topics for this vehicle")) \
        .set_defaults(func=cmd_ros_info)

    args = p.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
