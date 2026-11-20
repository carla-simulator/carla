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

import socket
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
        self.load_world("Town10HD_Opt")
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

    @staticmethod
    def _free_port():
        # A TM port of its own: a TM is per process and per port, so a port an
        # earlier test used would hand back that test's TM and its client.
        with socket.socket() as sock:
            sock.bind(("127.0.0.1", 0))
            return sock.getsockname()[1]

    def _release(self, actors, tm):
        for actor in actors:
            actor.set_autopilot(False, tm.get_port())
            actor.destroy()
        tm.set_synchronous_mode(False)

    def test_traffic_manager_of_another_client_steps_on_the_ticked_frame(self):
        # The TM steps inside the tick of whichever client of the process ticks,
        # but reads the world through the client that created it. That client
        # must already hold the frame the tick returns, or the TM plans from
        # part of the previous frame.
        owner = carla.Client(*self.testing_address)
        owner.set_timeout(120.0)
        owner_world = owner.get_world()
        tm = owner.get_trafficmanager(self._free_port())
        tm.set_synchronous_mode(True)
        tm.set_hybrid_physics_mode(True)
        spawned = self._spawn_with_autopilot(20, tm.get_port())
        self.assertGreater(len(spawned), 0)
        # A slow callback holds the owner's stream, so each frame reaches the
        # owner well after the client that ticks.
        callback = owner_world.on_tick(lambda _: time.sleep(0.02))
        try:
            for _ in range(200):
                frame = self.world.tick()
                self.assertGreaterEqual(
                    owner_world.get_snapshot().frame, frame,
                    "the TM stepped before its own client received the ticked frame")
        finally:
            owner_world.remove_on_tick(callback)
            self._release(spawned, tm)

    def test_hybrid_mode_drives_vehicles_near_and_far_from_the_hero(self):
        # Hybrid mode turns physics on near the hero and off elsewhere, and
        # sends those toggles with the step's controls and teleports.
        tm = self.client.get_trafficmanager(self._free_port())
        tm.set_synchronous_mode(True)
        tm.set_hybrid_physics_mode(True)
        tm.set_hybrid_physics_radius(40.0)
        tm_port = tm.get_port()

        bp_lib = self.world.get_blueprint_library()
        hero_bp = bp_lib.find("vehicle.lincoln.mkz")
        hero_bp.set_attribute("role_name", "hero")
        spawn_points = self.world.get_map().get_spawn_points()
        hero = self.world.spawn_actor(hero_bp, spawn_points[0])
        origin = spawn_points[0].location
        by_distance = sorted(spawn_points[1:], key=lambda sp: sp.location.distance(origin))
        car_bp = bp_lib.find("vehicle.lincoln.mkz")
        car_bp.set_attribute("role_name", "autopilot")
        near, far = [], []
        for sp in by_distance:
            if len(near) < 4 and sp.location.distance(origin) < 30.0:
                group = near
            elif len(far) < 4 and sp.location.distance(origin) > 120.0:
                group = far
            else:
                continue
            actor = self.world.try_spawn_actor(car_bp, sp)
            if actor is not None:
                group.append(actor)
        spawned = [hero] + near + far
        self.assertTrue(near and far, "Town10HD_Opt has spawn points near and far from the first one")
        try:
            hero.set_autopilot(True, tm_port)
            for actor in near + far:
                actor.set_autopilot(True, tm_port)
                tm.ignore_lights_percentage(actor, 100.0)
            start = {a.id: a.get_location() for a in spawned}
            for _ in range(300):
                self.world.tick()
            moved = {a.id: a.get_location().distance(start[a.id]) for a in spawned}
            self.assertTrue(any(moved[a.id] > 2.0 for a in near), "no vehicle near the hero moved: {}".format(moved))
            self.assertTrue(any(moved[a.id] > 2.0 for a in far), "no vehicle far from the hero moved: {}".format(moved))
        finally:
            self._release(spawned, tm)
