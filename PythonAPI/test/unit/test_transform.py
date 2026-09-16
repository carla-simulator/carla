# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import carla

import unittest
import math

WGS84 = carla.GeoEllipsoid(a=6378137.0, f_inv=298.257223563)

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


class TestVector(unittest.TestCase):
    def test_make_unit_vector_3d_no_argument(self):
        # The defaulted epsilon must remain optional through the Boost.Python
        # binding, so a no-argument call keeps working.
        error = .001
        unit = carla.Vector3D(10.0, 0.0, 0.0).make_unit_vector()
        self.assertTrue(abs(unit.x - 1.0) <= error)
        self.assertTrue(abs(unit.y - 0.0) <= error)
        self.assertTrue(abs(unit.z - 0.0) <= error)

    def test_make_unit_vector_3d_epsilon_keyword(self):
        # A length at or below epsilon returns the input unchanged.
        v = carla.Vector3D(0.0, 0.0, 0.5)
        unchanged = v.make_unit_vector(epsilon=1.0)
        self.assertEqual(unchanged.x, 0.0)
        self.assertEqual(unchanged.y, 0.0)
        self.assertEqual(unchanged.z, 0.5)

    def test_make_unit_vector_3d_zero_length(self):
        zero = carla.Vector3D().make_unit_vector()
        self.assertEqual(zero.x, 0.0)
        self.assertEqual(zero.y, 0.0)
        self.assertEqual(zero.z, 0.0)

    def test_make_unit_vector_2d_no_argument(self):
        error = .001
        unit = carla.Vector2D(0.0, 4.0).make_unit_vector()
        self.assertTrue(abs(unit.x - 0.0) <= error)
        self.assertTrue(abs(unit.y - 1.0) <= error)

    def test_make_unit_vector_2d_epsilon_keyword(self):
        v = carla.Vector2D(0.5, 0.0)
        unchanged = v.make_unit_vector(epsilon=1.0)
        self.assertEqual(unchanged.x, 0.5)
        self.assertEqual(unchanged.y, 0.0)


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

        # Engine convention: +pitch is nose up, so pitch=90 sends the up axis
        # {0, 0, 2} onto {-2, 0, 0}; the translation by {0, 0, -1} then brings
        # it to {-2, 0, -1}. (#9751 briefly mirrored this to {2, 0, -1}.)
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

        # Engine convention: pitch=90 maps (x, y, z) -> (-z, y, x); the
        # translation by {0, 0, -1} is then added.
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

        # Engine convention: pitch=90 maps (x, y, z) -> (-z, y, x); the
        # translation by {0, 0, -1} is then added.
        solution_list = [carla.Vector3D(-2.0, 0.0, -1.0),
                         carla.Vector3D(-1.0, 10.0, -1.0),
                         carla.Vector3D(-2.0, 18.0, -1.0)
                         ]

        for i in range(len(point_list)):
            self.assertTrue(abs(point_list[i].x - solution_list[i].x) <= error)
            self.assertTrue(abs(point_list[i].y - solution_list[i].y) <= error)
            self.assertTrue(abs(point_list[i].z - solution_list[i].z) <= error)

    def test_geo_offset_transform(self):
        error = 0.001
        t = carla.GeoOffsetTransform(1.0, 2.0, 3.0, 0.0)

        self.assertTrue(abs(t.offset_x - 1.0) <= error)
        self.assertTrue(abs(t.offset_y - 2.0) <= error)
        self.assertTrue(abs(t.offset_z - 3.0) <= error)

    def test_geo_offset_transform_translation(self):
        error = 0.001
        t = carla.GeoOffsetTransform(10.0, 20.0, 5.0, 0.0)
        loc = carla.Location(1.0, 2.0, 3.0)

        out = t.ApplyTransformation(loc)

        solution_list = carla.Location(11.0, 18.0, 8.0)

        self.assertTrue(abs(out.x - solution_list.x) <= error)
        self.assertTrue(abs(out.y - solution_list.y) <= error)
        self.assertTrue(abs(out.z - solution_list.z) <= error)

    def test_geo_offset_transform_rotation(self):
        error = 0.001
        t = carla.GeoOffsetTransform(0.0, 0.0, 0.0, math.pi / 2.0)
        loc = carla.Location(1.0, 0.0, 0.0)

        out = t.ApplyTransformation(loc)

        solution_list = carla.Location(0.0, -1.0, 0.0)

        self.assertTrue(abs(out.x - solution_list.x) <= error)
        self.assertTrue(abs(out.y - solution_list.y) <= error)

    def test_geo_offset_transform_and_rotation(self):
        error = 0.001
        t =  carla.GeoOffsetTransform(5.0, 0.0, 0.0, math.pi / 2.0)
        loc = carla.Location(1.0, 0.0, 0.0)

        out = t.ApplyTransformation(loc)

        solution_list = carla.Location(0.0, -6.0, 0.0)
        self.assertTrue(abs(out.x - solution_list.x) <= error)
        self.assertTrue(abs(out.y - solution_list.y) <= error)

    def test_geo_offset_transform_equality(self):
        t1 = carla.GeoOffsetTransform(1.0, 2.0, 3.0, math.pi / 2.0)
        t2 = carla.GeoOffsetTransform(1.0, 2.0, 3.0, math.pi / 2.0)
        t3 = carla.GeoOffsetTransform(1.0, 2.0, 3.0, 0.0)

        self.assertTrue(t1 == t2)
        self.assertFalse(t1 == t3)

    def test_geo_projection_utm(self):
        p = carla.GeoProjectionUTM()

        self.assertEqual(p.zone, 31)
        self.assertTrue(p.north)
        self.assertIsInstance(p.ellps, carla.GeoEllipsoid)
        self.assertIsNone(p.offset)

    def test_geo_projection_utm_offset_none(self):
        p = carla.GeoProjectionUTM()
        p.zone=32
        p.north=True
        p.ellps=carla.GeoEllipsoid()
        p.offset=None

        self.assertEqual(p.zone, 32)
        self.assertIsNone(p.offset)

    def test_geo_projection_utm_with_offset(self):
        t = carla.GeoOffsetTransform(1.0, 2.0, 3.0, 0.0)

        p = carla.GeoProjectionUTM()
        p.zone = 32
        p.north = True
        p.ellps = carla.GeoEllipsoid()
        p.offset = t

        self.assertIsNotNone(p.offset)
        self.assertEqual(p.offset, t)

    def test_geo_projection_utm_offset_setter(self):
        p = carla.GeoProjectionUTM()

        offset = carla.GeoOffsetTransform(5.0, 6.0, 7.0, 0.1)
        p.offset = offset

        self.assertEqual(p.offset, offset)

        p.offset = None
        self.assertIsNone(p.offset)

    def test_geo_projection_utm_equality(self):
        t = carla.GeoOffsetTransform(1.0, 2.0, 3.0, 0.0)

        p1 = carla.GeoProjectionUTM()
        p1.zone = 31
        p1.north = True
        p1.ellps = carla.GeoEllipsoid()
        p1.offset = t

        p2 = carla.GeoProjectionUTM()
        p2.zone = 31
        p2.north = True
        p2.ellps = carla.GeoEllipsoid()
        p2.offset = None

        self.assertFalse(p1 == p2)

    def test_geo_projection_utm_constructor_3_args(self):
        p =  carla.GeoProjectionUTM(zone=31, north=True, ellps=WGS84)

        self.assertEqual(p.zone, 31)
        self.assertTrue(p.north)
        self.assertEqual(p.ellps, WGS84)
        self.assertIsNone(p.offset)

    def test_geo_projection_utm_constructor_3_args_positional(self):
        p = carla.GeoProjectionUTM(31, True, WGS84)

        self.assertEqual(p.zone, 31)
        self.assertTrue(p.north)
        self.assertEqual(p.ellps, WGS84)
        self.assertIsNone(p.offset)


class TestRightHandedBoundary(unittest.TestCase):
    """`to_right_handed()` / `from_right_handed()` on Location, Rotation, Transform.

    CARLA is left-handed (x forward, y right, z up); ROS / REP-103 "FLU" is
    right-handed (x forward, y **left**, z up).  The mapping, documented in
    ``Docs/coordinate_conventions.md``, is::

        location (x, y, z)        -> (x, -y, z)
        rotation (roll,pitch,yaw) -> (roll, -pitch, -yaw)   intrinsic ZYX
        matrix   M                -> S @ M @ S              S = diag(1,-1,1,1)

    numpy-free on purpose: the rest of this file is too.
    """

    PLACES = 4

    SAMPLES = [
        # (location, rotation) in CARLA's frame
        ((0.0, 0.0, 0.0), (0.0, 0.0, 0.0)),
        ((1.0, 2.0, 3.0), (20.0, 30.0, 10.0)),
        ((-4.5, 6.25, 0.5), (-15.0, 120.0, 40.0)),
        ((100.0, -50.0, 2.0), (0.0, -90.0, 0.0)),
        ((0.0, 7.0, -1.0), (35.0, 0.0, 0.0)),
        ((3.0, 3.0, 3.0), (0.0, 0.0, -25.0)),
        ((-9.0, 0.25, 11.0), (13.0, 47.0, -31.0)),
    ]

    @staticmethod
    def _transform(sample):
        (x, y, z), (pitch, yaw, roll) = sample
        return carla.Transform(
            carla.Location(x=x, y=y, z=z),
            carla.Rotation(pitch=pitch, yaw=yaw, roll=roll))

    def test_rotation_negates_pitch_and_yaw(self):
        # The reference case: CARLA (roll, pitch, yaw) = (10, 20, 30) is FLU
        # (10, -20, -30).  Note it is pitch and yaw that flip, not roll.
        flu = carla.Rotation(pitch=20.0, yaw=30.0, roll=10.0).to_right_handed()
        self.assertAlmostEqual(flu.roll, 10.0, self.PLACES)
        self.assertAlmostEqual(flu.pitch, -20.0, self.PLACES)
        self.assertAlmostEqual(flu.yaw, -30.0, self.PLACES)

    def test_rotation_round_trip(self):
        for _, (pitch, yaw, roll) in self.SAMPLES:
            rotation = carla.Rotation(pitch=pitch, yaw=yaw, roll=roll)
            back = carla.Rotation.from_right_handed(rotation.to_right_handed())
            self.assertAlmostEqual(back.pitch, pitch, self.PLACES)
            self.assertAlmostEqual(back.yaw, yaw, self.PLACES)
            self.assertAlmostEqual(back.roll, roll, self.PLACES)

    def test_location_mirrors_y(self):
        flu = carla.Location(10.0, 20.0, 30.0).to_right_handed()
        self.assertAlmostEqual(flu.x, 10.0, self.PLACES)
        self.assertAlmostEqual(flu.y, -20.0, self.PLACES)
        self.assertAlmostEqual(flu.z, 30.0, self.PLACES)

        back = carla.Location.from_right_handed(flu)
        self.assertAlmostEqual(back.x, 10.0, self.PLACES)
        self.assertAlmostEqual(back.y, 20.0, self.PLACES)
        self.assertAlmostEqual(back.z, 30.0, self.PLACES)

    def test_transform_round_trip(self):
        for sample in self.SAMPLES:
            transform = self._transform(sample)
            flu = transform.to_right_handed()
            self.assertAlmostEqual(flu.location.x, transform.location.x, self.PLACES)
            self.assertAlmostEqual(flu.location.y, -transform.location.y, self.PLACES)
            self.assertAlmostEqual(flu.location.z, transform.location.z, self.PLACES)
            self.assertAlmostEqual(flu.rotation.roll, transform.rotation.roll, self.PLACES)
            self.assertAlmostEqual(flu.rotation.pitch, -transform.rotation.pitch, self.PLACES)
            self.assertAlmostEqual(flu.rotation.yaw, -transform.rotation.yaw, self.PLACES)

            back = carla.Transform.from_right_handed(flu)
            self.assertAlmostEqual(back.location.x, transform.location.x, self.PLACES)
            self.assertAlmostEqual(back.location.y, transform.location.y, self.PLACES)
            self.assertAlmostEqual(back.location.z, transform.location.z, self.PLACES)
            self.assertAlmostEqual(back.rotation.pitch, transform.rotation.pitch, self.PLACES)
            self.assertAlmostEqual(back.rotation.yaw, transform.rotation.yaw, self.PLACES)
            self.assertAlmostEqual(back.rotation.roll, transform.rotation.roll, self.PLACES)

    def test_matrix_is_s_m_s(self):
        # S @ M @ S with S = diag(1, -1, 1, 1) negates every entry whose row
        # or column -- but not both -- is the mirrored Y index.
        for sample in self.SAMPLES:
            transform = self._transform(sample)
            m = transform.get_matrix()
            flu = transform.to_right_handed().get_matrix()
            for row in range(4):
                for col in range(4):
                    sign = -1.0 if (row == 1) != (col == 1) else 1.0
                    self.assertAlmostEqual(
                        flu[row][col], sign * m[row][col], self.PLACES,
                        msg=f"element ({row}, {col}) of {sample}")

    def test_matrix_is_a_proper_rotation(self):
        # det(R) == +1: a right-handed frame.  The left-handed CARLA matrix
        # has det == -1 when read as right-handed math.
        for sample in self.SAMPLES:
            m = self._transform(sample).to_right_handed().get_matrix()
            det = (m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
                   - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
                   + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]))
            self.assertAlmostEqual(det, 1.0, self.PLACES, msg=str(sample))

    def test_quaternion_matches_matrix(self):
        # The quaternion and the matrix of the same RightHandedTransform must
        # describe the same rotation, column for column.
        for sample in self.SAMPLES:
            flu = self._transform(sample).to_right_handed()
            m = flu.get_matrix()
            q = flu.get_quaternion()
            # Rotation matrix of a right-handed (x, y, z, w) quaternion.
            x, y, z, w = q.x, q.y, q.z, q.w
            expected = [
                [1 - 2 * (y * y + z * z), 2 * (x * y - z * w), 2 * (x * z + y * w)],
                [2 * (x * y + z * w), 1 - 2 * (x * x + z * z), 2 * (y * z - x * w)],
                [2 * (x * z - y * w), 2 * (y * z + x * w), 1 - 2 * (x * x + y * y)],
            ]
            for row in range(3):
                for col in range(3):
                    self.assertAlmostEqual(
                        m[row][col], expected[row][col], self.PLACES,
                        msg=f"element ({row}, {col}) of {sample}")

    def test_quaternion_is_unit_norm(self):
        for sample in self.SAMPLES:
            q = self._transform(sample).to_right_handed().get_quaternion()
            norm = math.sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w)
            self.assertAlmostEqual(norm, 1.0, self.PLACES)

    def test_quaternion_agrees_with_carla_rotation_quaternion(self):
        # carla.Quaternion has always been the right-handed one; the boundary
        # type must not disagree with it.
        for _, (pitch, yaw, roll) in self.SAMPLES:
            rotation = carla.Rotation(pitch=pitch, yaw=yaw, roll=roll)
            a = carla.Quaternion(rotation)
            b = carla.Quaternion(rotation.to_right_handed())
            for component in ("x", "y", "z", "w"):
                self.assertAlmostEqual(
                    getattr(a, component), getattr(b, component), self.PLACES)

    def test_right_handed_rotation_get_quaternion_round_trip(self):
        for _, (pitch, yaw, roll) in self.SAMPLES:
            flu = carla.Rotation(pitch=pitch, yaw=yaw, roll=roll).to_right_handed()
            back = flu.get_quaternion().to_right_handed_rotation()
            # Compare through the matrix-free identity: the same quaternion.
            again = carla.RightHandedRotation(
                roll=back.roll, pitch=back.pitch, yaw=back.yaw).get_quaternion()
            original = flu.get_quaternion()
            # Quaternions q and -q are the same rotation.
            dot = (original.x * again.x + original.y * again.y
                   + original.z * again.z + original.w * again.w)
            self.assertAlmostEqual(abs(dot), 1.0, self.PLACES)

    def test_engine_convention_is_untouched(self):
        # The adapters must not have moved the left-handed math: a camera at
        # pitch=+20 looks up, at roll=+25 its right side drops.
        forward = carla.Rotation(pitch=20.0).get_forward_vector()
        self.assertAlmostEqual(forward.z, math.sin(math.radians(20.0)), self.PLACES)
        right = carla.Rotation(roll=25.0).get_right_vector()
        self.assertAlmostEqual(right.z, -math.sin(math.radians(25.0)), self.PLACES)
