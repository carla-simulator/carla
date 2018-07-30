# Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import carla

import unittest


class testLocation(unittest.TestCase):
    def test_default_values(self):
        location = carla.Location()
        self.assertEqual(location.x, 0.0)
        self.assertEqual(location.y, 0.0)
        self.assertEqual(location.z, 0.0)
        location = carla.Location(1.0)
        self.assertEqual(location.x, 1.0)
        self.assertEqual(location.y, 0.0)
        self.assertEqual(location.z, 0.0)
        location = carla.Location(1.0, 2.0)
        self.assertEqual(location.x, 1.0)
        self.assertEqual(location.y, 2.0)
        self.assertEqual(location.z, 0.0)
        location = carla.Location(1.0, 2.0, 3.0)
        self.assertEqual(location.x, 1.0)
        self.assertEqual(location.y, 2.0)
        self.assertEqual(location.z, 3.0)

    def test_named_args(self):
        location = carla.Location(x=42.0)
        self.assertEqual(location.x, 42.0)
        self.assertEqual(location.y, 0.0)
        self.assertEqual(location.z, 0.0)
        location = carla.Location(y=42.0)
        self.assertEqual(location.x, 0.0)
        self.assertEqual(location.y, 42.0)
        self.assertEqual(location.z, 0.0)
        location = carla.Location(z=42.0)
        self.assertEqual(location.x, 0.0)
        self.assertEqual(location.y, 0.0)
        self.assertEqual(location.z, 42.0)
        location = carla.Location(z=3.0, x=1.0, y=2.0)
        self.assertEqual(location.x, 1.0)
        self.assertEqual(location.y, 2.0)
        self.assertEqual(location.z, 3.0)


class testRotation(unittest.TestCase):
    def test_default_values(self):
        rotation = carla.Rotation()
        self.assertEqual(rotation.pitch, 0.0)
        self.assertEqual(rotation.yaw, 0.0)
        self.assertEqual(rotation.roll, 0.0)
        rotation = carla.Rotation(1.0)
        self.assertEqual(rotation.pitch, 1.0)
        self.assertEqual(rotation.yaw, 0.0)
        self.assertEqual(rotation.roll, 0.0)
        rotation = carla.Rotation(1.0, 2.0)
        self.assertEqual(rotation.pitch, 1.0)
        self.assertEqual(rotation.yaw, 2.0)
        self.assertEqual(rotation.roll, 0.0)
        rotation = carla.Rotation(1.0, 2.0, 3.0)
        self.assertEqual(rotation.pitch, 1.0)
        self.assertEqual(rotation.yaw, 2.0)
        self.assertEqual(rotation.roll, 3.0)

    def test_named_args(self):
        rotation = carla.Rotation(pitch=42.0)
        self.assertEqual(rotation.pitch, 42.0)
        self.assertEqual(rotation.yaw, 0.0)
        self.assertEqual(rotation.roll, 0.0)
        rotation = carla.Rotation(yaw=42.0)
        self.assertEqual(rotation.pitch, 0.0)
        self.assertEqual(rotation.yaw, 42.0)
        self.assertEqual(rotation.roll, 0.0)
        rotation = carla.Rotation(roll=42.0)
        self.assertEqual(rotation.pitch, 0.0)
        self.assertEqual(rotation.yaw, 0.0)
        self.assertEqual(rotation.roll, 42.0)
        rotation = carla.Rotation(roll=3.0, pitch=1.0, yaw=2.0)
        self.assertEqual(rotation.pitch, 1.0)
        self.assertEqual(rotation.yaw, 2.0)
        self.assertEqual(rotation.roll, 3.0)


class testTransform(unittest.TestCase):
    def test_values(self):
        t = carla.Transform()
        self.assertEqual(t.location.x, 0.0)
        self.assertEqual(t.location.y, 0.0)
        self.assertEqual(t.location.z, 0.0)
        self.assertEqual(t.rotation.pitch, 0.0)
        self.assertEqual(t.rotation.yaw, 0.0)
        self.assertEqual(t.rotation.roll, 0.0)
        t = carla.Transform(carla.Location(y=42.0))
        self.assertEqual(t.location.x, 0.0)
        self.assertEqual(t.location.y, 42.0)
        self.assertEqual(t.location.z, 0.0)
        self.assertEqual(t.rotation.pitch, 0.0)
        self.assertEqual(t.rotation.yaw, 0.0)
        self.assertEqual(t.rotation.roll, 0.0)
        t = carla.Transform(rotation=carla.Rotation(yaw=42.0))
        self.assertEqual(t.location.x, 0.0)
        self.assertEqual(t.location.y, 0.0)
        self.assertEqual(t.location.z, 0.0)
        self.assertEqual(t.rotation.pitch, 0.0)
        self.assertEqual(t.rotation.yaw, 42.0)
        self.assertEqual(t.rotation.roll, 0.0)

    def test_print(self):
        t = carla.Transform(
            carla.Location(x=1.0, y=2.0, z=3.0),
            carla.Rotation(pitch=4.0, yaw=5.0, roll=6.0))
        s = 'Transform(Location(x=1, y=2, z=3), Rotation(pitch=4, yaw=5, roll=6))'
        self.assertEqual(str(t), s)
