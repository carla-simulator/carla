# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import random
import time

import carla

from . import SmokeTest

# The packaged build only ships Town10HD_Opt and Mine_01; the default
# SmokeTest.tearDown loads Town03, which does not exist on the package.
TEST_TOWN = "Town10HD_Opt"


class TestGeoConversion(SmokeTest):

    def setUp(self):
        super().setUp()
        self.client.load_world(TEST_TOWN)
        time.sleep(5)
        self.world = self.client.get_world()
        self.map = self.world.get_map()

    def tearDown(self):
        self.client.load_world(TEST_TOWN)
        time.sleep(5)
        self.world = None
        self.client = None

    def _assert_location_close(self, a, b, tol_m=0.1):
        self.assertAlmostEqual(a.x, b.x, delta=tol_m)
        self.assertAlmostEqual(a.y, b.y, delta=tol_m)
        self.assertAlmostEqual(a.z, b.z, delta=tol_m)

    def test_location_round_trip(self):
        print("TestGeoConversion.test_location_round_trip")
        rng = random.Random(42)
        for _ in range(10):
            loc = carla.Location(
                x=rng.uniform(-500.0, 500.0),
                y=rng.uniform(-500.0, 500.0),
                z=rng.uniform(0.0, 20.0),
            )
            geo = self.map.transform_to_geolocation(loc)
            back = self.map.geolocation_to_transform(geo)
            self._assert_location_close(loc, back, tol_m=0.1)

    def test_geo_round_trip(self):
        print("TestGeoConversion.test_geo_round_trip")
        # Anchor at the map's origin geolocation so the offsets remain
        # close enough to stay numerically stable through Mercator.
        origin = self.map.transform_to_geolocation(carla.Location(0.0, 0.0, 0.0))
        test_geos = [
            carla.GeoLocation(origin.latitude + 0.001, origin.longitude + 0.001, 50.0),
            carla.GeoLocation(origin.latitude - 0.001, origin.longitude - 0.001, -10.0),
            carla.GeoLocation(origin.latitude + 0.002, origin.longitude + 0.002, 100.0),
        ]
        for geo in test_geos:
            loc = self.map.geolocation_to_transform(geo)
            geo_back = self.map.transform_to_geolocation(loc)
            self.assertAlmostEqual(geo.latitude, geo_back.latitude, delta=1e-4)
            self.assertAlmostEqual(geo.longitude, geo_back.longitude, delta=1e-4)
            self.assertAlmostEqual(geo.altitude, geo_back.altitude, delta=1.0)

    def test_zero_conversion(self):
        print("TestGeoConversion.test_zero_conversion")
        origin = carla.Location(0.0, 0.0, 0.0)
        geo = self.map.transform_to_geolocation(origin)
        back = self.map.geolocation_to_transform(geo)
        self._assert_location_close(origin, back, tol_m=0.001)

    def test_relative_offset_preserved(self):
        print("TestGeoConversion.test_relative_offset_preserved")
        # An XY offset of 10 m round-tripped through the geo conversion
        # must come back as the same 10 m offset.
        a = carla.Location(x=100.0, y=200.0, z=0.0)
        b = carla.Location(x=110.0, y=210.0, z=0.0)

        geo_a = self.map.transform_to_geolocation(a)
        geo_b = self.map.transform_to_geolocation(b)
        back_a = self.map.geolocation_to_transform(geo_a)
        back_b = self.map.geolocation_to_transform(geo_b)

        self.assertAlmostEqual(back_b.x - back_a.x, b.x - a.x, delta=0.01)
        self.assertAlmostEqual(back_b.y - back_a.y, b.y - a.y, delta=0.01)
