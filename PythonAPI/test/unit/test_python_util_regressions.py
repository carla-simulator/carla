# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Run actual utility entry points with inert clients and real native textures."""

import contextlib
import importlib.util
import io
import os
from pathlib import Path
import sys
from types import ModuleType, SimpleNamespace
import unittest
from unittest import mock

import carla

SOURCE_ROOT = Path(os.environ.get(
    "CARLA_REGRESSION_SOURCE_ROOT", str(Path(__file__).resolve().parents[3])))

def _load(name):
    path = SOURCE_ROOT / "PythonAPI" / "util" / (name + ".py")
    spec = importlib.util.spec_from_file_location("regression_" + name, path)
    module = importlib.util.module_from_spec(spec)
    # Image loading is not reached by these tests; texture conversion stays real.
    with mock.patch.dict(sys.modules, {"imageio": ModuleType("imageio")}):
        spec.loader.exec_module(module)
    return module


TEXTURES = _load("apply_texture")
LIGHTS = _load("manage_traffic_light")


class TestPythonUtilRegressions(unittest.TestCase):
    def test_absent_diffuse_is_an_empty_color_texture(self):
        texture = TEXTURES.get_8bit_texture(None)
        self.assertIsInstance(texture, carla.TextureColor)
        self.assertEqual((texture.width, texture.height), (0, 0))

    def test_rgb_diffuse_gets_opaque_alpha(self):
        texture = TEXTURES.get_8bit_texture([[[12, 34, 56]]])
        pixel = texture.get(0, 0)
        self.assertEqual((pixel.r, pixel.g, pixel.b, pixel.a), (12, 34, 56, 255))

    def test_rgba_diffuse_retains_alpha_and_vertical_orientation(self):
        texture = TEXTURES.get_8bit_texture([[[1, 2, 3, 4]], [[5, 6, 7, 8]]])
        pixel = texture.get(0, 0)
        self.assertEqual((pixel.r, pixel.g, pixel.b, pixel.a), (5, 6, 7, 8))

    def test_list_prints_all_object_names(self):
        world = SimpleNamespace(get_names_of_all_objects=lambda: ["Apartment01", "Tree01", "Bench01"])
        client = SimpleNamespace(set_timeout=lambda _: None, get_world=lambda: world)
        output = io.StringIO()
        with mock.patch.object(TEXTURES.carla, "Client", return_value=client), \
                mock.patch.object(sys, "argv", ["apply_texture.py", "--list"]), \
                contextlib.redirect_stdout(output):
            TEXTURES.main()
        self.assertEqual(output.getvalue().splitlines(), ["Apartment01", "Tree01", "Bench01"])

    @staticmethod
    def _light_world(light, in_junction=True):
        junction = SimpleNamespace(id=17)
        waypoint = SimpleNamespace(is_junction=True, junction_id=17, get_junction=lambda: junction)
        topology = [(waypoint, waypoint)] if in_junction else []
        return SimpleNamespace(
            get_map=lambda: SimpleNamespace(get_topology=lambda: topology),
            get_traffic_lights_in_junction=lambda _: [light],
            get_traffic_light_from_opendrive_id=lambda sign_id:
                light if sign_id == light.get_opendrive_id() else None)

    def test_freeze_option_requests_freezing(self):
        light = mock.Mock()
        light.get_opendrive_id.return_value = "42"
        world = self._light_world(light)
        with mock.patch.object(LIGHTS.carla, "Client", return_value=SimpleNamespace(get_world=lambda: world)), \
                mock.patch.object(sys, "argv", ["manage_traffic_light.py", "--id", "42", "--freeze"]), \
                contextlib.redirect_stdout(io.StringIO()):
            LIGHTS.main()
        light.freeze.assert_called_once_with(True)

    def test_lookup_includes_nonjunction_traffic_lights(self):
        light = SimpleNamespace(get_opendrive_id=lambda: "42")
        self.assertIs(LIGHTS.get_traffic_light(self._light_world(light, False), "42"), light)

    def test_unknown_opendrive_light_is_not_matched(self):
        light = SimpleNamespace(get_opendrive_id=lambda: "42")
        self.assertIsNone(LIGHTS.get_traffic_light(self._light_world(light), "missing"))


if __name__ == "__main__":
    unittest.main()
