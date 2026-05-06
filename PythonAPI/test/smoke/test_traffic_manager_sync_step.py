# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Smoke test for the Traffic Manager mutex removal.

After the refactor, synchronous mode no longer uses the worker-thread +
condition-variable handshake -- TrafficManagerLocal::SynchronousTick()
calls Step() inline. This test exercises that path end-to-end and
asserts no tick takes longer than a generous bound (catches deadlock).
"""

import time

from . import SyncSmokeTest

import carla


class TestTrafficManagerSyncStep(SyncSmokeTest):

    def tearDown(self):
        try:
            tm = self.client.get_trafficmanager()
            tm.set_synchronous_mode(False)
        except Exception:
            pass
        # Ue5 packaged build only ships Town10HD_Opt; do not try to load
        # the base class's Town03.
        self.world.apply_settings(self.settings)
        self.world.tick()
        self.settings = None
        self.client.load_world("Town10HD_Opt")
        time.sleep(5)
        self.world = None
        self.client = None

    def _spawn_with_autopilot(self, count, tm_port):
        bp_lib = self.world.get_blueprint_library()
        vehicle_bps = bp_lib.filter("vehicle.*")
        spawn_points = self.world.get_map().get_spawn_points()
        spawned = []
        for i in range(min(count, len(spawn_points))):
            actor = self.world.try_spawn_actor(vehicle_bps[i % len(vehicle_bps)], spawn_points[i])
            if actor is not None:
                actor.set_autopilot(True, tm_port)
                spawned.append(actor)
        return spawned

    def test_synchronous_step_no_deadlock(self):
        # Already in synchronous mode via the SyncSmokeTest base.
        tm = self.client.get_trafficmanager()
        tm.set_synchronous_mode(True)
        tm_port = tm.get_port()

        spawned = self._spawn_with_autopilot(20, tm_port)
        self.assertGreater(len(spawned), 0)

        # 200 ticks at 50ms each is 10 seconds of sim time; any single
        # tick exceeding 5s is treated as a deadlock signal.
        max_tick_seconds = 5.0
        for _ in range(200):
            tick_start = time.monotonic()
            self.world.tick()
            tick_elapsed = time.monotonic() - tick_start
            self.assertLess(
                tick_elapsed, max_tick_seconds,
                "world.tick() took {:.2f}s; TM synchronous Step() may have "
                "regressed (the mutex removal should keep this bounded).".format(tick_elapsed))

        for actor in spawned:
            actor.set_autopilot(False, tm_port)
            actor.destroy()
        tm.set_synchronous_mode(False)
