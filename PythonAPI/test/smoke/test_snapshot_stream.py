# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Smoke tests for the client side of the world-snapshot stream:
carla.World.get_snapshot_stream_stats and the checked snapshot header."""

import os
import time

import carla

from . import SyncSmokeTest

TICKS = 20


class TestSnapshotStream(SyncSmokeTest):
    """Every snapshot the client publishes passed its header and hash check."""

    def test_every_tick_is_received_and_checked(self):
        before = self.world.get_snapshot_stream_stats()
        frames = [self.world.tick() for _ in range(TICKS)]
        self.assertEqual(self.world.get_snapshot().frame, frames[-1])
        after = self.world.get_snapshot_stream_stats()

        messages = after['messages'] - before['messages']
        self.assertGreaterEqual(messages, TICKS)
        self.assertEqual(
            (after['keyframes'] - before['keyframes']) + (after['deltas'] - before['deltas']),
            messages)
        self.assertEqual(after['resyncs'], before['resyncs'])
        self.assertEqual(after['incompatible'], 0)
        self.assertGreater(after['wire_bytes'] - before['wire_bytes'], 0)
        self.assertGreater(after['published_bytes'] - before['published_bytes'], 0)
        self.assertGreater(after['decode_ns'], before['decode_ns'])
        # The callback time includes decoding.
        self.assertGreaterEqual(
            after['callback_ns'] - before['callback_ns'], after['decode_ns'] - before['decode_ns'])
        self.assertNotIn('digests', after)

    def test_two_clients_publish_the_same_snapshots(self):
        previous = os.environ.get('CARLA_SNAPSHOT_DIGEST')
        os.environ['CARLA_SNAPSHOT_DIGEST'] = '1'
        try:
            first = carla.Client(*self.testing_address)
            first.set_timeout(120.0)
            first_world = first.get_world()
            second = carla.Client(*self.testing_address)
            second.set_timeout(120.0)
            second_world = second.get_world()
        finally:
            if previous is None:
                del os.environ['CARLA_SNAPSHOT_DIGEST']
            else:
                os.environ['CARLA_SNAPSHOT_DIGEST'] = previous

        blueprint = self.world.get_blueprint_library().filter('vehicle.*')[0]
        spawn_point = self.world.get_map().get_spawn_points()[0]
        vehicle = self.world.spawn_actor(blueprint, spawn_point)
        try:
            frames = []
            for _ in range(TICKS):
                frames.append(self.world.tick())
            # The other connections receive the last frame shortly after the
            # tick returns.
            for world in (first_world, second_world):
                deadline = time.monotonic() + 10.0
                while world.get_snapshot().frame < frames[-1] and time.monotonic() < deadline:
                    time.sleep(0.01)
                self.assertGreaterEqual(world.get_snapshot().frame, frames[-1])
            first_digests = first_world.get_snapshot_stream_stats()['digests']
            second_digests = second_world.get_snapshot_stream_stats()['digests']
            for frame in frames:
                self.assertIn(frame, first_digests)
                self.assertIn(frame, second_digests)
                self.assertEqual(first_digests[frame], second_digests[frame], 'frame %d' % frame)
            # The vehicle is in every snapshot after its spawn.
            self.assertIsNotNone(first_world.get_snapshot().find(vehicle.id))
        finally:
            vehicle.destroy()
            self.world.tick()
