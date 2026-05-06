# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Smoke test for the replay aliasing fix.

The bug being prevented: when the recorder replays a session and finds an
actor in the registry whose id matches a freshly-replayed actor, the
replayer used to silently reuse that actor and inherit its leftover state
(velocity, lights, sensor handles). The fix removes the same-id reuse
fast-path, so every replayed actor is spawned fresh.
"""

import math
import os
import time

from . import SmokeTest

import carla


REPLAY_LOG_PATH = "/tmp/carla_replay_aliasing_smoke.log"


class TestReplayNoActorAliasing(SmokeTest):

    def tearDown(self):
        # The packaged ue5 build only ships Town10HD_Opt + Mine_01; the
        # base SmokeTest.tearDown calls load_world("Town03") which fails.
        if os.path.exists(REPLAY_LOG_PATH):
            try:
                os.remove(REPLAY_LOG_PATH)
            except OSError:
                pass
        try:
            self.client.stop_replayer(False)
        except Exception:
            pass
        self.client.load_world("Town10HD_Opt")
        time.sleep(5)
        self.world = None
        self.client = None

    def _spawn_vehicles(self, count):
        bp_lib = self.world.get_blueprint_library()
        vehicle_bps = bp_lib.filter("vehicle.*")
        spawn_points = self.world.get_map().get_spawn_points()
        spawned = []
        for i in range(min(count, len(spawn_points))):
            actor = self.world.try_spawn_actor(vehicle_bps[i % len(vehicle_bps)], spawn_points[i])
            if actor is not None:
                spawned.append(actor)
        return spawned

    def test_replayed_actors_have_fresh_ids_and_clean_state(self):
        # Settle the world.
        self.client.load_world("Town10HD_Opt")
        time.sleep(5)
        self.world = self.client.get_world()

        # Record a short session with a few vehicles in autopilot.
        self.client.start_recorder(REPLAY_LOG_PATH, True)
        recorded = self._spawn_vehicles(3)
        self.assertGreater(len(recorded), 0, "no vehicles spawned for recording")
        for actor in recorded:
            actor.set_autopilot(True)
        time.sleep(2.0)

        # Snapshot ids of the recorded actors before destroying them.
        pre_replay_ids = {actor.id for actor in recorded}

        # Stop and clean up the recorded actors.
        for actor in recorded:
            actor.set_autopilot(False)
            actor.destroy()
        self.client.stop_recorder()
        time.sleep(1.0)

        # Replay. The fix removes the same-id reuse path, so the replayer
        # always spawns fresh actors with fresh ids.
        self.client.replay_file(REPLAY_LOG_PATH, 0.0, 0.0, 0)
        time.sleep(2.0)

        # Inspect the replayed actors.
        replayed = [
            actor for actor in self.world.get_actors().filter("vehicle.*")
            if actor.id not in pre_replay_ids
        ]
        self.assertGreater(
            len(replayed), 0,
            "no replayed vehicles found; replayer may have silently reused "
            "the original ids (the bug this fix prevents)")

        # Aliased state would surface as nan velocity components on the
        # very first frames after replay (leaked from a destroyed actor).
        for actor in replayed:
            velocity = actor.get_velocity()
            for component_name, component in (("x", velocity.x),
                                              ("y", velocity.y),
                                              ("z", velocity.z)):
                self.assertFalse(
                    math.isnan(component),
                    "replayed actor {} has nan velocity.{}".format(actor.id, component_name))
