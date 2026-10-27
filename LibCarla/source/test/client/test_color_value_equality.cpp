// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/MsgPack.h>
#include <carla/client/ActorAttribute.h>
#include <carla/sensor/data/Color.h>

#include <array>
#include <utility>

namespace {

  using carla::sensor::data::Color;

  carla::client::ActorAttributeValue RgbAttribute() {
    carla::rpc::ActorAttributeValue attribute;
    attribute.id = "color";
    attribute.type = carla::rpc::ActorAttributeType::RGBColor;
    attribute.value = "4,5,6";
    return carla::client::ActorAttributeValue(std::move(attribute));
  }

  class ColorAlphaEquality : public testing::TestWithParam<uint8_t> {};

  TEST_P(ColorAlphaEquality, different_alpha_distinguishes_rgba_values) {
    const Color transparent(4u, 5u, 6u, GetParam());
    const Color opaque(4u, 5u, 6u, 255u);
    EXPECT_FALSE(transparent == opaque);
    EXPECT_FALSE(opaque == transparent);
    EXPECT_TRUE(transparent != opaque);
    EXPECT_TRUE(opaque != transparent);
  }

  INSTANTIATE_TEST_SUITE_P(
      AlphaValues, ColorAlphaEquality, testing::Values(uint8_t{0u}, uint8_t{1u}, uint8_t{127u}, uint8_t{254u}));

  TEST(color_equality, default_transparent_and_explicit_opaque_black_differ) {
    const Color transparent;
    const Color opaque(0u, 0u, 0u);
    EXPECT_EQ(transparent.a, 0u);
    EXPECT_EQ(opaque.a, 255u);
    EXPECT_NE(transparent, opaque);
  }

  TEST(color_equality, rgb_channel_differences_are_preserved_control) {
    const Color original(4u, 5u, 6u, 127u);
    EXPECT_NE(original, Color(3u, 5u, 6u, 127u));
    EXPECT_NE(original, Color(4u, 3u, 6u, 127u));
    EXPECT_NE(original, Color(4u, 5u, 3u, 127u));
    EXPECT_EQ(original, Color(4u, 5u, 6u, 127u));
  }

  TEST(color_equality, copied_and_serialized_rgba_values_remain_equal_control) {
    const Color original(4u, 5u, 6u, 127u);
    const auto copy = original;
    const auto packed = carla::MsgPack::Pack(original);
    const auto restored = carla::MsgPack::UnPack<Color>(packed);
    EXPECT_EQ(original, copy);
    EXPECT_EQ(original, restored);
    EXPECT_EQ(restored.a, 127u);
    EXPECT_FALSE(original != restored);
  }

  TEST(color_equality, float_color_conversion_preserves_value_distinctions) {
    const Color first(4u, 5u, 6u, 0u);
    const Color second(4u, 5u, 6u, 255u);
    const carla::rpc::FloatColor first_float = first;
    const carla::rpc::FloatColor second_float = second;
    ASSERT_NE(first_float, second_float);
    EXPECT_EQ(first == second, first_float == second_float);
  }

  TEST(color_equality, rgb_actor_attribute_comparisons_ignore_alpha_control) {
    const auto attribute = RgbAttribute();
    for (const uint8_t alpha : std::array<uint8_t, 4u>{0u, 1u, 127u, 255u}) {
      EXPECT_TRUE(attribute == Color(4u, 5u, 6u, alpha));
      EXPECT_FALSE(attribute != Color(4u, 5u, 6u, alpha));
      EXPECT_FALSE(attribute == Color(3u, 5u, 6u, alpha));
    }
  }

  TEST(color_equality, rgb_actor_attribute_parsing_retains_opaque_alpha_control) {
    const auto attribute = RgbAttribute();
    const auto parsed = attribute.As<Color>();
    EXPECT_EQ(parsed.r, 4u);
    EXPECT_EQ(parsed.g, 5u);
    EXPECT_EQ(parsed.b, 6u);
    EXPECT_EQ(parsed.a, 255u);
  }

} // namespace
