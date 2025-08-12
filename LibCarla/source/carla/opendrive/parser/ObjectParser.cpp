// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/opendrive/parser/ObjectParser.h"

#include "carla/road/MapBuilder.h"
#include "carla/road/element/RoadInfoCrosswalk.h"
#include "carla/road/Road.h"


#include <pugixml/pugixml.hpp>

namespace carla {
namespace opendrive {
namespace parser {

  std::string ObjectParser::get_string_attribute(const pugi::xml_node& node, const char* attribute_name){
    return node.attribute(attribute_name)? node.attribute(attribute_name).as_string() : std::string{};
  }

  double ObjectParser::get_double_attribute(const pugi::xml_node& node, const char* attribute_name){
    return node.attribute(attribute_name)? node.attribute(attribute_name).as_double() : 0.0;
  }

  void ObjectParser::Parse(
      const pugi::xml_document &xml,
      carla::road::MapBuilder &map_builder) {

    std::vector<road::element::CrosswalkPoint> points;
    for (pugi::xml_node node_road : xml.child("OpenDRIVE").children("road")) {

      // parse all objects
      pugi::xml_node node_objects = node_road.child("objects");
      if (node_objects) {
        
        for (pugi::xml_node node_object : node_objects.children("object")) {
          std::string type = node_object.attribute("type").as_string();
          std::string name = node_object.attribute("name").as_string();
          std::string name_lower = name;
          std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(),
                [](unsigned char c){ return std::tolower(c); });
          bool bIsCrosswalk = (type == "crosswalk" || (name_lower.find("crosswalk") != std::string::npos));
          if (bIsCrosswalk) {
              // read all points
              pugi::xml_node node_outline = node_object.child("outline");
              if (node_outline) {
                points.clear();
                for (pugi::xml_node node_corner : node_outline.children("cornerLocal")) {
                  points.emplace_back(node_corner.attribute("u").as_double(),
                                      node_corner.attribute("v").as_double(),
                                      node_corner.attribute("z").as_double());
                }
              }
            // get road id
            road::RoadId road_id = node_road.attribute("id").as_uint();
            road::Road *road = map_builder.GetRoad(road_id);

            // create the object
            map_builder.AddRoadObjectCrosswalk(road,
                node_object.attribute("name").as_string(),
                node_object.attribute("s").as_double(),
                node_object.attribute("t").as_double(),
                node_object.attribute("zOffset").as_double(),
                node_object.attribute("hdg").as_double(),
                node_object.attribute("pitch").as_double(),
                node_object.attribute("roll").as_double(),
                node_object.attribute("orientation").as_string(),
                node_object.attribute("width").as_double(),
                node_object.attribute("length").as_double(),
                points);

          } else if (name.substr(0, 6) == "Speed_" || name.substr(0, 6) == "speed_") {
            road::RoadId road_id = node_road.attribute("id").as_uint();
            road::Road *road = map_builder.GetRoad(road_id);
            // speed signal by roadrunner
            std::string speed_str;
            if (name.find("STATIC") != std::string::npos) {
               speed_str = name.substr(13);
            } else {
               speed_str = name.substr(6);
            }
            double speed = std::stod(speed_str);
            map_builder.AddSignal(road,
                node_object.attribute("id").as_string(),
                node_object.attribute("s").as_double(),
                node_object.attribute("t").as_double(),
                node_object.attribute("name").as_string(),
                "no",
                node_object.attribute("orientation").as_string(),
                node_object.attribute("zOffset").as_double(),
                "OpenDRIVE",
                "274",
                speed_str,
                speed,
                "mph",
                node_object.attribute("height").as_double(),
                node_object.attribute("width").as_double(),
                speed_str,
                node_object.attribute("hdg").as_double(),
                node_object.attribute("pitch").as_double(),
                node_object.attribute("roll").as_double());
          } else if (name.find("Stencil_") != std::string::npos) {
            // Parse all stencil types, not just STOP
            road::RoadId road_id = node_road.attribute("id").as_uint();
            road::Road *road = map_builder.GetRoad(road_id);
            
            // Extract the stencil type from the name (e.g., "STOP", "ArrowType4R", etc.)
            std::string stencil_type = name.substr(8); // Skip "Stencil_" prefix
            
            // Extract text if it exists (some stencils might have text content)
            std::string text = "";
            pugi::xml_node userData = node_object.child("userData");
            if (userData) {
              pugi::xml_node textNode = userData.child("text");
              if (textNode) {
                text = textNode.text().get();
              }
            }
            
            map_builder.AddStencil(road,
                node_object.attribute("id").as_string(),
                node_object.attribute("s").as_double(),
                node_object.attribute("t").as_double(),
                node_object.attribute("name").as_string(),
                node_object.attribute("orientation").as_string(),
                stencil_type,
                text,
                node_object.attribute("zOffset").as_double(),
                node_object.attribute("hdg").as_double(),
                node_object.attribute("length").as_double(),
                node_object.attribute("width").as_double(),
                node_object.attribute("pitch").as_double(),
                node_object.attribute("roll").as_double());
          }
        }
      }
    }


  }
} // namespace parser
} // namespace opendrive
} // namespace carla
