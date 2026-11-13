# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import statistics
import time

import carla

from . import SmokeTest


WORLD = "Town10HD_Opt"
WALKERS = 8
SPEED = 1.4
# Median speed a crowd of walkers set to SPEED reaches: they stop at
# crossings, avoid each other and turn, about 1.1-1.3 m/s in sync mode too.
MIN_SPEED = 0.8
# Simulation time each client step covers, and the longest sleep for it.
CLIENT_STEP = 0.25
MAX_SLEEP = 0.25


class TestWalkerAsyncSpeed(SmokeTest):
    """AI walkers keep their set speed in asynchronous mode (#10255).

    On maps where the client runs the navigation, the crowd only advances when
    the client processes a snapshot. It used to advance by the delta of that
    one frame, so the time of every frame the client skipped was lost and the
    walkers slowed down in proportion. A client that sleeps between ticks
    must still see them walk at their set speed.
    """

    def setUp(self):
        super(TestWalkerAsyncSpeed, self).setUp()
        if self.world.get_map().name.split("/")[-1] != WORLD:
            self.load_world(WORLD)
            self.world = self.client.get_world()
        self.settings = self.world.get_settings()
        self.world.apply_settings(carla.WorldSettings(
            no_rendering_mode=False, synchronous_mode=False, fixed_delta_seconds=0.01))
        self.world.wait_for_tick()

    def tearDown(self):
        # SmokeTest.tearDown() loads Town03, which is not shipped in the
        # packaged build; reload Town10HD_Opt instead.
        self.world.apply_settings(self.settings)
        if self.settings.synchronous_mode:
            self.world.tick()
        self.settings = None
        self.load_world(WORLD)
        self.world = None
        self.client = None

    def _spawn_walkers(self):
        blueprint_library = self.world.get_blueprint_library()
        walker_bp = blueprint_library.filter("walker.pedestrian.*")[0]
        if walker_bp.has_attribute("is_invincible"):
            walker_bp.set_attribute("is_invincible", "false")
        controller_bp = blueprint_library.find("controller.ai.walker")
        pairs = []
        for _ in range(WALKERS * 3):
            if len(pairs) == WALKERS:
                break
            location = self.world.get_random_location_from_navigation()
            if location is None:
                continue
            walker = self.world.try_spawn_actor(
                walker_bp, carla.Transform(location + carla.Location(z=1.0)))
            if walker is None:
                continue
            controller = self.world.spawn_actor(controller_bp, carla.Transform(), attach_to=walker)
            pairs.append((walker, controller))
        self.assertEqual(len(pairs), WALKERS, "could not spawn the walkers")
        self.world.wait_for_tick()
        for _, controller in pairs:
            controller.start()
            controller.go_to_location(self.world.get_random_location_from_navigation())
            controller.set_max_speed(SPEED)
        return pairs

    def test_walkers_keep_their_speed_when_the_client_skips_frames(self):
        print("TestWalkerAsyncSpeed.test_walkers_keep_their_speed_when_the_client_skips_frames")
        pairs = self._spawn_walkers()
        try:
            # Let them start walking before measuring.
            for _ in range(20):
                self.world.wait_for_tick()

            # A slow client: it sleeps between ticks, so the server keeps
            # producing frames it never sees. The sleep is adjusted after every
            # step so that each covers about CLIENT_STEP of simulation, well
            # within the crowd's 1 s catch-up limit, however fast the server
            # runs (a packaged one in asynchronous mode runs many times faster
            # than real time).
            sleep = 0.0
            first = self.world.wait_for_tick()
            start = {walker.id: walker.get_location() for walker, _ in pairs}
            snapshot = first
            steps = []
            deadline = time.time() + 10.0
            while time.time() < deadline:
                wall_start = time.time()
                time.sleep(sleep)
                previous = snapshot.timestamp.elapsed_seconds
                snapshot = self.world.wait_for_tick()
                wall = time.time() - wall_start
                step = snapshot.timestamp.elapsed_seconds - previous
                steps.append(step)
                # Sleep that would make this step CLIENT_STEP long at this server speed, half applied.
                wanted = CLIENT_STEP * wall / max(step, 1e-3) - (wall - sleep)
                sleep = min(MAX_SLEEP, max(0.0, 0.5 * (sleep + wanted)))

            seen = len(steps) + 1
            frames = snapshot.frame - first.frame + 1
            self.assertLess(
                seen, 0.2 * frames,
                "the client saw %d of %d frames; it must skip most of them for this test "
                "to mean anything" % (seen, frames))
            self.assertLess(
                statistics.median(steps), 0.8,
                "client steps covered a median %.2f s of simulation; past the crowd's 1 s "
                "catch-up limit walkers lose time by design" % statistics.median(steps))

            elapsed = snapshot.timestamp.elapsed_seconds - first.timestamp.elapsed_seconds
            speeds = [walker.get_location().distance(start[walker.id]) / elapsed
                      for walker, _ in pairs]
            median = statistics.median(speeds)
            print("  client saw %d of %d frames, median step %.2f s, median walker speed %.2f m/s"
                  % (seen, frames, statistics.median(steps), median))
            self.assertGreater(
                median, MIN_SPEED,
                "walkers move at a median %.2f m/s with max speed %.1f m/s; the crowd is "
                "losing the time of the frames the client skips" % (median, SPEED))
            self.assertLess(
                median, 1.8,
                "walkers move at a median %.2f m/s with max speed %.1f m/s; the crowd "
                "advances by more time than elapsed" % (median, SPEED))
        finally:
            for walker, controller in pairs:
                controller.stop()
            self.client.apply_batch(
                [carla.command.DestroyActor(actor) for pair in pairs for actor in pair])
            self.world.wait_for_tick()
