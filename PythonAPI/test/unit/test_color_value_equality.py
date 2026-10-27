# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Value semantics for the public native RGBA color bindings."""

import unittest

import carla


class TestColorValueEquality(unittest.TestCase):
    """RGBA identity includes alpha; existing opaque/RGB behavior is retained."""

    def test_alpha_distinguishes_rgba_values(self):
        """Different alpha values identify different colors in both directions."""
        opaque = carla.Color(4, 5, 6, 255)
        for alpha in (0, 1, 127, 254):
            with self.subTest(alpha=alpha):
                color = carla.Color(4, 5, 6, alpha)
                self.assertNotEqual(color, opaque)
                self.assertNotEqual(opaque, color)
                self.assertFalse(color == opaque)

    def test_explicit_transparent_black_differs_from_default_opaque_black(self):
        """The Python default stays opaque, while explicit zero alpha differs."""
        default = carla.Color()
        self.assertEqual(default.a, 255)
        self.assertNotEqual(default, carla.Color(0, 0, 0, 0))

    def test_rgba_and_float_rgba_distinctions_agree(self):
        """Both public color value types include the alpha channel."""
        first = carla.Color(4, 5, 6, 0)
        second = carla.Color(4, 5, 6, 255)
        first_float = carla.FloatColor(4 / 255, 5 / 255, 6 / 255, 0)
        second_float = carla.FloatColor(4 / 255, 5 / 255, 6 / 255, 1)
        self.assertNotEqual(first_float, second_float)
        self.assertEqual(first == second, first_float == second_float)

    def test_opaque_defaults_and_rgb_differences_control(self):
        """Default alpha, identical values and RGB channel changes are retained."""
        original = carla.Color(4, 5, 6)
        self.assertEqual(original, carla.Color(4, 5, 6, 255))
        self.assertNotEqual(original, carla.Color(3, 5, 6))
        self.assertNotEqual(original, carla.Color(4, 3, 6))
        self.assertNotEqual(original, carla.Color(4, 5, 3))
