// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include "carla/geom/GeoLocation.h"
#include "carla/geom/GeoProjection.h"

namespace {

  constexpr double QuarterTurn = carla::geom::Math::Pi<double>() / 2.0;

  void ExpectLocation(const carla::geom::Location &actual, float x, float y, float z) {
    EXPECT_NEAR(actual.x, x, 1e-4f);
    EXPECT_NEAR(actual.y, y, 1e-4f);
    EXPECT_NEAR(actual.z, z, 1e-4f);
  }

  carla::geom::GeoProjection OffsetUtm() {
    return carla::geom::GeoProjection::Make(carla::geom::UniversalTransverseMercatorParams(
        32, true, carla::geom::Ellipsoid(6378137.0, 298.257223563),
        carla::geom::OffsetTransform(500000.0, 0.0, 5.0, QuarterTurn)));
  }

} // namespace

TEST(georeference_offset_heading, translated_origin_stays_at_world_offset) {
  const carla::geom::OffsetTransform offset(100.0, 200.0, 5.0, QuarterTurn);
  ExpectLocation(offset.ApplyTransformation({0.0f, 0.0f, 0.0f}), 100.0f, 200.0f, 5.0f);
}

TEST(georeference_offset_heading, positive_heading_rotates_local_axes_before_translation) {
  const carla::geom::OffsetTransform offset(100.0, 200.0, 5.0, QuarterTurn);
  // CARLA(10,-20) is ODR(10,20); R(+pi/2) gives(-20,10).
  ExpectLocation(offset.ApplyTransformation({10.0f, -20.0f, 3.0f}), 80.0f, 210.0f, 8.0f);
}

TEST(georeference_offset_heading, negative_heading_uses_the_opposite_rotation) {
  const carla::geom::OffsetTransform offset(100.0, 200.0, 5.0, -QuarterTurn);
  ExpectLocation(offset.ApplyTransformation({10.0f, -20.0f, 3.0f}), 120.0f, 190.0f, 8.0f);
}

TEST(georeference_offset_heading, rotation_without_translation_uses_odr_heading) {
  const carla::geom::OffsetTransform offset(0.0, 0.0, 0.0, QuarterTurn);
  ExpectLocation(offset.ApplyTransformation({10.0f, -20.0f, 3.0f}), -20.0f, 10.0f, 3.0f);
}

TEST(georeference_offset_heading, inverse_recovers_a_known_world_position) {
  const carla::geom::OffsetTransform offset(100.0, 200.0, 5.0, QuarterTurn);
  ExpectLocation(offset.ApplyInverseTransformation({80.0f, 210.0f, 8.0f}), 10.0f, -20.0f, 3.0f);
}

TEST(georeference_offset_heading, zero_heading_preserves_existing_translation_and_handedness) {
  const carla::geom::OffsetTransform offset(100.0, 200.0, 5.0, 0.0);
  ExpectLocation(offset.ApplyTransformation({10.0f, -20.0f, 3.0f}), 110.0f, 220.0f, 8.0f);
  ExpectLocation(offset.ApplyInverseTransformation({110.0f, 220.0f, 8.0f}), 10.0f, -20.0f, 3.0f);
}

TEST(georeference_offset_heading, projected_geo_origin_observes_rotated_offset) {
  const auto geo = OffsetUtm().TransformToGeoLocation({0.0f, 0.0f, 10.0f});
  EXPECT_NEAR(geo.latitude, 0.0, 1e-8);
  EXPECT_NEAR(geo.longitude, 9.0, 1e-8);
  EXPECT_NEAR(geo.altitude, 15.0, 1e-8);
}

TEST(georeference_offset_heading, geographic_origin_maps_to_local_offset_origin) {
  ExpectLocation(OffsetUtm().GeoLocationToTransform({0.0, 9.0, 15.0}), 0.0f, 0.0f, 10.0f);
}
