#!/usr/bin/env python3
"""As many autopilot vehicles as the map holds plus a crowd, synchronous, for minutes."""

import random

import carla

import _common as C


def spawn_walkers(c, world, n, rng):
    bps = world.get_blueprint_library().filter("walker.pedestrian.*")
    locs = []
    for _ in range(n * 2):
        loc = world.get_random_location_from_navigation()
        if loc is not None:
            locs.append(carla.Transform(loc + carla.Location(z=1)))
        if len(locs) == n:
            break
    res = c.apply_batch_sync([carla.command.SpawnActor(rng.choice(bps), t) for t in locs], True)
    walkers = [r.actor_id for r in res if not r.error]
    cbp = world.get_blueprint_library().find("controller.ai.walker")
    res = c.apply_batch_sync([carla.command.SpawnActor(cbp, carla.Transform(), w)
                              for w in walkers], True)
    ctrls = [r.actor_id for r in res if not r.error]
    world.tick()
    for a in world.get_actors(ctrls):
        a.start()
        a.go_to_location(world.get_random_location_from_navigation())
        a.set_max_speed(1.4)
    return walkers, ctrls


def main():
    ap = C.parser(__doc__)
    ap.add_argument("--vehicles", type=int, default=250)
    ap.add_argument("--walkers", type=int, default=200)
    ap.add_argument("--minutes", type=float, default=10)
    args = ap.parse_args()
    c = C.client(args)
    world = c.get_world()
    tm = c.get_trafficmanager(args.tm_port)
    rng = random.Random(0)
    C.set_sync(world, True)
    tm.set_synchronous_mode(True)
    tm.set_random_device_seed(0)
    try:
        sps = world.get_map().get_spawn_points()
        bps = C.safe_vehicle_bps(world)
        batch = [carla.command.SpawnActor(rng.choice(bps), sp).then(
                 carla.command.SetAutopilot(carla.command.FutureActor, True, args.tm_port))
                 for sp in sps[:args.vehicles]]
        vehicles = [r.actor_id for r in c.apply_batch_sync(batch, True) if not r.error]
        walkers, ctrls = spawn_walkers(c, world, args.walkers, rng)
        print(f"{len(vehicles)} vehicles, {len(walkers)} walkers", flush=True)
        ticks, end, start = [], C.now() + args.minutes * 60, C.now()
        sim0 = world.get_snapshot().timestamp.elapsed_seconds
        while C.now() < end:
            t = C.now()
            world.tick(30.0)
            ticks.append(C.now() - t)
        wall = C.now() - start
        sim = world.get_snapshot().timestamp.elapsed_seconds - sim0
        alive = len(world.get_actors().filter("vehicle.*"))
    finally:
        C.destroy_all(c)
        tm.set_synchronous_mode(False)
        C.set_sync(world, False)
    C.result(vehicles=len(vehicles), walkers=len(walkers), vehicles_alive_at_end=alive,
             ticks=len(ticks), tick_p50_s=C.percentile(ticks, 0.5),
             tick_p99_s=C.percentile(ticks, 0.99), tick_max_s=max(ticks),
             sim_over_wall=sim / wall, fps=len(ticks) / wall)


if __name__ == "__main__":
    main()
