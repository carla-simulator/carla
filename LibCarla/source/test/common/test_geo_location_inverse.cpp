// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/geom/GeoLocation.h>
#include <carla/geom/Location.h>

#include <cmath>
#include <random>

using carla::geom::GeoLocation;
using carla::geom::Location;

TEST(GeoLocation, inverse_zero_round_trip) {
  // Forward-then-inverse of the origin Location returns the same
  // zero Location within Mercator double-to-float rounding tolerance.
  // The exact value is not zero because the Mercator helpers compute
  // mx/my in double precision and the final cast to float introduces a
  // ~1e-11 rounding term.
  const GeoLocation reference{48.0, 2.0, 100.0};
  const Location origin{0.0f, 0.0f, 0.0f};

  const GeoLocation geo = reference.Transform(origin);
  const Location back = reference.InverseTransform(geo);

  ASSERT_NEAR(back.x, 0.0f, 1e-6f);
  ASSERT_NEAR(back.y, 0.0f, 1e-6f);
  ASSERT_NEAR(back.z, 0.0f, 1e-6f);
}

TEST(GeoLocation, inverse_round_trip_random) {
  // For ten deterministic random Locations within ±500 m of the origin
  // (the working scale of CARLA maps), forward-then-inverse must
  // recover the input within sub-millimetre Mercator round-trip
  // tolerance.
  std::mt19937 rng(42u);
  std::uniform_real_distribution<float> xy_dist(-500.0f, 500.0f);
  std::uniform_real_distribution<float> z_dist(-20.0f, 200.0f);

  const GeoLocation reference{40.7128, -74.0060, 10.0};
  constexpr float tolerance_m = 1e-3f;

  for (int i = 0; i < 10; ++i) {
    const Location input{xy_dist(rng), xy_dist(rng), z_dist(rng)};
    const GeoLocation geo = reference.Transform(input);
    const Location back = reference.InverseTransform(geo);

    ASSERT_NEAR(back.x, input.x, tolerance_m) << "iteration " << i;
    ASSERT_NEAR(back.y, input.y, tolerance_m) << "iteration " << i;
    ASSERT_NEAR(back.z, input.z, tolerance_m) << "iteration " << i;
  }
}

TEST(GeoLocation, inverse_preserves_relative_offsets) {
  // Two Locations separated by a known XY delta must produce
  // GeoLocations whose inverse projection recovers the same delta. This
  // pins the sign convention: forward stores +x along east and -y along
  // north, so the inverse must mirror that.
  const GeoLocation reference{37.7749, -122.4194, 0.0};
  const Location a{100.0f, 50.0f, 0.0f};
  const Location b{120.0f, 70.0f, 0.0f};

  const GeoLocation geo_a = reference.Transform(a);
  const GeoLocation geo_b = reference.Transform(b);
  const Location back_a = reference.InverseTransform(geo_a);
  const Location back_b = reference.InverseTransform(geo_b);

  constexpr float tolerance_m = 1e-3f;
  ASSERT_NEAR(back_b.x - back_a.x, b.x - a.x, tolerance_m);
  ASSERT_NEAR(back_b.y - back_a.y, b.y - a.y, tolerance_m);
}

TEST(GeoLocation, inverse_altitude_passes_through) {
  // Altitude is unsigned offset from the reference altitude. Inverse
  // must return the original z regardless of horizontal position.
  const GeoLocation reference{0.0, 0.0, 250.0};
  for (float z : {0.0f, 50.0f, 500.0f, 1000.0f, 3000.0f}) {
    const Location input{10.0f, 10.0f, z};
    const GeoLocation geo = reference.Transform(input);
    const Location back = reference.InverseTransform(geo);
    ASSERT_NEAR(back.z, z, 1e-3f);
  }
}
