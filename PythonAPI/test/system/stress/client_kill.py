#!/usr/bin/env python3
"""SIGKILL a client that owns the clock (sync mode + Traffic Manager); can a new client recover?"""

import os
import signal
import subprocess
import sys
import time

import carla

import _common as C


def victim(args):
    c = C.client(args)
    world = c.get_world()
    tm = c.get_trafficmanager(args.tm_port)
    C.set_sync(world, True)
    tm.set_synchronous_mode(True)
    batch = [carla.command.SpawnActor(bp, sp).then(
             carla.command.SetAutopilot(carla.command.FutureActor, True, args.tm_port))
             for bp, sp in zip(C.safe_vehicle_bps(world) * 5, world.get_map().get_spawn_points()[:20])]
    c.apply_batch_sync(batch, True)
    print("ticking", flush=True)
    while True:
        world.tick(30.0)


def main():
    ap = C.parser(__doc__)
    ap.add_argument("--victim", action="store_true")
    args = ap.parse_args()
    if args.victim:
        return victim(args)

    cmd = [sys.executable, __file__, "--victim", "--port", str(args.port),
           "--tm-port", str(args.tm_port)]
    p = subprocess.Popen(cmd, stdout=subprocess.PIPE, text=True)
    line = p.stdout.readline()
    if "ticking" not in line:
        C.result(error=f"victim did not start: {line!r}", recovered=False)
    time.sleep(10)
    os.kill(p.pid, signal.SIGKILL)
    p.wait()

    t = C.now()
    c = C.client(args, timeout=30.0)
    world = c.get_world()
    was_sync = world.get_settings().synchronous_mode
    C.set_sync(world, False)
    world.wait_for_tick(30.0)
    recovered_s = C.now() - t
    leftover = len(world.get_actors().filter("vehicle.*"))

    # A new Traffic Manager must be able to take over the port the dead one held.
    tm_ok, tm_err = False, ""
    try:
        tm = c.get_trafficmanager(args.tm_port)
        bp = C.safe_vehicle_bps(world)[0]
        v = world.try_spawn_actor(bp, world.get_map().get_spawn_points()[50])
        if v is not None:
            v.set_autopilot(True, tm.get_port())
            time.sleep(5)
            sp = v.get_velocity()
            tm_ok = (sp.x ** 2 + sp.y ** 2) ** 0.5 > 0.5
            v.set_autopilot(False, tm.get_port())
    except RuntimeError as e:
        tm_err = str(e)[:300]
    C.destroy_all(c, ("vehicle.*",))
    C.result(recovered=True, recovered_s=recovered_s, was_sync_after_kill=was_sync,
             leftover_vehicles=leftover, tm_takeover_drives=tm_ok, tm_error=tm_err)


if __name__ == "__main__":
    main()
