// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/opendrive/parser/RoadParser.h"

#include "carla/Logging.h"
#include "carla/StringUtil.h"
#include "carla/road/MapBuilder.h"
#include "carla/road/RoadTypes.h"

#include <third-party/pugixml/pugixml.hpp>

namespace carla {
namespace opendrive {
namespace parser {

  using RoadId = road::RoadId;
  using LaneId = road::LaneId;
  using JuncId = road::JuncId;

  struct Polynomial {
    double s;
    double a, b, c, d;
  };

  struct Lane {
    LaneId id;
    road::Lane::LaneType type;
    bool level;
    LaneId predecessor;
    LaneId successor;
  };

  struct LaneOffset {
    double s;
    double a, b, c, d;
  };

  struct LaneSection {
    double s;
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
    JuncId junction_id;
    RoadId predecessor;
    RoadId successor;
    std::vector<RoadTypeSpeed> speed;
    std::vector<LaneOffset> section_offsets;
    std::vector<LaneSection> sections;
  };

  static road::Lane::LaneType StringToLaneType(std::string &&str) {
    StringUtil::ToLower(str);
    if (str == "driving") {
      return road::Lane::LaneType::Driving;
    } else if (str == "stop") {
      return road::Lane::LaneType::Stop;
    } else if (str == "shoulder") {
      return road::Lane::LaneType::Shoulder;
    } else if (str == "biking") {
      return road::Lane::LaneType::Biking;
    } else if (str == "sidewalk") {
      return road::Lane::LaneType::Sidewalk;
    } else if (str == "border") {
      return road::Lane::LaneType::Border;
    } else if (str == "restricted") {
      return road::Lane::LaneType::Restricted;
    } else if (str == "parking") {
      return road::Lane::LaneType::Parking;
    } else if (str == "bidirectional") {
      return road::Lane::LaneType::Bidirectional;
    } else if (str == "median") {
      return road::Lane::LaneType::Median;
    } else if (str == "special1") {
      return road::Lane::LaneType::Special1;
    } else if (str == "special2") {
      return road::Lane::LaneType::Special2;
    } else if (str == "special3") {
      return road::Lane::LaneType::Special3;
    } else if (str == "roadworks") {
      return road::Lane::LaneType::RoadWorks;
    } else if (str == "tram") {
      return road::Lane::LaneType::Tram;
    } else if (str == "rail") {
      return road::Lane::LaneType::Rail;
    } else if (str == "entry") {
      return road::Lane::LaneType::Entry;
    } else if (str == "exit") {
      return road::Lane::LaneType::Exit;
    } else if (str == "offramp") {
      return road::Lane::LaneType::OffRamp;
    } else if (str == "onramp") {
      return road::Lane::LaneType::OnRamp;
    } else {
      return road::Lane::LaneType::None;
    }
  }

  void RoadParser::Parse(
      const pugi::xml_document &xml,
      carla::road::MapBuilder &map_builder) {

    std::vector<Road> roads;

    for (pugi::xml_node node_road : xml.child("OpenDRIVE").children("road")) {
      Road road { 0, "", 0.0, -1, 0, 0, {}, {}, {} };

      // attributes
      road.id = node_road.attribute("id").as_uint();
      road.name = node_road.attribute("name").value();
      road.length = node_road.attribute("length").as_double();
      road.junction_id = node_road.attribute("junction").as_int();

      // link
      pugi::xml_node link = node_road.child("link");
      if (link) {
        if (link.child("predecessor")) {
          road.predecessor = link.child("predecessor").attribute("elementId").as_uint();
        }
        if (link.child("successor")) {
          road.successor = link.child("successor").attribute("elementId").as_uint();
        }
      }

      // types
      for (pugi::xml_node node_type : node_road.children("type")) {
        RoadTypeSpeed type { 0.0, "", 0.0, "" };

        type.s = node_type.attribute("s").as_double();
        type.type = node_type.attribute("type").value();

        // speed type
        pugi::xml_node speed = node_type.child("speed");
        if (speed) {
          type.max = speed.attribute("max").as_double();
          type.unit = speed.attribute("unit").value();
        }

        // add it
        road.speed.emplace_back(type);
      }

      // section offsets
      for (pugi::xml_node node_offset : node_road.child("lanes").children("laneOffset")) {
        LaneOffset offset { 0.0, 0.0, 0.0, 0.0, 0.0 };
        offset.s = node_offset.attribute("s").as_double();
        offset.a = node_offset.attribute("a").as_double();
        offset.b = node_offset.attribute("b").as_double();
        offset.c = node_offset.attribute("c").as_double();
        offset.d = node_offset.attribute("d").as_double();
        road.section_offsets.emplace_back(offset);
      }
      // Add default lane offset if none is found
      if(road.section_offsets.size() == 0) {
        LaneOffset offset { 0.0, 0.0, 0.0, 0.0, 0.0 };
        road.section_offsets.emplace_back(offset);
      }

      // lane sections
      for (pugi::xml_node node_section : node_road.child("lanes").children("laneSection")) {
        LaneSection section { 0.0, {} };

        section.s = node_section.attribute("s").as_double();

        // left lanes
        for (pugi::xml_node node_lane : node_section.child("left").children("lane")) {
          Lane lane { 0, road::Lane::LaneType::None, false, 0, 0 };

          lane.id = node_lane.attribute("id").as_int();
          lane.type = StringToLaneType(node_lane.attribute("type").value());
          lane.level = node_lane.attribute("level").as_bool();

          // link
          pugi::xml_node link2 = node_lane.child("link");
          if (link2) {
            if (link2.child("predecessor")) {
              lane.predecessor = link2.child("predecessor").attribute("id").as_int();
            }
            if (link2.child("successor")) {
              lane.successor = link2.child("successor").attribute("id").as_int();
            }
          }

          // add it
          section.lanes.emplace_back(lane);
        }

        // center lane
        for (pugi::xml_node node_lane : node_section.child("center").children("lane")) {
          Lane lane { 0, road::Lane::LaneType::None, false, 0, 0 };

          lane.id = node_lane.attribute("id").as_int();
          lane.type = StringToLaneType(node_lane.attribute("type").value());
          lane.level = node_lane.attribute("level").as_bool();

          // link (probably it never exists)
          pugi::xml_node link2 = node_lane.child("link");
          if (link2) {
            if (link2.child("predecessor")) {
              lane.predecessor = link2.child("predecessor").attribute("id").as_int();
            }
            if (link2.child("successor")) {
              lane.successor = link2.child("successor").attribute("id").as_int();
            }
          }

          // add it
          section.lanes.emplace_back(lane);
        }

        // right lane
        for (pugi::xml_node node_lane : node_section.child("right").children("lane")) {
          Lane lane { 0, road::Lane::LaneType::None, false, 0, 0 };

          lane.id = node_lane.attribute("id").as_int();
          lane.type = StringToLaneType(node_lane.attribute("type").value());
          lane.level = node_lane.attribute("level").as_bool();

          // link
          pugi::xml_node link2 = node_lane.child("link");
          if (link2) {
            if (link2.child("predecessor")) {
              lane.predecessor = link2.child("predecessor").attribute("id").as_int();
            }
            if (link2.child("successor")) {
              lane.successor = link2.child("successor").attribute("id").as_int();
            }
          }

          // add it
          section.lanes.emplace_back(lane);
        }

        // add section
        road.sections.emplace_back(section);
      }

      // add road
      roads.emplace_back(road);
    }

    // test print
    /*
       printf("Roads: %d\n", roads.size());
       for (const auto& r : roads) {
       printf("Road: %d\n", r.id);
       printf("  Name: %s\n", r.name.c_str());
       printf("  Length: %e\n", r.length);
       printf("  JunctionId: %d\n", r.junction_id);
       printf("  Predecessor: %d\n", r.predecessor);
       printf("  Successor: %d\n", r.successor);
       printf("  Speed: %d\n", r.speed.size());
       for (const auto& s : r.speed) {
        printf("    S offset: %e\n", s.s);
        printf("    Type: %s\n", s.type.c_str());
        printf("    Max: %e\n", s.max);
        printf("    Unit: %s\n", s.unit.c_str());
       }
       printf("LaneSections: %d\n", r.sections.size());
       for (const auto& s : r.sections) {
        printf("    S offset: %e\n", s.s);
        printf("    a,b,c,d: %e,%e,%e,%e\n", s.a, s.b, s.c, s.d);
        printf("    Lanes: %d\n", s.lanes.size());
        for (const auto& l : s.lanes) {
          printf("      Id: %d\n", l.id);
          printf("      Type: %s\n", l.type.c_str());
          printf("      Level: %d\n", l.level);
          printf("      Predecessor: %d\n", l.predecessor);
          printf("      Successor: %d\n", l.successor);
        }
       }
       }
     */

    // map_builder calls
    for (const auto& r : roads) {
      carla::road::Road *road = map_builder.AddRoad(r.id,
          r.name,
          r.length,
          r.junction_id,
          r.predecessor,
          r.successor);

      // type speed
      for (const auto& s : r.speed) {
        map_builder.CreateRoadSpeed(road, s.s, s.type, s.max, s.unit);
      }

      // section offsets
      for (const auto& s : r.section_offsets) {
        map_builder.CreateSectionOffset(road, s.s, s.a, s.b, s.c, s.d);
      }

      // lane sections
      road::SectionId i = 0;
      for (const auto& s : r.sections) {
        carla::road::LaneSection *section = map_builder.AddRoadSection(road, i++, s.s);

        // lanes
        for (const auto& l : s.lanes) {
          /*carla::road::Lane *lane = */ map_builder.AddRoadSectionLane(section, l.id,
              static_cast<uint32_t>(l.type), l.level, l.predecessor, l.successor);
        }
      }
    }
  }

} // namespace parser
} // namespace opendrive
} // namespace carla
