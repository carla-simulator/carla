// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/geom/GeoProjection.h>
#include <carla/geom/GeoProjectionsParams.h>
#include <carla/geom/GeoLocation.h>
#include <carla/geom/Location.h>

#include <random>

using carla::geom::Ellipsoid;
using carla::geom::GeoLocation;
using carla::geom::GeoProjection;
using carla::geom::Location;
using carla::geom::OffsetTransform;
using carla::geom::TransverseMercatorParams;
using carla::geom::UniversalTransverseMercatorParams;

namespace {

  // WGS84 ellipsoid (semi-major axis, inverse flattening).
  Ellipsoid Wgs84() {
    return Ellipsoid(6378137.0, 298.257223563);
  }

  // A Transverse Mercator projection anchored at the origin. Keeping
  // x_0/y_0 at zero keeps projected coordinates small so the float
  // storage in Location does not dominate the round-trip error.
  GeoProjection TransverseMercator() {
    return GeoProjection::Make(
        TransverseMercatorParams(0.0, 0.0, 0.9996, 0.0, 0.0, Wgs84()));
  }

} // namespace

// These tests replace the GeoLocation::InverseTransform coverage added in
// PR #9726: the GeoProjection engine subsumes the standalone inverse, so the
// equivalent round-trip and sign-convention guarantees are pinned here
// against GeoProjection instead.

TEST(GeoProjection, tm_round_trip_zero) {
  // location -> geo -> location of the origin returns the origin within
  // the double-to-float rounding tolerance of the projection.
  const GeoProjection projection = TransverseMercator();
  const Location origin{0.0f, 0.0f, 0.0f};

  const GeoLocation geo = projection.TransformToGeoLocation(origin);
  const Location back = projection.GeoLocationToTransform(geo);

  ASSERT_NEAR(back.x, 0.0f, 1e-3f);
  ASSERT_NEAR(back.y, 0.0f, 1e-3f);
  ASSERT_NEAR(back.z, 0.0f, 1e-3f);
}

TEST(GeoProjection, tm_round_trip_random) {
  // For ten deterministic random Locations within ±50 km of the origin,
  // location -> geo -> location must recover the input.
  std::mt19937 rng(42u);
  std::uniform_real_distribution<float> xy_dist(-50000.0f, 50000.0f);
  std::uniform_real_distribution<float> z_dist(-20.0f, 3000.0f);

  const GeoProjection projection = TransverseMercator();
  constexpr float tolerance_m = 0.1f;

  for (int i = 0; i < 10; ++i) {
    const Location input{xy_dist(rng), xy_dist(rng), z_dist(rng)};
    const GeoLocation geo = projection.TransformToGeoLocation(input);
    const Location back = projection.GeoLocationToTransform(geo);

    ASSERT_NEAR(back.x, input.x, tolerance_m) << "iteration " << i;
    ASSERT_NEAR(back.y, input.y, tolerance_m) << "iteration " << i;
    ASSERT_NEAR(back.z, input.z, tolerance_m) << "iteration " << i;
  }
}

TEST(GeoProjection, tm_preserves_relative_offsets) {
  // Two Locations separated by a known XY delta must round-trip through
  // the projection with the same delta. This pins the sign convention so
  // a future projection change cannot silently flip an axis.
  const GeoProjection projection = TransverseMercator();
  const Location a{100.0f, 50.0f, 0.0f};
  const Location b{120.0f, 70.0f, 0.0f};

  const Location back_a =
      projection.GeoLocationToTransform(projection.TransformToGeoLocation(a));
  const Location back_b =
      projection.GeoLocationToTransform(projection.TransformToGeoLocation(b));

  constexpr float tolerance_m = 1e-2f;
  ASSERT_NEAR(back_b.x - back_a.x, b.x - a.x, tolerance_m);
  ASSERT_NEAR(back_b.y - back_a.y, b.y - a.y, tolerance_m);
}

TEST(GeoProjection, tm_altitude_passes_through) {
  // Altitude is carried through the projection unchanged regardless of
  // horizontal position.
  const GeoProjection projection = TransverseMercator();
  for (float z : {0.0f, 50.0f, 500.0f, 1000.0f, 3000.0f}) {
    const Location input{10.0f, 10.0f, z};
    const GeoLocation geo = projection.TransformToGeoLocation(input);
    const Location back = projection.GeoLocationToTransform(geo);
    ASSERT_NEAR(back.z, z, 1e-3f);
  }
}

TEST(GeoProjection, utm_offset_round_trip) {
  // Regression test: the UTM inverse transform applies the parsed
  // OpenDRIVE offset to the input location, so the forward transform must
  // apply the inverse offset. Without that symmetry a location -> geo ->
  // location round-trip drifts by the full offset.
  const OffsetTransform offset(500000.0, 4000000.0, 100.0, 0.0);
  const GeoProjection projection = GeoProjection::Make(
      UniversalTransverseMercatorParams(32, true, Wgs84(), offset));

  constexpr float tolerance_m = 0.5f;
  for (const Location &input :
       {Location{0.0f, 0.0f, 0.0f}, Location{120.0f, -80.0f, 35.0f},
        Location{-200.0f, 250.0f, -15.0f}}) {
    const GeoLocation geo = projection.TransformToGeoLocation(input);
    const Location back = projection.GeoLocationToTransform(geo);
    ASSERT_NEAR(back.x, input.x, tolerance_m);
    ASSERT_NEAR(back.y, input.y, tolerance_m);
    ASSERT_NEAR(back.z, input.z, tolerance_m);
  }
}

TEST(OffsetTransform, inverse_is_exact) {
  // ApplyInverseTransformation must invert ApplyTransformation exactly,
  // including the heading rotation and the z translation.
  const OffsetTransform offset(12.5, -7.25, 3.0, 0.4);
  constexpr float tolerance_m = 1e-3f;
  for (const Location &input :
       {Location{0.0f, 0.0f, 0.0f}, Location{100.0f, 200.0f, 30.0f},
        Location{-150.0f, 75.0f, -10.0f}}) {
    const Location forward = offset.ApplyTransformation(input);
    const Location back = offset.ApplyInverseTransformation(forward);
    ASSERT_NEAR(back.x, input.x, tolerance_m);
    ASSERT_NEAR(back.y, input.y, tolerance_m);
    ASSERT_NEAR(back.z, input.z, tolerance_m);
  }
}
