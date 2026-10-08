#!/usr/bin/env python3
"""Spawn pedestrians that wander via WalkerAIController, and destroy them cleanly.

Commands:

    spawn   --count 30 [--speed-min 1.0 --speed-max 1.8] [--cross-factor 0.1]
            [--seed 42] [--no-wander]            spawn walkers + AI controllers
    destroy                                      stop + destroy, in the right order

Each pedestrian is a `walker.pedestrian.*` actor paired with a
`controller.ai.walker`. By default `spawn` starts every controller and gives it
one random destination — and that is enough to wander FOREVER: CARLA's walker AI
automatically picks a fresh random target each time a walker arrives (verified in
LibCarla nav: WalkerManager::SetWalkerRoute). So no re-targeting loop is needed.
Pass `--no-wander` to spawn a stationary crowd (controllers left unstarted).

Works in ASYNCHRONOUS mode (the default) — sync mode is not required. The script
just advances a frame between spawn phases (`tick` if the world is already sync,
else `wait_for_tick`), so it is correct either way.

DESTROY ORDER MATTERS: stop() and destroy the controllers BEFORE the walkers.
`destroy` does this; do not destroy walkers first or you strand live controllers.

Needs a navmesh (see the debug-navmesh skill to validate one). Connection from
env.sh: CARLA_HOST/PORT/TIMEOUT.
"""
from __future__ import annotations

import argparse
import math
import os
import random

import carla  # provided by the active interpreter; check_env.sh verifies this

SpawnActor = carla.command.SpawnActor
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


def _advance(world: carla.World) -> None:
    """Advance one frame so freshly spawned actors register.

    Ticks only if this client owns the clock. The two-phase spawn depends on a
    frame landing between creating a controller and starting it, and calling
    tick() on someone else's sync world does not deliver that frame.
    """
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


def _cleanup(client, controller_ids, walker_ids, keep):
    """Destroy what this run spawned, controllers before walkers."""
    if not (controller_ids or walker_ids):
        return
    if keep:
        print(f"--keep: leaving {len(walker_ids)} walker(s) in the world",
              flush=True)
        return
    for c in client.get_world().get_actors(controller_ids):
        try:
            c.stop()
        except RuntimeError:
            pass
    try:
        responses = client.apply_batch_sync(
            [carla.command.DestroyActor(i) for i in controller_ids + walker_ids],
            False)
        gone = sum(1 for r in responses if not r.error)
    except RuntimeError:
        gone = 0
        for i in controller_ids + walker_ids:
            try:
                actor = client.get_world().get_actor(i)
                if actor is not None and actor.destroy():
                    gone += 1
            except RuntimeError:
                pass
    print(f"removed {gone} pedestrian actor(s) this run spawned", flush=True)


def _hold(world) -> None:
    """Stay alive pumping ticks until Ctrl+C, so the walkers actually walk.

    Verified on 0.10.0, async mode, identical spawn either way:
      idle loop time.sleep(30)      -> 0/10 walkers move, 0.00 m in 6 s
      idle loop wait_for_tick()     -> 10/10 walkers move, 10.07 m in 6 s

    The walker nav update only advances while some client holds a tick
    subscription, which is what wait_for_tick() registers. This is why CARLA's
    own generate_traffic.py ends in `while True: world.wait_for_tick()`. A
    fire-and-forget spawn leaves the crowd playing its walk animation on the
    spot. In sync mode world.tick() pumps it just the same.
    """
    print("holding the crowd (pumping ticks — walkers only move while a client "
          "does); Ctrl+C to release", flush=True)
    try:
        while True:
            _advance(world)
    except KeyboardInterrupt:
        print("\nreleased: the walkers will stop advancing")

def _ensure_walking(world, controllers, walker_ids, args):
    """Confirm the crowd is really walking, and re-issue until it is.

    Two measured traps, both of which made earlier versions of this report
    success while the crowd stood still (0.10.0, world ticked by another
    client):

    * `start()` issued by the spawning client can be silently dropped. Not an
      error, no exception -- the calls simply do not land, which is proven by a
      later re-issue being *accepted* for every controller (0 rejected as
      already-started). Re-issuing from freshly filtered handles works: 15/15
      walking, reproduced twice.

    * a walker spawns --z-offset above the navmesh and falls onto it, and that
      landing shifts x/y by more than a naive threshold. Checking "did any
      walker move" therefore passes on the drop alone, so a majority has to
      move before this reports success.
    """
    def displaced(before, frames):
        for _ in range(frames):
            _advance(world)
        live = world.get_actors(walker_ids)
        return sum(1 for a in live
                   if a.id in before
                   and math.hypot(a.get_location().x - before[a.id].x,
                                  a.get_location().y - before[a.id].y) > 0.5)

    want = max(1, len(walker_ids) // 2)
    before = {a.id: a.get_location() for a in world.get_actors(walker_ids)}
    moving = displaced(before, args.verify_frames)
    for attempt in range(args.retries):
        if moving >= want:
            break
        print(f"  only {moving}/{len(walker_ids)} walking — re-issuing "
              f"start()+destination from fresh handles "
              f"(attempt {attempt + 1}/{args.retries})", flush=True)
        for c in world.get_actors().filter("controller.ai.walker"):
            try:
                c.start()
            except RuntimeError:
                pass                      # already started; harmless
            dest = world.get_random_location_from_navigation()
            if dest is not None:
                c.go_to_location(dest)
            c.set_max_speed(random.uniform(args.speed_min, args.speed_max))
        before = {a.id: a.get_location() for a in world.get_actors(walker_ids)}
        moving = displaced(before, args.verify_frames)
    print(f"  {moving}/{len(walker_ids)} confirmed walking "
          f"(displacement over {args.verify_frames} frames; get_velocity() "
          f"reads 0 for nav-driven walkers on this build)", flush=True)


def cmd_spawn(args):
    client = _client()
    world = _fresh(client.get_world())
    previous = _claim_clock(world, args.delta, allow_sync=not args.no_sync)
    if args.seed is not None:
        world.set_pedestrians_seed(args.seed)
        random.seed(args.seed)
    # How often pedestrians cross roads (0 = never, 1 = always). Low by default so
    # they mostly use sidewalks rather than constantly jaywalking.
    world.set_pedestrians_cross_factor(args.cross_factor)

    bp_lib = world.get_blueprint_library()
    walker_bps = bp_lib.filter("walker.pedestrian.*")
    controller_bp = bp_lib.find("controller.ai.walker")

    # Phase 1: pick navmesh points and batch-spawn the walker bodies.
    spawn_batch = []
    for _ in range(args.count):
        loc = world.get_random_location_from_navigation()
        if loc is None:
            continue  # no navmesh point available this draw
        bp = random.choice(walker_bps)
        if bp.has_attribute("is_invincible"):
            bp.set_attribute("is_invincible", "false")  # so they can be hit/collide
        # Spawning at the navmesh z itself is rejected on 0.10.0 with "Spawn
        # failed because of collision at spawn position" (measured: +0.5 still
        # fails, +1.0 is the first that works). Offset up; they settle on foot.
        loc.z += args.z_offset
        spawn_batch.append(SpawnActor(bp, carla.Transform(loc)))
    if not spawn_batch:
        raise SystemExit("no navmesh spawn points — is the navmesh present? (debug-navmesh)")

    walker_ids = []
    for r in client.apply_batch_sync(spawn_batch, True):
        if not r.error:
            walker_ids.append(r.actor_id)
    _advance(world)  # let the walkers register before attaching controllers

    # Phase 2: batch-spawn one AI controller parented to each walker.
    ctrl_batch = [SpawnActor(controller_bp, carla.Transform(), wid) for wid in walker_ids]
    controller_ids = []
    for r in client.apply_batch_sync(ctrl_batch, True):
        if not r.error:
            controller_ids.append(r.actor_id)
    # Controllers must exist before start(), and one frame is not enough when
    # this client does not own the clock. Measured on 0.10.0 against a world
    # ticked by a traffic client: start() issued a single frame after the
    # controllers were created was silently dropped for all 15 -- they stood
    # still, and a later re-issue was *accepted* for every one of them (0
    # rejected as already-started), which is what proves the first call never
    # landed. Settling frames make it stick.
    for _ in range(args.settle):
        _advance(world)

    # Phase 3: start each controller with ONE random destination. The walker AI
    # then re-targets on its own forever, so this single call = infinite wander.
    # --no-wander skips it: controllers stay unstarted and the crowd is stationary.
    controllers = world.get_actors(controller_ids)
    if not args.no_wander:
        for c in controllers:
            c.start()
            dest = world.get_random_location_from_navigation()
            if dest is not None:
                c.go_to_location(dest)
            c.set_max_speed(random.uniform(args.speed_min, args.speed_max))
        for _ in range(args.settle):
            _advance(world)
        _ensure_walking(world, controllers, walker_ids, args)

    mode = ("stationary (--no-wander; controllers unstarted)" if args.no_wander
            else f"wandering indefinitely at {args.speed_min}-{args.speed_max} m/s "
                 f"(cross_factor={args.cross_factor})")
    print(f"spawned {len(walker_ids)} walkers + {len(controller_ids)} controllers; {mode}")
    if len(controller_ids) < args.count:
        print(f"  note: {args.count - len(controller_ids)} fewer than requested "
              f"(nav-point collisions / spawn failures — normal at high counts)")
    try:
        if args.hold and not args.no_wander:
            _hold(world)
        elif not args.no_wander and previous is not None:
            print("  note: this client owns the clock and is about to exit, so "
                  "nothing will tick — use --hold, or let another client (a "
                  "traffic hold, your own loop) drive the world.")
    finally:
        # Put the world back: controllers first, then walkers (the reverse
        # order leaves ghost actors), then the clock. Stopping a held crowd
        # otherwise leaves twenty frozen pedestrians standing in the road.
        if args.hold:
            try:
                _cleanup(client, controller_ids, walker_ids, args.keep)
            except RuntimeError as error:
                # Never let a failed cleanup escape: the clock restore below is
                # what keeps the server usable, and a cleanup that timed out
                # must not cost everyone else a frozen world.
                print(f"  cleanup failed ({error}); actors may remain — "
                      "run `destroy` when the server is responsive again",
                      flush=True)
        _release_clock(world, previous)


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
    controllers = list(world.get_actors().filter("controller.ai.walker"))
    # Correct order: stop controllers, destroy controllers, THEN destroy walkers.
    # stop() can throw on a controller that is not/no-longer attached to a walker
    # (e.g. one left by a replay) — tolerate it so one bad controller cannot abort
    # the whole cleanup; it gets destroyed regardless below.
    _verified_destroy(client, controllers, "walker controllers")
    walkers = list(world.get_actors().filter("walker.pedestrian.*"))
    _verified_destroy(client, walkers, "walkers")


def main() -> None:
    p = argparse.ArgumentParser(description="Spawn/steer/destroy CARLA pedestrians.")
    sub = p.add_subparsers(dest="cmd", required=True)

    ps = sub.add_parser("spawn", help="spawn walkers + AI controllers, set them wandering")
    ps.add_argument("--count", type=int, default=30)
    ps.add_argument("--speed-min", type=float, default=1.0, help="m/s (≈ slow walk)")
    ps.add_argument("--speed-max", type=float, default=1.8, help="m/s (≈ brisk walk)")
    ps.add_argument("--cross-factor", type=float, default=0.1, help="0-1 road-crossing probability")
    ps.add_argument("--seed", type=int, help="reproducible pedestrian placement")
    ps.add_argument("--no-wander", action="store_true",
                    help="spawn a stationary crowd (leave controllers unstarted)")
    ps.add_argument("--z-offset", type=float, default=2.0,
                    help="height above the navmesh point to spawn (m); 0 is rejected "
                         "as a spawn collision on 0.10.0 (default: 2.0)")
    ps.add_argument("--settle", type=int, default=10,
                    help="frames to advance between creating the controllers "
                         "and starting them, and again after. One frame is not "
                         "enough when another client owns the clock")
    ps.add_argument("--retries", type=int, default=3,
                    help="times to re-issue start()+destination if the crowd "
                         "is not walking. The first call is silently dropped "
                         "often enough that this is load-bearing")
    ps.add_argument("--verify-frames", type=int, default=40,
                    help="frames used to confirm the crowd is moving before "
                         "reporting success")
    ps.add_argument("--no-sync", action="store_true",
                    help="never switch the world to synchronous. Default: if the "
                         "world is async this client makes it sync and ticks it; "
                         "if it is already sync another client owns the clock and "
                         "this one only waits")
    ps.add_argument("--delta", type=float, default=0.05,
                    help="fixed_delta_seconds when this client makes the world "
                         "synchronous (default 0.05 = 20 Hz)")
    ps.add_argument("--keep", action="store_true",
                    help="leave the spawned pedestrians in the world on exit. "
                         "By default a held run destroys what it spawned")
    ps.add_argument("--hold", action="store_true",
                    help="stay alive pumping ticks so the crowd keeps walking (Ctrl+C to stop)")
    ps.set_defaults(func=cmd_spawn)

    sub.add_parser("destroy", help="stop + destroy controllers then walkers").set_defaults(func=cmd_destroy)

    args = p.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
