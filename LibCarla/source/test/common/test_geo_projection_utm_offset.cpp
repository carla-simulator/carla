// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include <carla/geom/GeoLocation.h>
#include <carla/geom/GeoProjection.h>
#include <carla/geom/Location.h>

#include <gtest/gtest.h>

using namespace carla::geom;

TEST(GeoProjection, utm_without_offset_matches_absolute_world_coordinates) {
  const auto projection = GeoProjection::Make(UniversalTransverseMercatorParams(
      31, true, Ellipsoid(6378137.0, 298.257223563)));
  // Independent WGS84 coordinates from pyproj 3.8.0 / PROJ 9.8.1.
  const GeoLocation geographic{0.0015, 3.0, 17.25};
  const Location expected{500000.0f, -165.79506916727894f, 17.25f};
  const auto world = projection.GeoLocationToTransform(geographic);
  EXPECT_NEAR(world.x, expected.x, 0.001);
  EXPECT_NEAR(world.y, expected.y, 0.001);
  EXPECT_NEAR(world.z, expected.z, 1e-5);
  const auto back = projection.TransformToGeoLocation(expected);
  EXPECT_NEAR(back.latitude, geographic.latitude, 1e-9);
  EXPECT_NEAR(back.longitude, geographic.longitude, 1e-9);
  EXPECT_NEAR(back.altitude, geographic.altitude, 1e-5);
}

TEST(GeoProjection, utm_zero_offset_preserves_absolute_world_coordinates) {
  const Ellipsoid wgs84(6378137.0, 298.257223563);
  const auto absent = GeoProjection::Make(UniversalTransverseMercatorParams(31, true, wgs84));
  const auto zero = GeoProjection::Make(UniversalTransverseMercatorParams(
      31, true, wgs84, OffsetTransform(0.0, 0.0, 0.0, 0.0)));
  const GeoLocation geographic{0.0015, 3.0, 17.25};
  const Location expected{500000.0f, -165.79506916727894f, 17.25f};
  const auto world = zero.GeoLocationToTransform(geographic);
  EXPECT_EQ(world, absent.GeoLocationToTransform(geographic));
  EXPECT_NEAR(world.x, expected.x, 0.001);
  EXPECT_NEAR(world.y, expected.y, 0.001);
  EXPECT_NEAR(world.z, expected.z, 1e-5);
  const auto back = zero.TransformToGeoLocation(expected);
  EXPECT_EQ(back, absent.TransformToGeoLocation(expected));
  EXPECT_NEAR(back.latitude, geographic.latitude, 1e-9);
  EXPECT_NEAR(back.longitude, geographic.longitude, 1e-9);
  EXPECT_NEAR(back.altitude, geographic.altitude, 1e-5);
}

TEST(GeoProjection, utm_xyz_offset_preserves_absolute_world_coordinates) {
  const auto projection = GeoProjection::Make(UniversalTransverseMercatorParams(
      32, true, Ellipsoid(6378137.0, 298.257223563),
      OffsetTransform(500000.0, 4500000.0, 100.0, 0.0)));
  const GeoLocation geographic{40.65235651557158, 9.0, 117.25};
  const Location expected{0.0f, -166.50414152722806f, 17.25f};
  const auto world = projection.GeoLocationToTransform(geographic);
  // UTM northing is rounded to float before the offset is removed.
  EXPECT_NEAR(world.x, expected.x, 0.01);
  EXPECT_NEAR(world.y, expected.y, 0.01);
  EXPECT_NEAR(world.z, expected.z, 1e-5);
  const auto back = projection.TransformToGeoLocation(expected);
  EXPECT_NEAR(back.latitude, geographic.latitude, 1e-7);
  EXPECT_NEAR(back.longitude, geographic.longitude, 1e-7);
  EXPECT_NEAR(back.altitude, geographic.altitude, 1e-5);
}
