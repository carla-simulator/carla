// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/opendrive/parser/SignalParser.h"

#include "carla/road/MapBuilder.h"

#include <third-party/pugixml/pugixml.hpp>

namespace carla {
namespace opendrive {
namespace parser {

  static void AddValidity(
    road::element::RoadInfoSignal* signal_reference,
    pugi::xml_node parent_node,
    const std::string &node_name,
    road::MapBuilder &map_builder) {
    for (pugi::xml_node validity_node = parent_node.child(node_name.c_str());
        validity_node;
        validity_node = validity_node.next_sibling("validity")) {
      const auto from_lane = validity_node.attribute("fromLane").as_int();
      const auto to_lane = validity_node.attribute("toLane").as_int();
      map_builder.AddValidityToSignalReference(signal_reference, from_lane, to_lane);
    }
  }

  void SignalParser::Parse(
      const pugi::xml_document &xml,
      carla::road::MapBuilder &map_builder) {

    // Extracting the OpenDRIVE
    const pugi::xml_node opendrive_node = xml.child("OpenDRIVE");
    const std::string validity = "validity";
    for (pugi::xml_node road_node = opendrive_node.child("road");
        road_node;
        road_node = road_node.next_sibling("road")) {

      road::RoadId road_id = road_node.attribute("id").as_uint();

      const pugi::xml_node signals_node = road_node.child("signals");
      if(signals_node){
        for (pugi::xml_node signal_node : signals_node.children("signal")) {
          const double s_position = signal_node.attribute("s").as_double();
          const double t_position = signal_node.attribute("t").as_double();
          const road::SignId signal_id = signal_node.attribute("id").value();
          const std::string name = signal_node.attribute("name").value();
          const std::string dynamic =  signal_node.attribute("dynamic").value();
          const std::string orientation =  signal_node.attribute("orientation").value();
          const double zOffset = signal_node.attribute("zOffset").as_double();
          const std::string country =  signal_node.attribute("country").value();
          const std::string type =  signal_node.attribute("type").value();
          const std::string subtype =  signal_node.attribute("subtype").value();
          const double value = signal_node.attribute("value").as_double();
          const std::string unit =  signal_node.attribute("unit").value();
          const double height = signal_node.attribute("height").as_double();
          const double width = signal_node.attribute("width").as_double();
          const std::string text =  signal_node.attribute("text").value();
          const double hOffset = signal_node.attribute("hOffset").as_double();
          const double pitch = signal_node.attribute("pitch").as_double();
          const double roll = signal_node.attribute("roll").as_double();
          log_debug("Road: ",
              road_id,
              "Adding Signal: ",
              s_position,
              t_position,
              signal_id,
              name,
              dynamic,
              orientation,
              zOffset,
              country,
              type,
              subtype,
              value,
              unit,
              height,
              width,
              text,
              hOffset,
              pitch,
              roll);

          carla::road::Road *road = map_builder.GetRoad(road_id);
          auto signal_reference = map_builder.AddSignal(road,
              signal_id,
              s_position,
              t_position,
              name,
              dynamic,
              orientation,
              zOffset,
              country,
              type,
              subtype,
              value,
              unit,
              height,
              width,
              text,
              hOffset,
              pitch,
              roll);
          AddValidity(signal_reference, signal_node, "validity", map_builder);

          for (pugi::xml_node dependency_node : signal_node.children("dependency")) {
            const std::string dependency_id = dependency_node.attribute("id").value();
            const std::string dependency_type = dependency_node.attribute("type").value();
            log_debug("Added dependency to signal ", signal_id, ":", dependency_id, dependency_type);
            map_builder.AddDependencyToSignal(signal_id, dependency_id, dependency_type);
          }
          for (pugi::xml_node position_node : signal_node.children("positionInertial")) {
            const double x = position_node.attribute("x").as_double();
            const double y = position_node.attribute("y").as_double();
            const double z = position_node.attribute("z").as_double();
            const double hdg = position_node.attribute("hdg").as_double();
            const double inertial_pitch = position_node.attribute("pitch").as_double();
            const double inertial_roll = position_node.attribute("roll").as_double();
            map_builder.AddSignalPositionInertial(
                signal_id,
                x, y, z,
                hdg, inertial_pitch, inertial_roll);
          }
        }
        for (pugi::xml_node signal_reference_node : signals_node.children("signalReference")) {
          const double s_position = signal_reference_node.attribute("s").as_double();
          const double t_position = signal_reference_node.attribute("t").as_double();
          const road::SignId signal_id = signal_reference_node.attribute("id").value();
          const std::string signal_reference_orientation =
              signal_reference_node.attribute("orientation").value();
          log_debug("Road: ",
              road_id,
              "Added SignalReference ",
              s_position,
              t_position,
              signal_reference_orientation);
          carla::road::Road *road = map_builder.GetRoad(road_id);
          auto signal_reference = map_builder.AddSignalReference(
              road,
              signal_id,
              s_position,
              t_position,
              signal_reference_orientation);
          AddValidity(signal_reference, signal_reference_node, "validity", map_builder);
        }
      }
    }
  }
} // namespace parser
} // namespace opendrive
} // namespace carla
