# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Execute the walker tutorial's actual snippets without a server.

The doubles record calls and pose values; Unreal animation is not simulated.
"""
from pathlib import Path
import re
import sys
import types
import unittest
from unittest.mock import patch


GUIDE = Path(__file__).resolve().parents[3] / "Docs" / "tuto_G_control_walker_skeletons.md"


class Rotation:
    def __init__(self, pitch=0, yaw=0, roll=0):
        self.pitch, self.yaw, self.roll = pitch, yaw, roll


class Transform:
    def __init__(self, location=None, rotation=None):
        self.location = location if location is not None else (0, 0, 0)
        self.rotation = rotation if rotation is not None else Rotation()


class BoneControl:
    pass


class Walker:
    def __init__(self):
        self.calls = []
        self.control = None
        self.pose = [
            types.SimpleNamespace(name="crl_hand__R", relative=Transform((1, 2, 3), Rotation(4, 5, 6))),
            types.SimpleNamespace(name="crl_hand__L", relative=Transform((7, 8, 9), Rotation(10, 11, 12))),
        ]

    def get_bones(self):
        self.calls.append("get_bones")
        return types.SimpleNamespace(bone_transforms=self.pose)

    def set_bones(self, control):
        if not isinstance(control, BoneControl):
            raise TypeError("set_bones expects WalkerBoneControlIn")
        self.calls.append("set_bones")
        self.control = control

    def show_pose(self):
        self.calls.append("show_pose")


class World:
    def __init__(self, walker, spawn_points):
        self.walker = walker
        self.spawn_points = spawn_points
        self.blueprint = object()
        self.spawn_calls = []

    def get_blueprint_library(self):
        return types.SimpleNamespace(filter=lambda pattern: [self.blueprint])

    def get_map(self):
        return types.SimpleNamespace(get_spawn_points=lambda: self.spawn_points)

    def try_spawn_actor(self, blueprint, transform):
        self.spawn_calls.append((blueprint, transform))
        return self.walker


def run_example(world):
    code = "\n\n".join(re.findall(
        r"```(?:py|python)\n(.*?)\n```", GUIDE.read_text(encoding="utf-8"), re.DOTALL))
    if not code:
        raise AssertionError("no executable Python examples found")
    client = types.SimpleNamespace(
        set_timeout=lambda seconds: None,
        get_world=lambda: world)
    carla = types.ModuleType("carla")
    carla.Client = lambda host, port: client
    carla.Transform = Transform
    carla.Rotation = Rotation
    carla.WalkerBoneControlIn = BoneControl
    with patch.dict(sys.modules, {"carla": carla}):
        exec(compile(code, str(GUIDE), "exec"), {"__name__": "__main__"})


class WalkerDocumentationExample(unittest.TestCase):
    def test_spawned_walker_receives_pose_without_collapsing_bones(self):
        walker = Walker()
        spawn_point = Transform((20, 30, 40))
        world = World(walker, [spawn_point])
        run_example(world)
        self.assertEqual(world.spawn_calls, [(world.blueprint, spawn_point)])
        self.assertEqual(walker.calls, ["get_bones", "set_bones", "show_pose"])
        changes = dict(walker.control.bone_transforms)
        self.assertEqual(set(changes), {"crl_hand__R", "crl_hand__L"})
        for original in walker.pose:
            changed = changes[original.name]
            self.assertEqual(changed.location, original.relative.location)
            self.assertEqual(changed.rotation.pitch, original.relative.rotation.pitch)
            self.assertEqual(changed.rotation.yaw, original.relative.rotation.yaw)
            self.assertEqual(changed.rotation.roll, original.relative.rotation.roll + 90)

    def test_no_map_spawn_points_uses_default_transform(self):
        walker = Walker()
        world = World(walker, [])
        run_example(world)
        self.assertEqual(len(world.spawn_calls), 1)
        self.assertEqual(world.spawn_calls[0][1].location, (0, 0, 0))
        self.assertEqual(walker.calls[-1], "show_pose")

    def test_failed_spawn_stops_before_pose_commands(self):
        world = World(None, [Transform()])
        with self.assertRaisesRegex(RuntimeError, "could not spawn the walker"):
            run_example(world)
        self.assertEqual(len(world.spawn_calls), 1)


if __name__ == "__main__":
    unittest.main()
