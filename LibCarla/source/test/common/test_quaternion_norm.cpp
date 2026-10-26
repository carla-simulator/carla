// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB). Licensed under the MIT license.

#include "test.h"

#include <carla/geom/Quaternion.h>

#include <cmath>
#include <limits>

using carla::geom::Quaternion;

TEST(quaternion_norm, finite_representable_large_and_small_lengths) {
  for (const float value : {1e20f, 1e-30f, std::numeric_limits<float>::max(),
                           std::numeric_limits<float>::denorm_min()}) {
    EXPECT_FLOAT_EQ(Quaternion(value, 0.0f, 0.0f, 0.0f).Length(), value);
  }
}

TEST(quaternion_norm, normalization_avoids_squared_norm_overflow_and_underflow) {
  for (const float scale : {1e20f, 1e-30f}) {
    const auto unit = Quaternion(scale, -2.0f * scale, 2.0f * scale, 0.0f).UnitQuaternion();
    EXPECT_NEAR(unit.x, 1.0f / 3.0f, 1e-6f);
    EXPECT_NEAR(unit.y, -2.0f / 3.0f, 1e-6f);
    EXPECT_NEAR(unit.z, 2.0f / 3.0f, 1e-6f);
    EXPECT_FLOAT_EQ(unit.w, 0.0f);
    EXPECT_NEAR(unit.Length(), 1.0f, 1e-6f);
  }
}

TEST(quaternion_norm, normalization_does_not_require_float_representable_length) {
  const float maximum = std::numeric_limits<float>::max();
  const auto unit = Quaternion(maximum, maximum, maximum, maximum).UnitQuaternion();
  EXPECT_FLOAT_EQ(unit.x, 0.5f);
  EXPECT_FLOAT_EQ(unit.y, 0.5f);
  EXPECT_FLOAT_EQ(unit.z, 0.5f);
  EXPECT_FLOAT_EQ(unit.w, 0.5f);
  const auto tiny = Quaternion(std::numeric_limits<float>::denorm_min(), 0.0f, 0.0f, 0.0f).UnitQuaternion();
  EXPECT_EQ(tiny, Quaternion(1.0f, 0.0f, 0.0f, 0.0f));
}

TEST(quaternion_norm, representable_extreme_inverse_preserves_identity_product) {
  for (const float scale : {1e20f, 1e-30f, 1e-38f, -1e20f, -1e-30f, -1e-38f}) {
    const Quaternion q(scale, 0.0f, 0.0f, 0.0f);
    const auto inverse = q.Inverse();
    EXPECT_TRUE(std::isfinite(inverse.x));
    EXPECT_NEAR(inverse.x * scale, -1.0f, 1e-6f);
    const auto identity = q * inverse;
    EXPECT_FLOAT_EQ(identity.x, 0.0f);
    EXPECT_FLOAT_EQ(identity.y, 0.0f);
    EXPECT_FLOAT_EQ(identity.z, 0.0f);
    EXPECT_NEAR(identity.w, 1.0f, 1e-6f);
  }
}

TEST(quaternion_norm, all_components_preserve_representable_extreme_results) {
  for (const float scale : {std::numeric_limits<float>::max() / 4.0f, 1e-38f}) {
    const Quaternion q(scale, -scale, scale, -scale);
    EXPECT_FLOAT_EQ(q.Length(), 2.0f * scale);
    EXPECT_EQ(q.UnitQuaternion(), Quaternion(0.5f, -0.5f, 0.5f, -0.5f));
    const auto inverse = q.Inverse();
    EXPECT_TRUE(std::isfinite(inverse.x));
    EXPECT_TRUE(std::isfinite(inverse.y));
    EXPECT_TRUE(std::isfinite(inverse.z));
    EXPECT_TRUE(std::isfinite(inverse.w));
    const auto identity = q * inverse;
    EXPECT_NEAR(identity.x, 0.0f, 1e-6f);
    EXPECT_NEAR(identity.y, 0.0f, 1e-6f);
    EXPECT_NEAR(identity.z, 0.0f, 1e-6f);
    EXPECT_NEAR(identity.w, 1.0f, 1e-6f);
  }
}

TEST(quaternion_norm, ordinary_and_zero_controls) {
  const Quaternion ordinary(1.0f, -2.0f, 2.0f, 0.0f);
  EXPECT_FLOAT_EQ(ordinary.Length(), 3.0f);
  EXPECT_NEAR(ordinary.UnitQuaternion().Length(), 1.0f, 1e-6f);
  EXPECT_NEAR((ordinary * ordinary.Inverse()).w, 1.0f, 1e-6f);
  const Quaternion zero(0.0f, 0.0f, 0.0f, 0.0f);
  EXPECT_FLOAT_EQ(zero.Length(), 0.0f);
  EXPECT_EQ(zero.UnitQuaternion(), Quaternion());
  EXPECT_EQ(zero.Inverse(), Quaternion());
}
