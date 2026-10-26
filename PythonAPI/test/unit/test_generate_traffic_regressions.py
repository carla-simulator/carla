# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Actual traffic CLI with inert RPC/clock boundaries and real native geometry."""

import contextlib
import copy
import importlib.util
import io
import os
from pathlib import Path
import sys
from types import SimpleNamespace
import unittest
from unittest import mock

import carla

SOURCE_ROOT = Path(os.environ.get(
    "CARLA_REGRESSION_SOURCE_ROOT", str(Path(__file__).resolve().parents[3])))
SPEC = importlib.util.spec_from_file_location(
    "regression_generate_traffic", SOURCE_ROOT / "PythonAPI" / "examples" / "generate_traffic.py")
TRAFFIC = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(TRAFFIC)


class _Finished(Exception):
    pass


class _Blueprint:
    def __init__(self, name, speed=1.4):
        self.id = name
        self.speed = speed

    def has_attribute(self, name):
        return name in ("speed", "is_invincible")

    def get_attribute(self, _):
        return SimpleNamespace(recommended_values=["0", str(self.speed), "3"])

    def set_attribute(self, *_):
        pass


class _Command:
    def __init__(self, kind, *args):
        self.kind, self.args = kind, args

    def then(self, _):
        return self


class _Controller:
    def __init__(self):
        self.destinations = []
        self.speed = None
        self.stopped = False

    def start(self):
        pass

    def go_to_location(self, location):
        if not isinstance(location, carla.Location):
            raise TypeError("go_to_location requires a carla.Location")
        self.destinations.append(location)

    def set_max_speed(self, speed):
        self.speed = speed

    def stop(self):
        self.stopped = True


class _World:
    def __init__(self, walker_count, initial_sync=False, fixed_delta=0.1,
                 no_rendering=False, missing_target=False):
        self.settings = SimpleNamespace(synchronous_mode=initial_sync,
                                        fixed_delta_seconds=fixed_delta,
                                        no_rendering_mode=no_rendering)
        self.walker_count = walker_count
        self.missing_target = missing_target
        self.navigation_calls = 0
        self.ready = False
        self.controllers = {}
        self.seed_calls = []
        self.blueprints = [_Blueprint("walker.a", 1.4), _Blueprint("walker.b", 2.4)]

    def get_settings(self):
        return copy.copy(self.settings)

    def apply_settings(self, settings):
        self.settings = copy.copy(settings)

    def get_blueprint_library(self):
        return SimpleNamespace(
            filter=lambda pattern: self.blueprints if pattern.startswith("walker")
            else [_Blueprint("vehicle.a")], find=lambda _: _Blueprint("controller.ai.walker"))

    def get_map(self):
        return SimpleNamespace(get_spawn_points=lambda: [])

    def get_random_location_from_navigation(self):
        self.navigation_calls += 1
        if self.missing_target and self.navigation_calls > self.walker_count:
            return None
        return carla.Location(self.navigation_calls, 0, 0)

    def get_actors(self, actor_ids):
        return [self.controllers.get(actor_id, SimpleNamespace(id=actor_id))
                for actor_id in actor_ids]

    def set_pedestrians_seed(self, seed):
        self.seed_calls.append(seed)

    def set_pedestrians_cross_factor(self, _):
        pass

    def tick(self):
        if self.ready:
            raise _Finished()

    def wait_for_tick(self):
        if self.ready:
            raise _Finished()


class _Client:
    def __init__(self, world, failed_controllers=()):
        self.world = world
        self.failed_controllers = failed_controllers
        self.tick_requests = []
        self.destroyed = []
        self.controller_by_walker = {}
        self.manager = mock.Mock()
        self.manager.get_port.return_value = 8000
        self.manager.global_percentage_speed_difference.side_effect = self._ready

    def _ready(self, _):
        self.world.ready = True

    def set_timeout(self, _):
        pass

    def get_world(self):
        return self.world

    def get_trafficmanager(self, _):
        return self.manager

    def apply_batch_sync(self, batch, do_tick):
        self.tick_requests.append(do_tick)
        results = []
        for index, command in enumerate(batch):
            blueprint = command.args[0]
            if blueprint.id == "controller.ai.walker":
                walker_id = command.args[2]
                if index in self.failed_controllers:
                    results.append(SimpleNamespace(error="controller spawn failed", actor_id=0))
                    continue
                actor_id = 200 + index
                controller = _Controller()
                self.world.controllers[actor_id] = controller
                self.controller_by_walker[walker_id] = controller
            else:
                actor_id = 100 + index
            results.append(SimpleNamespace(error="", actor_id=actor_id))
        return results

    def apply_batch(self, batch):
        self.destroyed.extend(command.args[0] for command in batch)


class TestGenerateTrafficRegressions(unittest.TestCase):
    def _run(self, arguments=(), walker_count=1, failed_controllers=(), **world_options):
        world = _World(walker_count, **world_options)
        client = _Client(world, failed_controllers)
        selections = iter(world.blueprints * max(1, walker_count))
        argv = ["generate_traffic.py", "-n", "0", "-w", str(walker_count), *arguments]
        with mock.patch.object(TRAFFIC.carla, "Client", return_value=client), \
                mock.patch.object(TRAFFIC.carla.command, "SpawnActor",
                                  side_effect=lambda *args: _Command("spawn", *args)), \
                mock.patch.object(TRAFFIC.carla.command, "DestroyActor",
                                  side_effect=lambda actor_id: _Command("destroy", actor_id)), \
                mock.patch.object(TRAFFIC.random, "choice", side_effect=lambda _: next(selections)), \
                mock.patch.object(TRAFFIC.time, "sleep"), \
                mock.patch.object(TRAFFIC.logging, "error"), \
                mock.patch.object(sys, "argv", argv), \
                contextlib.redirect_stdout(io.StringIO()):
            try:
                TRAFFIC.main()
            except _Finished:
                pass
        return world, client

    def test_failed_controller_cleans_its_walker_and_keeps_other_pair(self):
        _, client = self._run(walker_count=2, failed_controllers=(0,))
        self.assertIn(100, client.destroyed)
        self.assertEqual(client.controller_by_walker[101].speed, 2.4)
        self.assertTrue(client.controller_by_walker[101].stopped)
        self.assertEqual(set(client.destroyed), {100, 101, 201})

    def test_explicit_zero_pedestrian_seed_is_applied(self):
        world, _ = self._run(["--seedw", "0"])
        self.assertEqual(world.seed_calls, [0])

    def test_positive_pedestrian_seed_still_works(self):
        world, _ = self._run(["--seedw", "7"])
        self.assertEqual(world.seed_calls, [7])

    def test_missing_navigation_target_is_not_sent_to_controller(self):
        _, client = self._run(missing_target=True)
        self.assertEqual(client.controller_by_walker[100].destinations, [])

    def test_sync_follower_never_requests_batch_ticks(self):
        _, client = self._run(initial_sync=True)
        self.assertEqual(client.tick_requests, [False, False, False])

    def test_sync_owner_still_requests_batch_ticks(self):
        _, client = self._run()
        self.assertEqual(client.tick_requests, [True, True, True])

    def test_initial_timestep_and_rendering_are_restored(self):
        world, _ = self._run(fixed_delta=0.1, no_rendering=True)
        self.assertFalse(world.settings.synchronous_mode)
        self.assertEqual(world.settings.fixed_delta_seconds, 0.1)
        self.assertTrue(world.settings.no_rendering_mode)

    def test_async_rendering_change_is_restored(self):
        world, _ = self._run(["--asynch", "--no-rendering"])
        self.assertFalse(world.settings.no_rendering_mode)


if __name__ == "__main__":
    unittest.main()
