#!/usr/bin/env python

# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Measure how fast a server simulates traffic with no cameras.

Runs the world in synchronous mode with no-rendering mode on, spawns N NPC
vehicles on Traffic Manager autopilot for each N given, and reports ticks per
second, the real-time factor and the tick time. Meant for a server started
without a GPU (-nullrhi), but works against any server.

    ./CarlaUnreal.sh -nullrhi -RenderOffScreen
    python3 headless_benchmark.py --npcs 0,10,30,60,100 --ticks 300

See https://carla.readthedocs.io/en/latest/core_rendering/#running-without-a-gpu
"""

import argparse
import collections
import json
import random
import time

import carla


def parse_args():
    """Command-line options."""
    parser = argparse.ArgumentParser(description=__doc__.split('\n\n', maxsplit=1)[0].strip())
    parser.add_argument('--host', default='localhost')
    parser.add_argument('-p', '--port', type=int, default=2000)
    parser.add_argument('--tm-port', type=int, default=8000)
    parser.add_argument('--map', default='Town10HD_Opt',
                        help='map to load first if it is not the current one (default: Town10HD_Opt)')
    parser.add_argument('--npcs', default='0,10,30,60,100',
                        help='comma-separated NPC vehicle counts (default: 0,10,30,60,100)')
    parser.add_argument('--ticks', type=int, default=300, help='measured ticks per count (default: 300)')
    parser.add_argument('--warmup', type=int, default=40, help='ticks before measuring (default: 40)')
    parser.add_argument('--delta', type=float, default=0.05, help='fixed delta seconds (default: 0.05)')
    parser.add_argument('--seed', type=int, default=7)
    parser.add_argument('--json', action='store_true', help='print one JSON object per count')
    return parser.parse_args()


# what every measurement spawns from: the Traffic Manager port, the car
# blueprints and the map's spawn points
Fleet = collections.namedtuple('Fleet', 'tm_port blueprints spawn_points')


def destroy_vehicles(client, world):
    """Remove every vehicle from the world."""
    client.apply_batch_sync(
        [carla.command.DestroyActor(a) for a in world.get_actors().filter('vehicle.*')], True)


def measure(client, world, args, count, fleet):
    """Spawn ``count`` NPC vehicles on autopilot and time ``args.ticks`` ticks."""
    destroy_vehicles(client, world)
    random.shuffle(fleet.spawn_points)
    batch = [carla.command.SpawnActor(random.choice(fleet.blueprints), sp)
             .then(carla.command.SetAutopilot(carla.command.FutureActor, True, fleet.tm_port))
             for sp in fleet.spawn_points[:count]]
    spawned = sum(1 for r in client.apply_batch_sync(batch, True) if not r.error)
    for _ in range(args.warmup):
        world.tick()
    ticks = []
    start = time.perf_counter()
    for _ in range(args.ticks):
        tick_start = time.perf_counter()
        world.tick()
        ticks.append(time.perf_counter() - tick_start)
    elapsed = time.perf_counter() - start
    ticks.sort()
    return {
        'npcs': count,
        'spawned': spawned,
        'ticks_per_s': round(args.ticks / elapsed, 1),
        'realtime_factor': round(args.ticks * args.delta / elapsed, 2),
        'tick_ms_p50': round(1000 * ticks[len(ticks) // 2], 1),
        'tick_ms_p95': round(1000 * ticks[int(len(ticks) * 0.95)], 1),
    }


def main():
    """Run the benchmark for every requested NPC count."""
    args = parse_args()
    client = carla.Client(args.host, args.port)
    client.set_timeout(300.0)
    world = client.get_world()
    if args.map and args.map not in world.get_map().name:
        world = client.load_world(args.map)
    original = world.get_settings()
    settings = world.get_settings()
    settings.synchronous_mode = True
    settings.fixed_delta_seconds = args.delta
    settings.no_rendering_mode = True
    world.apply_settings(settings)

    tm = client.get_trafficmanager(args.tm_port)
    tm.set_synchronous_mode(True)
    tm.set_random_device_seed(args.seed)
    random.seed(args.seed)
    fleet = Fleet(tm.get_port(),
                  [bp for bp in world.get_blueprint_library().filter('vehicle.*')
                   if int(bp.get_attribute('number_of_wheels')) == 4],
                  world.get_map().get_spawn_points())

    if not args.json:
        print(f"{'npcs':>5} {'spawned':>8} {'ticks/s':>10} {'realtime':>10} "
              f"{'tick p50 ms':>12} {'tick p95 ms':>12}")
    try:
        for count in [int(n) for n in args.npcs.split(',') if n.strip()]:
            row = measure(client, world, args, count, fleet)
            if args.json:
                print(json.dumps(row), flush=True)
            else:
                print(f"{row['npcs']:>5} {row['spawned']:>8} {row['ticks_per_s']:>10} "
                      f"{row['realtime_factor']:>9}x {row['tick_ms_p50']:>12} {row['tick_ms_p95']:>12}",
                      flush=True)
    finally:
        destroy_vehicles(client, world)
        world.apply_settings(original)
        tm.set_synchronous_mode(False)


if __name__ == '__main__':
    main()
