# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import carla

import unittest


class TestLocation(unittest.TestCase):
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


class TestRotation(unittest.TestCase):
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


class TestTransform(unittest.TestCase):
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
        s = 'Transform(Location(x=1.000000, y=2.000000, z=3.000000), Rotation(pitch=4.000000, yaw=5.000000, roll=6.000000))'
        self.assertEqual(str(t), s)

    def test_translation(self):
        error = .001
        t = carla.Transform(
            carla.Location(x=8.0, y=19.0, z=20.0),
            carla.Rotation(pitch=0.0, yaw=0.0, roll=0.0))
        point = carla.Location(x=0.0, y=0.0, z=0.0)
        t.transform(point)
        self.assertTrue(abs(point.x - 8.0) <= error)
        self.assertTrue(abs(point.y - 19.0) <= error)
        self.assertTrue(abs(point.z - 20.0) <= error)

    def test_rotation(self):
        error = .001
        t = carla.Transform(
            carla.Location(x=0.0, y=0.0, z=0.0),
            carla.Rotation(pitch=180.0, yaw=0.0, roll=0.0))
        point = carla.Location(x=0.0, y=0.0, z=1.0)
        t.transform(point)

        self.assertTrue(abs(point.x - 0.0) <= error)
        self.assertTrue(abs(point.y - 0.0) <= error)
        self.assertTrue(abs(point.z - (-1.0)) <= error)

    def test_rotation_and_translation(self):
        error = .001
        t = carla.Transform(
            carla.Location(x=0.0, y=0.0, z=-1.0),
            carla.Rotation(pitch=90.0, yaw=0.0, roll=0.0))
        point = carla.Location(x=0.0, y=0.0, z=2.0)
        t.transform(point)

        self.assertTrue(abs(point.x - (-2.0)) <= error)
        self.assertTrue(abs(point.y - 0.0) <= error)
        self.assertTrue(abs(point.z - (-1.0)) <= error)

    def test_list_rotation_and_translation_location(self):
        error = .001
        t = carla.Transform(
            carla.Location(x=0.0, y=0.0, z=-1.0),
            carla.Rotation(pitch=90.0, yaw=0.0, roll=0.0))

        point_list = [carla.Location(x=0.0, y=0.0, z=2.0),
                      carla.Location(x=0.0, y=10.0, z=1.0),
                      carla.Location(x=0.0, y=18.0, z=2.0)
                      ]
        t.transform(point_list)

        solution_list = [carla.Location(-2.0, 0.0, -1.0),
                         carla.Location(-1.0, 10.0, -1.0),
                         carla.Location(-2.0, 18.0, -1.0)
                         ]

        for i in range(len(point_list)):
            self.assertTrue(abs(point_list[i].x - solution_list[i].x) <= error)
            self.assertTrue(abs(point_list[i].y - solution_list[i].y) <= error)
            self.assertTrue(abs(point_list[i].z - solution_list[i].z) <= error)

    def test_list_rotation_and_translation_vector3d(self):
        error = .001
        t = carla.Transform(
            carla.Location(x=0.0, y=0.0, z=-1.0),
            carla.Rotation(pitch=90.0, yaw=0.0, roll=0.0))

        point_list = [carla.Vector3D(0.0, 0.0, 2.0),
                      carla.Vector3D(0.0, 10.0, 1.0),
                      carla.Vector3D(0.0, 18.0, 2.0)
                      ]
        t.transform(point_list)

        solution_list = [carla.Vector3D(-2.0, 0.0, -1.0),
                         carla.Vector3D(-1.0, 10.0, -1.0),
                         carla.Vector3D(-2.0, 18.0, -1.0)
                         ]

        for i in range(len(point_list)):
            self.assertTrue(abs(point_list[i].x - solution_list[i].x) <= error)
            self.assertTrue(abs(point_list[i].y - solution_list[i].y) <= error)
            self.assertTrue(abs(point_list[i].z - solution_list[i].z) <= error)
