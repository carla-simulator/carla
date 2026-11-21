# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Smoke tests for the streaming counters: carla.Client.get_stream_stats and
the stream fields of carla.ServerPerfCounters."""

from . import SyncSmokeTest
from .test_rt_lens_capture_sync import console_command

TICKS = 10


class TestStreamStats(SyncSmokeTest):
    """The server counts what the world snapshot's sessions send."""

    def test_snapshot_messages_are_counted(self):
        stats_before = self.client.get_stream_stats()
        counters_before = self.client.get_server_perf_counters()
        for _ in range(TICKS):
            self.world.tick()
        stats_after = self.client.get_stream_stats()
        counters_after = self.client.get_server_perf_counters()

        before, after = stats_before['episode'], stats_after['episode']
        # Every tick reaches this client's session: synchronous mode never
        # skips a message.
        self.assertGreaterEqual(after['writes'] - before['writes'], TICKS)
        self.assertGreater(after['wire_bytes'] - before['wire_bytes'], 0)
        self.assertEqual(after['busy_drops'], before['busy_drops'])
        self.assertEqual(len(after['queue_histogram']), 24)
        self.assertGreaterEqual(
            sum(after['send_histogram']) - sum(before['send_histogram']), TICKS)

        sessions = [row for row in stats_after['sessions'] if row['episode']]
        self.assertGreaterEqual(len(sessions), 1)
        self.assertTrue(any(row['loopback'] for row in sessions))
        self.assertTrue(all(row['remote'] for row in sessions))

        # ServerPerfCounters carries the same totals.
        self.assertGreaterEqual(
            counters_after.snapshot_writes - counters_before.snapshot_writes, TICKS)
        self.assertGreater(
            counters_after.snapshot_wire_bytes - counters_before.snapshot_wire_bytes, 0)
        self.assertIn('snapshot_wire_bytes=', str(counters_after))

    def test_snapshot_changes_are_counted_only_when_asked(self):
        before = self.client.get_stream_stats()['snapshot_changes']
        for _ in range(3):
            self.world.tick()
        idle = self.client.get_stream_stats()['snapshot_changes']
        self.assertEqual(idle['frames'], before['frames'])
        self.assertEqual(idle['keyframes'], before['keyframes'])

        console_command("carla.Snapshot.Stats 1", self.testing_address)
        try:
            for _ in range(TICKS):
                self.world.tick()
        finally:
            console_command("carla.Snapshot.Stats 0", self.testing_address)
        after = self.client.get_stream_stats()['snapshot_changes']

        # The server compares every snapshot with the previous one anyway, so
        # every tick is compared (a keyframe only after a new episode).
        frames = after['frames'] - idle['frames']
        self.assertGreaterEqual(frames, TICKS - 1)
        self.assertEqual(after['keyframes'], idle['keyframes'])

        # A world nobody moves: the delta is far smaller than the snapshot.
        keyframe_bytes = after['keyframe_bytes'] - idle['keyframe_bytes']
        delta_bytes = after['delta_bytes'] - idle['delta_bytes']
        self.assertGreater(keyframe_bytes, 0)
        self.assertLess(delta_bytes, keyframe_bytes / 2)

        # Every compared record is counted once, under its actor type; traffic
        # signs never change.
        rows = {name: {key: (value - idle['by_type'][name][key]
                             if not isinstance(value, list) else
                             [b - a for a, b in zip(idle['by_type'][name][key], value)])
                       for key, value in row.items()}
                for name, row in after['by_type'].items()}
        records = sum(row['records'] for row in rows.values())
        self.assertGreater(records, 0)
        for name, row in rows.items():
            self.assertLessEqual(row['unchanged'], row['records'], name)
            for changed in row['group_changed']:
                self.assertLessEqual(changed, row['records'], name)
        signs = rows['traffic_sign']
        if signs['records'] > 0:
            self.assertEqual(signs['unchanged'], signs['records'])
