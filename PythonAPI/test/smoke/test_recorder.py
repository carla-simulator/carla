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
from queue import Queue

import carla
import numpy as np

from . import SyncSmokeTest
from .test_lidar import wait_for_frame

SEMANTIC_POINT = np.dtype([('x', 'f4'), ('y', 'f4'), ('z', 'f4'), ('cos', 'f4'),
                           ('object_idx', 'u4'), ('object_tag', 'u4')])


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

    def _record_vehicle_alive_at_stop(self, basename):
        """Record a vehicle that is still alive when the recorder stops (so the
        log has no destroy event for it), then destroy the original."""
        log_path = self._log_path(basename)
        self.client.start_recorder(log_path)
        vehicle = self._spawn_test_vehicle()
        for _ in range(20):
            self.world.tick()
        self.client.stop_recorder()
        vehicle.destroy()
        for _ in range(5):
            self.world.tick()
        return log_path

    def _vehicle_count(self):
        self.world.tick()
        return len(self.world.get_actors().filter('vehicle.*'))

    def test_stop_replayer_removes_replayed_actors(self):
        log_path = self._record_vehicle_alive_at_stop('stop_replayer_mid.log')
        before = self._vehicle_count()

        self.client.replay_file(log_path, 0.0, 0.0, 0)
        for _ in range(5):
            self.world.tick()
        self.assertGreater(self._vehicle_count(), before)

        self.client.stop_replayer(False)
        self.assertEqual(
            self._vehicle_count(), before,
            'stop_replayer(keep_actors=False) left the replayed vehicle behind')

    def test_stop_replayer_after_replay_ended_removes_actors(self):
        log_path = self._record_vehicle_alive_at_stop('stop_replayer_end.log')
        before = self._vehicle_count()

        self.client.replay_file(log_path, 0.0, 0.0, 0)
        # Past the end of the log: the replayer stops itself and keeps actors.
        for _ in range(40):
            self.world.tick()
        self.assertGreater(self._vehicle_count(), before)

        self.client.stop_replayer(False)
        self.assertEqual(
            self._vehicle_count(), before,
            'stop_replayer(keep_actors=False) after the replay ended left the '
            'replayed vehicle behind')


    def _semantic_lidar_points_on(self, actor):
        """Points of a semantic LiDAR 3 m above @a actor that hit it."""
        bp = self.world.get_blueprint_library().find('sensor.lidar.ray_cast_semantic')
        for key, value in {'channels': '32', 'range': '10', 'points_per_second': '200000',
                           'rotation_frequency': '20', 'upper_fov': '-30',
                           'lower_fov': '-90'}.items():
            bp.set_attribute(key, value)
        lidar = self.world.spawn_actor(
            bp, carla.Transform(actor.get_location() + carla.Location(z=3.0)))
        data_queue = Queue()
        lidar.listen(lambda data: data_queue.put((data.frame, bytes(data.raw_data))))
        try:
            frame = self.world.tick()
            points = np.frombuffer(wait_for_frame(self, data_queue, frame)[1], dtype=SEMANTIC_POINT)
        finally:
            lidar.stop()
            lidar.destroy()
        return int(np.count_nonzero(points['object_idx'] == actor.id))

    def test_replayed_actors_visible_to_sensors(self):
        """Replayed vehicles and walkers keep query collision, so sensors see them."""
        log_path = self._log_path('replay_visible_to_sensors.log')
        blueprint_library = self.world.get_blueprint_library()

        self.client.start_recorder(log_path)
        vehicle = self._spawn_test_vehicle()
        walker_bp = blueprint_library.filter('walker.pedestrian.*')[0]
        location = self.world.get_random_location_from_navigation()
        self.assertIsNotNone(location, 'Town10HD_Opt has no navigation mesh')
        walker = self.world.spawn_actor(walker_bp, carla.Transform(location + carla.Location(z=1.0)))
        controller = self.world.spawn_actor(
            blueprint_library.find('controller.ai.walker'), carla.Transform(), attach_to=walker)
        self.world.tick()
        controller.start()
        controller.go_to_location(self.world.get_random_location_from_navigation())
        for _ in range(40):
            self.world.tick()
        self.client.stop_recorder()
        controller.stop()
        for actor in (controller, walker, vehicle):
            actor.destroy()
        for _ in range(5):
            self.world.tick()

        self.client.replay_file(log_path, 0.0, 0.0, 0)
        for _ in range(20):
            self.world.tick()
        replayed = {
            'vehicle': self.world.get_actors().filter('vehicle.*'),
            'walker': self.world.get_actors().filter('walker.pedestrian.*'),
        }
        for kind, actors in replayed.items():
            self.assertEqual(len(actors), 1, 'expected one replayed %s' % kind)
            actor = actors[0]
            location = actor.get_location()
            hits = self.world.cast_ray(location + carla.Location(z=3.0), location - carla.Location(z=1.5))
            label = carla.CityObjectLabel.Car if kind == 'vehicle' else carla.CityObjectLabel.Pedestrians
            self.assertTrue(any(hit.label == label for hit in hits),
                            'world.cast_ray does not hit the replayed %s' % kind)
            self.assertGreater(self._semantic_lidar_points_on(actor), 0,
                               'The semantic LiDAR does not hit the replayed %s' % kind)

        self.client.stop_replayer(False)


if __name__ == '__main__':
    unittest.main()
