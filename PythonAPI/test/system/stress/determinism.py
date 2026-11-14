#!/usr/bin/env python3
"""Run the same seeded synchronous Traffic Manager scene twice and compare trajectories."""

import carla

import _common as C


def episode(c, args):
    world = c.load_world(args.map)
    tm = c.get_trafficmanager(args.tm_port)
    C.set_sync(world, True)
    tm.set_synchronous_mode(True)
    tm.set_random_device_seed(42)
    try:
        bps = C.safe_vehicle_bps(world)
        sps = world.get_map().get_spawn_points()[:args.vehicles]
        batch = [carla.command.SpawnActor(bps[i % len(bps)], sp).then(
                 carla.command.SetAutopilot(carla.command.FutureActor, True, args.tm_port))
                 for i, sp in enumerate(sps)]
        ids = [r.actor_id for r in c.apply_batch_sync(batch, True) if not r.error]
        actors = [world.get_actor(i) for i in ids]
        track = []
        for _ in range(args.ticks):
            world.tick(30.0)
            track.append([(a.get_location().x, a.get_location().y) for a in actors])
        return track
    finally:
        C.destroy_all(c, ("vehicle.*",))
        tm.set_synchronous_mode(False)
        C.set_sync(world, False)


def main():
    ap = C.parser(__doc__)
    ap.add_argument("--map", default="Town03_Opt")
    ap.add_argument("--vehicles", type=int, default=30)
    ap.add_argument("--ticks", type=int, default=600)
    args = ap.parse_args()
    c = C.client(args, timeout=300.0)
    a, b = episode(c, args), episode(c, args)
    diffs = [((p[0] - q[0]) ** 2 + (p[1] - q[1]) ** 2) ** 0.5
             for ra, rb in zip(a, b) for p, q in zip(ra, rb)]
    first = next((i for i, (ra, rb) in enumerate(zip(a, b))
                  if any(((p[0] - q[0]) ** 2 + (p[1] - q[1]) ** 2) ** 0.5 > 0.01
                         for p, q in zip(ra, rb))), None)
    moved = sum(((r0[0] - r1[0]) ** 2 + (r0[1] - r1[1]) ** 2) ** 0.5
                for r0, r1 in zip(a[0], a[-1])) / max(len(a[0]), 1)
    c.load_world("Town10HD_Opt")
    C.result(vehicles=len(a[0]), ticks=len(a), max_diff_m=max(diffs),
             mean_diff_m=sum(diffs) / len(diffs), first_divergent_tick=first,
             mean_travel_m=moved)


if __name__ == "__main__":
    main()
