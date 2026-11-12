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
#include <limits>

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

TEST(GeoProjection, tm_north_is_minus_y) {
  // CARLA locations are left-handed: north is -y and east is +x, as in the
  // OpenDRIVE inertial frame with y negated. Pins the absolute orientation,
  // which the round trips above cannot see.
  const GeoProjection projection = TransverseMercator();
  const GeoLocation origin = projection.TransformToGeoLocation(Location{0.0f, 0.0f, 0.0f});
  const GeoLocation north = projection.TransformToGeoLocation(Location{0.0f, -100.0f, 0.0f});
  const GeoLocation east = projection.TransformToGeoLocation(Location{100.0f, 0.0f, 0.0f});
  ASSERT_GT(north.latitude, origin.latitude);
  ASSERT_NEAR(north.longitude, origin.longitude, 1e-9);
  ASSERT_GT(east.longitude, origin.longitude);
  ASSERT_NEAR(east.latitude, origin.latitude, 1e-9);
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

namespace {

  struct ProjectionReference {
    double latitude, longitude;
    double easting, carla_y;
    float input_x, input_y;
    double inverse_latitude, inverse_longitude;
  };

  void CheckReference(const GeoProjection &projection, const ProjectionReference &reference) {
    const auto forward = projection.GeoLocationToTransform(
        GeoLocation(reference.latitude, reference.longitude, 25.0));
    EXPECT_NEAR(forward.x, reference.easting, 0.5);
    EXPECT_NEAR(forward.y, reference.carla_y, 0.5);
    EXPECT_FLOAT_EQ(forward.z, 25.0f);

    const auto inverse = projection.TransformToGeoLocation(
        Location(reference.input_x, reference.input_y, -12.0f));
    EXPECT_NEAR(inverse.latitude, reference.inverse_latitude, 1e-7);
    EXPECT_NEAR(inverse.longitude, reference.inverse_longitude, 1e-7);
    EXPECT_DOUBLE_EQ(inverse.altitude, -12.0);
  }

} // namespace

// Absolute values from PROJ 9.8.1 through pyproj 3.8.0, with the projected
// northing negated for CARLA. The inverse inputs are independent of the forward
// outputs. Tolerances include float Location storage and the existing series
// truncation; no PROJ dependency is needed to run these tests.
TEST(GeoProjection, tm_absolute_reference_values) {
  const auto ellipsoid = GeoProjection::Make(
      TransverseMercatorParams(40.0, -3.0, 0.9996, 1000.0, -5000.0, Wgs84()));
  for (const auto &reference : {
       ProjectionReference{41.25, -5.5, -208466.1815950938, -136766.9518588526,
           -140000.0f, -150000.0f, 41.38399198993458, -4.68633886928503},
       ProjectionReference{35.5, -0.5, 227756.3730288867, 501389.9219361335,
           170000.0f, 400000.0f, 36.4251548608718, -1.1148111277280819}}) {
    CheckReference(ellipsoid, reference);
  }

  const auto sphere = GeoProjection::Make(TransverseMercatorParams(
      -20.0, 179.0, 1.0, 12345.0, -23456.0,
      Ellipsoid(6370997.0, std::numeric_limits<double>::infinity())));
  for (const auto &reference : {
       ProjectionReference{-29.25, -178.5, 254927.9193250977, 1054595.5714658573,
           125000.0f, 850000.0f, -27.428643331970715, -179.8585967314688},
       ProjectionReference{-10.75, 176.5, -260844.2846590113, -1003984.3744574662,
           -135000.0f, -800000.0f, -12.591055991167458, 177.64235592521513}}) {
    CheckReference(sphere, reference);
  }
}

TEST(GeoProjection, utm_absolute_reference_values) {
  const auto north = GeoProjection::Make(UniversalTransverseMercatorParams(31, true, Wgs84()));
  for (const auto &reference : {
       ProjectionReference{25.0, 0.5, 247673.89996248946, -2767275.2071806956,
           350000.0f, -2500000.0f, 22.60022406288644, 1.5406959186871692},
       ProjectionReference{70.0, 5.5, 595404.9704624246, -7767829.407630525,
           580000.0f, -7500000.0f, 67.60463040686382, 4.881835645201831}}) {
    CheckReference(north, reference);
  }

  const auto south = GeoProjection::Make(UniversalTransverseMercatorParams(31, false, Wgs84()));
  for (const auto &reference : {
       ProjectionReference{-42.0, 0.5, 292948.961828621, -5347200.193424389,
           350000.0f, -4500000.0f, -49.63390362740616, 0.9226471068531007},
       ProjectionReference{-10.0, 5.5, 774071.0533869967, -8893548.721745655,
           650000.0f, -8500000.0f, -13.564602149676757, 4.386315841654348}}) {
    CheckReference(south, reference);
  }
}
