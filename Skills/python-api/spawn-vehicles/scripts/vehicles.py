#!/usr/bin/env python3
"""Spawn vehicles that drive themselves via autopilot, and destroy them cleanly.

Commands:

    spawn   --count 40 [--filter vehicle.*] [--safe] [--seed 42]
            [--tm-port 8000] [--no-autopilot]        spawn at map spawn points
    line    --at X,Y,Z --count 5 --gap 15 [--backward] [...]   a queue in one lane
    ego     [--at X,Y,Z] [--filter] [--autopilot] [--ros-name hero]
                                                     one hero vehicle, id printed
    destroy [--filter vehicle.*]                     remove all (or a subset)

`spawn` scatters vehicles across the map's predefined spawn points (one per
point). `line` places a row of vehicles in a SINGLE lane: it snaps --at to the
nearest driving lane and steps along it with the Waypoint API (`next(gap)`),
putting one vehicle every `--gap` metres — e.g. 5 cars 15 m apart in the same
lane. `--backward` lays them behind the start point instead of ahead.

By default vehicles are handed to the Traffic Manager autopilot at spawn time
(atomic SpawnActor.then(SetAutopilot)), so they drive off on their own;
`--no-autopilot` leaves them stationary (useful for a static queue). `--safe`
keeps only four-wheeled cars.

ROS 2 (only on a server started with --ros2, see run-carla-server): `ego` takes
--ros-name / --ros-frame-id / --no-ros-tf. Only a vehicle whose role_name is
"hero" is registered with the ROS 2 layer — the server checks that explicitly, so
`spawn`/`line` traffic never publishes and never accepts ROS control commands.
Registering a hero creates the two control subscribers (vehicle_control_cmd,
ackermann_control_cmd). It does NOT publish a vehicle transform: rt/tf carries
sensor->parent transforms only, so map->hero must be broadcast by something else.
"""
from __future__ import annotations

import argparse
import os
import random

import carla  # provided by the active interpreter; check_env.sh verifies this

SpawnActor = carla.command.SpawnActor
SetAutopilot = carla.command.SetAutopilot
DestroyActor = carla.command.DestroyActor
FutureActor = carla.command.FutureActor


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
    # 10 s is too tight for a client that owns the clock: one slow tick on a
    # loaded server (an HD camera saving frames, say) raises std::exception and
    # takes the whole holder down with it. CARLA_TIMEOUT still overrides.
    client.set_timeout(float(os.environ.get("CARLA_TIMEOUT", "60.0")))
    return client


def _tm_and_sync(client, world, tm_port):
    """Get the TM and, if the world is sync, put the TM in sync too (autopilot needs it)."""
    tm = client.get_trafficmanager(tm_port)
    if world.get_settings().synchronous_mode:
        tm.set_synchronous_mode(True)
    return tm





# --- clock ownership -------------------------------------------------------
# Read the world settings before doing anything, then obey one rule:
#
#   already synchronous -> someone else owns the clock. NEVER tick; wait_for_tick.
#   asynchronous        -> we may switch it to sync and become the ticker.
#
# Ticking a world you do not own is what breaks a shared scene. Measured on
# 0.10.0: with a traffic client owning a 20 Hz sync clock, a walker spawn that
# called tick() itself left 0 of 22 walkers moving -- the tick that has to land
# between spawning a controller and start()ing it never sequenced. Re-issuing
# start()+go_to_location from a client that only waited put 21 of 22 in motion.
_OWNS_CLOCK = False


def _claim_clock(world, delta, allow_sync=True):
    """Returns the settings to restore, or None if we did not change anything."""
    global _OWNS_CLOCK
    previous = world.get_settings()
    if previous.synchronous_mode:
        _OWNS_CLOCK = False
        print("world is already synchronous — another client owns the clock; "
              "observing with wait_for_tick()", flush=True)
        return None
    if not allow_sync:
        _OWNS_CLOCK = False
        return None
    settings = world.get_settings()
    settings.synchronous_mode = True
    settings.fixed_delta_seconds = delta
    world.apply_settings(settings)
    _OWNS_CLOCK = True
    print(f"world was asynchronous -> synchronous, "
          f"fixed_delta_seconds={delta:g} ({1.0 / delta:g} Hz), this client ticks",
          flush=True)
    return previous


def _release_clock(world, previous, tm=None):
    """Hand the clock back. A sync world with no ticker is a frozen server."""
    global _OWNS_CLOCK
    if previous is None:
        return
    if tm is not None:
        try:
            tm.set_synchronous_mode(False)
        except RuntimeError:
            pass
    world.apply_settings(previous)
    _OWNS_CLOCK = False
    print("world settings restored (asynchronous)", flush=True)


def _cleanup(client, ids, keep):
    """Destroy what this run spawned, unless --keep was asked for."""
    if not ids:
        return
    if keep:
        print(f"--keep: leaving {len(ids)} vehicle(s) in the world", flush=True)
        return
    # do_tick=False: this runs during teardown, when asking the server for a
    # tick as well is exactly what times out.
    try:
        responses = client.apply_batch_sync(
            [carla.command.DestroyActor(i) for i in ids], False)
        gone = sum(1 for r in responses if not r.error)
    except RuntimeError:
        gone = 0
        for i in ids:
            try:
                actor = client.get_world().get_actor(i)
                if actor is not None and actor.destroy():
                    gone += 1
            except RuntimeError:
                pass
    print(f"removed {gone} vehicle(s) this run spawned", flush=True)


def _pump(world) -> None:
    """Advance the world if we own its clock, otherwise just observe."""
    # A tick can exceed the client timeout when the server is loaded -- an HD
    # camera saving PNGs at 20 Hz did it here, and the raised std::exception
    # tore down a holder that was otherwise healthy. One retry absorbs that;
    # a second failure is a real problem and propagates.
    try:
        if _OWNS_CLOCK:
            world.tick()
        else:
            world.wait_for_tick()
    except RuntimeError:
        if _OWNS_CLOCK:
            world.tick()
        else:
            world.wait_for_tick()


def _hold(world, what: str) -> None:
    """Stay alive pumping ticks until Ctrl+C, so the traffic keeps driving.

    The Traffic Manager lives in the process that created it. Verified on
    0.10.0: when a spawning client exits, the TM its vehicles were registered
    with dies with it and every autopilot vehicle sits at throttle 0.00 for
    ever. A fire-and-forget spawn therefore cannot produce moving traffic --
    something has to hold the TM open, which is what this does.

    In synchronous mode this loop is also the world clock: exactly one client
    may call tick(), and this is it. Anything else that wants to observe the
    simulation must use wait_for_tick(), not tick(), or the two of them
    double-step the server.
    """
    print(f"holding {what}; Ctrl+C to release (the TM dies with this process "
          "and the vehicles coast to a stop)", flush=True)
    try:
        while True:
            _pump(world)
    except KeyboardInterrupt:
        print("\nreleased: TM gone, autopilot vehicles will stop")



def _vehicle_bps(world, filt, safe, base_types=""):
    """Blueprints to spawn from, filtered by id, wheel count and body type.

    `--safe` is a *wheel count*, not a body type: on 0.10.0 it admits 11 cars
    but also 3 trucks, a van and a bus, so "50 cars --safe" measured 37 cars,
    7 trucks, 4 buses and 1 van. `--base-type car` is the filter that means
    cars. Both are kept because they answer different questions -- and they are
    worth combining, since one blueprint reports base_type car on three wheels.
    """
    bps = list(world.get_blueprint_library().filter(filt))
    if safe:
        bps = [b for b in bps if b.has_attribute("number_of_wheels")
               and int(b.get_attribute("number_of_wheels")) == 4]
    wanted = {t.strip().lower() for t in base_types.split(",") if t.strip()}
    if wanted:
        bps = [b for b in bps if b.has_attribute("base_type")
               and b.get_attribute("base_type").as_str().lower() in wanted]
    if not bps:
        detail = f"no blueprints match filter {filt!r}"
        if safe:
            detail += " with --safe"
        if wanted:
            detail += f" and --base-type {','.join(sorted(wanted))}"
        raise SystemExit(detail)
    return bps


def _configure(bp, role="autopilot"):
    if bp.has_attribute("color"):
        bp.set_attribute("color", random.choice(bp.get_attribute("color").recommended_values))
    bp.set_attribute("role_name", role)
    return bp


def _spawn_at(client, transforms, bps, autopilot, tm_port):
    """Batch-spawn one vehicle per transform; return the spawned actor ids."""
    batch = []
    for tf in transforms:
        cmd = SpawnActor(_configure(random.choice(bps)), tf)
        if autopilot:
            cmd = cmd.then(SetAutopilot(FutureActor, True, tm_port))  # atomic hand-off
        batch.append(cmd)
    return [r.actor_id for r in client.apply_batch_sync(batch, True) if not r.error]


def cmd_spawn(args):
    client = _client()
    world = _fresh(client.get_world())
    # Sync first, then the TM: _tm_and_sync only puts the TM in sync when the
    # world already is, so the order matters.
    previous = None if args.no_sync else _claim_clock(world, args.delta)
    if args.seed is not None:
        random.seed(args.seed)
        tm = _tm_and_sync(client, world, args.tm_port)
        tm.set_random_device_seed(args.seed)
    else:
        tm = _tm_and_sync(client, world, args.tm_port)

    bps = _vehicle_bps(world, args.filter, args.safe, args.base_type)
    spawn_points = world.get_map().get_spawn_points()
    random.shuffle(spawn_points)
    want = min(args.count, len(spawn_points))
    if want < args.count:
        print(f"note: only {len(spawn_points)} spawn points; capping {args.count} -> {want} "
              "(one vehicle per point)")

    ids = _spawn_at(client, spawn_points[:want], bps, not args.no_autopilot, args.tm_port)
    mode = "parked (--no-autopilot)" if args.no_autopilot else f"on autopilot (TM :{args.tm_port})"
    print(f"spawned {len(ids)} vehicles at spawn points; {mode}")
    if len(ids) < want:
        print(f"  note: {want - len(ids)} failed (occupied points / collisions — normal)")
    try:
        if getattr(args, "hold", False):
            _hold(world, f"{len(ids)} vehicles")
        elif not args.no_autopilot:
            print("  note: this process owns the TM — it dies on exit and the vehicles "
                  "stop. Use --hold to keep them driving.")
    finally:
        # Leave the world as it was found: what this run spawned is destroyed,
        # and the clock is handed back. A held run that just stopped otherwise
        # leaves dozens of dead vehicles parked across the map for the next
        # person to trip over, and a synchronous world with no ticker stops the
        # server dead for every other client.
        if getattr(args, "hold", False):
            try:
                _cleanup(client, ids, args.keep)
            except RuntimeError as error:
                # Never let a failed cleanup escape: the clock restore below is
                # what keeps the server usable, and a cleanup that timed out
                # must not cost everyone else a frozen world.
                print(f"  cleanup failed ({error}); actors may remain — "
                      "run `destroy` when the server is responsive again",
                      flush=True)
        _release_clock(world, previous, tm)


def cmd_line(args):
    client = _client()
    world = _fresh(client.get_world())
    previous = None if args.no_sync else _claim_clock(world, args.delta)
    tm = _tm_and_sync(client, world, args.tm_port)
    if args.seed is not None:
        random.seed(args.seed)

    x, y, z = (float(v) for v in args.at.split(","))
    start = world.get_map().get_waypoint(carla.Location(x, y, z),
                                         project_to_road=True, lane_type=carla.LaneType.Driving)
    if start is None:
        raise SystemExit(f"no driving lane near ({x},{y},{z})")

    # Walk the lane in gap-metre steps, collecting one waypoint per vehicle. next()/
    # previous() return a list (a fork gives several); take the first branch, and
    # stop early if the lane ends before we have enough.
    wps, wp = [start], start
    for _ in range(args.count - 1):
        nxt = wp.previous(args.gap) if args.backward else wp.next(args.gap)
        if not nxt:
            print(f"note: lane ended after {len(wps)} vehicle(s); "
                  f"{args.count - len(wps)} short of {args.count}")
            break
        wp = nxt[0]
        wps.append(wp)

    # Lift spawn z a little so the car drops onto the road rather than clipping it.
    transforms = [carla.Transform(
        carla.Location(w.transform.location.x, w.transform.location.y,
                       w.transform.location.z + args.z_offset),
        w.transform.rotation) for w in wps]

    bps = _vehicle_bps(world, args.filter, args.safe, args.base_type)
    ids = _spawn_at(client, transforms, bps, not args.no_autopilot, args.tm_port)
    mode = "parked" if args.no_autopilot else f"on autopilot (TM :{args.tm_port})"
    print(f"placed {len(ids)} vehicles in road {start.road_id} lane {start.lane_id}, "
          f"{args.gap} m apart {'behind' if args.backward else 'ahead of'} "
          f"({x:.0f},{y:.0f}); {mode}")
    if len(ids) < len(wps):
        print(f"  note: {len(wps) - len(ids)} failed to spawn (collision at a point — "
              "raise --gap or --z-offset)")
    try:
        if getattr(args, "hold", False):
            _hold(world, f"{len(ids)} vehicles in lane")
    finally:
        _release_clock(world, previous, tm)


def cmd_ego(args):
    client = _client()
    world = _fresh(client.get_world())
    bp = _configure(random.choice(_vehicle_bps(world, args.filter, safe=False,
                                                base_types=getattr(args, "base_type", ""))),
                    role="hero")

    # ROS 2 naming: read once, at registration, so it must be set before spawn.
    # These attributes exist on every blueprint (ActorBlueprintFunctionLibrary).
    if args.ros_name:
        bp.set_attribute("ros_name", args.ros_name)
    if args.ros_frame_id:
        bp.set_attribute("ros_frame_id", args.ros_frame_id)
    if args.no_ros_tf:
        bp.set_attribute("ros_publish_tf", "false")

    if args.at:
        x, y, z = (float(v) for v in args.at.split(","))
        wp = world.get_map().get_waypoint(carla.Location(x, y, z),
                                          project_to_road=True, lane_type=carla.LaneType.Driving)
        if wp is None:
            raise SystemExit(f"no driving lane near ({x},{y},{z})")
        tf = carla.Transform(carla.Location(wp.transform.location.x, wp.transform.location.y,
                                            wp.transform.location.z + 0.3), wp.transform.rotation)
    else:
        tf = random.choice(world.get_map().get_spawn_points())

    ego = world.try_spawn_actor(bp, tf)
    if ego is None:
        raise SystemExit("spawn location occupied — pass a clear --at, or retry")
    if args.autopilot:
        _tm_and_sync(client, world, args.tm_port)
        ego.set_autopilot(True, args.tm_port)
    loc = ego.get_transform().location
    print(f"spawned ego id={ego.id} ({ego.type_id}) role=hero at "
          f"({loc.x:.0f},{loc.y:.0f},{loc.z:.0f}); autopilot={args.autopilot}")
    print(f"  reference it downstream with role 'hero' (spectator/sensors/telemetry) or id {ego.id}")

    # ROS 2 view of this vehicle. Registration happens server-side at spawn (only
    # for role_name == "hero"); these are the names it derived.
    ros_name = args.ros_name or f"actor{ego.id}"
    base = f"rt/carla/{ros_name}"
    # rt/tf carries sensor->parent transforms only: a vehicle alone publishes no
    # transform (verified). ros_publish_tf therefore affects sensors under it.
    print(f"  ros: name={ros_name} frame_id={args.ros_frame_id or ros_name} "
          f"ros_publish_tf={'false' if args.no_ros_tf else 'true'} "
          f"(no vehicle transform is published; rt/tf appears once a SENSOR does)")
    print(f"  ros: subscribes {base}/vehicle_control_cmd     [carla_msgs/CarlaEgoVehicleControl]")
    print(f"  ros: subscribes {base}/ackermann_control_cmd   [ackermann_msgs/AckermannDriveStamped]")
    print(f"  ros: sensors attached to it nest under {base}/<sensor ros_name>")


def _stalled(world):
    """True if the world is synchronous with nothing advancing it.

    In that state `get_actors()` hands back a stale snapshot, so a destroy can
    report "0 actors" -- or a partial count -- while the world is still full.
    Observed: 141 actors survived three `destroy` calls that all claimed
    success, because a holder had been interrupted and left the clock claimed.
    """
    if not world.get_settings().synchronous_mode:
        return False
    before = world.get_snapshot().frame
    try:
        world.wait_for_tick(2.0)
    except RuntimeError:
        return True
    return world.get_snapshot().frame == before


def _verified_destroy(client, actors, what):
    """Destroy, then re-read the world and report what is actually gone."""
    world = client.get_world()
    if _stalled(world):
        print("WARNING the world is synchronous and nothing is ticking it, so "
              "the actor list cannot be trusted. Restore asynchronous mode "
              "(set-world-settings) or start the client that owns the clock, "
              "then destroy again.")
    wanted = {a.id for a in actors}
    if not wanted:
        print(f"no {what} to destroy")
        return 0
    for a in actors:
        if a.type_id.startswith("controller."):
            try:
                a.stop()
            except RuntimeError:
                pass
    try:
        client.apply_batch_sync(
            [carla.command.DestroyActor(i) for i in wanted], False)
    except RuntimeError as error:
        print(f"  batch destroy failed ({error}); falling back one at a time")
    try:
        world.wait_for_tick(5.0)
    except RuntimeError:
        pass
    # Only retry what the batch actually left behind. Destroying every actor
    # again unconditionally makes the client print "ERROR: failed to destroy
    # actor N" for each one the batch already removed, which reads like a
    # failure when nothing is wrong.
    left = {a.id for a in world.get_actors()} & wanted
    if left:
        for a in actors:
            if a.id in left:
                try:
                    a.destroy()
                except RuntimeError:
                    pass
        try:
            world.wait_for_tick(5.0)
        except RuntimeError:
            pass
        left = {a.id for a in world.get_actors()} & wanted
    gone = len(wanted) - len(left)
    print(f"destroyed {gone} {what}"
          + (f" -- {len(left)} STILL PRESENT (ids {sorted(left)[:8]})" if left else ""))
    return gone


def cmd_destroy(args):
    client = _client()
    world = _fresh(client.get_world())
    vehicles = list(world.get_actors().filter(args.filter))
    # Autopilot detaches automatically when the actor is destroyed; no stop needed.
    _verified_destroy(client, vehicles, f"vehicles matching {args.filter!r}")


def main() -> None:
    p = argparse.ArgumentParser(description="Spawn/destroy autopilot vehicles.")
    sub = p.add_subparsers(dest="cmd", required=True)

    ps = sub.add_parser("spawn", help="spawn vehicles at spawn points, autopilot by default")
    ps.add_argument("--count", type=int, default=30)
    ps.add_argument("--filter", default="vehicle.*", help="blueprint filter (default all vehicles)")
    ps.add_argument("--safe", action="store_true", help="four-wheeled cars only (no bikes/oddities)")
    ps.add_argument("--base-type", default="",
                    help="comma-separated carla base_type values to spawn from "
                         "(car, van, truck, bus, motorcycle, bicycle). Empty "
                         "allows all. --safe is a wheel count and is not the "
                         "same thing: it admits trucks, vans and a bus")
    ps.add_argument("--seed", type=int, help="reproducible blueprint/point/TM choices")
    ps.add_argument("--tm-port", type=int, default=int(os.environ.get("TM_PORT", "8000")))
    ps.add_argument("--no-sync", action="store_true",
                    help="leave the world asynchronous. The Traffic Manager is "
                         "unreliable driving an async server (measured: 24 of 50 "
                         "vehicles moving, actors decaying on their own), so sync "
                         "is the default")
    ps.add_argument("--delta", type=float, default=0.05,
                    help="fixed_delta_seconds while synchronous (default 0.05 = 20 Hz)")
    ps.add_argument("--no-autopilot", action="store_true", help="spawn parked (no TM autopilot)")
    ps.add_argument("--hold", action="store_true",
                    help="stay alive pumping ticks so the TM keeps driving them (Ctrl+C to stop)")
    ps.add_argument("--keep", action="store_true",
                    help="leave the spawned vehicles in the world on exit. By "
                         "default a held run destroys what it spawned, so "
                         "stopping it puts the map back as it was")
    ps.set_defaults(func=cmd_spawn)

    pl = sub.add_parser("line", help="place a row of vehicles in one lane, gap metres apart")
    pl.add_argument("--at", required=True, help="X,Y,Z near the target lane")
    pl.add_argument("--count", type=int, default=5)
    pl.add_argument("--gap", type=float, default=15.0, help="spacing between vehicles in m")
    pl.add_argument("--backward", action="store_true", help="lay the row behind --at instead of ahead")
    pl.add_argument("--z-offset", type=float, default=0.3, help="height above the road to spawn (m)")
    pl.add_argument("--filter", default="vehicle.*")
    pl.add_argument("--safe", action="store_true")
    pl.add_argument("--base-type", default="",
                    help="comma-separated carla base_type values to spawn from "
                         "(car, van, truck, bus, motorcycle, bicycle). Empty "
                         "allows all. --safe is a wheel count and is not the "
                         "same thing: it admits trucks, vans and a bus")
    pl.add_argument("--seed", type=int)
    pl.add_argument("--tm-port", type=int, default=int(os.environ.get("TM_PORT", "8000")))
    pl.add_argument("--no-sync", action="store_true",
                    help="leave the world asynchronous. The Traffic Manager is "
                         "unreliable driving an async server (measured: 24 of 50 "
                         "vehicles moving, actors decaying on their own), so sync "
                         "is the default")
    pl.add_argument("--delta", type=float, default=0.05,
                    help="fixed_delta_seconds while synchronous (default 0.05 = 20 Hz)")
    pl.add_argument("--no-autopilot", action="store_true", help="static queue (no autopilot)")
    pl.add_argument("--hold", action="store_true",
                    help="stay alive pumping ticks so the TM keeps driving them (Ctrl+C to stop)")
    pl.add_argument("--keep", action="store_true",
                    help="leave the spawned vehicles in the world on exit. By "
                         "default a held run destroys what it spawned, so "
                         "stopping it puts the map back as it was")
    pl.set_defaults(func=cmd_line)

    pe = sub.add_parser("ego", help="spawn one hero vehicle (autopilot off by default)")
    pe.add_argument("--at", help="X,Y,Z near a lane (default: a random spawn point)")
    pe.add_argument("--filter", default="vehicle.*")
    pe.add_argument("--autopilot", action="store_true", help="also enrol the ego in autopilot")
    pe.add_argument("--ros-name", help="ROS topic segment for this vehicle (default actor<id>)")
    pe.add_argument("--ros-frame-id", help="TF frame id (default: the ros name)")
    pe.add_argument("--no-ros-tf", action="store_true", help="do not publish its transform on rt/tf")
    pe.add_argument("--tm-port", type=int, default=int(os.environ.get("TM_PORT", "8000")))
    pe.set_defaults(func=cmd_ego)

    pd = sub.add_parser("destroy", help="remove vehicles (all, or a --filter subset)")
    pd.add_argument("--filter", default="vehicle.*",
                    help="which to remove (e.g. 'vehicle.tesla.*'; default all vehicles)")
    pd.set_defaults(func=cmd_destroy)

    args = p.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
