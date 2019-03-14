// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/opendrive/parser/RoadLinkParser.h"
#include "carla/opendrive/parser/pugixml/pugixml.hpp"
#include "carla/road/MapBuilder.h"
#include "carla/Logging.h"
#include <deque>

namespace carla {
namespace opendrive {
namespace parser {

  void RoadLinkParser::Parse(
      const pugi::xml_document &xml,
      carla::road::MapBuilder &map_builder) {

      std::vector<Road> roads;

      for (pugi::xml_node node_road : xml.child("OpenDRIVE").children("road")) {
        Road road { 0, "", 0.0, -1, -1, -1, {}, {} };

        // attributes
        road.id = node_road.attribute("id").as_int();
        road.name = node_road.attribute("name").value();
        road.length = node_road.attribute("length").as_double();
        road.junction_id = node_road.attribute("junction").as_int();

        // link
        pugi::xml_node link = node_road.child("link");
        if (link) {
          if (link.child("predecessor"))
            road.predecessor = link.child("predecessor").attribute("elementId").as_int();
          if (link.child("successor"))
            road.successor = link.child("successor").attribute("elementId").as_int();
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

        // Todo: section offsets
        std::deque<Polynomial> lane_offsets;
        for (pugi::xml_node node_offset : node_road.child("lanes").children("laneOffset")) {
          Polynomial offset { 0.0, 0.0, 0.0, 0.0, 0.0 } ;
          offset.s = node_offset.attribute("s").as_double();
          offset.a = node_offset.attribute("a").as_double();
          offset.b = node_offset.attribute("b").as_double();
          offset.c = node_offset.attribute("c").as_double();
          offset.d = node_offset.attribute("d").as_double();
          lane_offsets.emplace_back(offset);
        }

        // lane sections
        for (pugi::xml_node node_section : node_road.child("lanes").children("laneSection")) {
          LaneSection section { 0.0, 0.0, 0.0, 0.0, 0.0, {} };

          section.s = node_section.attribute("s").as_double();

          // section offsets
          section.a = lane_offsets[0].a;
          section.b = lane_offsets[0].b;
          section.c = lane_offsets[0].c;
          section.d = lane_offsets[0].d;
          lane_offsets.pop_front();

          // left lanes
          for (pugi::xml_node node_lane : node_section.child("left").children("lane")) {
            Lane lane { 0, "none", false, 0, 0 };

            lane.id = node_lane.attribute("id").as_int();
            lane.type = node_lane.attribute("type").value();
            lane.level = node_lane.attribute("level").as_bool();

            // link
            pugi::xml_node link2 = node_lane.child("link");
            if (link2) {
              if (link2.child("predecessor"))
                lane.predecessor = link2.child("predecessor").attribute("id").as_int();
              if (link2.child("successor"))
                lane.successor = link2.child("successor").attribute("id").as_int();
            }

            // add it
            section.lanes.emplace_back(lane);
          }

          // add center lane
          // section.lanes.emplace_back( { 0, 0, 0 });

          // right lane
          for (pugi::xml_node node_lane : node_section.child("right").children("lane")) {
            Lane lane { 0, "none", false, 0, 0 };

            lane.id = node_lane.attribute("id").as_int();
            lane.type = node_lane.attribute("type").value();
            lane.level = node_lane.attribute("level").as_bool();

            // link
            pugi::xml_node link2 = node_lane.child("link");
            if (link2) {
              if (link2.child("predecessor"))
                lane.predecessor = link2.child("predecessor").attribute("id").as_int();
              if (link2.child("successor"))
                lane.successor = link2.child("successor").attribute("id").as_int();
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
    for (auto const r : roads) {
      printf("Road: %d\n", r.id);
      printf("  Name: %s\n", r.name.c_str());
      printf("  Length: %e\n", r.length);
      printf("  JunctionId: %d\n", r.junction_id);
      printf("  Predecessor: %d\n", r.predecessor);
      printf("  Successor: %d\n", r.successor);
      printf("  Speed: %d\n", r.speed.size());
      for (auto const s : r.speed) {
        printf("    S offset: %e\n", s.s);
        printf("    Type: %s\n", s.type.c_str());
        printf("    Max: %e\n", s.max);
        printf("    Unit: %s\n", s.unit.c_str());
      }
      printf("LaneSections: %d\n", r.sections.size());
      for (auto const s : r.sections) {
        printf("    S offset: %e\n", s.s);
        printf("    a,b,c,d: %e,%e,%e,%e\n", s.a, s.b, s.c, s.d);
        printf("    Lanes: %d\n", s.lanes.size());
        for (auto const l : s.lanes) {
          printf("      Id: %d\n", l.id);
          printf("      Predecessor: %d\n", l.predecessor);
          printf("      Successor: %d\n", l.successor);
        }
      }
    }
    */

    // mapbuilder calls
    for (auto const r : roads) {
      map_builder.AddRoad(r.id, r.name, r.length, r.junction_id, r.predecessor, r.successor);

      // type speed
      for (auto const s : r.speed) {
        map_builder.SetRoadTypeSpeed(r.id, s.s, s.type, s.max, s.unit);
      }

      // lane sections
      int i = 0;
      for (auto const s : r.sections) {
        map_builder.AddRoadSection(r.id, i, s.s, s.a, s.b, s.c, s.d);

        // lanes
        for (auto const l : s.lanes) {
          map_builder.SetRoadLaneLink(r.id, i, l.id, l.type, l.level, l.predecessor, l.successor);
        }

        ++i;
      }
    }
  }

} // namespace parser
} // namespace opendrive
} // namespace carla
