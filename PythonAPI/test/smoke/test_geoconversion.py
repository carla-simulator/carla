import carla
import random
import math

from . import SmokeTest


class TestGeoLocationConversion(SmokeTest):

    def setUp(self):
        super().setUp()
        self.map = self.client.get_world().get_map()

    def _assert_location_close(self, a, b, tol=0.1):
        self.assertAlmostEqual(a.x, b.x, delta=tol)
        self.assertAlmostEqual(a.y, b.y, delta=tol)
        self.assertAlmostEqual(a.z, b.z, delta=tol)

    def _assert_geolocation_close(self, a, b, latlon_tol=0.01, alt_tol=1.0):
        # Latitudes must stay in range [-90, 90]
        if not (-90.0 <= a.latitude <= 90.0 and -90.0 <= b.latitude <= 90.0):
            raise ValueError(f"Latitude out of bounds: {a.latitude}, {b.latitude}")

        # Longitudes must stay in range [-180, 180]
        if not (-180.0 <= a.longitude <= 180.0 and -180.0 <= b.longitude <= 180.0):
            raise ValueError(f"Longitude out of bounds: {a.longitude}, {b.longitude}")

        lat_diff = abs(a.latitude - b.latitude)
        lon_diff = abs(a.longitude - b.longitude)
        alt_diff = abs(a.altitude - b.altitude)

        # Catch hemisphere flips and wrap-around errors
        if lat_diff > 90.0 or lon_diff > 180.0:
            raise AssertionError(
                f"Geo conversion failed: large discrepancy (lat diff = {lat_diff}, lon diff = {lon_diff})\n"
                f"Original: lat={a.latitude}, lon={a.longitude}\n"
                f"Back:     lat={b.latitude}, lon={b.longitude}"
            )

        self.assertLessEqual(lat_diff, latlon_tol, f"Latitude mismatch: {a.latitude} vs {b.latitude}")
        self.assertLessEqual(lon_diff, latlon_tol, f"Longitude mismatch: {a.longitude} vs {b.longitude}")
        self.assertLessEqual(alt_diff, alt_tol, f"Altitude mismatch: {a.altitude} vs {b.altitude}")

    def test_location_to_geo_and_back(self):
        print("TestGeoLocationConversion.test_location_to_geo_and_back")
        for _ in range(10):
            loc = carla.Location(
                x=random.uniform(-500, 500),
                y=random.uniform(-500, 500),
                z=random.uniform(0, 20))
            geo = self.map.transform_to_geolocation(loc)
            loc2 = self.map.geolocation_to_transform(geo)
            self._assert_location_close(loc, loc2)

    def test_geo_to_location_and_back(self):
        print("TestGeoLocationConversion.test_geo_to_location_and_back")

        geo_origin = self.map.transform_to_geolocation(carla.Location(0, 0, 0))
        lat0, lon0 = geo_origin.latitude, geo_origin.longitude

        test_geos = [
            carla.GeoLocation(lat0 + 0.5, lon0 + 0.5, 50.0),
            carla.GeoLocation(lat0 - 0.5, lon0 - 0.5, -10.0),
            carla.GeoLocation(lat0 + 1.0, lon0 + 1.0, 100.0),
        ]

        for geo in test_geos:
            loc = self.map.geolocation_to_transform(geo)
            geo2 = self.map.transform_to_geolocation(loc)
            self._assert_geolocation_close(geo, geo2)

    def test_relative_distance_preserved(self):
        print("TestGeoLocationConversion.test_relative_distance_preserved")
        loc1 = carla.Location(x=100, y=200, z=0)
        loc2 = carla.Location(x=110, y=210, z=0)

        geo1 = self.map.transform_to_geolocation(loc1)
        geo2 = self.map.transform_to_geolocation(loc2)

        geo_diff = math.sqrt(
            (geo2.latitude - geo1.latitude) ** 2 +
            (geo2.longitude - geo1.longitude) ** 2 +
            (geo2.altitude - geo1.altitude) ** 2
        )
        self.assertGreater(geo_diff, 0.0)

    def test_altitude_variation(self):
        print("TestGeoLocationConversion.test_altitude_variation")
        for z in [0.0, 50.0, 500.0, 1000.0, 3000.0]:
            loc = carla.Location(x=10.0, y=10.0, z=z)
            geo = self.map.transform_to_geolocation(loc)
            loc2 = self.map.geolocation_to_transform(geo)
            self.assertAlmostEqual(loc.z, loc2.z, delta=1.0)

    def test_zero_conversion(self):
        print("TestGeoLocationConversion.test_zero_conversion")
        origin = carla.Location(0.0, 0.0, 0.0)
        geo = self.map.transform_to_geolocation(origin)
        loc_back = self.map.geolocation_to_transform(geo)
        self._assert_location_close(origin, loc_back)
