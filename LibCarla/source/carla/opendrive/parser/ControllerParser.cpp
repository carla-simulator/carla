// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/opendrive/parser/ControllerParser.h"

#include "carla/road/MapBuilder.h"

#include <third-party/pugixml/pugixml.hpp>

namespace carla {
namespace opendrive {
namespace parser {

  void ControllerParser::Parse(
      const pugi::xml_document &xml,
      carla::road::MapBuilder &map_builder) {

    // Extracting the OpenDRIVE
    const pugi::xml_node opendrive_node = xml.child("OpenDRIVE");
    for (pugi::xml_node controller_node = opendrive_node.child("controller");
        controller_node;
        controller_node = controller_node.next_sibling("controller")) {

      const road::ContId controller_id = controller_node.attribute("id").value();
      const std::string controller_name = controller_node.attribute("name").value();
      const uint32_t controller_sequence = controller_node.attribute("sequence").as_uint();

      log_debug("Controller: ",
        controller_id,
        controller_name,
        controller_sequence);

      std::set<road::SignId> signals;

      for (pugi::xml_node control_node : controller_node.children("control")) {
        const road::SignId signal_id = control_node.attribute("signalId").value();
        // The controller_type is included in the OpenDrive format but not used yet
        // const std::string controller_type = control_node.attribute("type").value();
        signals.insert(signal_id);
      }

      map_builder.CreateController(
        controller_id,
        controller_name,
        controller_sequence,
        std::move(signals)
      );
    }

  }

} // namespace parser
} // namespace opendrive
} // namespace carla
