// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include "carla/client/ActorAttribute.h"

#include <limits>
#include <locale>

namespace {

  carla::client::ActorAttribute MakeAttribute(
      carla::rpc::ActorAttributeType type,
      std::string value,
      bool modifiable = true) {
    carla::rpc::ActorAttribute attribute;
    attribute.id = "test_attribute";
    attribute.type = type;
    attribute.value = std::move(value);
    attribute.is_modifiable = modifiable;
    return carla::client::ActorAttribute(std::move(attribute));
  }

  class CommaDecimal : public std::numpunct<char> {
  protected:
    char do_decimal_point() const override {
      return ',';
    }
  };

  class ScopedCommaLocale {
  public:
    ScopedCommaLocale()
      : _previous(std::locale()) {
      std::locale::global(std::locale(std::locale::classic(), new CommaDecimal));
    }

    ~ScopedCommaLocale() {
      std::locale::global(_previous);
    }

  private:
    std::locale _previous;
  };

} // namespace

#ifndef LIBCARLA_NO_EXCEPTIONS
TEST(actor_attribute_validation, rejects_invalid_integer_text) {
  using Type = carla::rpc::ActorAttributeType;
  for (const auto *value : {"", "hello", "12pixels", "1.5"}) {
    EXPECT_THROW(MakeAttribute(Type::Int, value),
                 carla::client::InvalidAttributeValue) << value;
  }
}

TEST(actor_attribute_validation, rejects_integer_overflow) {
  using Type = carla::rpc::ActorAttributeType;
  EXPECT_THROW(MakeAttribute(Type::Int, "99999999999999999999999999999"),
               carla::client::InvalidAttributeValue);
  EXPECT_THROW(MakeAttribute(Type::Int, "-99999999999999999999999999999"),
               carla::client::InvalidAttributeValue);
}

TEST(actor_attribute_validation, rejects_invalid_float_text) {
  using Type = carla::rpc::ActorAttributeType;
  for (const auto *value : {"", "hello", "1.25meters", "nan"}) {
    EXPECT_THROW(MakeAttribute(Type::Float, value),
                 carla::client::InvalidAttributeValue) << value;
  }
}

TEST(actor_attribute_validation, rejects_invalid_rgb_channels) {
  using Type = carla::rpc::ActorAttributeType;
  for (const auto *value : {"-1,0,0", "0,word,0", "0,0,12x", "0,,0"}) {
    EXPECT_THROW(MakeAttribute(Type::RGBColor, value),
                 carla::client::InvalidAttributeValue) << value;
  }
}

TEST(actor_attribute_validation, rejected_boolean_update_preserves_value) {
  auto attribute = MakeAttribute(carla::rpc::ActorAttributeType::Bool, "true");
  EXPECT_THROW(attribute.Set("not-a-bool"),
               carla::client::InvalidAttributeValue);
  EXPECT_EQ(attribute.GetValue(), "true");
  EXPECT_TRUE(attribute.As<bool>());
  attribute.Set("FALSE");
  EXPECT_FALSE(attribute.As<bool>());
}

TEST(actor_attribute_validation, rejected_rgb_update_preserves_value) {
  auto attribute = MakeAttribute(carla::rpc::ActorAttributeType::RGBColor, "1,2,3");
  EXPECT_THROW(attribute.Set("1,2"), carla::client::InvalidAttributeValue);
  EXPECT_EQ(attribute.GetValue(), "1,2,3");
  const auto color = attribute.As<carla::sensor::data::Color>();
  EXPECT_EQ(color.r, 1u);
  EXPECT_EQ(color.g, 2u);
  EXPECT_EQ(color.b, 3u);
}

TEST(actor_attribute_validation, readonly_control) {
  auto attribute = MakeAttribute(carla::rpc::ActorAttributeType::Int, "42", false);
  EXPECT_THROW(attribute.Set("43"), carla::client::InvalidAttributeValue);
  EXPECT_EQ(attribute.As<int>(), 42);
}
#endif // LIBCARLA_NO_EXCEPTIONS

TEST(actor_attribute_validation, integer_boundaries_and_whitespace_control) {
  using Type = carla::rpc::ActorAttributeType;
  EXPECT_EQ(MakeAttribute(Type::Int, "  +42 \t").As<int>(), 42);
  EXPECT_EQ(MakeAttribute(Type::Int, std::to_string((std::numeric_limits<int>::min)())).As<int>(),
            (std::numeric_limits<int>::min)());
  EXPECT_EQ(MakeAttribute(Type::Int, std::to_string((std::numeric_limits<int>::max)())).As<int>(),
            (std::numeric_limits<int>::max)());
}

TEST(actor_attribute_validation, float_and_global_locale_control) {
  const ScopedCommaLocale locale;
  using Type = carla::rpc::ActorAttributeType;
  EXPECT_FLOAT_EQ(MakeAttribute(Type::Float, " +1.25e1 \t").As<float>(), 12.5f);
  EXPECT_FLOAT_EQ(MakeAttribute(Type::Float, "-0.5").As<float>(), -0.5f);
}

TEST(actor_attribute_validation, valid_color_string_and_updates_control) {
  using Type = carla::rpc::ActorAttributeType;
  const auto color = MakeAttribute(Type::RGBColor, "0, 255, +42").As<carla::sensor::data::Color>();
  EXPECT_EQ(color.r, 0u);
  EXPECT_EQ(color.g, 255u);
  EXPECT_EQ(color.b, 42u);
  EXPECT_EQ(MakeAttribute(Type::String, "arbitrary text").As<std::string>(), "arbitrary text");
  auto attribute = MakeAttribute(Type::Int, "1");
  attribute.Set("2");
  EXPECT_EQ(attribute.As<int>(), 2);
}
