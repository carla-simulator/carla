#!/usr/bin/env python3
"""Alternate between two maps many times; time each load and watch server memory."""

import time

import _common as C


def main():
    ap = C.parser(__doc__)
    ap.add_argument("--loads", type=int, default=20)
    ap.add_argument("--maps", default="Town01_Opt,Town10HD_Opt")
    args = ap.parse_args()
    maps = args.maps.split(",")
    c = C.client(args, timeout=300.0)
    secs, rss, wrong = [], [], 0
    for i in range(args.loads):
        m = maps[i % len(maps)]
        t = C.now()
        world = c.load_world(m)
        world.wait_for_tick(60.0)
        secs.append(round(C.now() - t, 1))
        if not world.get_map().name.endswith(m):
            wrong += 1
        time.sleep(5)
        rss.append(C.rss_mb(args.server_pid))
        print(f"load {i + 1}: {m} {secs[-1]}s rss {rss[-1]} MB", flush=True)
    if not c.get_world().get_map().name.endswith("Town10HD_Opt"):
        c.load_world("Town10HD_Opt")
    C.result(loads=len(secs), load_s=secs, max_load_s=max(secs), wrong_map=wrong, rss_mb=rss,
             rss_growth=(rss[-1] / rss[1]) if len(rss) > 1 and rss[1] else None)


if __name__ == "__main__":
    main()
