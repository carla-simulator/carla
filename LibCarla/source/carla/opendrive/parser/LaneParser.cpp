// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/opendrive/parser/LaneParser.h"

#include "carla/opendrive/parser/pugixml/pugixml.hpp"

#include "carla/road/MapBuilder.h"

namespace carla {
namespace opendrive {
namespace parser {

  void LaneParser::ParseLanes(
      RoadId road_id,
      int32_t lane_section_id,
      const pugi::xml_node &parent_node,
      carla::road::MapBuilder &map_builder) {
    for (pugi::xml_node lane_node : parent_node.children("lane")) {

      LaneId lane_id = lane_node.attribute("id").as_int();

      // Lane Width
      for (pugi::xml_node lane_width_node : lane_node.children("width")) {
        float s = lane_width_node.attribute("sOffset").as_float();
        float a = lane_width_node.attribute("a").as_float();
        float b = lane_width_node.attribute("b").as_float();
        float c = lane_width_node.attribute("c").as_float();
        float d = lane_width_node.attribute("d").as_float();

        // Call Map builder create Lane Width function
        map_builder.CreateLaneWidth(road_id, lane_section_id, lane_id, s, a, b, c, d);
      }

      // Lane Border
      for (pugi::xml_node lane_border_node : lane_node.children("border")) {
        float s = lane_border_node.attribute("sOffset").as_float();
        float a = lane_border_node.attribute("a").as_float();
        float b = lane_border_node.attribute("b").as_float();
        float c = lane_border_node.attribute("c").as_float();
        float d = lane_border_node.attribute("d").as_float();

        // Call Map builder create Lane Border function
        map_builder.CreateLaneBorder(road_id, lane_section_id, lane_id, s, a, b, c, d);
      }

      // Lane Road Mark
      int32_t road_mark_id = 0;
      for (pugi::xml_node lane_road_mark : lane_node.children("roadMark")) {

        float s = lane_road_mark.attribute("sOffset").as_float();
        std::string type = lane_road_mark.attribute("type").value();
        std::string weight = lane_road_mark.attribute("weight").value();
        std::string color = lane_road_mark.attribute("color").value();
        std::string material = lane_road_mark.attribute("material").value();
        float width = lane_road_mark.attribute("width").as_float();
        std::string lane_change = lane_road_mark.attribute("laneChange").value();
        float height = lane_road_mark.attribute("height").as_float();

        // Call map builder for LaneRoadMarkType

        std::string type_name = "";
        float type_width = 0.0;
        pugi::xml_node road_mark_type = lane_road_mark.child("type");
        if (road_mark_type) {
          type_name = road_mark_type.attribute("name").value();
          type_width = road_mark_type.attribute("width").as_float();
        }

        // Call map builder for LaneRoadMark
        map_builder.CreateRoadMark(
            road_id,
            lane_section_id,
            lane_id,
            road_mark_id,
            s,
            type,
            weight,
            color,
            material,
            width,
            lane_change,
            height,
            type_name,
            type_width);

        for (pugi::xml_node road_mark_type_line_node : road_mark_type.children("line")) {

          float length = road_mark_type_line_node.attribute("length").as_float();
          float space = road_mark_type_line_node.attribute("space").as_float();
          float t = road_mark_type_line_node.attribute("tOffset").as_float();
          float s = road_mark_type_line_node.attribute("sOffset").as_float();
          std::string rule = road_mark_type_line_node.attribute("rule").value();
          float width = road_mark_type_line_node.attribute("width").as_float();

          // Call map builder for LaneRoadMarkType LaneRoadMarkTypeLine
          map_builder.CreateRoadMarkTypeLine(
              road_id,
              lane_section_id,
              lane_id,
              road_mark_id,
              length,
              space,
              t,
              s,
              rule,
              width);
        }
        ++road_mark_id;
      }

      // Lane Material
      for (pugi::xml_node lane_material_node : lane_node.children("material")) {

        float s = lane_material_node.attribute("sOffset").as_float();
        std::string surface = lane_material_node.attribute("surface").value();
        float friction = lane_material_node.attribute("friction").as_float();
        float roughness = lane_material_node.attribute("roughness").as_float();

        // Create map builder for Lane Material
        map_builder.CreateLaneMaterial(road_id, lane_section_id, lane_id, s, surface, friction, roughness);
      }

      // Lane Visibility
      for (pugi::xml_node lane_visibility_node : lane_node.children("visibility")) {
        float s = lane_visibility_node.attribute("sOffset").as_float();
        float forward = lane_visibility_node.attribute("forward").as_float();
        float back = lane_visibility_node.attribute("back").as_float();
        float left = lane_visibility_node.attribute("left").as_float();
        float right = lane_visibility_node.attribute("right").as_float();

        // Create map builder for Lane Visibility
        map_builder.CreateLaneVisibility(road_id, lane_section_id, lane_id, s, forward, back, left, right);
      }

      // Lane Speed
      for (pugi::xml_node lane_speed_node : lane_node.children("speed")) {
        float s = lane_speed_node.attribute("sOffset").as_float();
        float max = lane_speed_node.attribute("max").as_float();
        std::string unit = lane_speed_node.attribute("unit").value();

        // Create map builder for Lane Speed
        map_builder.CreateLaneSpeed(road_id, lane_section_id, lane_id, s, max, unit);
      }

      // Lane Access
      for (pugi::xml_node lane_access_node : lane_node.children("access")) {
        float s = lane_access_node.attribute("sOffset").as_float();
        std::string restriction = lane_access_node.attribute("restriction").value();

        // Create map builder for Lane Access
        map_builder.CreateLaneAccess(road_id, lane_section_id, lane_id, s, restriction);
      }

      // Lane Height
      for (pugi::xml_node lane_height_node : lane_node.children("height")) {
        float s = lane_height_node.attribute("sOffset").as_float();
        float inner = lane_height_node.attribute("inner").as_float();
        float outer = lane_height_node.attribute("outer").as_float();

        // Create map builder for Lane Height
        map_builder.CreateLaneHeight(road_id, lane_section_id, lane_id, s, inner, outer);
      }

      // Lane Rule
      for (pugi::xml_node lane_rule_node : lane_node.children("rule")) {
        float s = lane_rule_node.attribute("sOffset").as_float();
        std::string value = lane_rule_node.attribute("value").value();

        // Create map builder for Lane Height
        map_builder.CreateLaneRule(road_id, lane_section_id, lane_id, s, value);
      }

    }
  }

  void LaneParser::Parse(
      const pugi::xml_document &xml,
      carla::road::MapBuilder &map_builder) {

    pugi::xml_node open_drive_node = xml.child("OpenDRIVE");

    // Lanes
    for (pugi::xml_node road_node : open_drive_node.children("road")) {
      RoadId road_id = road_node.attribute("id").as_uint();

      for (pugi::xml_node lanes_node : road_node.children("lanes")) {
        int32_t lane_section_id = 0;
        for (pugi::xml_node lane_section_node : lanes_node.children("laneSection")) {

          pugi::xml_node left_node = lane_section_node.child("left");
          if (left_node) {
            ParseLanes(road_id, lane_section_id, left_node, map_builder);
          }

          pugi::xml_node center_node = lane_section_node.child("center");
          if (center_node) {
            ParseLanes(road_id, lane_section_id, center_node, map_builder);
          }

          pugi::xml_node right_node = lane_section_node.child("right");
          if (right_node) {
            ParseLanes(road_id, lane_section_id, right_node, map_builder);
          }
          ++lane_section_id;
        }
      }
    }
  }

} // namespace parser
} // namespace opendrive
} // namespace carla
