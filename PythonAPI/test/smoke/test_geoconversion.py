import carla
import numpy as np

from . import SmokeTest

WGS84 = carla.GeoEllipsoid(a=6378137.0, f_inv=298.257223563)
SPHERE = carla.GeoEllipsoid(a=6378137.0, f_inv=float("inf"))

TM = carla.GeoProjectionTM(
    lat_0=0.0, lon_0=3.0, k=0.9996, x_0=500000.0, y_0=0.0, ellps=WGS84)
UTMNORTH = carla.GeoProjectionUTM(zone=31, north=True, ellps=WGS84)
UTMSOUTH = carla.GeoProjectionUTM(zone=31, north=False, ellps=WGS84)
WEBM = carla.GeoProjectionWebMerc(ellps=SPHERE)
LCC = carla.GeoProjectionLCC2SP(
    lon_0=3.0, lat_0=46.5, lat_1=44.0, lat_2=49.0, x_0=700000.0, y_0=6600000.0, ellps=WGS84)

AMOUNT_STEPS = 10

class TestGeoLocationConversion(SmokeTest):

    def setUp(self):
        super().setUp()
        self.map = self.client.get_world().get_map()

    def _assert_location_close(self, a, b, tol=0.1):
        self.assertAlmostEqual(a.x, b.x, delta=tol)
        self.assertAlmostEqual(a.y, b.y, delta=tol)
        self.assertAlmostEqual(a.z, b.z, delta=tol)

    def _assert_geolocation_close(self, a, b, latlon_tol=0.00001, alt_tol=0.01):
        # Latitudes must stay in range [-90, 90]
        if not (-90.0 <= a.latitude <= 90.0 and -90.0 <= b.latitude <= 90.0):
            raise ValueError(f"Latitude out of bounds: {a.latitude}, {b.latitude}")

        # Longitudes must stay in range [-180, 180]
        if not (-180.0 <= a.longitude <= 180.0 and -180.0 <= b.longitude <= 180.0):
            raise ValueError(f"Longitude out of bounds: {a.longitude}, {b.longitude}")

        lon_diff = (a.longitude - b.longitude + 180.0) % 360.0 - 180.0
        lat_diff = abs(a.latitude - b.latitude)
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

    def test_geo_reference(self):
        print("TestGeoLocationConversion.test_location_to_geo_and_back")
        geo_reference = self.map.get_georeference()
        geo_reference_2 = self.map.transform_to_geolocation(carla.Location())
        self._assert_geolocation_close(geo_reference, geo_reference_2)

    def test_geo_projection(self):
        print("TestGeoLocationConversion.test_location_to_geo_and_back")
        geo_projection = self.map.get_geoprojection()
        assert isinstance(geo_projection, (carla.GeoProjectionTM, carla.GeoProjectionUTM,
                                           carla.GeoProjectionWebMerc, carla.GeoProjectionLCC2SP))

    def test_location_to_geo_and_back(self):
        print("TestGeoLocationConversion.test_location_to_geo_and_back")
        x_list = np.linspace(-10000, 10000, AMOUNT_STEPS).tolist()
        y_list = np.linspace(-10000, 10000, AMOUNT_STEPS).tolist()
        z_list = np.linspace(-10, 10, AMOUNT_STEPS).tolist()

        for i in range(AMOUNT_STEPS):
            loc = carla.Location(x=x_list[i], y=y_list[i], z=z_list[i])
            geo = self.map.transform_to_geolocation(loc)
            loc2 = self.map.geolocation_to_transform(geo)
            self._assert_location_close(loc, loc2)

    def test_geo_to_location_and_back(self):
        print("TestGeoLocationConversion.test_geo_to_location_and_back")
        origin_geo = self.map.get_georeference()
        lat_0, lon_0 = origin_geo.latitude, origin_geo.longitude

        lat_list = np.linspace(-0.5, 0.5, AMOUNT_STEPS).tolist()
        lon_list = np.linspace(-0.5, 0.5, AMOUNT_STEPS).tolist()
        alt_list = np.linspace(-50, 3000, AMOUNT_STEPS).tolist()

        for i in range(AMOUNT_STEPS):
            geo = carla.GeoLocation(
                latitude=lat_0 + lat_list[i],
                longitude=lon_0 + lon_list[i],
                altitude=alt_list[i])
            loc = self.map.geolocation_to_transform(geo)
            geo2 = self.map.transform_to_geolocation(loc)
            self._assert_geolocation_close(geo, geo2)

    def test_tm_location_to_geo_and_back(self):
        print("TestGeoLocationConversion.test_tm_location_to_geo_and_back")
        x_list = np.linspace(300000, 700000, AMOUNT_STEPS).tolist()
        y_list = np.linspace(-2000000, 2000000, AMOUNT_STEPS).tolist()
        z_list = np.linspace(-10, 10, AMOUNT_STEPS).tolist()

        for i in range(AMOUNT_STEPS):
            loc = carla.Location(x=x_list[i], y=y_list[i], z=z_list[i])
            geo = self.map.transform_to_geolocation(loc, TM)
            loc2 = self.map.geolocation_to_transform(geo, TM)
            self._assert_location_close(loc, loc2)

    def test_tm_geo_to_location_and_back(self):
        print("TestGeoLocationConversion.test_tm_geo_to_location_and_back")
        lat_list = np.linspace(-75, 75, AMOUNT_STEPS).tolist()
        lon_list = np.linspace(0.1, 5.9, AMOUNT_STEPS).tolist()
        alt_list = np.linspace(-50, 3000, AMOUNT_STEPS).tolist()

        for i in range(AMOUNT_STEPS):
            geo = carla.GeoLocation(latitude=lat_list[i], longitude=lon_list[i], altitude=alt_list[i])
            loc = self.map.geolocation_to_transform(geo, TM)
            geo2 = self.map.transform_to_geolocation(loc, TM)
            self._assert_geolocation_close(geo, geo2)

    def test_utm_north_location_to_geo_and_back(self):
        print("TestGeoLocationConversion.test_utm_location_to_geo_and_back")
        x_list = np.linspace(300000, 700000, AMOUNT_STEPS).tolist()
        y_list = np.linspace(-200000, 200000, AMOUNT_STEPS).tolist()
        z_list = np.linspace(-10, 10, AMOUNT_STEPS).tolist()

        for i in range(AMOUNT_STEPS):
            loc = carla.Location(x=x_list[i], y=y_list[i], z=z_list[i])
            geo = self.map.transform_to_geolocation(loc, UTMNORTH)
            loc2 = self.map.geolocation_to_transform(geo, UTMNORTH)
            self._assert_location_close(loc, loc2)

    def test_utm_north_geo_to_location_and_back(self):
        print("TestGeoLocationConversion.test_utm_geo_to_location_and_back")
        lat_list = np.linspace(0.1, 70, AMOUNT_STEPS).tolist()
        lon_list = np.linspace(2.1, 3.9, AMOUNT_STEPS).tolist()
        alt_list = np.linspace(-50, 3000, AMOUNT_STEPS).tolist()

        for i in range(AMOUNT_STEPS):
            geo = carla.GeoLocation(latitude=lat_list[i], longitude=lon_list[i], altitude=alt_list[i])
            loc = self.map.geolocation_to_transform(geo, UTMNORTH)
            geo2 = self.map.transform_to_geolocation(loc, UTMNORTH)
            self._assert_geolocation_close(geo, geo2)

    def test_utm_south_location_to_geo_and_back(self):
        print("TestGeoLocationConversion.test_utm_location_to_geo_and_back")
        x_list = np.linspace(300000, 700000, AMOUNT_STEPS).tolist()
        y_list = np.linspace(8000000, 12000000, AMOUNT_STEPS).tolist()
        z_list = np.linspace(-10, 10, AMOUNT_STEPS).tolist()

        for i in range(AMOUNT_STEPS):
            loc = carla.Location(x=x_list[i], y=y_list[i], z=z_list[i])
            geo = self.map.transform_to_geolocation(loc, UTMSOUTH)
            loc2 = self.map.geolocation_to_transform(geo, UTMSOUTH)
            self._assert_location_close(loc, loc2)

    def test_utm_south_geo_to_location_and_back(self):
        print("TestGeoLocationConversion.test_utm_geo_to_location_and_back")
        lat_list = np.linspace(0.1, 70, AMOUNT_STEPS).tolist()
        lon_list = np.linspace(0.1, 5.9, AMOUNT_STEPS).tolist()
        alt_list = np.linspace(-50, 3000, AMOUNT_STEPS).tolist()

        for i in range(AMOUNT_STEPS):
            geo = carla.GeoLocation(latitude=lat_list[i], longitude=lon_list[i], altitude=alt_list[i])
            loc = self.map.geolocation_to_transform(geo, UTMSOUTH)
            geo2 = self.map.transform_to_geolocation(loc, UTMSOUTH)
            self._assert_geolocation_close(geo, geo2)

    def test_webmerc_location_to_geo_and_back(self):
        print("TestGeoLocationConversion.test_webmerc_location_to_geo_and_back")
        x_list = np.linspace(-20000000, 20000000, AMOUNT_STEPS).tolist()
        y_list = np.linspace(-20000000, 20000000, AMOUNT_STEPS).tolist()
        z_list = np.linspace(-10, 10, AMOUNT_STEPS).tolist()

        for i in range(AMOUNT_STEPS):
            loc = carla.Location(x=x_list[i], y=y_list[i], z=z_list[i])
            geo = self.map.transform_to_geolocation(loc, WEBM)
            loc2 = self.map.geolocation_to_transform(geo, WEBM)
            self._assert_location_close(loc, loc2)

    def test_webmerc_geo_to_location_and_back(self):
        print("TestGeoLocationConversion.test_webmerc_geo_to_location_and_back")
        lat_list = np.linspace(-80, 80, AMOUNT_STEPS).tolist()
        lon_list = np.linspace(-170, 170, AMOUNT_STEPS).tolist()
        alt_list = np.linspace(-50, 3000, AMOUNT_STEPS).tolist()

        for i in range(AMOUNT_STEPS):
            geo = carla.GeoLocation(latitude=lat_list[i], longitude=lon_list[i], altitude=alt_list[i])
            loc = self.map.geolocation_to_transform(geo, WEBM)
            geo2 = self.map.transform_to_geolocation(loc, WEBM)
            self._assert_geolocation_close(geo, geo2)

    def test_lcc_location_to_geo_and_back(self):
        print("TestGeoLocationConversion.test_lcc_location_to_geo_and_back")
        x_list = np.linspace(200000, 1200000, AMOUNT_STEPS).tolist()
        y_list = np.linspace(5000000, 8000000, AMOUNT_STEPS).tolist()
        z_list = np.linspace(-10, 10, AMOUNT_STEPS).tolist()

        for i in range(AMOUNT_STEPS):
            loc = carla.Location(x=x_list[i], y=y_list[i], z=z_list[i])
            geo = self.map.transform_to_geolocation(loc, LCC)
            loc2 = self.map.geolocation_to_transform(geo, LCC)
            self._assert_location_close(loc, loc2)

    def test_lcc_geo_to_location_and_back(self):
        print("TestGeoLocationConversion.test_lcc_geo_to_location_and_back")
        lat_list = np.linspace(42, 52, AMOUNT_STEPS).tolist()
        lon_list = np.linspace(-5, 10, AMOUNT_STEPS).tolist()
        alt_list = np.linspace(-50, 3000, AMOUNT_STEPS).tolist()

        for i in range(AMOUNT_STEPS):
            geo = carla.GeoLocation(latitude=lat_list[i], longitude=lon_list[i], altitude=alt_list[i])
            loc = self.map.geolocation_to_transform(geo, LCC)
            geo2 = self.map.transform_to_geolocation(loc, LCC)
            self._assert_geolocation_close(geo, geo2)
