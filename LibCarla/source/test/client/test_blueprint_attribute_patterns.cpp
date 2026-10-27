// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include "carla/client/BlueprintLibrary.h"

namespace {

  carla::rpc::ActorDefinition MakeDefinition(
      const std::string &id,
      const std::string &value,
      std::vector<std::string> recommended_values = {}) {
    carla::rpc::ActorAttribute attribute;
    attribute.id = "category";
    attribute.type = carla::rpc::ActorAttributeType::String;
    attribute.value = value;
    attribute.recommended_values = std::move(recommended_values);
    carla::rpc::ActorDefinition definition;
    definition.id = id;
    definition.attributes.push_back(std::move(attribute));
    return definition;
  }

  carla::client::BlueprintLibrary MakeLibrary() {
    auto missing = MakeDefinition("vehicle.no_category", "unused");
    missing.attributes.clear();
    return carla::client::BlueprintLibrary({
        MakeDefinition("vehicle.fixed", "car"),
        MakeDefinition("vehicle.options", "default", {"truck", "car"}),
        MakeDefinition("vehicle.other", "motorcycle"),
        MakeDefinition("vehicle.empty", ""),
        std::move(missing)});
  }

} // namespace

TEST(blueprint_attribute_patterns, star_matches_plain_and_recommended_values) {
  const auto result = MakeLibrary().FilterByAttribute("category", "ca*");
  ASSERT_EQ(result->size(), 2u);
  EXPECT_NE(result->Find("vehicle.fixed"), nullptr);
  EXPECT_NE(result->Find("vehicle.options"), nullptr);
}

TEST(blueprint_attribute_patterns, question_mark_matches_one_character) {
  const auto result = MakeLibrary().FilterByAttribute("category", "ca?");
  ASSERT_EQ(result->size(), 2u);
  EXPECT_NE(result->Find("vehicle.fixed"), nullptr);
  EXPECT_NE(result->Find("vehicle.options"), nullptr);
}

TEST(blueprint_attribute_patterns, all_values_include_empty_but_not_missing_attributes) {
  const auto result = MakeLibrary().FilterByAttribute("category", "*");
  EXPECT_EQ(result->size(), 4u);
  EXPECT_EQ(result->Find("vehicle.no_category"), nullptr);
}

TEST(blueprint_attribute_patterns, exact_values_keep_recommended_value_selection) {
  const auto result = MakeLibrary().FilterByAttribute("category", "car");
  ASSERT_EQ(result->size(), 2u);
  EXPECT_NE(result->Find("vehicle.fixed"), nullptr);
  EXPECT_NE(result->Find("vehicle.options"), nullptr);
  EXPECT_TRUE(MakeLibrary().FilterByAttribute("category", "default")->empty());
}

TEST(blueprint_attribute_patterns, nonmatching_pattern_does_not_select_other_values) {
  EXPECT_TRUE(MakeLibrary().FilterByAttribute("category", "bus*")->empty());
  EXPECT_TRUE(MakeLibrary().FilterByAttribute("category", "ca?x")->empty());
  EXPECT_TRUE(MakeLibrary().FilterByAttribute("absent", "*")->empty());
}

TEST(blueprint_attribute_patterns, empty_pattern_matches_only_empty_value) {
  const auto result = MakeLibrary().FilterByAttribute("category", "");
  ASSERT_EQ(result->size(), 1u);
  EXPECT_NE(result->Find("vehicle.empty"), nullptr);
}

TEST(blueprint_attribute_patterns, filter_does_not_modify_source_library) {
  const auto library = MakeLibrary();
  const auto result = library.FilterByAttribute("category", "ca*");
  ASSERT_EQ(result->size(), 2u);
  EXPECT_EQ(library.size(), 5u);
  EXPECT_EQ(library.at("vehicle.options").GetAttribute("category").GetValue(), "default");
}
