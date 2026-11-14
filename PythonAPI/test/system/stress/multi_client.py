#!/usr/bin/env python3
"""Several client processes hammering one server with mixed requests at the same time."""

import multiprocessing as mp
import random
import time

import carla

import _common as C

ROLES = ("query", "spawn", "draw", "weather", "map")


def worker(idx, args, out):
    role = ROLES[idx % len(ROLES)]
    ops, errors, samples = 0, 0, []
    rng = random.Random(idx)
    try:
        c = C.client(args, timeout=20.0)
        world = c.get_world()
        bps = C.safe_vehicle_bps(world)
        sps = world.get_map().get_spawn_points()
    except RuntimeError as e:
        out.put((role, 0, 1, [f"connect: {e}"]))
        return
    end = time.time() + args.minutes * 60
    while time.time() < end:
        try:
            if role == "query":
                world.get_actors().filter("vehicle.*")
                world.get_snapshot()
                c.get_server_version()
            elif role == "spawn":
                a = world.try_spawn_actor(rng.choice(bps), rng.choice(sps))
                world.wait_for_tick(10.0)
                if a is not None:
                    a.destroy()
            elif role == "draw":
                p = rng.choice(sps).location
                world.debug.draw_point(p + carla.Location(z=2), 0.2, carla.Color(0, 255, 0), 1.0)
                world.debug.draw_string(p + carla.Location(z=3), f"c{idx}", life_time=1.0)
            elif role == "weather":
                w = world.get_weather()
                w.cloudiness = rng.uniform(0, 100)
                world.set_weather(w)
            elif role == "map":
                m = world.get_map()
                m.get_waypoint(rng.choice(sps).location).next(10.0)
            ops += 1
        except RuntimeError as e:
            errors += 1
            if len(samples) < 5:
                samples.append(str(e)[:200])
    out.put((role, ops, errors, samples))


def main():
    ap = C.parser(__doc__)
    ap.add_argument("--clients", type=int, default=10)
    ap.add_argument("--minutes", type=float, default=5)
    args = ap.parse_args()
    q = mp.Queue()
    procs = [mp.Process(target=worker, args=(i, args, q)) for i in range(args.clients)]
    for p in procs:
        p.start()
    results = [q.get(timeout=args.minutes * 60 + 300) for _ in procs]
    for p in procs:
        p.join(60)
    by_role = {}
    for role, ops, errors, _ in results:
        r = by_role.setdefault(role, [0, 0])
        r[0] += ops
        r[1] += errors
    C.destroy_all(C.client(args), ("vehicle.*",))
    C.result(clients=len(results), ops=sum(r[1] for r in results),
             errors=sum(r[2] for r in results), by_role=by_role,
             error_samples=[s for r in results for s in r[3]][:10])


if __name__ == "__main__":
    main()
