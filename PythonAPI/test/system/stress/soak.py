#!/usr/bin/env python3
"""Long synchronous run with traffic and the recorder on; sample server memory each minute."""

import re
import random

import carla

import _common as C


def main():
    ap = C.parser(__doc__)
    ap.add_argument("--minutes", type=float, default=60)
    ap.add_argument("--vehicles", type=int, default=100)
    ap.add_argument("--rec", default="system_test_soak.log", help="recorder file (server side)")
    args = ap.parse_args()
    c = C.client(args)
    world = c.get_world()
    tm = c.get_trafficmanager(args.tm_port)
    C.set_sync(world, True)
    tm.set_synchronous_mode(True)
    tm.set_random_device_seed(1)
    rng = random.Random(1)
    samples, ticks = [], 0
    try:
        bps = C.safe_vehicle_bps(world)
        batch = [carla.command.SpawnActor(rng.choice(bps), sp).then(
                 carla.command.SetAutopilot(carla.command.FutureActor, True, args.tm_port))
                 for sp in world.get_map().get_spawn_points()[:args.vehicles]]
        n = len([r for r in c.apply_batch_sync(batch, True) if not r.error])
        c.start_recorder(args.rec)
        start = C.now()
        next_sample = start
        while C.now() - start < args.minutes * 60:
            world.tick(30.0)
            ticks += 1
            if C.now() >= next_sample:
                samples.append((round((C.now() - start) / 60, 2), C.rss_mb(args.server_pid)))
                print(f"t={samples[-1][0]} min ticks={ticks} rss={samples[-1][1]} MB", flush=True)
                next_sample += 60
        c.stop_recorder()
    finally:
        C.destroy_all(c)
        tm.set_synchronous_mode(False)
        C.set_sync(world, False)
    info = c.show_recorder_file_info(args.rec, False)
    m = re.search(r"Frames:\s*(\d+)", info)
    # Memory trend after a 5-minute warm-up, least squares, in MB per 10 minutes.
    pts = [(t, r) for t, r in samples if t >= 5 and r is not None]
    slope = None
    if len(pts) >= 3:
        mt = sum(t for t, _ in pts) / len(pts)
        mr = sum(r for _, r in pts) / len(pts)
        slope = 10 * sum((t - mt) * (r - mr) for t, r in pts) / sum((t - mt) ** 2 for t, _ in pts)
    C.result(vehicles=n, minutes=args.minutes, ticks=ticks, rss_samples=samples,
             rss_slope_mb_per_10min=slope, recorder_frames=int(m.group(1)) if m else 0)


if __name__ == "__main__":
    main()
