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
      road::RoadId road_id,
      double s,
      const pugi::xml_node &parent_node,
      carla::road::MapBuilder &map_builder) {
    for (pugi::xml_node lane_node : parent_node.children("lane")) {

      road::LaneId lane_id = lane_node.attribute("id").as_int();

      road::Lane *lane = map_builder.GetLane(road_id, lane_id, s);

      // Lane Width
      for (pugi::xml_node lane_width_node : lane_node.children("width")) {
        double s_offset = lane_width_node.attribute("sOffset").as_double();
        double a = lane_width_node.attribute("a").as_double();
        double b = lane_width_node.attribute("b").as_double();
        double c = lane_width_node.attribute("c").as_double();
        double d = lane_width_node.attribute("d").as_double();

        // Call Map builder create Lane Width function
        map_builder.CreateLaneWidth(lane, s_offset, a, b, c, d);
      }

      // Lane Border
      for (pugi::xml_node lane_border_node : lane_node.children("border")) {
        double s_offset = lane_border_node.attribute("sOffset").as_double();
        double a = lane_border_node.attribute("a").as_double();
        double b = lane_border_node.attribute("b").as_double();
        double c = lane_border_node.attribute("c").as_double();
        double d = lane_border_node.attribute("d").as_double();

        // Call Map builder create Lane Border function
        map_builder.CreateLaneBorder(lane, s_offset, a, b, c, d);
      }

      // Lane Road Mark
      int road_mark_id = 0;
      for (pugi::xml_node lane_road_mark : lane_node.children("roadMark")) {

        double s_offset = lane_road_mark.attribute("sOffset").as_double();
        std::string type = lane_road_mark.attribute("type").value();
        std::string weight = lane_road_mark.attribute("weight").value();
        std::string color = lane_road_mark.attribute("color").value();
        std::string material = lane_road_mark.attribute("material").value();
        double width = lane_road_mark.attribute("width").as_double();
        std::string lane_change = lane_road_mark.attribute("laneChange").value();
        double height = lane_road_mark.attribute("height").as_double();

        // Call map builder for LaneRoadMarkType

        std::string type_name = "";
        double type_width = 0.0;
        pugi::xml_node road_mark_type = lane_road_mark.child("type");
        if (road_mark_type) {
          type_name = road_mark_type.attribute("name").value();
          type_width = road_mark_type.attribute("width").as_double();
        }

        // Call map builder for LaneRoadMark
        map_builder.CreateRoadMark(
            lane,
            road_mark_id,
            s_offset,
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

          double length = road_mark_type_line_node.attribute("length").as_double();
          double space = road_mark_type_line_node.attribute("space").as_double();
          double t = road_mark_type_line_node.attribute("tOffset").as_double();
          double s_offset = road_mark_type_line_node.attribute("sOffset").as_double();
          std::string rule = road_mark_type_line_node.attribute("rule").value();
          double width = road_mark_type_line_node.attribute("width").as_double();

          // Call map builder for LaneRoadMarkType LaneRoadMarkTypeLine
          map_builder.CreateRoadMarkTypeLine(
              lane,
              road_mark_id,
              length,
              space,
              t,
              s_offset,
              rule,
              width);
        }
        ++road_mark_id;
      }

      // Lane Material
      for (pugi::xml_node lane_material_node : lane_node.children("material")) {

        double s_offset = lane_material_node.attribute("sOffset").as_double();
        std::string surface = lane_material_node.attribute("surface").value();
        double friction = lane_material_node.attribute("friction").as_double();
        double roughness = lane_material_node.attribute("roughness").as_double();

        // Create map builder for Lane Material
        map_builder.CreateLaneMaterial(lane, s_offset, surface, friction, roughness);
      }

      // Lane Visibility
      for (pugi::xml_node lane_visibility_node : lane_node.children("visibility")) {
        double s_offset = lane_visibility_node.attribute("sOffset").as_double();
        double forward = lane_visibility_node.attribute("forward").as_double();
        double back = lane_visibility_node.attribute("back").as_double();
        double left = lane_visibility_node.attribute("left").as_double();
        double right = lane_visibility_node.attribute("right").as_double();

        // Create map builder for Lane Visibility
        map_builder.CreateLaneVisibility(lane, s_offset, forward, back, left, right);
      }

      // Lane Speed
      for (pugi::xml_node lane_speed_node : lane_node.children("speed")) {
        double s_offset = lane_speed_node.attribute("sOffset").as_double();
        double max = lane_speed_node.attribute("max").as_double();
        std::string unit = lane_speed_node.attribute("unit").value();

        // Create map builder for Lane Speed
        map_builder.CreateLaneSpeed(lane, s_offset, max, unit);
      }

      // Lane Access
      for (pugi::xml_node lane_access_node : lane_node.children("access")) {
        double s_offset = lane_access_node.attribute("sOffset").as_double();
        std::string restriction = lane_access_node.attribute("restriction").value();

        // Create map builder for Lane Access
        map_builder.CreateLaneAccess(lane, s_offset, restriction);
      }

      // Lane Height
      for (pugi::xml_node lane_height_node : lane_node.children("height")) {
        double s_offset = lane_height_node.attribute("sOffset").as_double();
        double inner = lane_height_node.attribute("inner").as_double();
        double outer = lane_height_node.attribute("outer").as_double();

        // Create map builder for Lane Height
        map_builder.CreateLaneHeight(lane, s_offset, inner, outer);
      }

      // Lane Rule
      for (pugi::xml_node lane_rule_node : lane_node.children("rule")) {
        double s_offset = lane_rule_node.attribute("sOffset").as_double();
        std::string value = lane_rule_node.attribute("value").value();

        // Create map builder for Lane Height
        map_builder.CreateLaneRule(lane, s_offset, value);
      }

    }
  }

  void LaneParser::Parse(
      const pugi::xml_document &xml,
      carla::road::MapBuilder &map_builder) {

    pugi::xml_node open_drive_node = xml.child("OpenDRIVE");

    // Lanes
    for (pugi::xml_node road_node : open_drive_node.children("road")) {
      road::RoadId road_id = road_node.attribute("id").as_int();

      for (pugi::xml_node lanes_node : road_node.children("lanes")) {

        for (pugi::xml_node lane_section_node : lanes_node.children("laneSection")) {
          double s = lane_section_node.attribute("s").as_double();
          pugi::xml_node left_node = lane_section_node.child("left");
          if (left_node) {
            ParseLanes(road_id, s, left_node, map_builder);
          }

          pugi::xml_node center_node = lane_section_node.child("center");
          if (center_node) {
            ParseLanes(road_id, s, center_node, map_builder);
          }

          pugi::xml_node right_node = lane_section_node.child("right");
          if (right_node) {
            ParseLanes(road_id, s, right_node, map_builder);
          }
        }
      }
    }
  }

} // namespace parser
} // namespace opendrive
} // namespace carla
