// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/opendrive/parser/SignalParser.h"

#include "carla/road/MapBuilder.h"

#include <pugixml/pugixml.hpp>

namespace carla {
namespace opendrive {
namespace parser {

  using RoadID = uint32_t;
  using SignalID = uint32_t;
  using DependencyID = uint32_t;

  template <typename T>
  static void AddValidity(
      pugi::xml_node parent_node,
      const std::string &node_name,
      const RoadID roadID,
      const SignalID &signalID,
      T &&function) {
    for (pugi::xml_node validity_node = parent_node.child(node_name.c_str());
        validity_node;
        validity_node = validity_node.next_sibling("validity")) {
      const auto from_lane = static_cast<int16_t>(validity_node.attribute("fromLane").as_int());
      const auto to_lane = static_cast<int16_t>(validity_node.attribute("toLane").as_int());
      log_debug("Added validity to signal ", signalID, ":", from_lane, to_lane);
      function(roadID, signalID, from_lane, to_lane);
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
      const RoadID road_id = static_cast<RoadID>(road_node.attribute("id").as_int());
      const pugi::xml_node signals_node = road_node.child("signals");
      for (pugi::xml_node signal_node = signals_node.child("signal");
          signal_node;
          signal_node = signal_node.next_sibling("signal")) {
        const double s_position = signal_node.attribute("s").as_double();
        const double t_position = signal_node.attribute("t").as_double();
        const SignalID signal_id = static_cast<SignalID>(signal_node.attribute("id").as_int());
        const std::string name = signal_node.attribute("name").value();
        const std::string dynamic =  signal_node.attribute("dynamic").value();
        const std::string orientation =  signal_node.attribute("orientation").value();
        const double zOffset = signal_node.attribute("zOffSet").as_double();
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
        map_builder.AddSignal(road_id,
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
        AddValidity(signal_node, "validity", road_id, signal_id,
            ([&map_builder](auto &&... args)
            { map_builder.AddValidityToSignal(args...);
            }));

        for (pugi::xml_node dependency_node = signal_node.child("dependency");
            dependency_node;
            dependency_node = dependency_node.next_sibling("validity")) {
          const DependencyID dependency_id = dependency_node.attribute("id").as_uint();
          const std::string dependency_type = dependency_node.attribute("type").value();
          log_debug("Added dependency to signal ", signal_id, ":", dependency_id, dependency_type);
          map_builder.AddDependencyToSignal(road_id, signal_id, dependency_id, dependency_type);
        }
      }
      for (pugi::xml_node signalreference_node = signals_node.child("signalReference");
          signalreference_node;
          signalreference_node = signalreference_node.next_sibling("signalReference")) {
        const double s_position = signalreference_node.attribute("s").as_double();
        const double t_position = signalreference_node.attribute("t").as_double();
        const SignalID signal_reference_id = signalreference_node.attribute("id").as_uint();
        const std::string signal_reference_orientation =
            signalreference_node.attribute("orientation").value();
        log_debug("Road: ",
            road_id,
            "Added SignalReference ",
            s_position,
            t_position,
            signal_reference_id,
            signal_reference_orientation);
        map_builder.AddSignalReference(road_id,
            signal_reference_id,
            s_position,
            t_position,
            signal_reference_orientation);
        AddValidity(signalreference_node, validity, road_id, signal_reference_id,
            ([&map_builder](const RoadID &road_id, const SignalID &signal_id, const int16_t from_lane,
            const int16_t to_lane)
            { map_builder.AddValidityToSignalReference(road_id, signal_id, from_lane, to_lane);
            }));
      }
    }
  }
} // namespace parser
} // namespace opendrive
} // namespace carla
