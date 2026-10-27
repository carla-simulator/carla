// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include "carla/StringUtil.h"
#include "carla/client/BlueprintLibrary.h"

#include <array>
#include <tuple>

namespace {

  using MatchCase = std::tuple<const char *, const char *, bool>;

  template <size_t Size>
  void CheckMatches(const std::array<MatchCase, Size> &cases) {
    for (const auto &[input, pattern, expected] : cases) {
      SCOPED_TRACE(testing::Message() << "input=" << input << " pattern=" << pattern);
      EXPECT_EQ(carla::StringUtil::Match(input, pattern), expected);
    }
  }

} // namespace

TEST(wildcard_matching, question_mark_requires_exactly_one_character) {
  CheckMatches(std::array<MatchCase, 7>{{
      {"car", "ca?", true}, {"car", "ca??", false}, {"ca", "ca?", false},
      {"cart", "ca??", true}, {"", "?", false}, {"c", "?", true},
      {"car", "ca?x", false}}});
}

TEST(wildcard_matching, bracket_members_and_ranges) {
  CheckMatches(std::array<MatchCase, 8>{{
      {"car", "[bc]ar", true}, {"tar", "[bc]ar", false},
      {"car", "[a-f]ar", true}, {"zar", "[a-f]ar", false},
      {"car2", "car[0-9]", true}, {"carx", "car[0-9]", false},
      {"-", "[-a]", true}, {"-", "[a-]", true}}});
}

TEST(wildcard_matching, negated_bracket_classes) {
  CheckMatches(std::array<MatchCase, 6>{{
      {"b", "[!a]", true}, {"a", "[!a]", false},
      {"z", "[!a-f]", true}, {"c", "[!a-f]", false},
      {"a", "[!]]", true}, {"]", "[!]]", false}}});
}

TEST(wildcard_matching, bracket_literals_and_unclosed_brackets) {
  CheckMatches(std::array<MatchCase, 5>{{
      {"[", "[[]", true}, {"]", "[]]", true},
      {"[car", "[car", true}, {"[", "[", true}, {"[]", "[]", true}}});
}

TEST(wildcard_matching, semicolon_is_a_literal) {
  CheckMatches(std::array<MatchCase, 3>{{
      {"truck", "car;truck", false}, {"car", "car;truck", false},
      {"car;truck", "car;truck", true}}});
}

TEST(wildcard_matching, extension_pattern_does_not_match_extensionless_names) {
  CheckMatches(std::array<MatchCase, 3>{{
      {"car", "*.*", false}, {"car.txt", "*.*", true}, {".car", "*.*", true}}});
}

TEST(wildcard_matching, empty_and_star_controls) {
  CheckMatches(std::array<MatchCase, 8>{{
      {"", "", true}, {"car", "", false}, {"", "*", true}, {"car", "*", true},
      {"vehicle.car", "vehicle.*", true}, {"car", "truck*", false},
      {"car", "**c**a*r**", true}, {"cab", "*a*c", false}}});
}

TEST(wildcard_matching, escaped_metacharacters_are_literal) {
  CheckMatches(std::array<MatchCase, 5>{{
      {"car?", R"(car\?)", true}, {"cars", R"(car\?)", false},
      {"car*", R"(car\*)", true}, {"[", R"(\[)", true}, {R"(a\b)", R"(a\\b)", true}}});
}

TEST(wildcard_matching, star_backtracks_across_character_classes) {
  CheckMatches(std::array<MatchCase, 5>{{
      {"vehicle.car2", "*.[ct]*[0-9]", true}, {"vehicle.van2", "*.[ct]*[0-9]", false},
      {"abc123tail", "a*[0-9]*tail", true}, {"abc123tailx", "a*[0-9]*tail", false},
      {"folder/file.png", "*.png", true}}});
}

TEST(wildcard_matching, existing_platform_case_policy_control) {
#ifdef _WIN32
  EXPECT_TRUE(carla::StringUtil::Match("VEHICLE.CAR", "vehicle.car"));
#else
  EXPECT_FALSE(carla::StringUtil::Match("VEHICLE.CAR", "vehicle.car"));
#endif
  EXPECT_TRUE(carla::StringUtil::Match("vehicle.car", "vehicle.car"));
}

TEST(wildcard_matching, blueprint_filter_uses_bracket_members) {
  carla::rpc::ActorDefinition car;
  car.id = "vehicle.car";
  carla::rpc::ActorDefinition truck;
  truck.id = "vehicle.truck";
  carla::rpc::ActorDefinition van;
  van.id = "vehicle.van";
  const carla::client::BlueprintLibrary library({car, truck, van});
  const auto selected = library.Filter("vehicle.[ct]*");
  ASSERT_EQ(selected->size(), 2u);
  EXPECT_NE(selected->Find("vehicle.car"), nullptr);
  EXPECT_NE(selected->Find("vehicle.truck"), nullptr);
  EXPECT_EQ(selected->Find("vehicle.van"), nullptr);
}

TEST(wildcard_matching, bracket_classes_retain_platform_case_policy) {
#ifdef _WIN32
  EXPECT_TRUE(carla::StringUtil::Match("VEHICLE.CAR", "VEHICLE.[a-z]*"));
#else
  EXPECT_FALSE(carla::StringUtil::Match("VEHICLE.CAR", "VEHICLE.[a-z]*"));
#endif
}

TEST(wildcard_matching, ordinary_literals_and_star_controls) {
  CheckMatches(std::array<MatchCase, 6>{{
      {"car", "car", true}, {"car", "truck", false},
      {"vehicle.car", "vehicle.*", true}, {"vehicle.truck", "vehicle.c*", false},
      {"car", "*", true}, {"car", "c*r", true}}});
}
