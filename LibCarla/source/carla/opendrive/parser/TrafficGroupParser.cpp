// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/opendrive/parser/TrafficGroupParser.h"

#include "carla/road/MapBuilder.h"

#include <pugixml/pugixml.hpp>

namespace carla {
namespace opendrive {
namespace parser {

  /*
    <userData>
      <trafficGroup id="137" type="trafficGroup" redTime="10" yellowTime="5" greenTime="5"/>
      <trafficGroup id="138" type="trafficGroup" redTime="10" yellowTime="5" greenTime="5"/>
      <trafficGroup id="139" type="trafficGroup" redTime="10" yellowTime="5" greenTime="5"/>
      <trafficGroup id="140" type="trafficGroup" redTime="10" yellowTime="5" greenTime="5"/>
      <trafficGroup id="141" type="trafficGroup" redTime="10" yellowTime="5" greenTime="5"/>
    </userData>
  */

  void TrafficGroupParser::Parse(
      const pugi::xml_document & /*xml*/,
      carla::road::MapBuilder & /* map_builder */) {
    // pugi::xml_node opendrive_node = xml.child("OpenDRIVE");
    // for (pugi::xml_node userdata_node = opendrive_node.child("userData");
    //     userdata_node;
    //     userdata_node = userdata_node.next_sibling("userData")) {
    //   for (pugi::xml_node trafficgroup_node = userdata_node.child("trafficGroup");
    //       trafficgroup_node;
    //       trafficgroup_node = trafficgroup_node.next_sibling("trafficGroup")) {
    //     uint16_t id = trafficgroup_node.attribute("id").as_int(0);
    //     uint16_t redTime = trafficgroup_node.attribute("redTime").as_int(0);
    //     uint16_t yellowTime = trafficgroup_node.attribute("yellowTime").as_int(0);
    //     uint16_t greenTime = trafficgroup_node.attribute("greenTime").as_int(0);
    //     log_debug("Found TrafficGroup with ID: ",
    //         id,
    //         "Times (",
    //         redTime,
    //         ", ",
    //         yellowTime,
    //         ", ",
    //         greenTime,
    //         ")");
    //   }
    //   map_builder.AddTrafficGroup(id, redTime, yellowTime, greenTime);
    // }
  }

} // namespace parser
} // namespace opendrive
} // namespace carla
