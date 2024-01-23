// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

namespace pugi {
  class xml_document;
} // namespace pugi

namespace carla {

namespace road {
  class MapBuilder;
} // namespace road

namespace opendrive {
namespace parser {

  class GeoReferenceParser {
  public:

    static void Parse(
        const pugi::xml_document &xml,
        carla::road::MapBuilder &map_builder);
  };

} // namespace parser
} // namespace opendrive
} // namespace carla
