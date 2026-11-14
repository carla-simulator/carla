#!/usr/bin/env python3
"""Spawn and destroy a full map of parked vehicles, many times; watch for leaks."""

import random

import carla

import _common as C


def main():
    ap = C.parser(__doc__)
    ap.add_argument("--count", type=int, default=300)
    ap.add_argument("--cycles", type=int, default=30)
    args = ap.parse_args()
    c = C.client(args)
    world = c.get_world()
    bps = C.safe_vehicle_bps(world)
    sps = world.get_map().get_spawn_points()
    n = min(args.count, len(sps))
    rng = random.Random(0)
    spawned, leaked, rss, cycle_s = [], [], [], []
    for i in range(args.cycles):
        t = C.now()
        res = c.apply_batch_sync([carla.command.SpawnActor(rng.choice(bps), sp)
                                  for sp in sps[:n]], False)
        ids = [r.actor_id for r in res if not r.error]
        spawned.append(len(ids))
        for _ in range(20):
            world.wait_for_tick(10.0)
        c.apply_batch_sync([carla.command.DestroyActor(x) for x in ids], False)
        for _ in range(10):
            world.wait_for_tick(10.0)
        leaked.append(len(world.get_actors().filter("vehicle.*")))
        rss.append(C.rss_mb(args.server_pid))
        cycle_s.append(round(C.now() - t, 2))
        print(f"cycle {i + 1}: spawned {len(ids)} leaked {leaked[-1]} rss {rss[-1]} MB", flush=True)
    C.destroy_all(c)
    C.result(spawn_points=len(sps), per_cycle=n, min_spawned=min(spawned),
             max_leaked=max(leaked), rss_mb=rss, cycle_s=cycle_s,
             rss_growth=(rss[-1] / rss[1]) if rss[1] else None)


if __name__ == "__main__":
    main()
