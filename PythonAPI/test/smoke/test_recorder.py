# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Smoke tests for the recorder/replayer compatibility fixes from PR #0j of
issue #9628 (port of ue4-dev `7b0707548` and the dispatcher portion of
`c9f2e3ab6`).

The two test cases below cover the two regressions the PR fixes:

  1. `test_record_replay_blueprint_id_round_trips` exercises
     `UActorDispatcher::SpawnActor` going through `Classes.Find(Description.Id)`.
     A vehicle recorded under one blueprint id must replay back as the same
     blueprint id, even when re-spawned by the replayer.

  2. `test_record_survives_same_tick_actor_destroy` exercises the
     `FindCarlaActor` null-check guards in `ACarlaRecorder::AddPhysicsControl`,
     `AddTrafficLightTime`, and `AddTriggerVolume` (and the matching
     `FFrameData` mirrors). Recording with `additional_data=True` while a
     vehicle is destroyed inside the same tick used to dereference a null
     `FCarlaActor*` and crash the server. The fix lets the recorder drop the
     event cleanly and keep going.
"""

import os
import tempfile
import time
import unittest

from . import SyncSmokeTest


class TestRecorder(SyncSmokeTest):
    def setUp(self):
        super(TestRecorder, self).setUp()
        self.client.load_world('Town10HD_Opt')
        time.sleep(2)
        self.world = self.client.get_world()
        self._original_settings = self.world.get_settings()
        settings = self.world.get_settings()
        settings.synchronous_mode = True
        settings.fixed_delta_seconds = 0.05
        self.world.apply_settings(settings)
        self.world.tick()
        self._log_paths = []

    def tearDown(self):
        for log_path in self._log_paths:
            try:
                os.remove(log_path)
            except OSError:
                pass
        try:
            self.client.stop_recorder()
        except RuntimeError:
            pass
        try:
            self.client.stop_replayer(True)
        except RuntimeError:
            pass
        if self._original_settings is not None:
            self.world.apply_settings(self._original_settings)
            self.world.tick()
        self._original_settings = None
        # SmokeTest.tearDown reloads Town03, which is not shipped in the
        # packaged ue5-dev build, so reset to Town10HD_Opt directly here
        # instead of chaining to super().
        self.client.load_world('Town10HD_Opt')
        time.sleep(2)
        self.world = None
        self.client = None

    def _log_path(self, basename):
        path = os.path.join(tempfile.gettempdir(), basename)
        self._log_paths.append(path)
        return path

    def _spawn_test_vehicle(self):
        blueprint_library = self.world.get_blueprint_library()
        blueprint = blueprint_library.find('vehicle.lincoln.mkz')
        spawn_points = self.world.get_map().get_spawn_points()
        self.assertGreater(
            len(spawn_points),
            0,
            'Town10HD_Opt has no spawn points; cannot run the recorder smoke test')
        return self.world.spawn_actor(blueprint, spawn_points[0])

    def test_record_replay_blueprint_id_round_trips(self):
        log_path = self._log_path('pr0j_round_trip.log')

        self.client.start_recorder(log_path)
        vehicle = self._spawn_test_vehicle()
        try:
            for _ in range(30):
                self.world.tick()
        finally:
            try:
                vehicle.destroy()
            except RuntimeError:
                pass
            self.client.stop_recorder()

        for _ in range(5):
            self.world.tick()

        self.client.replay_file(log_path, 0.0, 0.0, 0)
        replayed_blueprint_ids = set()
        for _ in range(30):
            self.world.tick()
            replayed_blueprint_ids.update(
                actor.type_id for actor in self.world.get_actors().filter('vehicle.*'))

        self.assertIn(
            'vehicle.lincoln.mkz',
            replayed_blueprint_ids,
            'Replayer did not re-spawn the vehicle under its recorded blueprint id; '
            'this is the failure mode the dispatcher rekey-by-Id (port of c9f2e3ab6) fixes.')

        self.client.stop_replayer(True)

    def test_record_survives_same_tick_actor_destroy(self):
        log_path = self._log_path('pr0j_destroy.log')

        self.client.start_recorder(log_path, True)
        vehicle = self._spawn_test_vehicle()
        for _ in range(3):
            self.world.tick()

        vehicle.destroy()
        for _ in range(3):
            self.world.tick()

        self.client.stop_recorder()

        snapshot = self.world.get_snapshot()
        self.assertIsNotNone(
            snapshot,
            'Server did not respond to get_snapshot() after recording across an actor destroy; '
            'pre-PR this faulted the server inside ACarlaRecorder::AddPhysicsControl '
            '/ AddTrafficLightTime / AddTriggerVolume on a null FindCarlaActor lookup.')


if __name__ == '__main__':
    unittest.main()
