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

The test runs in synchronous mode so that recorder spawn / destroy events
are deterministic and not lost to async timing.
"""

import math
import os
import tempfile
import time

from . import SyncSmokeTest

import carla


def make_temp_log_path():
    """Return a unique, OS-agnostic path for the recorder log.

    Uses the platform temporary directory and a unique filename so the test
    never relies on a hardcoded location and never collides with another run.
    The caller owns the file and is responsible for removing it.
    """
    handle, path = tempfile.mkstemp(
        prefix="carla_replay_aliasing_smoke_", suffix=".log")
    os.close(handle)
    return path


class TestReplayNoActorAliasing(SyncSmokeTest):

    def setUp(self):
        super().setUp()
        self.replay_log_path = make_temp_log_path()

    def tearDown(self):
        log_path = getattr(self, "replay_log_path", None)
        if log_path and os.path.exists(log_path):
            try:
                os.remove(log_path)
            except OSError:
                pass
        try:
            self.client.stop_replayer(False)
        except Exception:
            pass
        # Restore world settings (synchronous_mode -> off) before reloading
        # the map; otherwise the next test inherits sync mode.
        self.world.apply_settings(self.settings)
        self.world.tick()
        self.settings = None
        # The packaged ue5 build only ships Town10HD_Opt + Mine_01; the
        # base SmokeTest.tearDown calls load_world("Town03") which fails.
        self.client.load_world("Town10HD_Opt")
        time.sleep(5)
        self.world = None
        self.client = None

    def _spawn_vehicles(self, count):
        bp_lib = self.world.get_blueprint_library()
        vehicle_bps = [
            bp for bp in bp_lib.filter("vehicle.*")
            if not bp.has_attribute("base_type")
            or bp.get_attribute("base_type").as_str() in ("car", "truck", "bus", "van")
        ]
        spawn_points = self.world.get_map().get_spawn_points()
        spawned = []
        for i in range(min(count, len(spawn_points))):
            actor = self.world.try_spawn_actor(vehicle_bps[i % len(vehicle_bps)], spawn_points[i])
            if actor is not None:
                spawned.append(actor)
                # Tick once per spawn so the recorder registers each one
                # in its own frame; rapid back-to-back spawns inside a
                # single frame can be missed under load.
                self.world.tick()
        return spawned

    def test_replayed_actors_have_clean_state(self):
        # Already in synchronous mode via SyncSmokeTest.setUp.
        # Start the recorder then spawn vehicles so the spawn events are
        # captured. Crucially, stop the recorder BEFORE destroying the
        # original actors -- otherwise the destroy events end up in the
        # log and replay re-executes them, leaving zero vehicles by the
        # time we inspect the world.
        self.client.start_recorder(self.replay_log_path, True)
        self.world.tick()

        recorded = self._spawn_vehicles(3)
        self.assertGreater(len(recorded), 0, "no vehicles spawned for recording")
        recorded_count = len(recorded)
        tm = self.client.get_trafficmanager()
        tm.set_synchronous_mode(True)
        tm_port = tm.get_port()
        for actor in recorded:
            actor.set_autopilot(True, tm_port)

        # Drive long enough for the recorder to capture stable transforms.
        for _ in range(50):
            self.world.tick()

        # Stop the recorder first so that the subsequent destroys do NOT
        # land in the log.
        self.client.stop_recorder()
        self.world.tick()

        # Now clean up the original recorded actors.
        for actor in recorded:
            actor.set_autopilot(False, tm_port)
            actor.destroy()
        for _ in range(5):
            self.world.tick()

        # Replay. With the fix, the replayer always proceeds to
        # SpawnActorWithInfo() instead of the legacy "reuse a live actor
        # with the same id" fast-path. We assert that replay produces at
        # least one vehicle and that none of them carry leaked state
        # (nan velocity / nan transform).
        self.client.replay_file(self.replay_log_path, 0.0, 0.0, 0)
        # Drive enough ticks for the replay to spawn the recorded actors,
        # but the replay log has no destroy events (we stopped the
        # recorder before destroying), so the replayed actors stay alive.
        for _ in range(40):
            self.world.tick()

        post_replay_vehicles = list(self.world.get_actors().filter("vehicle.*"))
        self.assertGreater(
            len(post_replay_vehicles), 0,
            "replay produced no vehicles; the replay path may have silently "
            "failed (recorded={}).".format(recorded_count))

        # Aliased state from a previous actor would surface as nan velocity
        # components or nan transform on the first frames after replay.
        for actor in post_replay_vehicles:
            velocity = actor.get_velocity()
            transform = actor.get_transform()
            for label, value in (("velocity.x", velocity.x),
                                 ("velocity.y", velocity.y),
                                 ("velocity.z", velocity.z),
                                 ("location.x", transform.location.x),
                                 ("location.y", transform.location.y),
                                 ("location.z", transform.location.z)):
                self.assertFalse(
                    math.isnan(value),
                    "replayed actor {} ({}) has nan {}".format(
                        actor.id, actor.type_id, label))

        tm.set_synchronous_mode(False)
