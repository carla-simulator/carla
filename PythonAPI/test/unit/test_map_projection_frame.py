# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Check default and explicit Map projection frames against independent coordinates."""

import math
import unittest

import carla


# Static reference values use pyproj 3.8.0 / PROJ 9.8.1.
# WGS84 is used for TM, UTM and LCC. Web Mercator uses a sphere with radius 6378137 m.
# Every offset heading is zero. Both conversion directions use absolute inputs.
CASES = [{'name': 'tm_asymmetric_false_origin',
  'family': 'GeoProjectionTM',
  'arguments': [52.5, 13.4, 0.9996, 1234.5, -6789.25],
  'ellipsoid': 'wgs84',
  'proj': '+proj=tmerc +lat_0=52.5 +lon_0=13.4 +k=0.9996 +x_0=1234.5 +y_0=-6789.25 +ellps=WGS84 +units=m',
  'geographic': [52.5015, 13.4, 17.25],
  'offset': None,
  'world_tolerance': [0.001, 0.001, 1e-05],
  'geographic_tolerance': [1e-08, 1e-08, 1e-05],
  'world': [1234.5, 6622.401495397568, 17.25]},
 {'name': 'utm_north_absent_offset',
  'family': 'GeoProjectionUTM',
  'arguments': [31, True],
  'ellipsoid': 'wgs84',
  'proj': '+proj=utm +zone=31 +ellps=WGS84 +units=m',
  'geographic': [0.0015, 3.0, 17.25],
  'offset': None,
  'world_tolerance': [0.001, 0.001, 1e-05],
  'geographic_tolerance': [1e-09, 1e-09, 1e-05],
  'world': [500000.000000001, -165.79506916727894, 17.25]},
 {'name': 'utm_north_zero_offset',
  'family': 'GeoProjectionUTM',
  'arguments': [31, True],
  'ellipsoid': 'wgs84',
  'proj': '+proj=utm +zone=31 +ellps=WGS84 +units=m',
  'geographic': [0.0015, 3.0, 17.25],
  'offset': [0.0, 0.0, 0.0],
  'world_tolerance': [0.001, 0.001, 1e-05],
  'geographic_tolerance': [1e-09, 1e-09, 1e-05],
  'world': [500000.000000001, -165.79506916727894, 17.25]},
 {'name': 'utm_north_xyz_offset',
  'family': 'GeoProjectionUTM',
  'arguments': [32, True],
  'ellipsoid': 'wgs84',
  'proj': '+proj=utm +zone=32 +ellps=WGS84 +units=m',
  'geographic': [40.65235651557158, 9.0, 117.25],
  'offset': [500000.0, 4500000.0, 100.0],
  'world_tolerance': [0.01, 0.01, 1e-05],
  'geographic_tolerance': [1e-07, 1e-07, 1e-05],
  'world': [0.0, -166.50414152722806, 17.25]},
 {'name': 'utm_south_absent_offset',
  'family': 'GeoProjectionUTM',
  'arguments': [56, False],
  'ellipsoid': 'wgs84',
  'proj': '+proj=utm +zone=56 +south +ellps=WGS84 +units=m',
  'geographic': [-32.9985, 153.0, 17.25],
  'offset': None,
  'world_tolerance': [0.5, 0.5, 1e-05],
  'geographic_tolerance': [5e-06, 1e-07, 1e-05],
  'world': [500000.0, -6348879.34616326, 17.25]},
 {'name': 'webmercator_spherical',
  'family': 'GeoProjectionWebMerc',
  'arguments': [],
  'ellipsoid': 'sphere',
  'proj': '+proj=merc +a=6378137 +b=6378137 +units=m',
  'geographic': [0.3015, -0.7, 17.25],
  'offset': None,
  'world_tolerance': [0.02, 0.02, 1e-05],
  'geographic_tolerance': [1e-07, 1e-07, 1e-05],
  'world': [-77923.64355529149, -33562.98136997545, 17.25]},
 {'name': 'lcc_north_false_origin',
  'family': 'GeoProjectionLCC2SP',
  'arguments': [48.0, 44.0, 49.0, 3.0, 1250.0, -2340.0],
  'ellipsoid': 'wgs84',
  'proj': '+proj=lcc +lat_0=48 +lat_1=44 +lat_2=49 +lon_0=3 +x_0=1250 +y_0=-2340 +ellps=WGS84 +units=m',
  'geographic': [48.0015, 3.0, 17.25],
  'offset': None,
  'world_tolerance': [0.02, 0.02, 1e-05],
  'geographic_tolerance': [1e-07, 1e-07, 1e-05],
  'world': [1250.0, 2173.316719869088, 17.25]},
 {'name': 'lcc_south_false_origin',
  'family': 'GeoProjectionLCC2SP',
  'arguments': [-35.0, -30.0, -40.0, 149.0, -321.25, 1234.75],
  'ellipsoid': 'wgs84',
  'proj': '+proj=lcc +lat_0=-35 +lat_1=-30 +lat_2=-40 +lon_0=149 +x_0=-321.25 +y_0=1234.75 +ellps=WGS84 '
          '+units=m',
  'geographic': [-34.9985, 149.0, 17.25],
  'offset': None,
  'world_tolerance': [0.02, 0.02, 1e-05],
  'geographic_tolerance': [1e-07, 1e-07, 1e-05],
  'world': [-321.25, -1400.530247594064, 17.25]}]


def make_map(case):
    offset_xml = ""
    if case["offset"] is not None:
        x, y, z = case["offset"]
        offset_xml = f'<offset x="{x}" y="{y}" z="{z}" hdg="0"/>'
    xodr = f'''<OpenDRIVE>
      <header revMajor="1" revMinor="4">
        <geoReference>{case["proj"]}</geoReference>
        {offset_xml}
      </header>
      <road id="1" junction="-1" length="40">
        <planView><geometry s="0" x="0" y="0" hdg="0" length="40">
          <line/>
        </geometry></planView>
        <lanes><laneSection s="0"><center><lane id="0" type="none"/>
          </center><right><lane id="-1" type="driving">
            <width sOffset="0" a="3.5" b="0" c="0" d="0"/>
          </lane></right></laneSection></lanes>
      </road>
    </OpenDRIVE>'''
    return carla.Map("frame_probe_" + case["name"], xodr)


class TestMapProjectionFrame(unittest.TestCase):
    def _assert_coordinates(self, actual, expected, tolerances):
        self.assertTrue(
            all(math.isclose(value, reference, rel_tol=0.0, abs_tol=tolerance)
                for value, reference, tolerance in zip(actual, expected, tolerances)),
            msg=f"actual={actual}, expected={expected}, absolute_tolerances={tolerances}",
        )

    def _check_case(self, case):
        ellipsoid = (carla.GeoEllipsoid(6378137.0) if case["ellipsoid"] == "sphere"
                     else carla.GeoEllipsoid(6378137.0, 298.257223563))
        projection = getattr(carla, case["family"])(*case["arguments"], ellipsoid)
        if case["offset"] is not None:
            projection.offset = carla.GeoOffsetTransform(*case["offset"], 0.0)
        map_value = make_map(case)
        geographic = carla.GeoLocation(*case["geographic"])
        world = carla.Location(*case["world"])
        for path, parameters in [("default_parsed", ()), ("explicit_projection", (projection,))]:
            with self.subTest(case=case["name"], path=path, direction="geographic_to_world"):
                actual = map_value.geolocation_to_transform(geographic, *parameters)
                self._assert_coordinates(
                    [actual.x, actual.y, actual.z], case["world"], case["world_tolerance"])
            with self.subTest(case=case["name"], path=path, direction="world_to_geographic"):
                actual = map_value.transform_to_geolocation(world, *parameters)
                self._assert_coordinates(
                    [actual.latitude, actual.longitude, actual.altitude],
                    case["geographic"], case["geographic_tolerance"])

    def test_tm_asymmetric_false_origin(self):
        self._check_case(CASES[0])

    def test_utm_north_absent_offset(self):
        self._check_case(CASES[1])

    def test_utm_north_zero_offset(self):
        self._check_case(CASES[2])

    def test_utm_north_xyz_offset(self):
        self._check_case(CASES[3])

    def test_utm_south_absent_offset(self):
        self._check_case(CASES[4])

    def test_webmercator_spherical(self):
        self._check_case(CASES[5])

    def test_lcc_north_false_origin(self):
        self._check_case(CASES[6])

    def test_lcc_south_false_origin(self):
        self._check_case(CASES[7])
