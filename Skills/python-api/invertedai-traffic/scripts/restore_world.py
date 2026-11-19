#!/usr/bin/env python
"""Safety net after invertedai_traffic.py: put the world back in asynchronous
mode and report the vehicles, walkers and walker controllers still in it.

    restore_world.py HOST PORT CLEAN

The example destroys what it spawned on exit. Examples older than that never
did, so CLEAN=1 destroys every vehicle, walker and walker controller left. That
is only safe right after the example, which reloads the map first: everything
in the world is then its own, unless another client spawned something since.
"""
import sys

import carla


def main():
    """Clean up after the example, or report what it left."""
    host, port, clean = sys.argv[1], int(sys.argv[2]), sys.argv[3] == "1"
    client = carla.Client(host, port)
    client.set_timeout(20.0)
    world = client.get_world()
    # Async first: in a synchronous world nobody ticks, a fresh client has no
    # snapshot yet and get_actors() comes back empty (measured: 0 found, 25 left).
    s = world.get_settings()
    s.synchronous_mode, s.fixed_delta_seconds = False, None
    world.apply_settings(s)
    world.wait_for_tick(10.0)
    print("[run_example] world set back to asynchronous")
    actors = world.get_actors()
    controllers = list(actors.filter("controller.ai.walker"))
    bodies = list(actors.filter("vehicle.*")) + list(actors.filter("walker.*"))
    if not clean:
        print(f"[run_example] left in the world: {len(bodies)} vehicles/walkers, "
              f"{len(controllers)} walker controllers (CLEAN=1 destroys them)")
        return
    for c in controllers:
        c.stop()
    client.apply_batch_sync([carla.command.DestroyActor(a.id) for a in controllers + bodies])
    print(f"[run_example] destroyed {len(bodies)} vehicles/walkers and {len(controllers)} controllers")


if __name__ == "__main__":
    main()
