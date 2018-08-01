# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import carla

import unittest


class testVehicleControl(unittest.TestCase):
    def test_default_values(self):
        c = carla.VehicleControl()
        self.assertEqual(c.throttle, 0.0)
        self.assertEqual(c.steer, 0.0)
        self.assertEqual(c.brake, 0.0)
        self.assertEqual(c.hand_brake, False)
        self.assertEqual(c.reverse, False)
        c = carla.VehicleControl(1.0, 2.0, 3.0, True, True)
        self.assertEqual(c.throttle, 1.0)
        self.assertEqual(c.steer, 2.0)
        self.assertEqual(c.brake, 3.0)
        self.assertEqual(c.hand_brake, True)
        self.assertEqual(c.reverse, True)

    def test_named_args(self):
        c = carla.VehicleControl(
            throttle=1.0,
            steer=2.0,
            brake=3.0,
            hand_brake=True,
            reverse=True)
        self.assertEqual(c.throttle, 1.0)
        self.assertEqual(c.steer, 2.0)
        self.assertEqual(c.brake, 3.0)
        self.assertEqual(c.hand_brake, True)
        self.assertEqual(c.reverse, True)
