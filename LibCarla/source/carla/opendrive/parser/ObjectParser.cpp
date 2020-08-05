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

  void ObjectParser::Parse(
      const pugi::xml_document &xml,
      carla::road::MapBuilder &map_builder) {

    std::vector<road::element::CrosswalkPoint> points;

    for (pugi::xml_node node_road : xml.child("OpenDRIVE").children("road")) {

      // parse all objects
      pugi::xml_node node_objects = node_road.child("objects");
      if (node_objects) {

        for (pugi::xml_node node_object : node_objects.children("object")) {

          // type Crosswalk
          std::string type = node_object.attribute("type").as_string();
          if (type == "crosswalk") {

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

          }
        }
      }
    }
  }
} // namespace parser
} // namespace opendrive
} // namespace carla
