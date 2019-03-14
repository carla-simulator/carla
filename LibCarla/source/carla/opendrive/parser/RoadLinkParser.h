// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <vector>
#include <string>

namespace pugi {
  class xml_document;
} // namespace pugi

namespace carla {

namespace road {
  class MapBuilder;
} // namespace road

namespace opendrive {
namespace parser {

  using RoadId = int;
  using LaneId = int;

  struct Polynomial {
    double s;
    double a, b, c, d;
  };

  // struct LaneRoadMark {
  //   double s;
  //   std::string type;
  //   std::string weight;
  //   std::string material;
  //   std::string color;
  //   std::string lane_change;
  //   double width;
  //   double height;
  // };

  struct Lane {
    LaneId id;
    std::string type;
    bool level;
    LaneId predecessor;
    LaneId successor;
    // std::vector<Polynomial> width;
    // std::vector<Polynomial> borders;
    // std::vector<LaneRoadMark> road_marks;
  };

  struct LaneSection {
    double s;
    double a, b, c, d;
    std::vector<Lane> lanes;
  };

  struct RoadTypeSpeed {
    double s;
    std::string type;
    double max;
    std::string unit;
  };

  struct Road {
    RoadId id;
    std::string name;
    double length;
    RoadId junction_id;
    RoadId predecessor;
    RoadId successor;
    std::vector<RoadTypeSpeed> speed;
    std::vector<LaneSection> sections;
  };

  class RoadLinkParser {
  public:

    static void Parse(
        const pugi::xml_document &xml,
        carla::road::MapBuilder &map_builder);

  };

} // namespace parser
} // namespace opendrive
} // namespace carla
