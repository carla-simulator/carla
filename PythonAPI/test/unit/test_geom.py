# Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import carla

import sys
import unittest

from subprocess import check_output


class TestGeom(unittest.TestCase):
    def test_vector3d(self):
        c = carla.Vector3D(1.0, 2.0, 3.0)
        self.assertEqual(c.x, 1.0)
        self.assertEqual(c.y, 2.0)
        self.assertEqual(c.z, 3.0)
        c_unit = c.make_unit_vector()
        # c should not be modified by make_unit_vector
        self.assertEqual(c.x, 1.0)
        self.assertEqual(c.y, 2.0)
        self.assertEqual(c.z, 3.0)
        # the length of c_unit should be 1.0
        length = (c_unit.x ** 2 + c_unit.y ** 2 + c_unit.z ** 2) ** 0.5
        self.assertAlmostEqual(length, 1.0)

        # test the behavior of make_unit_vector when the length of the vector is smaller than epsilon
        with self.assertRaisesRegex(RuntimeError, "length >= 2.0f"):
            zero = carla.Vector3D(0.0, 0.0, 0.0)
            zero_unit = zero.make_unit_vector()

        with self.assertRaisesRegex(RuntimeError, "length >= 2.0f"):
            small = carla.Vector3D(1e-8, 0.0, 0.0)
            small_unit = small.make_unit_vector(epsilon=1e-6)

        large = carla.Vector3D(1e-4, 0.0, 0.0)
        large_unit = large.make_unit_vector(epsilon=1e-6)
        self.assertEqual(large_unit.x, 1.0)
        self.assertEqual(large_unit.y, 0.0)
        self.assertEqual(large_unit.z, 0.0)
