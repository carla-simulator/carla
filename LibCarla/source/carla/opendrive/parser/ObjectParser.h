// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <pugixml/pugixml.hpp>

namespace pugi {
  class xml_document;
} // namespace pugi

namespace carla {

namespace road {
  class MapBuilder;
} // namespace road

namespace opendrive {
namespace parser {

  class ObjectParser {
  public:

    static void Parse(
        const pugi::xml_document &xml,
        carla::road::MapBuilder &map_builder);

  private:
    std::string get_string_attribute(const pugi::xml_node& node, const char* attribute_name);
    double get_double_attribute(const pugi::xml_node& node, const char* attribute_name);

  };

} // namespace parser
} // namespace opendrive
} // namespace carla
