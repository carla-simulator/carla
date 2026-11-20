#!/usr/bin/env python

# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Measures how long a simulation step takes when a client updates or reads many
actors, so that changes to the client/server batching can be compared.

Modes:
  set   co-simulation: the client teleports every actor once per tick.
  tm    the Traffic Manager drives every actor on autopilot.
  read  the client reads every actor's transform and velocity once per tick.

--api picks how the client talks to the server in 'set' and 'read' mode:
  per_actor     one call per actor (actor.set_transform / actor.get_transform)
  legacy_sync   client.apply_batch_sync(commands), then world.tick()
  legacy_tick   client.apply_batch_sync(commands, True)
  legacy_async  client.apply_batch(commands), then world.tick()
  snapshot      'read' only: WorldSnapshot.find(id) on the tick's snapshot

--clients K runs K client processes. The first one ticks; all of them update
or read their share of the actors every frame, in step through a barrier.

With --hash, the transforms of every actor after every measured tick are
hashed (SHA-256, chained over frames), so two builds can be checked for
identical results with the same --seed.

The result is printed as JSON (and written to --out).
"""

import argparse
import hashlib
import json
import multiprocessing
import random
import resource
import statistics
import struct
import sys
import time

import carla

APIS = ('per_actor', 'legacy_sync', 'legacy_tick', 'legacy_async', 'snapshot')
PATH_LENGTH = 200
PATH_STEP = 0.5


def percentile(values, fraction):
    """Nearest-rank percentile of a list of numbers."""
    if not values:
        return 0.0
    ordered = sorted(values)
    index = min(len(ordered) - 1, max(0, int(round(fraction * (len(ordered) - 1)))))
    return ordered[index]


def cpu_seconds():
    """User + system CPU time of this process."""
    usage = resource.getrusage(resource.RUSAGE_SELF)
    return usage.ru_utime + usage.ru_stime


def counters_dict(counters):
    """carla.ServerPerfCounters as a dict, or None on servers without it."""
    if counters is None:
        return None
    names = ('frames', 'sync_calls', 'sync_call_ns', 'async_calls', 'rpc_loop_ns',
             'backlog_sum', 'backlog_max', 'snapshots', 'snapshot_ns', 'snapshot_bytes')
    return {name: getattr(counters, name) for name in names}


def read_server_counters(client):
    """Server counters, or None when the server predates them."""
    try:
        return counters_dict(client.get_server_perf_counters())
    except (AttributeError, RuntimeError):
        return None


def counters_delta(before, after, ticks):
    """Per-tick averages of the server counters between two readings."""
    if before is None or after is None:
        return None
    delta = {name: after[name] - before[name] for name in before}
    frames = max(1, delta['frames'])
    return {
        'frames': delta['frames'],
        'sync_calls_per_frame': delta['sync_calls'] / frames,
        'sync_call_ms_per_frame': delta['sync_call_ns'] / frames / 1e6,
        'async_calls_per_frame': delta['async_calls'] / frames,
        'rpc_loop_ms_per_frame': delta['rpc_loop_ns'] / frames / 1e6,
        'backlog_mean': delta['backlog_sum'] / frames,
        'backlog_max_total': after['backlog_max'],
        'snapshot_ms': delta['snapshot_ns'] / max(1, delta['snapshots']) / 1e6,
        'snapshot_bytes': delta['snapshot_bytes'] / max(1, delta['snapshots']),
        'ticks': ticks,
    }


def build_paths(world_map, count, seed):
    """One looping path of transforms per actor, spread over the road network."""
    rng = random.Random(seed)
    waypoints = world_map.generate_waypoints(4.0)
    rng.shuffle(waypoints)
    paths = []
    for start in waypoints:
        if len(paths) == count:
            break
        path = [start.transform]
        current = start
        for _ in range(PATH_LENGTH - 1):
            following = current.next(PATH_STEP)
            if not following:
                break
            current = following[0]
            path.append(current.transform)
        if len(path) >= 2:
            paths.append(path)
    if len(paths) < count:
        sys.exit(f'only {len(paths)} paths on this map, asked for {count} actors')
    return paths


def lifted(transform, height):
    """Copy of a transform raised by height metres."""
    location = transform.location
    return carla.Transform(
        carla.Location(location.x, location.y, location.z + height), transform.rotation)


def to_tuples(paths):
    """Paths as plain tuples, so they can be sent to other processes."""
    return [[(t.location.x, t.location.y, t.location.z,
              t.rotation.pitch, t.rotation.yaw, t.rotation.roll) for t in path]
            for path in paths]


def to_transforms(paths):
    """Inverse of to_tuples."""
    return [[carla.Transform(carla.Location(x, y, z), carla.Rotation(pitch, yaw, roll))
             for (x, y, z, pitch, yaw, roll) in path] for path in paths]


def spawn_actors(client, world, args):
    """Spawns the vehicles and returns (ids, paths)."""
    blueprints = [bp for bp in world.get_blueprint_library().filter('vehicle.*')
                  if int(bp.get_attribute('number_of_wheels')) == 4]
    blueprints.sort(key=lambda bp: bp.id)
    rng = random.Random(args.seed)
    paths = build_paths(world.get_map(), args.actors, args.seed)
    physics = args.mode == 'tm'
    commands = []
    for path in paths:
        blueprint = rng.choice(blueprints)
        blueprint.set_attribute('role_name', 'benchmark')
        spawn = carla.command.SpawnActor(blueprint, lifted(path[0], 0.5))
        if not physics:
            spawn = spawn.then(carla.command.SetSimulatePhysics(carla.command.FutureActor, False))
        commands.append(spawn)
    ids, kept = [], []
    for response, path in zip(client.apply_batch_sync(commands, True), paths):
        if not response.error:
            ids.append(response.actor_id)
            kept.append(path)
    return ids, kept


def transforms_digest(world, ids, step, digest):
    """Feeds every actor's transform of the current snapshot into digest.

    Actors are keyed by their spawn order and frames by the step number, not
    by actor ids and frame numbers, which differ from one run to the next.
    """
    snapshot = world.get_snapshot()
    digest.update(struct.pack('<I', step))
    for index, actor_id in enumerate(ids):
        state = snapshot.find(actor_id)
        if state is None:
            digest.update(struct.pack('<I', index))
            continue
        transform = state.get_transform()
        digest.update(struct.pack(
            '<I6f', index,
            transform.location.x, transform.location.y, transform.location.z,
            transform.rotation.pitch, transform.rotation.yaw, transform.rotation.roll))


class BenchmarkClient:
    """One client process. Client 0 ticks the world and collects the result."""

    @property
    def ticker(self):
        """Client 0 ticks the world."""
        return self.index == 0

    def __init__(self, args, share, index, client=None):
        self.args = args
        self.ids, plain_paths = share
        self.paths = to_transforms(plain_paths)
        self.index = index
        if client is None:
            client = carla.Client(args.host, args.port)
            client.set_timeout(args.timeout)
        self.client = client
        self.world = self.client.get_world()
        self.actors = {}
        if args.api == 'per_actor' or (args.mode == 'read' and args.api != 'snapshot'):
            self.actors = {actor.id: actor for actor in self.world.get_actors(self.ids)}

    def teleports(self, step):
        """The teleports of one step."""
        return [carla.command.ApplyTransform(actor_id, path[(step + i) % len(path)])
                for i, (actor_id, path) in enumerate(zip(self.ids, self.paths))]

    def update(self, step):
        """Sends this client's work for one step. Returns True if it ticked."""
        api = self.args.api
        if self.args.mode == 'set':
            if api == 'per_actor':
                for i, (actor_id, path) in enumerate(zip(self.ids, self.paths)):
                    self.actors[actor_id].set_transform(path[(step + i) % len(path)])
            elif api == 'legacy_tick' and self.ticker:
                self.client.apply_batch_sync(self.teleports(step), True)
                return True
            elif api == 'legacy_async':
                self.client.apply_batch(self.teleports(step))
            else:
                self.client.apply_batch_sync(self.teleports(step))
        elif self.args.mode == 'read':
            if api == 'snapshot':
                snapshot = self.world.get_snapshot()
                states = [snapshot.find(actor_id) for actor_id in self.ids]
                for state in states:
                    if state is not None:
                        state.get_transform()
                        state.get_velocity()
            else:
                for actor_id in self.ids:
                    self.actors[actor_id].get_transform()
                    self.actors[actor_id].get_velocity()
        return False

    def advance(self, ticked):
        """Moves the simulation one frame forward, or waits for it."""
        if self.ticker and not ticked and self.args.synchronous:
            self.world.tick(self.args.timeout)
        elif not self.args.synchronous:
            self.world.wait_for_tick(self.args.timeout)

    def run(self, barrier, results):
        """Runs the warm-up and the measured ticks; puts the result in results."""
        args = self.args
        digest = hashlib.sha256()
        tick_ms = []
        counters_before = None
        cpu_before = 0.0
        for step in range(args.warmup + args.ticks):
            if step == args.warmup:
                counters_before = read_server_counters(self.client) if self.ticker else None
                cpu_before = cpu_seconds()
            if barrier is not None:
                barrier.wait()
            start = time.perf_counter()
            ticked = self.update(step)
            if barrier is not None:
                barrier.wait()
            self.advance(ticked)
            if self.ticker and step >= args.warmup:
                tick_ms.append((time.perf_counter() - start) * 1e3)
                if args.hash:
                    transforms_digest(self.world, self.ids, step - args.warmup, digest)
        result = {'client': self.index, 'cpu_s': cpu_seconds() - cpu_before}
        if self.ticker:
            mean = statistics.fmean(tick_ms) if tick_ms else 0.0
            result.update({
                'tick_ms_p50': percentile(tick_ms, 0.50),
                'tick_ms_p95': percentile(tick_ms, 0.95),
                'tick_ms_p99': percentile(tick_ms, 0.99),
                'tick_ms_mean': mean,
                'ticks_per_s': 1e3 / mean if mean else 0.0,
                'server': counters_delta(
                    counters_before, read_server_counters(self.client), len(tick_ms)),
                'hash': digest.hexdigest() if args.hash else None,
            })
        results.put(result)


def run_client(args, share, index, barrier, results):
    """Process entry point."""
    BenchmarkClient(args, share, index).run(barrier, results)


def run_clients(args, ids, paths, client):
    """Runs every client; returns their results, client 0 first."""
    plain = to_tuples(paths)
    if args.clients == 1:
        # In this process, and through the client that owns the Traffic
        # Manager: a tick steps the TM once that client holds the new frame, so
        # a TM read through another client could see part of the previous one.
        queue = multiprocessing.Queue()
        BenchmarkClient(args, (ids, plain), 0, client).run(None, queue)
        return [queue.get()]
    # 'spawn', not 'fork': this process already holds a connected client whose
    # network threads would not survive a fork.
    context = multiprocessing.get_context('spawn')
    barrier = context.Barrier(args.clients)
    queue = context.Queue()
    processes = [
        context.Process(target=run_client,
                        args=(args, (ids[i::args.clients], plain[i::args.clients]), i,
                              barrier, queue))
        for i in range(args.clients)]
    for process in processes:
        process.start()
    results = [queue.get() for _ in processes]
    for process in processes:
        process.join()
    return sorted(results, key=lambda result: result['client'])


def parse_args():
    """Command line."""
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--host', default='127.0.0.1')
    parser.add_argument('-p', '--port', type=int, default=2000)
    parser.add_argument('--tm-port', type=int, default=8000)
    parser.add_argument('--mode', choices=('set', 'tm', 'read'), default='set')
    parser.add_argument('--api', choices=APIS, default='legacy_sync')
    parser.add_argument('-n', '--actors', type=int, default=500)
    parser.add_argument('--clients', type=int, default=1)
    parser.add_argument('--warmup', type=int, default=50)
    parser.add_argument('--ticks', type=int, default=500)
    parser.add_argument('--delta', type=float, default=0.05,
                        help='fixed_delta_seconds (default 0.05)')
    parser.add_argument('--asynchronous', dest='synchronous', action='store_false',
                        help='run the server in asynchronous mode')
    parser.add_argument('--hybrid', action='store_true',
                        help="'tm' mode: Traffic Manager hybrid physics")
    parser.add_argument('--seed', type=int, default=7)
    parser.add_argument('--hash', action='store_true',
                        help='hash every actor transform of every measured tick')
    parser.add_argument('--reload', action='store_true',
                        help='reload the world first, so that runs start from the same state')
    parser.add_argument('--timeout', type=float, default=60.0)
    parser.add_argument('--out', help='also write the JSON result here')
    args = parser.parse_args()
    if args.api == 'snapshot' and args.mode != 'read':
        parser.error('--api snapshot only applies to --mode read')
    if args.api == 'legacy_tick' and not args.synchronous:
        parser.error('--api legacy_tick needs synchronous mode')
    if args.mode == 'tm' and args.clients != 1:
        parser.error("--mode tm runs with a single client")
    return args


def main():
    """Sets the world up, runs the clients and prints the result."""
    args = parse_args()
    client = carla.Client(args.host, args.port)
    client.set_timeout(args.timeout)
    if args.reload:
        client.reload_world()
    world = client.get_world()
    original = world.get_settings()
    settings = world.get_settings()
    settings.synchronous_mode = args.synchronous
    settings.fixed_delta_seconds = args.delta
    world.apply_settings(settings)
    traffic_manager = client.get_trafficmanager(args.tm_port)
    traffic_manager.set_synchronous_mode(args.synchronous)
    traffic_manager.set_random_device_seed(args.seed)
    traffic_manager.set_hybrid_physics_mode(args.hybrid)
    ids = []
    try:
        ids, paths = spawn_actors(client, world, args)
        if args.mode == 'tm':
            client.apply_batch_sync(
                [carla.command.SetAutopilot(actor_id, True, traffic_manager.get_port())
                 for actor_id in ids], True)
        results = run_clients(args, ids, paths, client)
        summary = dict(results[0])
        summary.update({
            'mode': args.mode, 'api': args.api, 'actors': len(ids), 'clients': args.clients,
            'synchronous': args.synchronous, 'delta': args.delta, 'seed': args.seed,
            'hybrid': args.hybrid, 'server_version': client.get_server_version(),
            'map': world.get_map().name,
            'cpu_s_all_clients': sum(result['cpu_s'] for result in results),
        })
        text = json.dumps(summary, indent=2, sort_keys=True)
        print(text)
        if args.out:
            with open(args.out, 'w', encoding='utf-8') as out:
                out.write(text + '\n')
    finally:
        if ids:
            client.apply_batch_sync([carla.command.DestroyActor(actor_id) for actor_id in ids], True)
        traffic_manager.set_synchronous_mode(False)
        world.apply_settings(original)


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        pass
