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
  updates       client.apply_actor_updates(batch) from numpy arrays, then
                world.tick(); the ticking client sends apply_actor_updates(batch,
                True) instead
  updates_async client.apply_actor_updates_async(batch), then world.tick()
  snapshot      'read' only: WorldSnapshot.find(id) on the tick's snapshot
  bulk          'read' only: WorldSnapshot.get_transforms(ids) and
                get_velocities(ids) on the tick's snapshot, into numpy arrays

--clients K runs K client processes. The first one ticks; all of them update
or read their share of the actors every frame, in step through a barrier. In
'tm' mode the first client owns the Traffic Manager and the others only read
the whole snapshot every frame (with the bulk getters).

With --hash, the transforms of every actor after every measured tick are
hashed (SHA-256, chained over frames), so two builds can be checked for
identical results with the same --seed. With --hash-all (synchronous mode),
every client waits for the ticked frame and hashes the whole snapshot: every
actor's location (double), rotation, velocity, angular velocity and
acceleration. The clients of one run must agree; two runs agree only when the
simulation itself is deterministic (see --deterministic).

Socket load: --parked M adds M vehicles that stand still with physics on,
--cameras N attaches N 1920x1080 RGB cameras and --lidar a 64-channel LiDAR
(listened to by the first process), and --reader-delay-ms makes the last
client's on_tick callback slow. --change-stats turns on the server's snapshot
change statistics (console variable carla.Snapshot.Stats) for the run.

The result is printed as JSON (and written to --out). Besides the per-tick
server counters it reports what the streaming sessions sent
(client.get_stream_stats) and, in asynchronous mode, how many frames each
client received and skipped.
"""

import argparse
import hashlib
import json
import multiprocessing
import os
import random
import resource
import socket
import statistics
import struct
import sys
import time

import carla

APIS = ('per_actor', 'legacy_sync', 'legacy_tick', 'legacy_async', 'updates', 'updates_async',
        'snapshot', 'bulk')
READ_ONLY_APIS = ('snapshot', 'bulk')
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


COUNTER_NAMES = (
    'frames', 'sync_calls', 'sync_call_ns', 'async_calls', 'rpc_loop_ns',
    'backlog_sum', 'backlog_max', 'snapshots', 'snapshot_ns', 'snapshot_bytes',
    'snapshot_writes', 'snapshot_wire_bytes', 'snapshot_drops', 'snapshot_blocked_ns',
    'snapshot_queue_ns', 'snapshot_send_ns',
    'stream_writes', 'stream_wire_bytes', 'stream_drops', 'stream_blocked_ns',
    'stream_queue_ns', 'stream_send_ns', 'snapshot_encode_ns')


def counters_dict(counters):
    """carla.ServerPerfCounters as a dict, or None on servers without it.

    Counters a server does not have read as 0.
    """
    if counters is None:
        return None
    return {name: getattr(counters, name, 0) for name in COUNTER_NAMES}


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
        'snapshot_encode_ms': delta['snapshot_encode_ns'] / max(1, delta['snapshots']) / 1e6,
        'snapshot_wire_bytes_per_frame': delta['snapshot_wire_bytes'] / frames,
        'snapshot_writes_per_frame': delta['snapshot_writes'] / frames,
        'snapshot_drops': delta['snapshot_drops'],
        'snapshot_blocked_ms_per_frame': delta['snapshot_blocked_ns'] / frames / 1e6,
        'snapshot_queue_us_per_write':
            delta['snapshot_queue_ns'] / max(1, delta['snapshot_writes']) / 1e3,
        'snapshot_send_us_per_write':
            delta['snapshot_send_ns'] / max(1, delta['snapshot_writes']) / 1e3,
        'stream_wire_bytes_per_frame': delta['stream_wire_bytes'] / frames,
        'stream_drops': delta['stream_drops'],
        'stream_blocked_ms_per_frame': delta['stream_blocked_ns'] / frames / 1e6,
        'stream_queue_us_per_write':
            delta['stream_queue_ns'] / max(1, delta['stream_writes']) / 1e3,
        'ticks': ticks,
    }


def read_stream_stats(client):
    """client.get_stream_stats(), or None when the server predates it."""
    try:
        return client.get_stream_stats()
    except (AttributeError, RuntimeError):
        return None


def histogram_percentile_us(buckets, fraction):
    """Upper bound in microseconds of the log2 bucket holding the percentile."""
    total = sum(buckets)
    if total == 0:
        return 0.0
    rank = fraction * total
    seen = 0
    for index, count in enumerate(buckets):
        seen += count
        if seen >= rank:
            return float(2 ** (index + 1))
    return float(2 ** len(buckets))


def stream_stats_delta(before, after, ticks):
    """What the streaming sessions did between two get_stream_stats readings."""
    if before is None or after is None:
        return None
    result = {}
    for name in ('episode', 'other'):
        old, new = before[name], after[name]
        delta = {key: new[key] - old[key] for key in new if not isinstance(new[key], list)}
        histograms = {key: [b - a for a, b in zip(old[key], new[key])]
                      for key in new if isinstance(new[key], list)}
        writes = max(1, delta['writes'])
        result[name] = {
            'writes': delta['writes'],
            'wire_bytes_per_tick': delta['wire_bytes'] / max(1, ticks),
            'wire_bytes_per_write': delta['wire_bytes'] / writes,
            'busy_drops': delta['busy_drops'],
            'closed_drops': delta['closed_drops'],
            'timeouts': delta['timeouts'],
            'blocked_writes': delta['blocked_writes'],
            'blocked_ms_per_tick': delta['blocked_ns'] / max(1, ticks) / 1e6,
            'queue_us_mean': delta['queue_ns'] / writes / 1e3,
            'send_us_mean': delta['send_ns'] / writes / 1e3,
            'queue_us_p99': histogram_percentile_us(histograms['queue_histogram'], 0.99),
            'send_us_p99': histogram_percentile_us(histograms['send_histogram'], 0.99),
            'blocked_us_p99': histogram_percentile_us(histograms['blocked_histogram'], 0.99),
        }
    result['episode_sessions'] = sum(1 for row in after['sessions'] if row['episode'])
    result['other_sessions'] = sum(1 for row in after['sessions'] if not row['episode'])
    result['snapshot_changes'] = snapshot_changes_delta(
        before['snapshot_changes'], after['snapshot_changes'])
    return result


def read_snapshot_stream(world):
    """world.get_snapshot_stream_stats(), or None when the client predates it."""
    try:
        return world.get_snapshot_stream_stats()
    except AttributeError:
        return None


def snapshot_stream_delta(before, after):
    """What one client's snapshot stream received between two readings."""
    if before is None or after is None:
        return None
    delta = {key: after[key] - before[key] for key in after if key != 'digests'}
    messages = max(1, delta['messages'])
    return {
        'messages': delta['messages'],
        'keyframes': delta['keyframes'],
        'deltas': delta['deltas'],
        'resyncs': delta['resyncs'],
        'incompatible': delta['incompatible'],
        'wire_bytes_per_message': delta['wire_bytes'] / messages,
        'published_bytes_per_message': delta['published_bytes'] / messages,
        'decode_us_per_message': delta['decode_ns'] / messages / 1e3,
        'callback_us_per_message': delta['callback_ns'] / messages / 1e3,
    }


def snapshot_changes_delta(before, after):
    """Change statistics of the snapshot (carla.Snapshot.Stats) between two readings."""
    frames = after['frames'] - before['frames']
    if frames <= 0:
        return None
    keyframe_bytes = after['keyframe_bytes'] - before['keyframe_bytes']
    delta_bytes = after['delta_bytes'] - before['delta_bytes']
    by_type = {}
    for name, new in after['by_type'].items():
        old = before['by_type'][name]
        records = new['records'] - old['records']
        if records <= 0:
            continue
        by_type[name] = {
            'records_per_frame': records / frames,
            'unchanged_fraction': (new['unchanged'] - old['unchanged']) / records,
            'added_per_frame': (new['added'] - old['added']) / frames,
            'group_changed_fraction': [
                (b - a) / records for a, b in zip(old['group_changed'], new['group_changed'])],
        }
    return {
        'frames': frames,
        'keyframes': after['keyframes'] - before['keyframes'],
        'removed_per_frame': (after['removed'] - before['removed']) / frames,
        'keyframe_bytes_per_frame': keyframe_bytes / frames,
        'delta_bytes_per_frame': delta_bytes / frames,
        'delta_to_keyframe': delta_bytes / keyframe_bytes if keyframe_bytes else None,
        'by_type': by_type,
    }


def console_command(host, port, command, timeout=10.0):
    """Runs an Unreal console command on the server through the raw
    console_command RPC (the Python API has no binding for it)."""
    command_bytes = command.encode('utf-8')
    method = b'console_command'
    request = bytearray(b'\x94\x00\x01')  # [request, msgid 1, method, params]
    request += bytes([0xa0 | len(method)]) + method
    request += b'\x92\x91\xc2'  # params: [[False], command]
    if len(command_bytes) <= 0x1f:
        request += bytes([0xa0 | len(command_bytes)]) + command_bytes
    else:
        request += b'\xd9' + struct.pack('B', len(command_bytes)) + command_bytes
    with socket.create_connection((host, port), timeout=timeout) as connection:
        connection.sendall(bytes(request))
        reply = connection.recv(1 << 16)
    # Reply [response, msgid, nil error, [[1, bool]]]: the bool is the last byte.
    if len(reply) < 8 or reply[3] != 0xc0 or reply[7] != 0xc3:
        sys.exit('the server refused the console command %r' % command)


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
        # Only whole paths: the array APIs need the same length for every
        # actor, and on a large map some roads end before PATH_LENGTH steps.
        if len(path) == PATH_LENGTH:
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
    """Spawns the vehicles and returns (ids, paths, parked_ids).

    The parked vehicles start where the paths after the benchmark's own begin,
    keep physics on and get no control.
    """
    blueprints = [bp for bp in world.get_blueprint_library().filter('vehicle.*')
                  if int(bp.get_attribute('number_of_wheels')) == 4]
    blueprints.sort(key=lambda bp: bp.id)
    rng = random.Random(args.seed)
    paths = build_paths(world.get_map(), args.actors + args.parked, args.seed)
    parked_paths = paths[args.actors:]
    paths = paths[:args.actors]
    physics = args.mode == 'tm'
    commands = []
    for path in paths:
        blueprint = rng.choice(blueprints)
        blueprint.set_attribute('role_name', 'benchmark')
        spawn = carla.command.SpawnActor(blueprint, lifted(path[0], args.lift))
        if not physics:
            spawn = spawn.then(carla.command.SetSimulatePhysics(carla.command.FutureActor, False))
        commands.append(spawn)
    for path in parked_paths:
        blueprint = rng.choice(blueprints)
        blueprint.set_attribute('role_name', 'parked')
        commands.append(carla.command.SpawnActor(blueprint, lifted(path[0], args.lift)))
    ids, kept, parked = [], [], []
    responses = client.apply_batch_sync(commands, True)
    for response, path in zip(responses[:len(paths)], paths):
        if not response.error:
            ids.append(response.actor_id)
            kept.append(path)
    for response in responses[len(paths):]:
        if not response.error:
            parked.append(response.actor_id)
    return ids, kept, parked


def spawn_sensors(world, args, ids):
    """Spawns --cameras RGB cameras and the --lidar, attached to the first
    benchmark vehicles, and listens to them. Returns (sensors, received), where
    received counts the messages and bytes of each kind."""
    library = world.get_blueprint_library()
    received = {'camera_frames': 0, 'camera_bytes': 0, 'lidar_frames': 0, 'lidar_points': 0}
    sensors = []
    mount = carla.Transform(carla.Location(x=0.5, z=2.0))
    for index in range(args.cameras):
        blueprint = library.find('sensor.camera.rgb')
        blueprint.set_attribute('image_size_x', '1920')
        blueprint.set_attribute('image_size_y', '1080')
        parent = world.get_actor(ids[index % len(ids)])
        camera = world.spawn_actor(blueprint, mount, attach_to=parent)

        def on_image(image):
            received['camera_frames'] += 1
            received['camera_bytes'] += image.width * image.height * 4
        camera.listen(on_image)
        sensors.append(camera)
    if args.lidar:
        blueprint = library.find('sensor.lidar.ray_cast')
        blueprint.set_attribute('channels', '64')
        blueprint.set_attribute('points_per_second', '1300000')
        blueprint.set_attribute('rotation_frequency', str(1.0 / args.delta))
        blueprint.set_attribute('range', '100')
        lidar = world.spawn_actor(blueprint, mount, attach_to=world.get_actor(ids[0]))

        def on_scan(scan):
            received['lidar_frames'] += 1
            received['lidar_points'] += len(scan)
        lidar.listen(on_scan)
        sensors.append(lidar)
    return sensors, received


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


def snapshot_digest(snapshot, keyed_ids, step, digest):
    """Feeds the whole snapshot into digest: first the actors of keyed_ids in
    that order (keyed by position, since actor ids differ between runs), then
    every other actor in ascending id order, without its id. Each actor adds
    its location (double), rotation, velocity, angular velocity and
    acceleration, as the bulk getters return them."""
    import numpy  # pylint: disable=import-outside-toplevel
    all_ids = snapshot.get_actor_ids()
    keyed = set(keyed_ids)
    others = numpy.array([i for i in all_ids if i not in keyed], dtype=numpy.uint32)
    digest.update(struct.pack('<II', step, len(all_ids)))
    for ids in (numpy.array(keyed_ids, dtype=numpy.uint32), others):
        transforms, mask = snapshot.get_transforms(ids, return_mask=True)
        digest.update(mask.tobytes())
        digest.update(transforms.tobytes())
        digest.update(snapshot.get_velocities(ids).tobytes())
        digest.update(snapshot.get_angular_velocities(ids).tobytes())
        digest.update(snapshot.get_accelerations(ids).tobytes())


def wait_for_frame(world, frame, timeout):
    """The snapshot of frame (or a later one) once this client has it."""
    deadline = time.monotonic() + timeout
    while True:
        snapshot = world.get_snapshot()
        if snapshot.frame >= frame:
            return snapshot
        if time.monotonic() > deadline:
            raise RuntimeError('frame %d did not arrive in %.0f s' % (frame, timeout))
        time.sleep(0.0002)


class FrameCounter:  # pylint: disable=too-few-public-methods
    """Counts the snapshots an on_tick callback receives, and the frames it
    never saw between two of them."""

    def __init__(self, delay_ms=0.0):
        self.delivered = 0
        self.skipped = 0
        self.last = None
        self.delay = delay_ms / 1e3

    def __call__(self, snapshot):
        if self.last is not None and snapshot.frame > self.last + 1:
            self.skipped += snapshot.frame - self.last - 1
        if self.last is None or snapshot.frame > self.last:
            self.last = snapshot.frame
        self.delivered += 1
        if self.delay > 0.0:
            time.sleep(self.delay)


class TeleportArrays:  # pylint: disable=too-few-public-methods
    """The teleports of every step as numpy arrays, for carla.ActorUpdateBatch."""

    def __init__(self, ids, plain_paths):
        import numpy  # pylint: disable=import-outside-toplevel
        self.batch = carla.ActorUpdateBatch()
        self.ids = numpy.array(ids, dtype=numpy.uint32)
        poses = numpy.array(plain_paths, dtype=numpy.float64).reshape((len(ids), -1, 6))
        self.locations = poses[:, :, 0:3].copy()
        self.rotations = poses[:, :, 3:6].astype(numpy.float32)
        self.rows = numpy.arange(len(ids))

    def fill(self, step):
        """The batch of teleports of one step."""
        columns = (step + self.rows) % self.locations.shape[1]
        self.batch.add_transforms(
            self.ids, self.locations[self.rows, columns], self.rotations[self.rows, columns])
        return self.batch


class BenchmarkClient:  # pylint: disable=too-many-instance-attributes
    """One client process. Client 0 ticks the world and collects the result."""

    @property
    def ticker(self):
        """Client 0 ticks the world."""
        return self.index == 0

    def __init__(self, args, share, index, client=None):
        self.args = args
        self.ids, plain_paths, self.all_ids, self.parked_ids = share
        self.paths = to_transforms(plain_paths)
        self.index = index
        if client is None:
            client = carla.Client(args.host, args.port)
            client.set_timeout(args.timeout)
        self.client = client
        self.world = self.client.get_world()
        self.actors = {}
        if args.api == 'per_actor' or (args.mode == 'read' and args.api not in READ_ONLY_APIS):
            self.actors = {actor.id: actor for actor in self.world.get_actors(self.ids)}
        self.updates = TeleportArrays(self.ids, plain_paths) \
            if args.api.startswith('updates') else None
        self.bulk = None
        self.frames = None
        # The last client (never the ticker) is the slow reader.
        delay = args.reader_delay_ms if 0 < index == args.clients - 1 else 0.0
        if not args.synchronous or delay > 0.0:
            self.frames = FrameCounter(delay)
            self.world.on_tick(self.frames)
        if args.api == 'bulk' or (args.mode == 'tm' and not self.ticker):
            import numpy  # pylint: disable=import-outside-toplevel
            self.bulk = (numpy.array(self.ids, dtype=numpy.uint32),
                         numpy.empty((len(self.ids), 6)),
                         numpy.empty((len(self.ids), 3)))

    def teleports(self, step):
        """The teleports of one step."""
        return [carla.command.ApplyTransform(actor_id, path[(step + i) % len(path)])
                for i, (actor_id, path) in enumerate(zip(self.ids, self.paths))]

    def update(self, step):
        """Sends this client's work for one step. Returns True if it ticked."""
        if self.args.mode == 'set':
            return self.update_set(step)
        if self.args.mode == 'read':
            self.update_read()
        elif self.args.mode == 'tm' and not self.ticker:
            self.update_listener()
        return False

    def update_listener(self):
        """'tm' mode with several clients: the clients that do not own the
        Traffic Manager read every actor of the snapshot."""
        snapshot = self.world.get_snapshot()
        ids = snapshot.get_actor_ids()
        snapshot.get_transforms(ids)
        snapshot.get_velocities(ids)

    def update_set(self, step):
        """Teleports this client's actors. Returns True if it ticked."""
        api = self.args.api
        ticks_here = self.ticker and self.args.synchronous
        if api == 'per_actor':
            for i, (actor_id, path) in enumerate(zip(self.ids, self.paths)):
                self.actors[actor_id].set_transform(path[(step + i) % len(path)])
        elif api == 'legacy_tick' and self.ticker:
            self.client.apply_batch_sync(self.teleports(step), True)
            return True
        elif api == 'legacy_async':
            self.client.apply_batch(self.teleports(step))
        elif api == 'updates':
            self.client.apply_actor_updates(self.updates.fill(step), ticks_here)
            return ticks_here
        elif api == 'updates_async':
            self.client.apply_actor_updates_async(self.updates.fill(step))
        else:
            self.client.apply_batch_sync(self.teleports(step))
        return False

    def update_read(self):
        """Reads every actor of this client."""
        if self.args.api == 'bulk':
            ids, transforms, velocities = self.bulk
            snapshot = self.world.get_snapshot()
            snapshot.get_transforms(ids, out=transforms)
            snapshot.get_velocities(ids, out=velocities)
        elif self.args.api == 'snapshot':
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

    def advance(self, ticked):
        """Moves the simulation one frame forward, or waits for it. Returns
        the frame ticked by this client, if any."""
        if self.ticker and not ticked and self.args.synchronous:
            return self.world.tick(self.args.timeout)
        if not self.args.synchronous:
            self.world.wait_for_tick(self.args.timeout)
        return None

    def parked_stability(self, previous):
        """Bytes of the parked vehicles' location, rotation and velocity in
        this client's latest snapshot, and how many of them equal previous."""
        import numpy  # pylint: disable=import-outside-toplevel
        snapshot = self.world.get_snapshot()
        ids = numpy.array(self.parked_ids, dtype=numpy.uint32)
        current = numpy.concatenate(
            [snapshot.get_transforms(ids), snapshot.get_velocities(ids)], axis=1)
        if previous is None:
            return current, 0
        same = numpy.all(current.view(numpy.uint64) == previous.view(numpy.uint64), axis=1)
        return current, int(same.sum())

    def run(self, barrier, results, frame=None):  # pylint: disable=too-many-branches,too-many-locals
        """Runs the warm-up and the measured ticks; puts the result in results.

        frame is a shared value through which the ticker tells the others which
        frame it ticked (--hash-all)."""
        args = self.args
        digest = hashlib.sha256()
        all_digest = hashlib.sha256()
        tick_ms = []
        counters_before = None
        streams_before = None
        frames_before = (0, 0)
        cpu_before = 0.0
        start_wall = time.perf_counter()
        parked_previous, parked_same, parked_seen = None, 0, 0
        stream_before = None
        first_frame = None
        for step in range(args.warmup + args.ticks):
            if step == args.warmup:
                stream_before = read_snapshot_stream(self.world)
                first_frame = self.world.get_snapshot().frame
                if self.ticker:
                    counters_before = read_server_counters(self.client)
                    streams_before = read_stream_stats(self.client)
                cpu_before = cpu_seconds()
                start_wall = time.perf_counter()
                if self.frames is not None:
                    frames_before = (self.frames.delivered, self.frames.skipped)
            if barrier is not None:
                barrier.wait()
            start = time.perf_counter()
            ticked = self.update(step)
            if barrier is not None:
                barrier.wait()
            ticked_frame = self.advance(ticked)
            if self.ticker and step >= args.warmup:
                tick_ms.append((time.perf_counter() - start) * 1e3)
                if args.hash:
                    transforms_digest(self.world, self.ids, step - args.warmup, digest)
                if self.parked_ids:
                    parked_previous, same = self.parked_stability(parked_previous)
                    if step > args.warmup:
                        parked_same += same
                        parked_seen += len(self.parked_ids)
            if args.hash_all:
                if self.ticker:
                    if ticked_frame is None:
                        ticked_frame = self.world.get_snapshot().frame
                    if frame is not None:
                        frame.value = ticked_frame
                if barrier is not None:
                    barrier.wait()
                target = ticked_frame if self.ticker else frame.value
                snapshot = wait_for_frame(self.world, target, args.timeout)
                if step >= args.warmup:
                    snapshot_digest(snapshot, self.all_ids, step - args.warmup, all_digest)
        wall = time.perf_counter() - start_wall
        stream_after = read_snapshot_stream(self.world)
        result = {'client': self.index, 'cpu_s': cpu_seconds() - cpu_before,
                  'snapshot_stream': snapshot_stream_delta(stream_before, stream_after)}
        if args.digests and stream_after is not None and 'digests' in stream_after:
            result['digests'] = {frame_number: value
                                 for frame_number, value in stream_after['digests'].items()
                                 if frame_number > first_frame}
        if args.hash_all:
            result['hash_all'] = all_digest.hexdigest()
        if self.frames is not None:
            delivered = self.frames.delivered - frames_before[0]
            result.update({
                'frames_delivered': delivered,
                'frames_skipped': self.frames.skipped - frames_before[1],
                'frames_delivered_per_s': delivered / wall if wall > 0 else 0.0,
            })
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
                'streams': stream_stats_delta(
                    streams_before, read_stream_stats(self.client), len(tick_ms)),
                'hash': digest.hexdigest() if args.hash else None,
            })
            if self.parked_ids:
                result['parked_bit_stable_fraction'] = \
                    parked_same / parked_seen if parked_seen else None
        results.put(result)


def run_client(args, share, index, barrier, results, frame):
    """Process entry point."""
    BenchmarkClient(args, share, index).run(barrier, results, frame)


def run_clients(args, ids, paths, parked, client):
    """Runs every client; returns their results, client 0 first."""
    plain = to_tuples(paths)
    keyed = list(ids) + list(parked)
    if args.clients == 1:
        # In this process, and through the client that owns the Traffic
        # Manager: a tick steps the TM once that client holds the new frame, so
        # a TM read through another client could see part of the previous one.
        queue = multiprocessing.Queue()
        BenchmarkClient(args, (ids, plain, keyed, parked), 0, client).run(None, queue)
        return [queue.get()]
    # 'spawn', not 'fork': this process already holds a connected client whose
    # network threads would not survive a fork.
    context = multiprocessing.get_context('spawn')
    barrier = context.Barrier(args.clients)
    queue = context.Queue()
    frame = context.Value('Q', 0)
    # In 'tm' mode the ticker is this process's client, the one that owns the
    # Traffic Manager; the other clients only read.
    first = 1 if args.mode == 'tm' else 0
    processes = [
        context.Process(target=run_client,
                        args=(args, (ids[i::args.clients], plain[i::args.clients], keyed,
                                     parked if i == 0 else []),
                              i, barrier, queue, frame))
        for i in range(first, args.clients)]
    for process in processes:
        process.start()
    if first == 1:
        BenchmarkClient(args, (ids, plain, keyed, parked), 0, client).run(barrier, queue, frame)
    results = [queue.get() for _ in range(args.clients)]
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
    parser.add_argument('--hash-all', action='store_true',
                        help='every client hashes the whole snapshot of every measured tick '
                        '(synchronous mode)')
    parser.add_argument('--digests', action='store_true',
                        help='every client keeps a digest of each snapshot it publishes '
                        '(CARLA_SNAPSHOT_DIGEST=1); compare them frame by frame')
    parser.add_argument('--reload', action='store_true',
                        help='reload the world first, so that runs start from the same state')
    parser.add_argument('--deterministic', action='store_true',
                        help='as test_determinism: deterministic collisions, synchronous '
                        'settings applied before reload_world(False), then the seed')
    parser.add_argument('--parked', type=int, default=0,
                        help='also spawn this many vehicles that stand still, physics on')
    parser.add_argument('--cameras', type=int, default=0,
                        help='attach this many 1920x1080 RGB cameras to benchmark vehicles')
    parser.add_argument('--lidar', action='store_true',
                        help='attach a 64-channel LiDAR to the first benchmark vehicle')
    parser.add_argument('--reader-delay-ms', type=float, default=0.0,
                        help="the last client's on_tick callback sleeps this long")
    parser.add_argument('--change-stats', action='store_true',
                        help='turn on carla.Snapshot.Stats on the server during the run')
    parser.add_argument('--lift', type=float, default=0.5,
                        help='spawn height above the road, in metres (Town15 needs about 5)')
    parser.add_argument('--timeout', type=float, default=60.0)
    parser.add_argument('--out', help='also write the JSON result here')
    args = parser.parse_args()
    if args.api in READ_ONLY_APIS and args.mode != 'read':
        parser.error('--api %s only applies to --mode read' % args.api)
    if args.api == 'legacy_tick' and not args.synchronous:
        parser.error('--api legacy_tick needs synchronous mode')
    if args.hash_all and not args.synchronous:
        parser.error('--hash-all needs synchronous mode')
    if args.deterministic and not args.synchronous:
        parser.error('--deterministic needs synchronous mode')
    if args.reader_delay_ms > 0 and args.clients < 2:
        parser.error('--reader-delay-ms needs --clients 2 or more')
    return args


def compare_digests(per_client):
    """Frames every client published, and whether their digests agree."""
    common = set(per_client[0])
    for digests in per_client[1:]:
        common &= set(digests)
    differing = sorted(frame_number for frame_number in common
                       if len({digests[frame_number] for digests in per_client}) > 1)
    return {
        'digest_frames_compared': len(common),
        'digest_frames_differing': len(differing),
        'digest_first_differing_frame': differing[0] if differing else None,
    }


def main():
    """Sets the world up, runs the clients and prints the result."""
    args = parse_args()
    if args.digests:
        # Read when a client connects; the client processes inherit it.
        os.environ['CARLA_SNAPSHOT_DIGEST'] = '1'
    client = carla.Client(args.host, args.port)
    client.set_timeout(args.timeout)
    world = client.get_world()
    original = world.get_settings()
    settings = world.get_settings()
    settings.synchronous_mode = args.synchronous
    settings.fixed_delta_seconds = args.delta
    if args.deterministic:
        # As test_determinism: the settings first, then a reload that keeps
        # them, so that the map starts in synchronous mode.
        settings.deterministic_ragdolls = True
        settings.deterministic_collisions = True
        world.apply_settings(settings)
        client.reload_world(False)
        world = client.get_world()
    elif args.reload:
        client.reload_world()
        world = client.get_world()
    world.apply_settings(settings)
    traffic_manager = client.get_trafficmanager(args.tm_port)
    traffic_manager.set_synchronous_mode(args.synchronous)
    traffic_manager.set_random_device_seed(args.seed)
    traffic_manager.set_hybrid_physics_mode(args.hybrid)
    ids, parked, sensors = [], [], []
    try:
        ids, paths, parked = spawn_actors(client, world, args)
        sensors, received = spawn_sensors(world, args, ids)
        if args.mode == 'tm':
            client.apply_batch_sync(
                [carla.command.SetAutopilot(actor_id, True, traffic_manager.get_port())
                 for actor_id in ids], True)
        if args.change_stats:
            console_command(args.host, args.port, 'carla.Snapshot.Stats 1')
        results = run_clients(args, ids, paths, parked, client)
        summary = dict(results[0])
        summary.update({
            'mode': args.mode, 'api': args.api, 'actors': len(ids), 'clients': args.clients,
            'parked': len(parked), 'cameras': args.cameras, 'lidar': args.lidar,
            'reader_delay_ms': args.reader_delay_ms,
            'synchronous': args.synchronous, 'delta': args.delta, 'seed': args.seed,
            'hybrid': args.hybrid, 'deterministic': args.deterministic,
            'server_version': client.get_server_version(),
            'map': world.get_map().name,
            'cpu_s_all_clients': sum(result['cpu_s'] for result in results),
            'sensors_received': dict(received) if sensors else None,
        })
        if args.hash_all:
            hashes = [result['hash_all'] for result in results]
            summary['hash_all'] = hashes[0]
            summary['hash_all_clients_agree'] = len(set(hashes)) == 1
        streams = [result.get('snapshot_stream') for result in results]
        summary['clients_snapshot_stream'] = streams
        summary['snapshot_resyncs'] = sum(
            stream['resyncs'] for stream in streams if stream is not None)
        summary.pop('digests', None)
        if args.digests:
            summary.update(compare_digests([result.get('digests') or {} for result in results]))
        if not args.synchronous or args.reader_delay_ms > 0:
            summary['clients_frames'] = [
                {key: result.get(key) for key in
                 ('client', 'frames_delivered', 'frames_skipped', 'frames_delivered_per_s')}
                for result in results]
        text = json.dumps(summary, indent=2, sort_keys=True)
        print(text)
        if args.out:
            with open(args.out, 'w', encoding='utf-8') as out:
                out.write(text + '\n')
    finally:
        if args.change_stats:
            console_command(args.host, args.port, 'carla.Snapshot.Stats 0')
        for sensor in sensors:
            sensor.stop()
        if sensors or ids or parked:
            client.apply_batch_sync(
                [carla.command.DestroyActor(actor.id) for actor in sensors] +
                [carla.command.DestroyActor(actor_id) for actor_id in ids + parked], True)
        traffic_manager.set_synchronous_mode(False)
        world.apply_settings(original)


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        pass
