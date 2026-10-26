# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB). Licensed under the MIT license.
"""Run skill helper functions with native geometry and inert RPC boundaries."""

import contextlib
import importlib.util
import io
import os
from pathlib import Path
from types import SimpleNamespace
import unittest
from unittest import mock

import carla


ROOT = Path(os.environ.get("CARLA_SKILL_TEST_ROOT", Path(__file__).resolve().parents[3]))


def _load(skill, script):
    path = ROOT / "Skills/python-api" / skill / "scripts" / script
    spec = importlib.util.spec_from_file_location("skill_" + skill.replace("-", "_"), path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


MAPS = _load("load-map", "load_map.py")
TRAFFIC = _load("control-traffic", "traffic.py")
SETTINGS = _load("set-world-settings", "world_settings.py")
REPLAY = _load("replay-recording", "replay.py")


class TestSkillMapResolution(unittest.TestCase):
    def test_classic_town_alias_uses_available_layered_map(self):
        self.assertEqual(MAPS.resolve_map("Town3", ["Town03_Opt"]), "Town03_Opt")

    def test_large_town_alias_preserves_nested_package_path(self):
        self.assertEqual(MAPS.resolve_map("12", ["Town12/Town12"]), "Town12/Town12")

    def test_explicit_map_and_existing_plain_preference_are_preserved(self):
        available = ["Town03", "Town03_Opt", "Town10HD_Opt"]
        self.assertEqual(MAPS.resolve_map("Town3", available), "Town03")
        self.assertEqual(MAPS.resolve_map("town03_opt", available), "Town03_Opt")
        self.assertEqual(MAPS.resolve_map("Town10", available), "Town10HD_Opt")

    def test_ambiguous_leaf_is_not_arbitrarily_selected(self):
        with self.assertRaises(SystemExit):
            MAPS.resolve_map("Town12", ["PackA/Town12", "PackB/Town12"])


class TestSkillClockHandling(unittest.TestCase):
    @staticmethod
    def _world():
        settings = carla.WorldSettings(synchronous_mode=True, fixed_delta_seconds=0.05)
        return SimpleNamespace(
            get_settings=lambda: settings,
            get_map=lambda: SimpleNamespace(name="TestMap"),
            apply_settings=mock.Mock(),
            wait_for_tick=mock.Mock(side_effect=RuntimeError("no clock owner")))

    def test_async_can_release_a_stalled_synchronous_world(self):
        world = self._world()
        manager = SimpleNamespace(set_synchronous_mode=mock.Mock())
        client = SimpleNamespace(get_world=lambda: world, get_trafficmanager=lambda port: manager)
        with mock.patch.object(SETTINGS, "_client", return_value=client), contextlib.redirect_stdout(io.StringIO()):
            SETTINGS.cmd_async(SimpleNamespace(no_rendering=False))
        world.wait_for_tick.assert_not_called()
        world.apply_settings.assert_called_once()
        self.assertFalse(world.get_settings().synchronous_mode)
        manager.set_synchronous_mode.assert_called_once_with(False)

    def test_sync_settings_can_change_without_an_existing_ticker(self):
        world = self._world()
        manager = SimpleNamespace(set_synchronous_mode=mock.Mock())
        client = SimpleNamespace(get_world=lambda: world, get_trafficmanager=lambda port: manager)
        with mock.patch.object(SETTINGS, "_client", return_value=client), contextlib.redirect_stdout(io.StringIO()):
            SETTINGS.cmd_sync(SimpleNamespace(fps=25.0, delta=None, no_rendering=False))
        world.wait_for_tick.assert_not_called()
        self.assertAlmostEqual(world.get_settings().fixed_delta_seconds, 0.04)
        manager.set_synchronous_mode.assert_called_once_with(True)

    def test_all_traffic_waits_for_its_initial_actor_snapshot(self):
        vehicle = SimpleNamespace(id=7)
        ready = []
        world = SimpleNamespace(
            get_settings=lambda: SimpleNamespace(synchronous_mode=True),
            wait_for_tick=lambda: ready.append(True),
            get_actors=lambda: SimpleNamespace(filter=lambda pattern: [vehicle] if ready else []))
        client = SimpleNamespace(get_world=lambda: world)
        args = SimpleNamespace(filter="vehicle.*", tm_port=8000)
        with mock.patch.object(TRAFFIC, "_client", return_value=client), \
                mock.patch.object(TRAFFIC, "_tm", return_value=object()), \
                mock.patch.object(TRAFFIC, "_apply_per_vehicle", return_value=[7]) as apply, \
                contextlib.redirect_stdout(io.StringIO()):
            TRAFFIC.cmd_all(args)
        self.assertEqual(ready, [True])
        self.assertEqual(apply.call_args.args[1], [vehicle])


class TestSkillReplayPortability(unittest.TestCase):
    def test_play_does_not_require_linux_proc_filesystem(self):
        args = SimpleNamespace(
            keep_scene=False, ignore_hero=False, ignore_spectator=False,
            time_factor=1.0, file="run.log", start=0.0, duration=0.0,
            follow=0, replay_sensors=False, replay_weather=False, map_override="")
        client = SimpleNamespace(set_replayer_time_factor=mock.Mock(), replay_file=mock.Mock(return_value="ok"))
        real_listdir = REPLAY.os.listdir

        def listdir(path):
            if str(path) == "/proc":
                raise FileNotFoundError("/proc is absent on this platform")
            return real_listdir(path)

        with mock.patch.object(REPLAY, "_client", return_value=client), \
                mock.patch.object(REPLAY.os, "listdir", side_effect=listdir), \
                mock.patch.object(REPLAY, "_release_clock_if_stalled"), \
                mock.patch.object(REPLAY, "_clear_scene"), \
                contextlib.redirect_stdout(io.StringIO()):
            REPLAY.cmd_play(args)
        client.replay_file.assert_called_once()


if __name__ == "__main__":
    unittest.main()
