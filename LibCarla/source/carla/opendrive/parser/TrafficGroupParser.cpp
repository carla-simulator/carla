// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "TrafficGroupParser.h"
#include <iostream>

void carla::opendrive::parser::TrafficGroupParser::Parse(
    const pugi::xml_node &xmlNode,
    std::vector<carla::opendrive::types::TrafficLightGroup> &out_trafficLights) {
  carla::opendrive::parser::TrafficGroupParser parser;
  carla::opendrive::types::TrafficLightGroup traffic_light_group;

  traffic_light_group.red_time = std::atoi(xmlNode.attribute("redTime").value());
  traffic_light_group.yellow_time = std::atoi(xmlNode.attribute("yellowTime").value());
  traffic_light_group.green_time = std::atoi(xmlNode.attribute("greenTime").value());

  parser.ParseTrafficLight(xmlNode, traffic_light_group.traffic_lights);
  out_trafficLights.emplace_back(traffic_light_group);
}

void carla::opendrive::parser::TrafficGroupParser::ParseTrafficLight(
    const pugi::xml_node &xmlNode,
    std::vector<carla::opendrive::types::TrafficLight> &out_trafficLight) {

  for (pugi::xml_node trafficlight = xmlNode.child("trafficlight");
      trafficlight;
      trafficlight = trafficlight.next_sibling("trafficlight")) {
    carla::opendrive::types::TrafficLight jTrafficlight;

    jTrafficlight.x_pos = std::stod(trafficlight.attribute("xPos").value());
    jTrafficlight.y_pos = std::stod(trafficlight.attribute("yPos").value());
    jTrafficlight.z_pos = std::stod(trafficlight.attribute("zPos").value());
    jTrafficlight.x_rot = std::stod(trafficlight.attribute("xRot").value());
    jTrafficlight.y_rot = std::stod(trafficlight.attribute("yRot").value());
    jTrafficlight.z_rot = std::stod(trafficlight.attribute("zRot").value());

    ParseBoxAreas(trafficlight, jTrafficlight.box_areas);

    out_trafficLight.emplace_back(jTrafficlight);
  }

}

void carla::opendrive::parser::TrafficGroupParser::ParseBoxAreas(
    const pugi::xml_node &xmlNode,
    std::vector<carla::opendrive::types::BoxComponent> &out_boxcomponent) {
  for (pugi::xml_node boxcomponent = xmlNode.child("tfBox");
      boxcomponent;
      boxcomponent = boxcomponent.next_sibling("tfBox")) {
    carla::opendrive::types::BoxComponent jBoxComponent;

    jBoxComponent.x_pos = std::stod(boxcomponent.attribute("xPos").value());
    jBoxComponent.y_pos = std::stod(boxcomponent.attribute("yPos").value());
    jBoxComponent.z_pos = std::stod(boxcomponent.attribute("zPos").value());
    jBoxComponent.x_rot = std::stod(boxcomponent.attribute("xRot").value());
    jBoxComponent.y_rot = std::stod(boxcomponent.attribute("yRot").value());
    jBoxComponent.z_rot = std::stod(boxcomponent.attribute("zRot").value());

    out_boxcomponent.emplace_back(jBoxComponent);
  }

}
